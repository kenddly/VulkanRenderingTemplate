#include <core/Process.hpp>
#include <array>
#include <cstdio>

ProcessResult runCommand(const std::string& command)
{
    std::array<char, 256> buffer;
    std::string output;

#if defined(_WIN32)
    FILE* pipe = _popen(command.c_str(), "r");
#else
    FILE* pipe = popen(command.c_str(), "r");
#endif

    if (!pipe)
        return { -1, "Failed to spawn process" };

    while (fgets(buffer.data(), buffer.size(), pipe))
        output += buffer.data();

#if defined(_WIN32)
    int code = _pclose(pipe);
#else
    int code = pclose(pipe);
#endif

    return { code, output };
}
