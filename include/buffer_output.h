#ifndef BUFFER_H_
#define BUFFER_H_

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

class BufferOutput {
private:
    std::stringstream buffer;
    std::ofstream output_file;
    size_t buffer_limit;

public:
    BufferOutput(const std::string &filename, size_t limit = 10 * 1024 * 1024);

    ~BufferOutput();

    template <typename T>
    BufferOutput &operator<<(const T &data);

    BufferOutput &operator<<(std::ostream &(*manip)(std::ostream &));

    void flush();
};

template <typename T>
BufferOutput &BufferOutput::operator<<(const T &data) {
    buffer << data;
    if (buffer.tellp() >= static_cast<std::streampos>(buffer_limit)) {
        flush();
    }
    return *this;
}

#endif // BUFFER_H_