#include "utils.h"
#include <chrono>
#pragma warning(disable : 4996)

std::string utils::RandomString(const int len) {
    srand(_Xtime_get_ticks());

    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    std::string tmp_s;
    tmp_s.reserve(len);

    for (int i = 0; i < len; ++i) {
        tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];
    }

    return tmp_s;
}

const char* utils::GetFileMD5(const char* file)
{
    //Start opening your file
    std::ifstream inBigArrayfile;
    inBigArrayfile.open(file, std::ios::binary | std::ios::in);

    //Find length of file
    inBigArrayfile.seekg(0, std::ios::end);
    long Length = inBigArrayfile.tellg();
    inBigArrayfile.seekg(0, std::ios::beg);

    //read in the data from your file
    char* InFileData = new char[Length];
    inBigArrayfile.read(InFileData, Length);

    //Calculate MD5 hash
    std::string Temp = md5(InFileData, Length);

#ifdef _DEBUG
    std::cout << Temp.c_str() << std::endl;
#endif // _DEBUG


    //Clean up
    delete[] InFileData;

    return Temp.c_str();
}

// just incase i need it
/* Gets the Client+Engine+CSGO Hash Sum */
std::string utils::GetGameSumHashMD5()
{
    std::string client = GetFileMD5("csgo\\bin\\client.dll");
    std::string engine = GetFileMD5("bin\\engine.dll");
    std::string csgo = GetFileMD5("csgo.exe");
    auto finalMD5 = md5(client + engine + csgo);

#ifdef _DEBUG
    printf("Client: %s\nEngine: %s\nCSGO: %s\nFinal: %s\n", client.c_str(), engine.c_str(), csgo.c_str(), finalMD5.c_str());
#endif // _DEBUG

    return finalMD5;
}

// source: https://stackoverflow.com/questions/7400418/writing-a-log-file-in-c-c
inline std::string getCurrentDateTime(std::string s) {
    time_t now = time(0);
    struct tm  tstruct;
    char  buf[80];
    tstruct = *localtime(&now);
    if (s == "now")
        strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct);
    else if (s == "date")
        strftime(buf, sizeof(buf), "%Y-%m-%d", &tstruct);
    return std::string(buf);
};
inline void Log(std::string logMsg) {

    std::string filePath = "%SYSTEMDRIVE%\\CSGOv2\\Logs\\log_" + getCurrentDateTime("date") + ".txt";
    std::string now = getCurrentDateTime("now");
    std::ofstream ofs(filePath.c_str(), std::ios_base::out | std::ios_base::app);
    ofs << "//////////////\n\n" << now << '\t' << logMsg << '\n';
    ofs.close();
}

/* Checks Game Version using HASH CHECK SUM checking. aka: cryptographic checksum */
bool utils::CheckVersion(const char* MD5Hash)
{
    // if Hash dosent check out, it prints to a file located at "C:\CSGOv2\\Logs\"
    if (strcmp(MD5Hash, GetGameSumHashMD5().c_str())) {
        Log(std::format("CheckVersion Failed!\nOld Hash [{}]\nNew Hash [{}]\n", MD5Hash, GetGameSumHashMD5().data()));
        /*
        * Example Preview:
        * //////////////
        *
        * 2022-08-31 13:05:03	CheckVersion Failed!
        * Old Hash [123]
        * New Hash [5184af3992d8916fa71e524f0bc32d8f]
        * 
        * //////////////
        * 
        * 2022-08-31 13:05:10	CheckVersion Failed!
        * Old Hash [123]
        * New Hash [5184af3992d8916fa71e524f0bc32d8f]
        * 
        * //////////////
        * 
        * 2022-08-31 13:05:15	CheckVersion Failed!
        * Old Hash [123]
        * New Hash [5184af3992d8916fa71e524f0bc32d8f]
        */
        return false;
    }
    else
        return true;
}

void utils::SetupConsole()
{
    AllocConsole();
    FILE* conStream;
    freopen_s(&conStream, "CONOUT$", "w", stdout);
    SetConsoleTitleA("DBG con");
    HWND conWindow = GetConsoleWindow();
    if (conStream && conWindow) {
        ::ShowWindow(conWindow, SW_SHOW);
        printf("Console Window Addr: 0x%x \n", conWindow != NULL ? conWindow : 0x0);
        std::cout << "Allocated a Console!\n";
    }
    system("echo %cd%");
}

float utils::SlideVal(float curVal, float Max, float fraction)
{
    float delta = Max - curVal;
    fraction = std::clamp(fraction, 0.0f, 1.0f);
    delta *= fraction;
    return curVal + delta;
}

bool utils::WolrdToScreen(math::Vector Pos, math::Vector& ScreenPos)
{
    const auto w = globals::game::viewMatrix._41 * Pos.x + globals::game::viewMatrix._42 * Pos.y + globals::game::viewMatrix._43 * Pos.z + globals::game::viewMatrix._44;
    if (w < 0.001f)
        return false;

    ScreenPos = math::Vector(ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y) / 2.0f;
    ScreenPos.x *= 1.0f + (globals::game::viewMatrix._11 * Pos.x + globals::game::viewMatrix._12 * Pos.y + globals::game::viewMatrix._13 * Pos.z + globals::game::viewMatrix._14) / w;
    ScreenPos.y *= 1.0f - (globals::game::viewMatrix._21 * Pos.x + globals::game::viewMatrix._22 * Pos.y + globals::game::viewMatrix._23 * Pos.z + globals::game::viewMatrix._24) / w;

    ScreenPos.floor();
    return true;
}

math::Vector utils::VectorTransform(const math::Vector& in, const math::Matrix3x4& matrix)
{
    math::Vector out;
    out.x = in.dotProduct(math::Vector(matrix.m[0][0], matrix.m[0][1], matrix.m[0][2])) + matrix.m[0][3];
    out.y = in.dotProduct(math::Vector(matrix.m[1][0], matrix.m[1][1], matrix.m[1][2])) + matrix.m[1][3];
    out.z = in.dotProduct(math::Vector(matrix.m[2][0], matrix.m[2][1], matrix.m[2][2])) + matrix.m[2][3];
    return out;
}

// Pasted
std::uint8_t* utils::PatternScan(void* module, const char* signature)
{
    static auto pattern_to_byte = [](const char* pattern) {
        auto bytes = std::vector<int>{};
        auto start = const_cast<char*>(pattern);
        auto end = const_cast<char*>(pattern) + strlen(pattern);

        for (auto current = start; current < end; ++current) {
            if (*current == '?') {
                ++current;
                if (*current == '?')
                    ++current;
                bytes.push_back(-1);
            }
            else {
                bytes.push_back(strtoul(current, &current, 16));
            }
        }
        return bytes;
    };

    auto dosHeader = (PIMAGE_DOS_HEADER)module;
    auto ntHeaders = (PIMAGE_NT_HEADERS)((std::uint8_t*)module + dosHeader->e_lfanew);

    auto sizeOfImage = ntHeaders->OptionalHeader.SizeOfImage;
    auto patternBytes = pattern_to_byte(signature);
    auto scanBytes = reinterpret_cast<std::uint8_t*>(module);

    auto s = patternBytes.size();
    auto d = patternBytes.data();

    for (auto i = 0ul; i < sizeOfImage - s; ++i) {
        bool found = true;
        for (auto j = 0ul; j < s; ++j) {
            if (scanBytes[i + j] != d[j] && d[j] != -1) {
                found = false;
                break;
            }
        }
        if (found) {
            return &scanBytes[i];
        }
    }

    return nullptr;
}
