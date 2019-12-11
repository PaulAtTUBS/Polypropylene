//
// Created by Paul on 06.01.2018.
//

#include <proteus/io/Path.h>
#include <iostream>
#include <stack>

#ifdef PAX_OS_WIN
#include <windows.h>
#endif

#ifdef PAX_OS_LINUX
#include <limits.h> /* PATH_MAX */
#endif

namespace PAX {
    Path::Path() : _path(EmptyPath) {

    }

    Path::Path(const char *path) : _path(path) {
        simplify(_path);
    }

    Path::Path(const std::string& path) : Path(path.c_str()) {

    }

    Path::Path(const Path &other) : _path(other._path) {
        // do not call other constructors to avoid unnecessary simplification
    }

    bool Path::isFile() const {
        auto dotPos = _path.find_last_of('.');
        if (dotPos != std::string::npos) {
            // the dot has to be followed or preceeded by text
            // otherwise it will be the current directory "."
            bool notCurrDir =
                    (
                            (dotPos > 0              && _path[dotPos - 1] != '.') ||
                            (dotPos < _path.size()-1 && _path[dotPos + 1] != '.')
                    ) && _path.size() > 1;

            if (notCurrDir) {
                auto lastSlashPos = _path.find_last_of(PathSeparator);
                return lastSlashPos == std::string::npos || lastSlashPos < dotPos;
            }
        }
        return false;
    }

    bool Path::isDirectory() const {
        return !isFile();
    }

    bool Path::isAbsolute() const {
        return isAbsolute(_path);
    }

    bool Path::isRelative() const {
        return !isAbsolute();
    }

    bool Path::isAbsolute(const std::string &path) {
#ifdef PAX_OS_WIN
        // Absolute paths on win are of the form
        // Drive:/
        for (size_t i = 0; i < path.size(); ++i) {
            if (path[i] == Path::PathSeparator) {
                break;
            }
            else if (path[i] == '.') {
                break;
            }
            else if (path[i] == ':') {
                return true;
            }
        }

        return false;
#else
        return Util::String::startsWith(path, "/");
#endif
    }

    Path Path::getDirectory() const {
        if (isDirectory()) {
            // Use copy constructor to avoid unnecessary simplification
            return Path(*this);
        } else {
            std::string dir = _path.substr(0, _path.find_last_of(PathSeparator));
            return Path(dir);
        }
    }

    Path Path::toAbsolute() const {
        return toAbsolute(_path);
    }

    std::string Path::toAbsolute(const std::string &path) {
#ifdef PAX_OS_WIN
        constexpr unsigned int HARDCODED_BUFSIZE = 4096;
        TCHAR  buffer[HARDCODED_BUFSIZE] = TEXT("");
        TCHAR** lppPart = {nullptr};

        DWORD success = GetFullPathName(path.c_str(), HARDCODED_BUFSIZE, buffer, lppPart);

        if (success == 0) {
            std::cerr << "Could not convert \"" << path << "\" to absolute path!" << std::endl;
            return path;
        }

        return Path(buffer);
#else
        char buf[PATH_MAX + 1]; /* not sure about the "+ 1" */
        char *res = realpath(path.c_str(), buf);
        if (res) {
            return buf;
        } else {
            std::cerr << "[Path::toAbsolute] realpath failed with message: " << res << std::endl;
            return path;
        }
#endif
    }

    void Path::convertToCurrentPlatform() {
        Path::convertToCurrentPlatform(_path);
    }

    void Path::convertToWin() {
        Path::convertToWin(_path);
    }

    void Path::convertToUnix() {
        Path::convertToUnix(_path);
    }

    Path Path::convertedToCurrentPlatform() const {
        Path copy = *this;
        copy.convertToCurrentPlatform();
        return copy;
    }

    Path Path::convertedToWin() const {
        Path copy = *this;
        copy.convertToWin();
        return copy;
    }

    Path Path::convertedToUnix() const {
        Path copy = *this;
        copy.convertToUnix();
        return copy;
    }

    void Path::simplify() {
        Path::simplify(_path);
    }

    void Path::convertToCurrentPlatform(std::string &path) {
        String::replace(path, PathSeparator_Unix, PathSeparator);
    }

    void Path::convertToWin(std::string& path) {
        String::replace(path, PathSeparator_Unix, PathSeparator_Win);
    }

