#pragma once
#include <iostream>
#include <fstream>
#include "entity.h"
#include "../ext/MD5/md5.h"

namespace utils
{
    std::string RandomString(const int len);
    std::string GetGameSumHashMD5();
    const char* GetFileMD5(const char* file);
    bool CheckVersion(const char* MD5Hash);
    void SetupConsole();
    float SlideVal(float curVal, float Max, float fraction);

    bool WolrdToScreen(math::Vector Pos, math::Vector& ScreenPos);
    math::Vector VectorTransform(const math::Vector& in, const math::Matrix3x4& matrix);

    std::uint8_t* PatternScan(void* module, const char* signature);
}