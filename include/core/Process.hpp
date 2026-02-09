#pragma once
#include <string>

struct ProcessResult
{
    int exitCode;
    std::string output;
};

ProcessResult runCommand(const std::string& command);
