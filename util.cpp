#include "util.h"

#include <fstream>

std::vector<char> Util::readFile(const std::string& filename)
{
    /*
        ate: Start reading at the end of the file.
        binary: Read the file as binary file (avoid text transformations).
    */
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open file: " + filename);
    }

    /*
        The advantage of starting to read at the end of the file is that
        we can use the read position to determine the size of the file and allocate a buffer.
    */
    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);

    /*
        After that, we can seek back to the beginning of the file
        and read all of the bytes at once.
    */
    file.seekg(0);
    file.read(buffer.data(), fileSize);

    // Close the file.
    file.close();

    // Return the bytes.
    return buffer;
}
