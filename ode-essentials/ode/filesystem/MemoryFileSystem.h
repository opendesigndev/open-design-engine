
#pragma once

#include <string>

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
        UNSUPPORTED_COMPRESSION_METHOD,
    };

    /// A single file in the filesystem, its contents and metadata.
    struct File {
        FilePath path;
        uint16_t compression_method;
        uint32_t compressed_size;
        uint32_t uncompressed_size;
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

    /// Clear files data.
    void clear();

private:
    std::vector<File> files;
};

} // namespace ode