    void Path::convertToUnix(std::string& path) {
        String::replace(path, PathSeparator_Win, PathSeparator_Unix);
    }

    void Path::simplify(std::string & path) {
#ifndef PAX_OS_WIN
        bool absolute = isAbsolute(path);
#endif

        convertToUnix(path);

        /// Implementation copied from https://www.geeksforgeeks.org/simplify-directory-path-unix-like/
        /// And fixed by Paul

        // stack to store the file's names.
        std::stack<std::string> st;

        // temporary string which stores the extracted
        // directory name or commands("." / "..")
        // Eg. "/a/b/../."
        // dir will contain "a", "b", "..", ".";
        std::string dir;

        // contains resultant simplifies string.
        std::string res;

        // stores length of input string.
        size_t len_A = path.length();

        for (size_t i = 0; i < len_A; i++) {

            // we will clear the temporary string
            // every time to accomodate new directory
            // name or command.
            dir.clear();

            // skip all the multiple '/' Eg. "/////""
            while (path[i] == '/')
                i++;

            // stores directory's name("a", "b" etc.)
            // or commands("."/"..") into dir
            while (i < len_A && path[i] != '/') {
                dir.push_back(path[i]);
                i++;
            }

            // if dir has ".." just pop the topmost
            // element if the stack is not empty and a regular directory
            if (dir == ".." && !st.empty() && st.top() != "..") {
                st.pop();
            }

                // if dir has "." then simply continue
                // with the process.
            else if (dir == ".") {
                continue;
            }
                // pushes if it encounters directory's
                // name("a", "b").
            else if (dir.length() != 0) {
                st.push(dir);
            }

        }

        // a temporary stack  (st1) which will contain
        // the reverse of original stack(st).
        std::stack<std::string> st1;
        while (!st.empty()) {
            st1.push(st.top());
            st.pop();
        }

        // the st1 will contain the actual res.
        while (!st1.empty()) {
            std::string temp = st1.top();

            // if it's the last element no need
            // to append "/"
            if (st1.size() != 1)
                res.append(temp + PathSeparator);
            else
                res.append(temp);

            st1.pop();
        }

        // every string starts from root directory.
#ifndef PAX_OS_WIN
        if (absolute)
            res = "/" + res;
#endif

        if (res.empty())
            res = EmptyPath;

        path = res;
    }

    bool Path::operator==(const Path &other) const {
        std::string me = toAbsolute(_path);
        std::string he = toAbsolute(other._path);

        convertToUnix(me);
        convertToUnix(he);

        return me == he;
    }

    bool Path::operator!=(const Path &other) const {
        return !this->operator==(other);
    }

    bool Path::operator<(const Path &other) const {
        std::string me = toAbsolute(_path);
        std::string he = toAbsolute(other._path);

        convertToUnix(me);
        convertToUnix(he);

        return me < he;
    }

    bool Path::operator>(const Path &other) const {
        std::string me = toAbsolute(_path);
        std::string he = toAbsolute(other._path);

        convertToUnix(me);
        convertToUnix(he);

        return me > he;
    }

    Path::operator const char*() const {
        return _path.c_str();
    }

    Path::operator std::string() const {
        return _path;
    }

    const char* Path::c_str() const {
        return _path.c_str();
    }

    const std::string& Path::toString() const {
        return _path;
    }

    Path& Path::operator=(const char *path) {
        _path = path;
        simplify(_path);
        return *this;
    }

    Path& Path::operator=(const std::string &path) {
        _path = path;
        simplify(_path);
        return *this;
    }

    Path& Path::operator=(const Path &other) {
        _path = other._path;
        return *this;
    }

    Path Path::operator+(const char *path) const {
        return Path(_path + PathSeparator + std::string(path));
    }

    Path Path::operator+(const std::string &path) const {
        return Path(_path + PathSeparator + path);
    }

    Path& Path::operator+=(const char *path) {
        _path += PathSeparator + path;
        simplify(_path);
        return *this;
    }

    Path& Path::operator+=(const std::string &path) {
        _path += PathSeparator + path;
        simplify(_path);
        return *this;
    }

    Path Path::operator+(const Path &other) const {
        return operator+(other._path);
    }

    Path& Path::operator+=(const Path &other) {
        return operator+=(other._path);
    }
}

std::ostream& operator<<(std::ostream& os, const PAX::Path & p)
{
    return os << p.toAbsolute().toString();
}