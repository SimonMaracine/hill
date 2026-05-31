#pragma once

#include <string>
#include <filesystem>

#include "hill/error.hpp"

namespace hill::file {
    using Buffer = std::string;
    using Path = std::filesystem::path;

    void read(const Path& path, Buffer& buffer);
    void write(const Path& path, const Buffer& buffer);

    struct FileError : error::Error {
        using Error::Error;
    };
}
