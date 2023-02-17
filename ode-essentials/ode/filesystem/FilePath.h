
#pragma once

#include <string>

namespace ode {

/// Represents a file path string, concatenation operations add separator in-between operands
class FilePath {

public:
    FilePath();
    FilePath(const std::string &path);
    FilePath(const char *path);
    FilePath & operator+=(const FilePath &suffix);
    FilePath & operator+=(FilePath &&suffix);
    FilePath & operator+=(const std::string &suffix);
    FilePath & operator+=(const char *suffix);
    FilePath operator+(const FilePath &suffix) const &;
    FilePath operator+(FilePath &&suffix) const &;
    FilePath operator+(const FilePath &suffix) &&;
    FilePath operator+(FilePath &&suffix) &&;
    FilePath operator+(const std::string &suffix) const &;
    FilePath operator+(const std::string &suffix) &&;
    FilePath operator+(const char *suffix) const &;
    FilePath operator+(const char *suffix) &&;
    explicit operator const std::string &() const;
    explicit operator bool() const;
    bool empty() const;
    /// Removes the last portion of the path (returns false if not possible)
    bool truncate();
    /// Returns a copy of path with the last portion removed
    FilePath truncated() const;
    /// Returns parent directory - unlike truncated(), understands and uses . and .. when necessary
    FilePath parent() const &;
    FilePath parent() &&;
    /// Returns the filename portion of the path
    const char *filename() const;
    /// Returns true if path is absolute
    bool isAbsolute() const;
    /// Returns true if path is relative
    bool isRelative() const;
    /// Attempts to transform the path to a canonical form - resolves . and .. (if possible) and removes trailing slash
    FilePath canonical() const;
    /// Returns path expressed as relative to basePath
    FilePath relativeTo(const FilePath &basePath) const;

    /// Returns true if paths a and b are equivalent (their canonical forms are equal)
    static bool equivalent(const FilePath &a, const FilePath &b);
    /// Returns the beginning portion of path that is common to a and b
    static FilePath commonPrefix(const FilePath &a, const FilePath &b);

    friend bool operator==(const FilePath &a, const FilePath &b);
    friend bool operator!=(const FilePath &a, const FilePath &b);

private:
    std::string path;

    void normalize();

};

}
