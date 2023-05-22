
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
        DECOMPRESSION_FAILED,
        UNSUPPORTED_COMPRESSION_METHOD
    };

    enum class CompressionMethod : uint16_t {
        NONE = 0,
        DEFLATE = 8
    };

    /// A single file in the filesystem, its contents and metadata.
    struct File {
        FilePath path;
        CompressionMethod compressionMethod;
        uint32_t compressedSize;
        uint32_t uncompressedSize;
        std::string data;
    };

    /// Initialize by opening Octopus file from the filesystem.
    bool openOctopusFile(const FilePath &octopusFilePath, Error *error = nullptr);
    /// Initialize by load Octopus data from a string.
    bool readOctopusData(const std::string &octopusFileData, Error *error = nullptr);

    /// Get list of all contained files as their paths.
    const std::vector<FilePath> filePaths() const;
    /// Read a single specified file. Decompress if the loaded file data is compressed.
    std::optional<std::string> getFileData(const FilePath& filePath, Error *error = nullptr) const;

    /// Adds an uncompressed data file at the specified path.
    void add(const FilePath &path, const std::string &data);

    /// Clear files data.
    void clear();

private:
    /// Check if the file from the file stream has a valid Octopus header
    bool checkOctopusFileHeader(std::ifstream &file, Error *error = nullptr);

    std::vector<File> files;
};

} // namespace ode
