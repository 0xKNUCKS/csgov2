#pragma once
#include <iostream>
#include <filesystem>

namespace utils
{
    bool FileExists(const char* file);

    // a Y/N choice function.
    bool choice(const char* str, ...);
    
    // source: https://lordhypersonic.blogspot.com/2019/02/c-ascii-art-generator.html
    void ascii_art(std::string input);
}
