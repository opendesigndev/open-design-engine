
#include "OctopusFile.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <map>

namespace ode {

#define OCTOPUS_FILE_SIGNATURE_NAME             "Octopus"
#define OCTOPUS_FILE_SIGNATURE_CONTENTS         " is universal design format. opendesign.dev."

#define PK_CENTRAL_DIR_FILE_HEADER_SIGNATURE    "PK\x01\x02"
#define PK_LOCAL_FILE_HEADER_SIGNATURE          "PK\x03\x04"
#define PK_END_OF_CENTRAL_DIRECTORY_SIGNATURE   "PK\x05\x06"

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

#define CENTRAL_DIR_FILE_DATA(type, offset) \
    *(type*)(centralDir.data() + centralDirFileOffset + offset);

bool OctopusFile::load(const FilePath &octopusFilePath, Error *error) {
    clear();

    if (error)
        *error = Error::OK;

    std::ifstream file((std::string)octopusFilePath, std::ios::binary);
    CHECK(file, ERROR_OPENING_FILE);

    // Check if the file is a valid Octopus file
    CHECK(checkOctopusFileHeader(file), INVALID_OCTOPUS_FILE);

    // Read end-of-central-directory record
    file.seekg(-22, std::ios::end);
    char eocd[22];
    file.read(eocd, 22);
    CHECK(std::memcmp(eocd, PK_END_OF_CENTRAL_DIRECTORY_SIGNATURE, 4)==0, INVALID_OCTOPUS_FILE);

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
        CHECK(std::memcmp(centralDir.data() + centralDirFileOffset, PK_CENTRAL_DIR_FILE_HEADER_SIGNATURE, 4)==0, INVALID_OCTOPUS_FILE);

        // File offset from central directory
        const uint32_t headerOffset = CENTRAL_DIR_FILE_DATA(uint32_t, 42);

        const uint16_t filenameLengthCentralDir = CENTRAL_DIR_FILE_DATA(uint16_t, 28);
        const uint16_t extraFieldLengthCentralDir = CENTRAL_DIR_FILE_DATA(uint16_t, 30);
        const uint16_t fileCommentLengthCentralDir = CENTRAL_DIR_FILE_DATA(uint16_t, 32);

        std::string filePath(filenameLengthCentralDir, '\0');
        std::memcpy(filePath.data(), centralDir.data() + centralDirFileOffset + 46, filenameLengthCentralDir);

        file.seekg(headerOffset, std::ios::beg);
        std::string signature(4, '\0');
        file.read(&signature[0], 4);

        CHECK(signature == PK_LOCAL_FILE_HEADER_SIGNATURE, INVALID_OCTOPUS_FILE);

        // Create a new file entry after central dir file header verified
        File &newFile = files.emplace_back();

        newFile.crc32 = CENTRAL_DIR_FILE_DATA(uint32_t, 16);
        newFile.compressionMethod = CENTRAL_DIR_FILE_DATA(CompressionMethod, 10);
        newFile.compressedSize = CENTRAL_DIR_FILE_DATA(uint32_t, 20);
        newFile.uncompressedSize = CENTRAL_DIR_FILE_DATA(uint32_t, 24);

        newFile.path = filePath;

        // Filename length
        file.seekg(22, std::ios::cur);
        uint16_t filenameLength;
        file.read(reinterpret_cast<char*>(&filenameLength), 2);

        CHECK(filenameLength == filenameLengthCentralDir, INVALID_OCTOPUS_FILE);

        uint16_t extraFieldLength;
        file.read(reinterpret_cast<char*>(&extraFieldLength), 2);
        file.seekg(filenameLength+extraFieldLength, std::ios::cur);

        newFile.data.resize(newFile.compressedSize, '\0');
        file.read(reinterpret_cast<char *>(newFile.data.data()), newFile.compressedSize);

        // Move central dir offset to the next file record
        centralDirFileOffset += 46 + filenameLengthCentralDir + extraFieldLengthCentralDir + fileCommentLengthCentralDir;
    }

    return true;
}

