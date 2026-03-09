#pragma once
#include <iostream>
#include <string>
#include <filesystem>

namespace utils
{
    bool FileExists(const std::string& file);

    // a Y/N choice function.
    bool choice(const std::string& prompt);

    // Resolve a path relative to the executable's directory
    std::string ResolvePathRelativeToExe(const std::string& relativePath);

    // source: https://lordhypersonic.blogspot.com/2019/02/c-ascii-art-generator.html
    void ascii_art(std::string input);
}
