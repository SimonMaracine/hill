#include "hill/file.hpp"

#include <fstream>

namespace hill::file {
    void read(const Path& path, Buffer& buffer) {
        std::ifstream stream {path, std::ios_base::binary};

        if (!stream.is_open()) {
            throw FileError("Could not open file");
        }

        stream.seekg(0, stream.end);
        const auto size = stream.tellg();
        stream.seekg(0, stream.beg);

        buffer.resize(std::size_t(size));
        stream.read(buffer.data(), size);

        if (stream.fail()) {
            throw FileError("Could not read from file");
        }
    }

    void write(const Path& path, const Buffer& buffer) {
        std::ofstream stream {path, std::ios_base::binary};

        if (!stream.is_open()) {
            throw FileError("Could not open file");
        }

        stream.write(buffer.data(), std::streamsize(buffer.size()));

        if (stream.fail()) {
            throw FileError("Could not write to file");
        }
    }
}
