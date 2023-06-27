
#include "MemoryFileSystem.h"

#include <cstring>
#include <algorithm>
#include <zlib.h>

namespace ode {

#define COMPRESSION_BUFFER_SIZE     1<<15
#define COMPRESSION_WINDOW_SIZE     -15
#define COMPRESSION_LEVEL           9

#define CHECK(condition, err) \
    if (!(condition)) { \
        if (error) { \
            *error = Error::err; \
        } \
        return std::nullopt; \
    }

const std::vector<FilePath> MemoryFileSystem::filePaths() const {
    std::vector<FilePath> result;
    for (const File &file : files) {
        result.emplace_back(file.path);
    }
    return result;
}

bool MemoryFileSystem::exists(const FilePath& filePath) const {
    return std::any_of(files.begin(), files.end(), [&filePath](const File &file) {
        return file.path == filePath;
    });
}

std::optional<std::vector<byte>> MemoryFileSystem::getFileData(const FilePath& filePath, Error *error) const {
    if (error)
        *error = Error::OK;

    const std::vector<File>::const_iterator fileIt = std::find_if(files.begin(), files.end(), [&filePath](const File &file) {
        return file.path == filePath;
    });
    if (fileIt == files.end()) {
        return std::nullopt;
    }

    return decompress(fileIt->data, fileIt->compressionMethod, error);
}

std::optional<MemoryFileSystem::FileRef> MemoryFileSystem::add(const FilePath &filePath, const std::vector<byte> &data, CompressionMethod compressionMethod, Error *error) {
    CHECK(exists(filePath)==false, DUPLICATE_FILE_PATH);

    const std::optional<std::vector<byte>> compressedData = compress(data, compressionMethod, error);
    if (compressedData.has_value()) {
        // CRC-32 of the uncompressed data
        const uLong crc32Checksum = crc32(0L, (Bytef*)data.data(), (uInt)data.size());

        return files.emplace_back(File {
            filePath,
            compressionMethod,
            static_cast<uint32_t>(crc32Checksum),
            static_cast<uint32_t>(compressedData->size()),
            static_cast<uint32_t>(data.size()),
            *compressedData
        });
    }
    return std::nullopt;
}

std::optional<MemoryFileSystem::FileRef> MemoryFileSystem::add(const FilePath &filePath, const std::string &data, CompressionMethod compressionMethod, Error *error) {
    return add(filePath, std::vector<byte>(data.data(), data.data() + data.size()), compressionMethod);
}

void MemoryFileSystem::clear() {
    files.clear();
}

std::optional<std::vector<byte>> MemoryFileSystem::compress(const std::vector<byte> &data, CompressionMethod compressionMethod, Error *error) const {
    switch (compressionMethod) {
        case CompressionMethod::NONE: {
            return data;
        }
        case CompressionMethod::DEFLATE: {
            z_stream zs;
            memset(&zs, 0, sizeof(zs));

            CHECK(deflateInit2(&zs, COMPRESSION_LEVEL, Z_DEFLATED, COMPRESSION_WINDOW_SIZE, COMPRESSION_LEVEL, Z_DEFAULT_STRATEGY) == Z_OK, COMPRESSION_FAILED);

            zs.next_in = (Bytef*)data.data();
            zs.avail_in = (uInt)data.size();

            int result;
            char outbuffer[COMPRESSION_BUFFER_SIZE];
            std::vector<byte> compressedData;

            do {
                zs.next_out = reinterpret_cast<Bytef*>(outbuffer);
                zs.avail_out = sizeof(outbuffer);
                result = deflate(&zs, Z_FINISH);
                if (compressedData.size() < zs.total_out) {
                    compressedData.insert(compressedData.end(), outbuffer, outbuffer + (zs.total_out - compressedData.size()));
                }
            } while (result == Z_OK);

            CHECK(deflateEnd(&zs) == Z_OK, COMPRESSION_FAILED);
            CHECK(result == Z_STREAM_END, COMPRESSION_FAILED);

            return compressedData;
        }
        default:
            CHECK(false, UNSUPPORTED_COMPRESSION_METHOD);
    }
    return std::nullopt;
}

std::optional<std::vector<byte>> MemoryFileSystem::decompress(const std::vector<byte> &data, CompressionMethod compressionMethod, Error *error) const {
    switch (compressionMethod) {
        case CompressionMethod::NONE: {
            return data;
        }
        case CompressionMethod::DEFLATE: {
            z_stream zs;
            memset(&zs, 0, sizeof(zs));

            CHECK(inflateInit2(&zs, COMPRESSION_WINDOW_SIZE) == Z_OK, DECOMPRESSION_FAILED);

            zs.next_in = (Bytef*)data.data();
            zs.avail_in = (uInt)data.size();

            int result = Z_OK;
            char outbuffer[COMPRESSION_BUFFER_SIZE];
            std::vector<byte> decompressedData;

            do {
                zs.next_out = reinterpret_cast<Bytef*>(outbuffer);
                zs.avail_out = sizeof(outbuffer);
                result = inflate(&zs, 0);
                if (decompressedData.size() < zs.total_out) {
                    decompressedData.insert(decompressedData.end(), outbuffer, outbuffer + (zs.total_out - decompressedData.size()));
                }
            } while (result == Z_OK);

            CHECK(inflateEnd(&zs) == Z_OK, DECOMPRESSION_FAILED);
            CHECK(result == Z_STREAM_END, DECOMPRESSION_FAILED);

            return decompressedData;
        }
        default:
            CHECK(false, UNSUPPORTED_COMPRESSION_METHOD);
    }
    return std::nullopt;
}

} // namespace ode
