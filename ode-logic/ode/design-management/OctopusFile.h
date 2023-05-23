
#pragma once

#include <ode-essentials.h>

namespace ode {

class OctopusFile: public MemoryFileSystem {
public:
    /// Initialize by opening Octopus file from the filesystem.
    bool load(const FilePath &octopusFilePath, Error *error = nullptr);
    /// Initialize by load Octopus data from a string.
    bool parseFromString(const std::string &octopusFileData, Error *error = nullptr);
    /// Save as Octopus file.
    bool save(const FilePath &octopusFilePath, Error *error = nullptr);

private:
    /// Check if the file from the file stream has a valid Octopus header
    bool checkOctopusFileHeader(std::ifstream &file, Error *error = nullptr);
};

}
