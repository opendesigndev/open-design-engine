
#pragma once

#include <vector>
#include <string>
#include <optional>

#include "FilePath.h"
#include "../utils.h"

namespace ode {

/// Representation of a filesystem loaded to memory.
class MemoryFileSystem {
public:
    enum class Error : uint8_t {
        OK = 0,
        ERROR_OPENING_FILE,
        INVALID_OCTOPUS_FILE,
        COMPRESSION_FAILED,
        DECOMPRESSION_FAILED,
        UNSUPPORTED_COMPRESSION_METHOD,
        DUPLICATE_FILE_PATH
    };

    enum class CompressionMethod : uint8_t {
        NONE = 0,
        DEFLATE = 8
    };

    /// A single compressed file in the filesystem, its contents and metadata.
    struct File {
        FilePath path;
        CompressionMethod compressionMethod;
        uint32_t crc32;
        uint32_t compressedSize;
        uint32_t uncompressedSize;
        std::vector<byte> data;
    };
    using FileRef = std::reference_wrapper<File>;
    using Files = std::vector<File>;

    /// Provide read-only access to the files.
    const std::vector<File> &files() const;
    /// Detects if the specified file exists.
    bool exists(const FilePath& filePath) const;
    /// Read a single specified file. Decompress if the loaded file data is compressed.
    std::optional<std::vector<byte>> getFileData(const FilePath& filePath, Error *error = nullptr) const;
    /// Adds a data file at the specified path, compress by the specified compression method.
    std::optional<FileRef> add(const FilePath &filePath, const std::vector<byte> &data, CompressionMethod compressionMethod, Error *error = nullptr);
    /// Adds a text file at the specified path, compress by the specified compression method.
    std::optional<FileRef> add(const FilePath &filePath, const std::string &data, CompressionMethod compressionMethod, Error *error = nullptr);

    /// Clear files data.
    void clear();

protected:
    std::optional<std::vector<byte>> compress(const std::vector<byte> &data, CompressionMethod compressionMethod, Error *error = nullptr) const;
    std::optional<std::vector<byte>> decompress(const std::vector<byte> &data, CompressionMethod compressionMethod, Error *error = nullptr) const;

    Files files_;
};

} // namespace ode
