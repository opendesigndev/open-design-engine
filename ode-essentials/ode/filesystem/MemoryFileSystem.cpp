
#include "MemoryFileSystem.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <zlib.h>

namespace ode {

#define PK_SIGNATURE "PK\x03\x04"

#define COMPRESSION_STORE       0
#define COMPRESSION_DEFLATE     8

#define CHECK(condition, err) \
    if (!(condition)) { \
        if (error) { \
            *error = Error::err; \
        } \
        clear(); \
        return false; \
    }

#define CHECK_HEADER(offset, value, length) \
    { \
        file.seekg(offset, std::ios::beg); \
        std::string str(length, '\0'); \
        file.read(&str[0], length); \
        CHECK(str == value, INVALID_OCTOPUS_FILE); \
    }

bool MemoryFileSystem::openOctopusFile(const FilePath &octopusFilePath, Error *error) {
    clear();

    if (error)
        *error = Error::OK;

    std::ifstream file((std::string)octopusFilePath, std::ios::binary);

    if (!file) {
        if (error)
            *error = Error::ERROR_OPENING_FILE;
        return false;
    }

    // Check if the file is a valid Octopus file
    CHECK(checkOctopusFileHeader(file), INVALID_OCTOPUS_FILE);

    // Read end-of-central-directory record
    file.seekg(-22, std::ios::end);
    char eocd[22];
    file.read(eocd, 22);

    // Parse end-of-central-directory record
    const uint16_t filesCount = *(uint16_t*)(eocd + 10);
    const uint32_t centralDirOffset = *(uint32_t*)(eocd + 16);
    const uint32_t centralDirSize = *(uint32_t*)(eocd + 12);

    // Read central directory
    file.seekg(centralDirOffset, std::ios::beg);
    std::string centralDir(centralDirSize, '\0');
    file.read(centralDir.data(), centralDirSize);

    uint16_t centralDirFileOffset = 0;

    for (int i = 0; i < filesCount; ++i) {
        const uint32_t headerOffset = *(uint32_t*)(centralDir.data() + centralDirFileOffset + 42);

        const uint16_t filenameLengthCentralDir = *(uint16_t*)(centralDir.data() + centralDirFileOffset + 28);
        const uint16_t m = *(uint16_t*)(centralDir.data() + centralDirFileOffset + 30);
        const uint16_t k = *(uint16_t*)(centralDir.data() + centralDirFileOffset + 32);

        std::string filePath(filenameLengthCentralDir, '\0');
        std::memcpy(filePath.data(), centralDir.data() + centralDirFileOffset + 46, filenameLengthCentralDir);

        file.seekg(headerOffset, std::ios::beg);
        std::string signature(4, '\0');
        file.read(&signature[0], 4);

        CHECK(signature == PK_SIGNATURE, INVALID_OCTOPUS_FILE);

        File &newFile = files.emplace_back();

        newFile.compressionMethod = *(uint16_t*)(centralDir.data() + centralDirFileOffset + 10);
        newFile.compressedSize = *(uint32_t*)(centralDir.data() + centralDirFileOffset + 20);
        newFile.uncompressedSize = *(uint32_t*)(centralDir.data() + centralDirFileOffset + 24);

        newFile.path = filePath;

        file.seekg(22, std::ios::cur);
        uint16_t filenameLength;
        file.read(reinterpret_cast<char*>(&filenameLength), 2);

        CHECK(filenameLength == filenameLengthCentralDir, INVALID_OCTOPUS_FILE);

        uint16_t extraFieldLength;
        file.read(reinterpret_cast<char*>(&extraFieldLength), 2);
        file.seekg(filenameLength+extraFieldLength, std::ios::cur);

        newFile.data.resize(newFile.compressedSize, '\0');
        file.read(&newFile.data[0], newFile.compressedSize);

        centralDirFileOffset += 46 + filenameLengthCentralDir + m + k;
    }

    return true;
}

const std::vector<FilePath> MemoryFileSystem::filePaths() const {
    std::vector<FilePath> result;
    for (const File &file : files) {
        result.emplace_back(file.path);
    }
    return result;
}

std::optional<std::string> MemoryFileSystem::getFileData(const FilePath& filePath, Error *error) const {
    if (error)
        *error = Error::OK;

    const std::vector<File>::const_iterator fileIt = std::find_if(files.begin(), files.end(), [&filePath](const File &file) {
        return file.path == filePath;
    });

    if (fileIt == files.end()) {
        return std::nullopt;
    }

    switch (fileIt->compressionMethod) {
        case COMPRESSION_STORE:
        {
            return fileIt->data;
        }
        case COMPRESSION_DEFLATE:
        {
            z_stream zs;
            memset(&zs, 0, sizeof(zs));

            if (inflateInit2(&zs, -15) != Z_OK) {
                if (error)
                    *error = Error::DECOMPRESSION_FAILED;
                return std::nullopt;
            }

            zs.next_in = (Bytef*)fileIt->data.data();
            zs.avail_in = (uInt)fileIt->data.size();

            int ret = Z_OK;
            char outbuffer[1 << 15];
            std::string decompressedData;

            do {
                zs.next_out = reinterpret_cast<Bytef*>(outbuffer);
                zs.avail_out = sizeof(outbuffer);

                ret = inflate(&zs, 0);

                if (decompressedData.size() < zs.total_out) {
                    decompressedData.append(outbuffer, zs.total_out - decompressedData.size());
                }
            } while (ret == Z_OK);

            if (inflateEnd(&zs) != Z_OK) {
                return std::nullopt;
            }

            if (ret != Z_STREAM_END) {
                if (error)
                    *error = Error::DECOMPRESSION_FAILED;
                return std::nullopt;
            }

            return decompressedData;
        }
        default:
            if (error)
                *error = Error::UNSUPPORTED_COMPRESSION_METHOD;
            return std::nullopt;
    }

    return std::nullopt;
}

void MemoryFileSystem::clear() {
    files.clear();
}

bool MemoryFileSystem::checkOctopusFileHeader(std::ifstream &file, Error *error) {
    CHECK(file.tellg() < 134, INVALID_OCTOPUS_FILE);

    CHECK_HEADER(0, PK_SIGNATURE, 4);
    CHECK_HEADER(8, std::string(3, 0), 3);
    CHECK_HEADER(12, std::string(1, 33), 1);
    CHECK_HEADER(13, std::string(1, 0), 1);
    CHECK_HEADER(26, std::string(1, 7), 1);
    CHECK_HEADER(27, std::string(3, 0), 3);
    CHECK_HEADER(30, "Octopus is universal design format. opendesign.dev.", 51);

    file.seekg(18, std::ios::beg);
    std::string s18(3, '\0');
    file.read(&s18[0], 3);
    const uint32_t i18 = (s18[0]<<0) | (s18[1]<<8) | (s18[2]<<16);
    CHECK(i18 >= 44, INVALID_OCTOPUS_FILE);

    file.seekg(22, std::ios::beg);
    std::string s22(3, '\0');
    file.read(&s22[0], 3);
    const uint32_t i22 = (s22[0]<<0) | (s22[1]<<8) | (s22[2]<<16);
    CHECK(i22 >= 44, INVALID_OCTOPUS_FILE);

    CHECK(s18 == s22, INVALID_OCTOPUS_FILE);

    return true;
}

} // namespace ode
