
#pragma once

#include <vector>
#include <string>
#include <optional>

#include "FilePath.h"

namespace ode {

/// Representation of a filesystem loaded to memory.
class MemoryFileSystem {
public:
    enum class Error {
        OK = 0,
        ERROR_OPENING_FILE,
        INVALID_OCTOPUS_FILE,
        COMPRESSION_FAILED,
        DECOMPRESSION_FAILED,
        UNSUPPORTED_COMPRESSION_METHOD,
        DUPLICATE_FILE_PATH
    };

    enum class CompressionMethod : uint16_t {
        NONE = 0,
        DEFLATE = 8
    };

    /// A single file in the filesystem, its contents and metadata.
    struct File {
        FilePath path;
        CompressionMethod compressionMethod;
        uint32_t crc32;
        uint32_t compressedSize;
        uint32_t uncompressedSize;
        std::string data;
    };
    using FileRef = std::reference_wrapper<File>;
    using Files = std::vector<File>;

    /// Get list of all contained files as their paths.
    const std::vector<FilePath> filePaths() const;
    /// Read a single specified file. Decompress if the loaded file data is compressed.
    std::optional<std::string> getFileData(const FilePath& filePath, Error *error = nullptr) const;

    /// Adds a data file at the specified path, compress by the specified compression method.
    std::optional<FileRef> add(const FilePath &path, const std::string &data, CompressionMethod compressionMethod, Error *error = nullptr);

    /// Clear files data.
    void clear();

protected:
    Files files;
};

} // namespace ode
