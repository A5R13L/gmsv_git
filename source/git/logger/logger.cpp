#include "logger.h"

std::string ReplaceColorCodes(const char *Format)
{
    static std::map<std::string, std::string> ColorCodes = {
        {"\x1B[94m", "{blue}"}, {"\x1B[97m", "{white}"},  {"\x1B[96m", "{cyan}"}, {"\x1B[92m", "{green}"},
        {"\x1B[91m", "{red}"},  {"\x1B[93m", "{yellow}"}, {"\x1B[90m", "{gray}"}};

    std::string Formatted = Format;

    for (const auto &[ColorCode, Color] : ColorCodes)
    {
        std::string::size_type Position = 0;

        while ((Position = Formatted.find(Color, Position)) != std::string::npos)
        {
            Formatted.replace(Position, Color.size(), ColorCode);
            Position += Color.size();
        }
    }

    return Formatted;
}

namespace Git::Logger
{
void Log(const char *Format, ...)
{
    char Buffer[1024];
    va_list Arguments;

    va_start(Arguments, Format);
    vsnprintf(Buffer, sizeof(Buffer), Format, Arguments);
    
    std::string Prefix = ReplaceColorCodes("{blue}[Git]: {white}");
    std::string Formatted = ReplaceColorCodes(Buffer);
    std::string Suffix = ReplaceColorCodes("{white}\n");

    std::cout << Prefix.c_str() << Formatted.c_str() << Suffix.c_str();
    va_end(Arguments);
}

void Log(const std::string Format, ...)
{
    char Buffer[1024];
    va_list Arguments;

    va_start(Arguments, Format);
    vsnprintf(Buffer, sizeof(Buffer), Format.c_str(), Arguments);
    
    std::string Prefix = ReplaceColorCodes("{blue}[Git]: {white}");
    std::string Formatted = ReplaceColorCodes(Buffer);
    std::string Suffix = ReplaceColorCodes("{white}\n");

    std::cout << Prefix.c_str() << Formatted.c_str() << Suffix.c_str();
    va_end(Arguments);
}

std::string Info(const char *Format)
{
    return std::string("{blue}[Info]{white} ") + Format;
}

std::string Success(const char *Format)
{
    return std::string("{green}[Success]{white} ") + Format;
}

std::string Error(const char *Format)
{
    return std::string("{red}[Error]{white} ") + Format;
}
} // namespace Git::Logger