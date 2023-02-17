
#include "FilePath.h"

// COPYRIGHT NOTICE
// The following code is adapted from the Artery Engine by V. Chlumsky
// It is licensed under the MIT license.

namespace ode {

static void appendPathSeparator(std::string &str) {
    if (!str.empty() && str[str.size()-1] != '/')
        str += '/';
}

FilePath::FilePath() { }

FilePath::FilePath(const std::string &path) : path(path) {
    normalize();
}

FilePath::FilePath(const char *path) : path(path) {
    normalize();
}

FilePath & FilePath::operator+=(const FilePath &suffix) {
    if (suffix) {
        if (!*this || suffix.isAbsolute())
            *this = suffix;
        else {
            appendPathSeparator(path);
            path += suffix.path;
        }
    }
    return *this;
}

FilePath & FilePath::operator+=(FilePath &&suffix) {
    if (suffix) {
        if (!*this || suffix.isAbsolute())
            *this = (FilePath &&) suffix;
        else {
            appendPathSeparator(path);
            path += suffix.path;
        }
    }
    return *this;
}

FilePath & FilePath::operator+=(const std::string &suffix) {
    return operator+=(FilePath(suffix));
}

FilePath & FilePath::operator+=(const char *suffix) {
    return operator+=(FilePath(suffix));
}

FilePath FilePath::operator+(const FilePath &suffix) const & {
    if (suffix) {
        if (!*this || suffix.isAbsolute())
            return suffix;
        FilePath result;
        result.path.reserve(path.size()+suffix.path.size()+1);
        result.path += path;
        appendPathSeparator(result.path);
        result.path += suffix.path;
        return result;
    }
    return *this;
}

FilePath FilePath::operator+(FilePath &&suffix) const & {
    if (suffix) {
        if (!*this || suffix.isAbsolute())
            return suffix;
        FilePath result;
        result.path.reserve(path.size()+suffix.path.size()+1);
        result.path += path;
        appendPathSeparator(result.path);
        result.path += suffix.path;
        return result;
    }
    return *this;
}

FilePath FilePath::operator+(const FilePath &suffix) && {
    return (FilePath &&) operator+=(suffix);
}

FilePath FilePath::operator+(FilePath &&suffix) && {
    return (FilePath &&) operator+=((FilePath &&) suffix);
}

FilePath FilePath::operator+(const std::string &suffix) const & {
    return *this+FilePath(suffix);
}

FilePath FilePath::operator+(const std::string &suffix) && {
    return (FilePath &&) *this+FilePath(suffix);
}

FilePath FilePath::operator+(const char *suffix) const & {
    return *this+FilePath(suffix);
}

FilePath FilePath::operator+(const char *suffix) && {
    return (FilePath &&) *this+FilePath(suffix);
}

FilePath::operator const std::string &() const {
    return path;
}

FilePath::operator bool() const {
    return !path.empty();
}

bool FilePath::empty() const {
    return path.empty();
}

bool FilePath::truncate() {
    const char *start = path.data();
    const char *cur = start+path.size();
    // Ignore slash at end
    if (cur > start)
        --cur;
    while (cur > start) {
        // Find last slash
        if (*--cur == '/') {
            // Potential double slash at the beginning of path
            if (cur > start && *(cur-1) == '/')
                --cur;
            // Parent of /root-dir is /
            else if (cur == start)
                ++cur;
            path.resize(cur-start);
            return true;
        }
    }
    if (!path.empty()) {
        path.clear();
        return true;
    }
    return false;
}

FilePath FilePath::truncated() const {
    FilePath result;
    const char *start = path.data();
    const char *cur = start+path.size();
    // Ignore slash at end
    if (cur > start)
        --cur;
    while (cur > start) {
        // Find last slash
        if (*--cur == '/') {
            // Potential double slash at the beginning of path
            if (cur > start && *(cur-1) == '/')
                --cur;
            // Parent of /root-dir is /
            else if (cur == start)
                ++cur;
            result.path = std::string(start, cur);
            break;
        }
    }
    return result;
}

FilePath FilePath::parent() const & {
    FilePath result;
    const char *start = path.data();
    const char *cur = start+path.size();
    // Ignore slash at end
    if (cur > start && *(cur-1) == '/')
        --cur;
    if (cur > start && *(cur-1) == '.') {
        // Remove any "."
        while (cur-1 > start && *(cur-1) == '.' && *(cur-2) == '/')
            cur -= 2;
        if (cur-1 == start && *(cur-1) == '.')
            --cur;
        // If end of path is ".." it won't be removed but added instead
        if (cur-1 > start && *(cur-1) == '.' && *(cur-2) == '.' && (cur-2 == start || *(cur-3) == '/')) {
            result.path = std::string(start, cur)+"/..";
            return result;
        }
    }
    bool pathNonEmpty = cur > start;
    while (cur > start) {
        // Find last slash
        if (*--cur == '/') {
            // Potential double slash at the beginning of path
            if (cur > start && *(cur-1) == '/')
                --cur;
            // Parent of /root-dir is /
            else if (cur == start)
                ++cur;
            result.path = std::string(start, cur);
            return result;
        }
    }
    if (isRelative())
        result.path = pathNonEmpty ? "." : "..";
    return result;
}

FilePath FilePath::parent() && {
    const char *start = path.data();
    const char *cur = start+path.size();
    // Ignore slash at end
    if (cur > start && *(cur-1) == '/')
        --cur;
    if (cur > start && *(cur-1) == '.') {
        // Remove any "."
        while (cur-1 > start && *(cur-1) == '.' && *(cur-2) == '/')
            cur -= 2;
        if (cur-1 == start && *(cur-1) == '.')
            --cur;
        // If end of path is ".." it won't be removed but added instead
        if (cur-1 > start && *(cur-1) == '.' && *(cur-2) == '.' && (cur-2 == start || *(cur-3) == '/')) {
            path.resize(cur-start+3);
            path[path.size()-3] = '/', path[path.size()-2] = '.', path[path.size()-1] = '.';
            return (FilePath &&) *this;
        }
    }
    bool pathNonEmpty = cur > start;
    while (cur > start) {
        // Find last slash
        if (*--cur == '/') {
            // Potential double slash at the beginning of path
            if (cur > start && *(cur-1) == '/')
                --cur;
            // Parent of /root-dir is /
            else if (cur == start)
                ++cur;
            path.resize(cur-start);
            return (FilePath &&) *this;
        }
    }
    if (isRelative())
        path = pathNonEmpty ? "." : "..";
    else
        path.clear();
    return (FilePath &&) *this;
}

const char *FilePath::filename() const {
    const char *start = path.data();
    const char *cur = start+path.size();
    while (cur > start) {
        if (*--cur == '/') {
            ++cur;
            break;
        }
    }
    return cur;
}

bool FilePath::isAbsolute() const {
    return !path.empty() && path[0] == '/';
}

bool FilePath::isRelative() const {
    return !path.empty() && path[0] != '/';
}

FilePath FilePath::canonical() const {
    FilePath result;
    bool absolute = isAbsolute();
    result.path.resize(path.size());
    char *start = &result.path[0];
    char *dst = start;
    const char *src = path.data();
    const char *end = src+path.size();
    char prev = '\0', prevPrev = '\0', prevPrevPrev = '\0';
    for (; src <= end; ++src) {
        char c = *src;
        if (!c || c == '/') {
            // Simplify x/y/../ to x/
            if (prev == '.' && prevPrev == '.' && prevPrevPrev == '/') {
                char *backtracker = dst-3;
                if (backtracker >= start) {
                    // Find start of previous level directory
                    while (backtracker > start) {
                        if (*--backtracker == '/') {
                            ++backtracker;
                            break;
                        }
                    }
                    // Make sure not to cancel out ../..
                    if (!(backtracker == dst-5 && *backtracker == '.' && *(backtracker+1) == '.')) {
                        // Ignore double-dots on root directory - /../ to / and //server/../ to //server/ only
                        if (absolute && (backtracker == start || backtracker == start+2))
                            dst -= 2;
                        else
                            dst = backtracker;
                        c = '\0';
                    }
                }
                // Simplify x/./ to x/
            } else if (prev == '.' && prevPrev == '/') {
                --dst;
                c = '\0';
            }
        }
        if (c)
            *dst++ = c;
        prevPrevPrev = prevPrev;
        prevPrev = prev;
        prev = *src; // not c because it may have been cleared
    }
    // Remove trailing slash
    if (dst >= start+2 && *(dst-1) == '/' && *(dst-2) != '/')
        --dst;
    result.path.resize(dst-start);
    return result;
}

static size_t commonPathPrefixLength(const char *aStart, const char *aEnd, const char *bStart, const char *bEnd) {
    size_t prefixLen = 0;
    const char *aCur = aStart;
    const char *bCur = bStart;
    // Match initial slash
    if (aCur < aEnd && bCur < bEnd && *aCur == '/' && *bCur == '/' && (aCur+1 >= aEnd || *(aCur+1) != '/') && (bCur+1 >= bEnd || *(bCur+1) != '/'))
        prefixLen = 1;
    bool nonSeparator = false;
    for (; aCur < aEnd && bCur < bEnd && *aCur == *bCur; ++aCur, ++bCur) {
        if (*aCur == '/') {
            // Prevent matching initial double-slash (e.g. //x and //y to //)
            if (nonSeparator)
                prefixLen = aCur-aStart;
        } else
            nonSeparator = true;
    }
    if ((aCur >= aEnd || *aCur == '/') && (bCur >= bEnd || *bCur == '/'))
        prefixLen = aCur-aStart;
    return prefixLen;
}

FilePath FilePath::relativeTo(const FilePath &basePath) const {
    bool absoluteSuffix = isAbsolute();
    if (absoluteSuffix && !basePath.isAbsolute())
        return *this;
    FilePath result;
    const char *baseStart = basePath.path.data();
    const char *baseEnd = baseStart+basePath.path.size();
    const char *suffixStart = path.data();
    const char *suffixEnd = suffixStart+path.size();
    // Remove trailing slash
    if (baseEnd >= baseStart+2 && *(baseEnd-1) == '/' && *(baseEnd-2) != '/')
        --baseEnd;
    if (suffixEnd >= suffixStart+2 && *(suffixEnd-1) == '/' && *(suffixEnd-2) != '/')
        --suffixEnd;
    // Find common prefix
    size_t prefixLen = commonPathPrefixLength(baseStart, baseEnd, suffixStart, suffixEnd);
    if (absoluteSuffix && !prefixLen)
        return *this;
    if (prefixLen == 1 && *baseStart == '/')
        prefixLen = 0;
    const char *baseCur = baseStart+prefixLen+1;
    const char *suffixCur = suffixStart+prefixLen+1;
    // cur pointers may be larger than end!
    result.path.reserve(basePath.path.size()+path.size()-2*prefixLen);
    // Both paths are equal
    if (baseCur >= baseEnd && suffixCur >= suffixEnd)
        return FilePath(".");
    // Add directory-up sequence
    if (baseCur < baseEnd) {
        int upCount = 1;
        for (; baseCur < baseEnd; ++baseCur)
            upCount += *baseCur == '/';
        for (int i = 0; i < upCount; ++i)
            result.path += "../";
        if (suffixCur >= suffixEnd)
            result.path.pop_back();
    }
    // Add suffix
    if (suffixCur < suffixEnd)
        result.path += std::string(suffixCur, suffixEnd);
    return result;
}

bool FilePath::equivalent(const FilePath &a, const FilePath &b) {
    return a.canonical() == b.canonical();
}

FilePath FilePath::commonPrefix(const FilePath &a, const FilePath &b) {
    FilePath result;
    result.path = a.path.substr(0, commonPathPrefixLength(a.path.data(), a.path.data()+a.path.size(), b.path.data(), b.path.data()+b.path.size()));
    return result;
}

void FilePath::normalize() {
    if (empty()) {
        return;
    }

    for (char &c : path) {
        if (c == '\\')
            c = '/';
    }
    char *start = &path[0];
    char *dst = start+1; // Allow double-slash at the beginning by ignoring the first character
    const char *src = dst;
    const char *end = start+path.size();
    char prev = '\0';
    for (; src < end; ++src) {
        char c = *src;
        // Convert multiple slashes to one
        if (c == '/' && prev == '/')
            continue;
        *dst++ = c;
        prev = c;
    }
    path.resize(dst-start);
}

bool operator==(const FilePath &a, const FilePath &b) {
    return a.path == b.path;
}

bool operator!=(const FilePath &a, const FilePath &b) {
    return a.path != b.path;
}

}
