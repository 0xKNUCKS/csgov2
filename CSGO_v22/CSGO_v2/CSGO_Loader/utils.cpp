#include "utils.h"

bool utils::FileExists(const char* file)
{
    return std::filesystem::exists(file);
}

bool utils::choice(const char* str, ...)
{
    char buf;
    std::cout << str;
    std::cin >> buf;
    return ::tolower(buf) == 'y';
}

void utils::ascii_art(std::string input)
{
    //loop will print first layer
    for (int i = 0; i < input.size(); i++)
    {
        if (input[i] == 'A' || input[i] == 'a')
            std::cout << "  ___   ";
        if (input[i] == 'B' || input[i] == 'b')
            std::cout << " _____  ";
        if (input[i] == 'C' || input[i] == 'c')
            std::cout << " _____  ";
        if (input[i] == 'D' || input[i] == 'd')
            std::cout << " _____  ";
        if (input[i] == 'E' || input[i] == 'e')
            std::cout << " _____  ";
        if (input[i] == 'F' || input[i] == 'f')
            std::cout << " _____  ";
        if (input[i] == 'G' || input[i] == 'g')
            std::cout << " _____  ";
        if (input[i] == 'H' || input[i] == 'h')
            std::cout << " _   _  ";
        if (input[i] == 'I' || input[i] == 'i')
            std::cout << " _____  ";
        if (input[i] == 'J' || input[i] == 'j')
            std::cout << "   ___  ";
        if (input[i] == 'K' || input[i] == 'k')
            std::cout << " _   __ ";
        if (input[i] == 'L' || input[i] == 'l')
            std::cout << " _      ";
        if (input[i] == 'M' || input[i] == 'm')
            std::cout << " __  __  ";
        if (input[i] == 'N' || input[i] == 'n')
            std::cout << " _   _  ";
        if (input[i] == 'O' || input[i] == 'o')
            std::cout << " _____  ";
        if (input[i] == 'P' || input[i] == 'p')
            std::cout << " _____  ";
        if (input[i] == 'Q' || input[i] == 'q')
            std::cout << " _____  ";
        if (input[i] == 'R' || input[i] == 'r')
            std::cout << " _____  ";
        if (input[i] == 'S' || input[i] == 's')
            std::cout << " _____  ";
        if (input[i] == 'T' || input[i] == 't')
            std::cout << " _____  ";
        if (input[i] == 'U' || input[i] == 'u')
            std::cout << " _   _  ";
        if (input[i] == 'V' || input[i] == 'v')
            std::cout << " _   _  ";
        if (input[i] == 'W' || input[i] == 'w')
            std::cout << " _    _  ";
        if (input[i] == 'X' || input[i] == 'x')
            std::cout << "__   __ ";
        if (input[i] == 'Y' || input[i] == 'y')
            std::cout << "__   __ ";
        if (input[i] == 'Z' || input[i] == 'z')
            std::cout << " ______ ";
        if (input[i] == ' ')
            std::cout << "  ";
        if (input[i] == '`')
            std::cout << " _  ";
        if (input[i] == '~')
            std::cout << "      ";
        if (input[i] == '1')
            std::cout << " __   ";
        if (input[i] == '2')
            std::cout << " _____  ";
        if (input[i] == '3')
            std::cout << " _____  ";
        if (input[i] == '4')
            std::cout << "   ___  ";
        if (input[i] == '5')
            std::cout << " _____  ";
        if (input[i] == '6')
            std::cout << "  ____  ";
        if (input[i] == '7')
            std::cout << " ______ ";
        if (input[i] == '.')
            std::cout << "    ";
        if (input[i] == '8')
            std::cout << " _____  ";
        if (input[i] == '9')
            std::cout << " _____  ";
        if (input[i] == '0')
            std::cout << " _____  ";
        if (input[i] == '!')
            std::cout << " _  ";
        if (input[i] == '@')
            std::cout << "   ____   ";
        if (input[i] == '#')
            std::cout << "   _  _    ";
        if (input[i] == '$')
            std::cout << "  _   ";
        if (input[i] == '%')
            std::cout << " _   __ ";
        if (input[i] == '^')
            std::cout << " /\\  ";
        if (input[i] == '&')
            std::cout << "         ";
        if (input[i] == '*')
            std::cout << "    _     ";
        if (input[i] == '(')
            std::cout << "  __ ";
        if (input[i] == ')')
            std::cout << "__   ";
        if (input[i] == '-')
            std::cout << "         ";
        if (input[i] == '_')
            std::cout << "         ";
        if (input[i] == '=')
            std::cout << "         ";
        if (input[i] == '+')
            std::cout << "        ";
        if (input[i] == '[')
            std::cout << " ___  ";
        if (input[i] == '{')
            std::cout << "   __ ";
        if (input[i] == ']')
            std::cout << " ___  ";
        if (input[i] == '}')
            std::cout << "__    ";
        if (input[i] == '|')
            std::cout << " _  ";
        if (input[i] == '\\')
            std::cout << "__      ";
        if (input[i] == ';')
            std::cout << " _  ";
        if (input[i] == ':')
            std::cout << "    ";
        if (input[i] == '\'')
            std::cout << " _  ";
        if (input[i] == '"')
            std::cout << " _ _  ";
        if (input[i] == '<')
            std::cout << "   __ ";
        if (input[i] == ',')
            std::cout << "    ";
        if (input[i] == '>')
            std::cout << "__    ";
        if (input[i] == '/')
            std::cout << "     __ ";
        if (input[i] == '?')
            std::cout << " ___   ";
    }
    std::cout << std::endl;
    //loop will print second layer
    for (int i = 0; i < input.size(); i++)
    {
        if (input[i] == 'A' || input[i] == 'a')
            std::cout << " / _ \\  ";
        if (input[i] == 'B' || input[i] == 'b')
            std::cout << "| ___ \\ ";
        if (input[i] == 'C' || input[i] == 'c')
            std::cout << "/  __ \\ ";
        if (input[i] == 'D' || input[i] == 'd')
            std::cout << "|  _  \\ ";
        if (input[i] == 'E' || input[i] == 'e')
            std::cout << "|  ___| ";
        if (input[i] == 'F' || input[i] == 'f')
            std::cout << "|  ___| ";
        if (input[i] == 'G' || input[i] == 'g')
            std::cout << "|  __ \\ ";
        if (input[i] == 'H' || input[i] == 'h')
            std::cout << "| | | | ";
        if (input[i] == 'I' || input[i] == 'i')
            std::cout << "|_   _| ";
        if (input[i] == 'J' || input[i] == 'j')
            std::cout << "  |_  | ";
        if (input[i] == 'K' || input[i] == 'k')
            std::cout << "| | / / ";
        if (input[i] == 'L' || input[i] == 'l')
            std::cout << "| |     ";
        if (input[i] == 'M' || input[i] == 'm')
            std::cout << "|  \\/  | ";
        if (input[i] == 'N' || input[i] == 'n')
            std::cout << "| \\ | | ";
        if (input[i] == 'O' || input[i] == 'o')
            std::cout << "|  _  | ";
        if (input[i] == 'P' || input[i] == 'p')
            std::cout << "| ___ \\ ";
        if (input[i] == 'Q' || input[i] == 'q')
            std::cout << "|  _  | ";
        if (input[i] == 'R' || input[i] == 'r')
            std::cout << "| ___ \\ ";
        if (input[i] == 'S' || input[i] == 's')
            std::cout << "/  ___| ";
        if (input[i] == 'T' || input[i] == 't')
            std::cout << "|_   _| ";
        if (input[i] == 'U' || input[i] == 'u')
            std::cout << "| | | | ";
        if (input[i] == 'V' || input[i] == 'v')
            std::cout << "| | | | ";
        if (input[i] == 'W' || input[i] == 'w')
            std::cout << "| |  | | ";
        if (input[i] == 'X' || input[i] == 'x')
            std::cout << "\\ \\ / / ";
        if (input[i] == 'Y' || input[i] == 'y')
            std::cout << "\\ \\ / / ";
        if (input[i] == 'Z' || input[i] == 'z')
            std::cout << "|___  / ";
        if (input[i] == ' ')
            std::cout << "  ";
        if (input[i] == '`')
            std::cout << "( ) ";
        if (input[i] == '~')
            std::cout << "      ";
        if (input[i] == '1')
            std::cout << "/  |  ";
        if (input[i] == '2')
            std::cout << "/ __  \\ ";
        if (input[i] == '3')
            std::cout << "|____ | ";
        if (input[i] == '4')
            std::cout << "  /   | ";
        if (input[i] == '5')
            std::cout << "|  ___| ";
        if (input[i] == '6')
            std::cout << " / ___| ";
        if (input[i] == '7')
            std::cout << "|___  / ";
        if (input[i] == '.')
            std::cout << "    ";
        if (input[i] == '8')
            std::cout << "|  _  | ";
        if (input[i] == '9')
            std::cout << "|  _  | ";
        if (input[i] == '0')
            std::cout << "|  _  | ";
        if (input[i] == '!')
            std::cout << "| | ";
        if (input[i] == '@')
            std::cout << "  / __ \\  ";
        if (input[i] == '#')
            std::cout << " _| || |_  ";
        if (input[i] == '$')
            std::cout << " | |  ";
        if (input[i] == '%')
            std::cout << "(_) / / ";
        if (input[i] == '^')
            std::cout << "|/\\| ";
        if (input[i] == '&')
            std::cout << "  ___    ";
        if (input[i] == '*')
            std::cout << " /\\| |/\\  ";
        if (input[i] == '(')
            std::cout << " / / ";
        if (input[i] == ')')
            std::cout << "\\ \\  ";
        if (input[i] == '-')
            std::cout << "         ";
        if (input[i] == '_')
            std::cout << "         ";
        if (input[i] == '=')
            std::cout << " ______  ";
        if (input[i] == '+')
            std::cout << "   _    ";
        if (input[i] == '[')
            std::cout << "|  _| ";
        if (input[i] == '{')
            std::cout << "  / / ";
        if (input[i] == ']')
            std::cout << "|_  | ";
        if (input[i] == '}')
            std::cout << "\\ \\   ";
        if (input[i] == '|')
            std::cout << "| | ";
        if (input[i] == '\\')
            std::cout << "\\ \\     ";
        if (input[i] == ';')
            std::cout << "(_) ";
        if (input[i] == ':')
            std::cout << " _  ";
        if (input[i] == '\'')
            std::cout << "( ) ";
        if (input[i] == '"')
            std::cout << "( | ) ";
        if (input[i] == '<')
            std::cout << "  / / ";
        if (input[i] == ',')
            std::cout << "    ";
        if (input[i] == '>')
            std::cout << "\\ \\   ";
        if (input[i] == '/')
            std::cout << "    / / ";
        if (input[i] == '?')
            std::cout << "|__ \\  ";
    }
    std::cout << std::endl;
    //loop will print third layer
    for (int i = 0; i < input.size(); i++)
    {
        if (input[i] == 'A' || input[i] == 'a')
            std::cout << "/ /_\\ \\ ";
        if (input[i] == 'B' || input[i] == 'b')
            std::cout << "| |_/ / ";
        if (input[i] == 'C' || input[i] == 'c')
            std::cout << "| /  \\/ ";
        if (input[i] == 'D' || input[i] == 'd')
            std::cout << "| | | | ";
        if (input[i] == 'E' || input[i] == 'e')
            std::cout << "| |__   ";
        if (input[i] == 'F' || input[i] == 'f')
            std::cout << "| |_    ";
        if (input[i] == 'G' || input[i] == 'g')
            std::cout << "| |  \\/ ";
        if (input[i] == 'H' || input[i] == 'h')
            std::cout << "| |_| | ";
        if (input[i] == 'I' || input[i] == 'i')
            std::cout << "  | |   ";
        if (input[i] == 'J' || input[i] == 'j')
            std::cout << "    | | ";
        if (input[i] == 'K' || input[i] == 'k')
            std::cout << "| |/ /  ";
        if (input[i] == 'L' || input[i] == 'l')
            std::cout << "| |     ";
        if (input[i] == 'M' || input[i] == 'm')
            std::cout << "| .  . | ";
        if (input[i] == 'N' || input[i] == 'n')
            std::cout << "|  \\| | ";
        if (input[i] == 'O' || input[i] == 'o')
            std::cout << "| | | | ";
        if (input[i] == 'P' || input[i] == 'p')
            std::cout << "| |_/ / ";
        if (input[i] == 'Q' || input[i] == 'q')
            std::cout << "| | | | ";
        if (input[i] == 'R' || input[i] == 'r')
            std::cout << "| |_/ / ";
        if (input[i] == 'S' || input[i] == 's')
            std::cout << "\\ `--.  ";
        if (input[i] == 'T' || input[i] == 't')
            std::cout << "  | |   ";
        if (input[i] == 'U' || input[i] == 'u')
            std::cout << "| | | | ";
        if (input[i] == 'V' || input[i] == 'v')
            std::cout << "| | | | ";
        if (input[i] == 'W' || input[i] == 'w')
            std::cout << "| |  | | ";
        if (input[i] == 'X' || input[i] == 'x')
            std::cout << " \\ V /  ";
        if (input[i] == 'Y' || input[i] == 'y')
            std::cout << " \\ V /  ";
        if (input[i] == 'Z' || input[i] == 'z')
            std::cout << "   / /  ";
        if (input[i] == ' ')
            std::cout << "  ";
        if (input[i] == '`')
            std::cout << " \\| ";
        if (input[i] == '~')
            std::cout << " /\\/| ";
        if (input[i] == '1')
            std::cout << "`| |  ";
        if (input[i] == '2')
            std::cout << "`' / /' ";
        if (input[i] == '3')
            std::cout << "    / / ";
        if (input[i] == '4')
            std::cout << " / /| | ";
        if (input[i] == '5')
            std::cout << "|___ \\  ";
        if (input[i] == '6')
            std::cout << "/ /___  ";
        if (input[i] == '7')
            std::cout << "   / /  ";
        if (input[i] == '.')
            std::cout << "    ";
        if (input[i] == '8')
            std::cout << " \\ V /  ";
        if (input[i] == '9')
            std::cout << "| |_| | ";
        if (input[i] == '0')
            std::cout << "| |/' | ";
        if (input[i] == '!')
            std::cout << "| | ";
        if (input[i] == '@')
            std::cout << " / / _` | ";
        if (input[i] == '#')
            std::cout << "|_  __  _| ";
        if (input[i] == '$')
            std::cout << "/ __) ";
        if (input[i] == '%')
            std::cout << "   / /  ";
        if (input[i] == '^')
            std::cout << "     ";
        if (input[i] == '&')
            std::cout << " ( _ )   ";
        if (input[i] == '*')
            std::cout << " \\ ` ' /  ";
        if (input[i] == '(')
            std::cout << "| |  ";
        if (input[i] == ')')
            std::cout << " | | ";
        if (input[i] == '-')
            std::cout << " ______  ";
        if (input[i] == '_')
            std::cout << "         ";
        if (input[i] == '=')
            std::cout << "|______| ";
        if (input[i] == '+')
            std::cout << " _| |_  ";
        if (input[i] == '[')
            std::cout << "| |   ";
        if (input[i] == '{')
            std::cout << " | |  ";
        if (input[i] == ']')
            std::cout << "  | | ";
        if (input[i] == '}')
            std::cout << " | |  ";
        if (input[i] == '|')
            std::cout << "| | ";
        if (input[i] == '\\')
            std::cout << " \\ \\    ";
        if (input[i] == ';')
            std::cout << "    ";
        if (input[i] == ':')
            std::cout << "(_) ";
        if (input[i] == '\'')
            std::cout << "|/  ";
        if (input[i] == '"')
            std::cout << " V V  ";
        if (input[i] == '<')
            std::cout << " / /  ";
        if (input[i] == ',')
            std::cout << "    ";
        if (input[i] == '>')
            std::cout << " \\ \\  ";
        if (input[i] == '/')
            std::cout << "   / /  ";
        if (input[i] == '?')
            std::cout << "   ) | ";
    }
    std::cout << std::endl;
    //loop will print fourth layer
    for (int i = 0; i < input.size(); i++)
    {
        if (input[i] == 'A' || input[i] == 'a')
            std::cout << "|  _  | ";
        if (input[i] == 'B' || input[i] == 'b')
            std::cout << "| ___ \\ ";
        if (input[i] == 'C' || input[i] == 'c')
            std::cout << "| |     ";
        if (input[i] == 'D' || input[i] == 'd')
            std::cout << "| | | | ";
        if (input[i] == 'E' || input[i] == 'e')
            std::cout << "|  __|  ";
        if (input[i] == 'F' || input[i] == 'f')
            std::cout << "|  _|   ";
        if (input[i] == 'G' || input[i] == 'g')
            std::cout << "| | __  ";
        if (input[i] == 'H' || input[i] == 'h')
            std::cout << "|  _  | ";
        if (input[i] == 'I' || input[i] == 'i')
            std::cout << "  | |   ";
        if (input[i] == 'J' || input[i] == 'j')
            std::cout << "    | | ";
        if (input[i] == 'K' || input[i] == 'k')
            std::cout << "|    \\  ";
        if (input[i] == 'L' || input[i] == 'l')
            std::cout << "| |     ";
        if (input[i] == 'M' || input[i] == 'm')
            std::cout << "| |\\/| | ";
        if (input[i] == 'N' || input[i] == 'n')
            std::cout << "| . ` | ";
        if (input[i] == 'O' || input[i] == 'o')
            std::cout << "| | | | ";
        if (input[i] == 'P' || input[i] == 'p')
            std::cout << "|  __/  ";
        if (input[i] == 'Q' || input[i] == 'q')
            std::cout << "| | | | ";
        if (input[i] == 'R' || input[i] == 'r')
            std::cout << "|    /  ";
        if (input[i] == 'S' || input[i] == 's')
            std::cout << " `--. \\ ";
        if (input[i] == 'T' || input[i] == 't')
            std::cout << "  | |   ";
        if (input[i] == 'U' || input[i] == 'u')
            std::cout << "| | | | ";
        if (input[i] == 'V' || input[i] == 'v')
            std::cout << "| | | | ";
        if (input[i] == 'W' || input[i] == 'w')
            std::cout << "| |/\\| | ";
        if (input[i] == 'X' || input[i] == 'x')
            std::cout << " / ^ \\  ";
        if (input[i] == 'Y' || input[i] == 'y')
            std::cout << "  \\ /   ";
        if (input[i] == 'Z' || input[i] == 'z')
            std::cout << "  / /   ";
        if (input[i] == ' ')
            std::cout << "  ";
        if (input[i] == '`')
            std::cout << "    ";
        if (input[i] == '~')
            std::cout << "|/\\/  ";
        if (input[i] == '1')
            std::cout << " | |  ";
        if (input[i] == '2')
            std::cout << "  / /   ";
        if (input[i] == '3')
            std::cout << "    \\ \\ ";
        if (input[i] == '4')
            std::cout << "/ /_| | ";
        if (input[i] == '5')
            std::cout << "    \\ \\ ";
        if (input[i] == '6')
            std::cout << "| ___ \\ ";
        if (input[i] == '7')
            std::cout << "  / /   ";
        if (input[i] == '.')
            std::cout << "    ";
        if (input[i] == '8')
            std::cout << " / _ \\  ";
        if (input[i] == '9')
            std::cout << "\\____ | ";
        if (input[i] == '0')
            std::cout << "|  /| | ";
        if (input[i] == '!')
            std::cout << "| | ";
        if (input[i] == '@')
            std::cout << "| | (_| | ";
        if (input[i] == '#')
            std::cout << " _| || |_  ";
        if (input[i] == '$')
            std::cout << "\\__ \\ ";
        if (input[i] == '%')
            std::cout << "  / /   ";
        if (input[i] == '^')
            std::cout << "     ";
        if (input[i] == '&')
            std::cout << " / _ \\/\\ ";
        if (input[i] == '*')
            std::cout << "|_     _| ";
        if (input[i] == '(')
            std::cout << "| |  ";
        if (input[i] == ')')
            std::cout << " | | ";
        if (input[i] == '-')
            std::cout << "|______| ";
        if (input[i] == '_')
            std::cout << "         ";
        if (input[i] == '=')
            std::cout << " ______  ";
        if (input[i] == '+')
            std::cout << "|_   _| ";
        if (input[i] == '[')
            std::cout << "| |   ";
        if (input[i] == '{')
            std::cout << "< <   ";
        if (input[i] == ']')
            std::cout << "  | | ";
        if (input[i] == '}')
            std::cout << "  > > ";
        if (input[i] == '|')
            std::cout << "| | ";
        if (input[i] == '\\')
            std::cout << "  \\ \\   ";
        if (input[i] == ';')
            std::cout << " _  ";
        if (input[i] == ':')
            std::cout << "    ";
        if (input[i] == '\'')
            std::cout << "    ";
        if (input[i] == '"')
            std::cout << "      ";
        if (input[i] == '<')
            std::cout << "< <   ";
        if (input[i] == ',')
            std::cout << " _  ";
        if (input[i] == '>')
            std::cout << "  > > ";
        if (input[i] == '/')
            std::cout << "  / /   ";
        if (input[i] == '?')
            std::cout << "  / /  ";
    }
    std::cout << std::endl;
    //loop will print fifth layer
    for (int i = 0; i < input.size(); i++)
    {
        if (input[i] == 'A' || input[i] == 'a')
            std::cout << "| | | | ";
        if (input[i] == 'B' || input[i] == 'b')
            std::cout << "| |_/ / ";
        if (input[i] == 'C' || input[i] == 'c')
            std::cout << "| \\__/\\ ";
        if (input[i] == 'D' || input[i] == 'd')
            std::cout << "| |/ /  ";
        if (input[i] == 'E' || input[i] == 'e')
            std::cout << "| |___  ";
        if (input[i] == 'F' || input[i] == 'f')
            std::cout << "| |     ";
        if (input[i] == 'G' || input[i] == 'g')
            std::cout << "| |_\\ \\ ";
        if (input[i] == 'H' || input[i] == 'h')
            std::cout << "| | | | ";
        if (input[i] == 'I' || input[i] == 'i')
            std::cout << " _| |_  ";
        if (input[i] == 'J' || input[i] == 'j')
            std::cout << "/\\__/ / ";
        if (input[i] == 'K' || input[i] == 'k')
            std::cout << "| |\\  \\ ";
        if (input[i] == 'L' || input[i] == 'l')
            std::cout << "| |____ ";
        if (input[i] == 'M' || input[i] == 'm')
            std::cout << "| |  | | ";
        if (input[i] == 'N' || input[i] == 'n')
            std::cout << "| |\\  | ";
        if (input[i] == 'O' || input[i] == 'o')
            std::cout << "\\ \\_/ / ";
        if (input[i] == 'P' || input[i] == 'p')
            std::cout << "| |     ";
        if (input[i] == 'Q' || input[i] == 'q')
            std::cout << "\\ \\/' / ";
        if (input[i] == 'R' || input[i] == 'r')
            std::cout << "| |\\ \\  ";
        if (input[i] == 'S' || input[i] == 's')
            std::cout << "/\\__/ / ";
        if (input[i] == 'T' || input[i] == 't')
            std::cout << "  | |   ";
        if (input[i] == 'U' || input[i] == 'u')
            std::cout << "| |_| | ";
        if (input[i] == 'V' || input[i] == 'v')
            std::cout << "\\ \\_/ / ";
        if (input[i] == 'W' || input[i] == 'w')
            std::cout << "\\  /\\  / ";
        if (input[i] == 'X' || input[i] == 'x')
            std::cout << "/ / \\ \\ ";
        if (input[i] == 'Y' || input[i] == 'y')
            std::cout << "  | |   ";
        if (input[i] == 'Z' || input[i] == 'z')
            std::cout << "./ /___ ";
        if (input[i] == ' ')
            std::cout << "  ";
        if (input[i] == '`')
            std::cout << "    ";
        if (input[i] == '~')
            std::cout << "      ";
        if (input[i] == '1')
            std::cout << "_| |_ ";
        if (input[i] == '2')
            std::cout << "./ /___ ";
        if (input[i] == '3')
            std::cout << ".___/ / ";
        if (input[i] == '4')
            std::cout << "\\___  | ";
        if (input[i] == '5')
            std::cout << "/\\__/ / ";
        if (input[i] == '6')
            std::cout << "| \\_/ | ";
        if (input[i] == '7')
            std::cout << "./ /    ";
        if (input[i] == '.')
            std::cout << " _  ";
        if (input[i] == '8')
            std::cout << "| |_| | ";
        if (input[i] == '9')
            std::cout << ".___/ / ";
        if (input[i] == '0')
            std::cout << "\\ |_/ / ";
        if (input[i] == '!')
            std::cout << "|_| ";
        if (input[i] == '@')
            std::cout << " \\ \\__,_| ";
        if (input[i] == '#')
            std::cout << "|_  __  _| ";
        if (input[i] == '$')
            std::cout << "(   / ";
        if (input[i] == '%')
            std::cout << " / / _  ";
        if (input[i] == '^')
            std::cout << "     ";
        if (input[i] == '&')
            std::cout << "| (_>  < ";
        if (input[i] == '*')
            std::cout << " / , . \\  ";
        if (input[i] == '(')
            std::cout << "| |  ";
        if (input[i] == ')')
            std::cout << " | | ";
        if (input[i] == '-')
            std::cout << "         ";
        if (input[i] == '_')
            std::cout << " ______  ";
        if (input[i] == '=')
            std::cout << "|______| ";
        if (input[i] == '+')
            std::cout << "  |_|   ";
        if (input[i] == '[')
            std::cout << "| |_  ";
        if (input[i] == '{')
            std::cout << " | |  ";
        if (input[i] == ']')
            std::cout << " _| | ";
        if (input[i] == '}')
            std::cout << " | |  ";
        if (input[i] == '|')
            std::cout << "| | ";
        if (input[i] == '\\')
            std::cout << "   \\ \\  ";
        if (input[i] == ';')
            std::cout << "( ) ";
        if (input[i] == ':')
            std::cout << " _  ";
        if (input[i] == '\'')
            std::cout << "    ";
        if (input[i] == '"')
            std::cout << "      ";
        if (input[i] == '<')
            std::cout << " \\ \\  ";
        if (input[i] == ',')
            std::cout << "( ) ";
        if (input[i] == '>')
            std::cout << " / /  ";
        if (input[i] == '/')
            std::cout << " / /    ";
        if (input[i] == '?')
            std::cout << " |_|   ";
    }
    std::cout << std::endl;
    //loop will print sixth layer
    for (int i = 0; i < input.size(); i++)
    {
        if (input[i] == 'A' || input[i] == 'a')
            std::cout << "\\_| |_/ ";
        if (input[i] == 'B' || input[i] == 'b')
            std::cout << "\\____/  ";
        if (input[i] == 'C' || input[i] == 'c')
            std::cout << " \\____/ ";
        if (input[i] == 'D' || input[i] == 'd')
            std::cout << "|___/   ";
        if (input[i] == 'E' || input[i] == 'e')
            std::cout << "\\____/  ";
        if (input[i] == 'F' || input[i] == 'f')
            std::cout << "\\_|     ";
        if (input[i] == 'G' || input[i] == 'g')
            std::cout << " \\____/ ";
        if (input[i] == 'H' || input[i] == 'h')
            std::cout << "\\_| |_/ ";
        if (input[i] == 'I' || input[i] == 'i')
            std::cout << " \\___/  ";
        if (input[i] == 'J' || input[i] == 'j')
            std::cout << "\\____/  ";
        if (input[i] == 'K' || input[i] == 'k')
            std::cout << "\\_| \\_/ ";
        if (input[i] == 'L' || input[i] == 'l')
            std::cout << "\\_____/ ";
        if (input[i] == 'M' || input[i] == 'm')
            std::cout << "\\_|  |_/ ";
        if (input[i] == 'N' || input[i] == 'n')
            std::cout << "\\_| \\_/ ";
        if (input[i] == 'O' || input[i] == 'o')
            std::cout << " \\___/  ";
        if (input[i] == 'P' || input[i] == 'p')
            std::cout << "\\_|     ";
        if (input[i] == 'Q' || input[i] == 'q')
            std::cout << " \\_/\\_\\ ";
        if (input[i] == 'R' || input[i] == 'r')
            std::cout << "\\_| \\_| ";
        if (input[i] == 'S' || input[i] == 's')
            std::cout << "\\____/  ";
        if (input[i] == 'T' || input[i] == 't')
            std::cout << "  \\_/   ";
        if (input[i] == 'U' || input[i] == 'u')
            std::cout << " \\___/  ";
        if (input[i] == 'V' || input[i] == 'v')
            std::cout << " \\___/  ";
        if (input[i] == 'W' || input[i] == 'w')
            std::cout << " \\/  \\/  ";
        if (input[i] == 'X' || input[i] == 'x')
            std::cout << "\\/   \\/ ";
        if (input[i] == 'Y' || input[i] == 'y')
            std::cout << "  \\_/   ";
        if (input[i] == 'Z' || input[i] == 'z')
            std::cout << "\\_____/ ";
        if (input[i] == ' ')
            std::cout << "  ";
        if (input[i] == '`')
            std::cout << "    ";
        if (input[i] == '~')
            std::cout << "      ";
        if (input[i] == '1')
            std::cout << "\\___/ ";
        if (input[i] == '2')
            std::cout << "\\_____/ ";
        if (input[i] == '3')
            std::cout << "\\____/  ";
        if (input[i] == '4')
            std::cout << "    |_/ ";
        if (input[i] == '5')
            std::cout << "\\____/  ";
        if (input[i] == '6')
            std::cout << "\\_____/ ";
        if (input[i] == '7')
            std::cout << "\\_/     ";
        if (input[i] == '.')
            std::cout << "(_) ";
        if (input[i] == '8')
            std::cout << "\\_____/ ";
        if (input[i] == '9')
            std::cout << "\\____/  ";
        if (input[i] == '0')
            std::cout << " \\___/  ";
        if (input[i] == '!')
            std::cout << "(_) ";
        if (input[i] == '@')
            std::cout << "  \\____/  ";
        if (input[i] == '#')
            std::cout << "  |_||_|   ";
        if (input[i] == '$')
            std::cout << " |_|  ";
        if (input[i] == '%')
            std::cout << "/_/ (_) ";
        if (input[i] == '^')
            std::cout << "     ";
        if (input[i] == '&')
            std::cout << " \\___/\\/ ";
        if (input[i] == '*')
            std::cout << " \\/|_|\\/  ";
        if (input[i] == '(')
            std::cout << " \\_\\ ";
        if (input[i] == ')')
            std::cout << "/_/  ";
        if (input[i] == '-')
            std::cout << "         ";
        if (input[i] == '_')
            std::cout << "|______| ";
        if (input[i] == '=')
            std::cout << "         ";
        if (input[i] == '+')
            std::cout << "        ";
        if (input[i] == '[')
            std::cout << "|___| ";
        if (input[i] == '{')
            std::cout << "  \\_\\ ";
        if (input[i] == ']')
            std::cout << "|___| ";
        if (input[i] == '}')
            std::cout << "/_/   ";
        if (input[i] == '|')
            std::cout << "|_| ";
        if (input[i] == '\\')
            std::cout << "    \\_\\ ";
        if (input[i] == ';')
            std::cout << "|/  ";
        if (input[i] == ':')
            std::cout << "(_) ";
        if (input[i] == '\'')
            std::cout << "    ";
        if (input[i] == '"')
            std::cout << "      ";
        if (input[i] == '<')
            std::cout << "  \\_\\ ";
        if (input[i] == ',')
            std::cout << "|/  ";
        if (input[i] == '>')
            std::cout << "/_/   ";
        if (input[i] == '/')
            std::cout << "/_/     ";
        if (input[i] == '?')
            std::cout << " (_)   ";
    }
    std::cout << std::endl;
}