bool OctopusFile::save(const FilePath &octopusFilePath, Error *error) {
    std::ofstream outputFile((std::string)octopusFilePath, std::ios::binary);
    CHECK(outputFile, ERROR_OPENING_FILE);

    const uint16_t filesCount = files.size();
    std::map<std::string, uint32_t> fileHeadersOffsets;

    // Individual file data
    for (const File &file : files) {
        fileHeadersOffsets[(std::string)file.path] = static_cast<uint32_t>(outputFile.tellp());
        const size_t fileNameLength = ((std::string)file.path).size();

        // Local file header signature
        outputFile.write(PK_LOCAL_FILE_HEADER_SIGNATURE, strlen(PK_LOCAL_FILE_HEADER_SIGNATURE));
        // Zip version
        outputFile.write("\x14\0", 2);
        // General purpose bit flag
        outputFile.write("\0\0", 2);
        // Compression method
        switch (file.compressionMethod) {
            case CompressionMethod::NONE:
                outputFile.write("\0\0", 2);
                break;
            case CompressionMethod::DEFLATE:
                outputFile.write("\x08\0", 2);
                break;
            default:
                CHECK(false, UNSUPPORTED_COMPRESSION_METHOD);
        }
        // Modification time and date
        outputFile.write("\0\0\x21\0", 4);
        // CRC-32 of uncompressed data
        outputFile.write(reinterpret_cast<const char*>(&file.crc32), 4);
        // Compressed size
        outputFile.write(reinterpret_cast<const char*>(&file.compressedSize), 4);
        // Uncompressed size
        outputFile.write(reinterpret_cast<const char*>(&file.uncompressedSize), 4);
        // File name length
        outputFile.write(reinterpret_cast<const char*>(&fileNameLength), 2);
        // Extra field length
        outputFile.write("\0\0", 2);
        // File name
        outputFile.write(((std::string)file.path).data(), fileNameLength);
        // Extra field - No data
        // File contents
        outputFile.write(reinterpret_cast<const char *>(file.data.data()), file.data.size());
    }

    // Zip central directory
    const uint32_t centralDirectoryOffset = static_cast<uint32_t>(outputFile.tellp());
    for (const File &file : files) {
        const uint32_t fileHeaderOffset = fileHeadersOffsets[(std::string)file.path];
        const size_t fileNameLength = ((std::string)file.path).size();

        // Central directory file header signature = 0x02014b50
        outputFile.write(PK_CENTRAL_DIR_FILE_HEADER_SIGNATURE, 4);
        // Version made by
        outputFile.write("\x14\0", 2);
        // Version needed to extract (minimum)
        outputFile.write("\x14\0", 2);
        // General purpose bit flag
        outputFile.write("\0\0", 2);
        // Compression method
        switch (file.compressionMethod) {
            case CompressionMethod::NONE:
                outputFile.write("\0\0", 2);
                break;
            case CompressionMethod::DEFLATE:
                outputFile.write("\x08\0", 2);
                break;
            default:
                CHECK(false, UNSUPPORTED_COMPRESSION_METHOD);
        }
        // File last modification time and date
        outputFile.write("\0\0\x21\0", 4);
        // CRC-32 of uncompressed data
        outputFile.write(reinterpret_cast<const char*>(&file.crc32), 4);
        // Compressed size
        outputFile.write(reinterpret_cast<const char*>(&file.compressedSize), 4);
        // Uncompressed size
        outputFile.write(reinterpret_cast<const char*>(&file.uncompressedSize), 4);
        // File name length
        outputFile.write(reinterpret_cast<const char*>(&fileNameLength), 2);
        // Extra field length
        outputFile.write("\0\0", 2);
        // File comment length
        outputFile.write("\0\0", 2);
        // Disk number where file starts
        outputFile.write("\0\0", 2);
        // Internal file attributes
        outputFile.write("\0\0", 2);
        // External file attributes
        outputFile.write("\0\0\0\0", 4);
        // Relative offset of local file header
        outputFile.write(reinterpret_cast<const char*>(&fileHeaderOffset), 4);
        // File name
        outputFile.write(((std::string)file.path).data(), fileNameLength);
    }

    // End of central directory record
    const uint32_t eocdOffset = static_cast<uint32_t>(outputFile.tellp());
    const uint32_t centralDirectorySize = eocdOffset - centralDirectoryOffset;
    // End of central directory signature
    outputFile.write(PK_END_OF_CENTRAL_DIRECTORY_SIGNATURE, 4);
    // Number of this disk
    outputFile.write("\0\0", 2);
    // Disk where central directory starts
    outputFile.write("\0\0", 2);
    // Number of central directory records on this disk
    outputFile.write(reinterpret_cast<const char*>(&filesCount), 2);
    // Total number of central directory records
    outputFile.write(reinterpret_cast<const char*>(&filesCount), 2);
    // Size of central directory (bytes)
    outputFile.write(reinterpret_cast<const char*>(&centralDirectorySize), 4);
    // Offset of start of central directory, relative to start of archive
    outputFile.write(reinterpret_cast<const char*>(&centralDirectoryOffset), 4);
    // Comment length
    outputFile.write("\0\0", 2);

    outputFile.close();

    return true;
}

bool OctopusFile::checkOctopusFileHeader(std::ifstream &file, Error *error) {
    CHECK(file.tellg() < 134, INVALID_OCTOPUS_FILE);

    CHECK_HEADER(0, PK_LOCAL_FILE_HEADER_SIGNATURE, 4);
    // Compression method NONE and last modification time 0
    CHECK_HEADER(8, std::string(4, 0), 4);
    // Last modification date
    CHECK_HEADER(12, std::string(1, 33), 1);
    CHECK_HEADER(13, std::string(1, 0), 1);
    // File name length and extra field length
    CHECK_HEADER(26, std::string(1, 7), 1);
    CHECK_HEADER(27, std::string(3, 0), 3);
    // File name and file data
    CHECK_HEADER(30, OCTOPUS_FILE_SIGNATURE_NAME OCTOPUS_FILE_SIGNATURE_CONTENTS, 51);

    // General purpose bit flag
    file.seekg(6, std::ios::beg);
    char c6;
    file.read(&c6, 1);
    CHECK(c6==0 || c6==8, INVALID_OCTOPUS_FILE);

    // Compressed size
    file.seekg(18, std::ios::beg);
    std::string s18(3, '\0');
    file.read(&s18[0], 3);
    const uint32_t i18 = (s18[0]<<0) | (s18[1]<<8) | (s18[2]<<16);
    CHECK(i18 >= 44, INVALID_OCTOPUS_FILE);
    // Uncompressed size
    file.seekg(22, std::ios::beg);
    std::string s22(3, '\0');
    file.read(&s22[0], 3);
    const uint32_t i22 = (s22[0]<<0) | (s22[1]<<8) | (s22[2]<<16);
    CHECK(i22 >= 44, INVALID_OCTOPUS_FILE);
    // Sizes are equal, the compression method is NONE
    CHECK(s18 == s22, INVALID_OCTOPUS_FILE);

    return true;
}

}
