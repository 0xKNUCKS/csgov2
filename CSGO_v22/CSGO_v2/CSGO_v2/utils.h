#pragma once
#include <iostream>
#include <fstream>
#include "entity.h"
#include "../ext/MD5/md5.h"

namespace utils
{
    // Internal System Related
    std::string RandomString(const int len);
    std::string GetGameSumHashMD5();
    const char* GetFileMD5(const char* file);
    bool CheckVersion(const char* MD5Hash);
    void SetupConsole();
    std::string VirtualKeyToString(unsigned int virtualKey);

    // Game Related
    bool WorldToScreen(math::Vector Pos, math::Vector& ScreenPos);
    math::Vector VectorTransform(const math::Vector& in, const math::Matrix3x4& matrix);

    // Memory Related
    std::uint8_t* PatternScan(void* module, const char* signature);
}