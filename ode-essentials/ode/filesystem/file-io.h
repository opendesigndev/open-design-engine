
#pragma once

#include <cstdio>
#include <vector>
#include <string>
#include "../utils.h"
#include "FilePath.h"

namespace ode {

/// Manages a FILE pointer (closes it when done)
class FilePtr {

public:
    inline FilePtr(FILE *file = nullptr) : file(file) { }
    FilePtr(const FilePtr &) = delete;
    FilePtr(FilePtr &&orig);
    ~FilePtr();
    FilePtr &operator=(const FilePtr &) = delete;
    FilePtr &operator=(FilePtr &&orig);
    operator FILE *() const;

private:
    FILE *file;

};

/// Opens FilePath as FILE *
FILE *openFile(const FilePath &path, bool write);

/// Reads entire file to std::vector<byte>
bool readFile(const FilePath &path, std::vector<byte> &dst);
/// Reads entire file to std::string
bool readFile(const FilePath &path, std::string &dst);
/// Writes a vector of bytes to a file
bool writeFile(const FilePath &path, const std::vector<byte> &data);
/// Writes a string to a file
bool writeFile(const FilePath &path, const std::string &data);
/// Writes an array of bytes to a file
bool writeFile(const FilePath &path, const byte *data, size_t length);
/// Writes an array of characters to a file
bool writeFile(const FilePath &path, const char *data, size_t length);

}
