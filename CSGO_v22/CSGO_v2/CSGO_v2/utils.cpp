#include "utils.h"
#include <chrono>

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
    std::string csgo =   GetFileMD5("csgo.exe");
    auto finalMD5 = md5(client + engine + csgo);

#ifdef _DEBUG
    printf("Client: %s\nEngine: %s\nCSGO: %s\nFinal: %s\n", client.c_str(), engine.c_str(), csgo.c_str(), finalMD5.c_str());
#endif // _DEBUG

    return finalMD5;
}

/* Checks Game Version using HASH CHECK SUM checking */
bool utils::CheckVersion(const char* MD5Hash)
{
    return !strcmp(MD5Hash, GetGameSumHashMD5().c_str());
}

void utils::SetupConsole()
{
    AllocConsole();
    FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);
    SetConsoleTitleA("DBG con");
    if (f)
        std::cout << "Allocated a Console!\n";
    system("echo %cd%");
}