
#include "file-io.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#ifdef _WIN32
    #include <io.h>
    #define NEW_FILE_PERMISSIONS (S_IREAD|S_IWRITE)
    #define S_ISREG(m) (((m)&S_IFMT) == S_IFREG)
#else
    #include <unistd.h>
    #define NEW_FILE_PERMISSIONS (S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH)
    #define O_BINARY 0
#endif

namespace ode {

FilePtr::FilePtr(FilePtr &&orig) : file(orig.file) {
    orig.file = nullptr;
}

FilePtr::~FilePtr() {
    if (file)
        fclose(file);
}

FilePtr &FilePtr::operator=(FilePtr &&orig) {
    if (this != &orig) {
        file = orig.file;
        orig.file = nullptr;
    }
    return *this;
}

FilePtr::operator FILE *() const {
    return file;
}

FILE *openFile(const FilePath &path, bool write) {
    // TODO non-ASCII on Windows
    return fopen(((const std::string &) path).c_str(), write ? "wb" : "rb");
}

static int fdOpen(const FilePath &path, bool write) {
    // TODO non-ASCII on Windows
    return open(
        ((const std::string &) path).c_str(),
        (write ? O_WRONLY|O_CREAT|O_TRUNC : O_RDONLY)|O_BINARY,
        write ? NEW_FILE_PERMISSIONS : 0
    );
}

bool readFile(const FilePath &path, std::vector<byte> &dst) {
    bool result = false;
    int fd = fdOpen(path, false);
    if (fd != -1) {
        struct stat fileStat;
        if (!fstat(fd, &fileStat)) {
            if (S_ISREG(fileStat.st_mode)) {
                dst.resize(fileStat.st_size);
                result = read(fd, dst.data(), fileStat.st_size) == fileStat.st_size;
            }
        }
        close(fd);
    }
    return result;
}

bool readFile(const FilePath &path, std::string &dst) {
    bool result = false;
    int fd = fdOpen(path, false);
    if (fd != -1) {
        struct stat fileStat;
        if (!fstat(fd, &fileStat)) {
            if (S_ISREG(fileStat.st_mode)) {
                dst.resize(fileStat.st_size);
                result = read(fd, &dst[0], fileStat.st_size) == fileStat.st_size;
            }
        }
        close(fd);
    }
    return result;
}

bool writeFile(const FilePath &path, const std::vector<byte> &data) {
    return writeFile(path, data.data(), data.size());
}

bool writeFile(const FilePath &path, const std::string &data) {
    return writeFile(path, reinterpret_cast<const byte *>(data.data()), data.size());
}

bool writeFile(const FilePath &path, const byte *data, size_t length) {
    bool result = false;
    int fd = fdOpen(path, true);
    if (fd != -1) {
        result = write(fd, data, length) == length;
        close(fd);
    }
    return result;
}

bool writeFile(const FilePath &path, const char *data, size_t length) {
    return writeFile(path, reinterpret_cast<const byte *>(data), length);
}

}
