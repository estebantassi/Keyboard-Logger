#include <string>
#include <sstream>
#include <cstdint>
#include <iomanip>
#include <windows.h>

std::string static HexToString(uint32_t hex)
{
    std::stringstream ss;
    ss << "0x"
        << std::setw(4)      // width = 4 digits
        << std::setfill('0') // pad with '0'
        << std::hex
        << std::uppercase
        << hex;
    return ss.str();
}

uint32_t static StringToHex(std::string str)
{
    return static_cast<uint32_t>(std::stoul(str, nullptr, 16));
}

inline const char* MouseButtonName(USHORT flag)
{
    switch (flag)
    {
    case RI_MOUSE_LEFT_BUTTON_DOWN:   return "LeftDown";
    case RI_MOUSE_LEFT_BUTTON_UP:     return "LeftUp";
    case RI_MOUSE_RIGHT_BUTTON_DOWN:  return "RightDown";
    case RI_MOUSE_RIGHT_BUTTON_UP:    return "RightUp";
    case RI_MOUSE_MIDDLE_BUTTON_DOWN: return "MiddleDown";
    case RI_MOUSE_MIDDLE_BUTTON_UP:   return "MiddleUp";
    case RI_MOUSE_BUTTON_4_DOWN:      return "XButton1Down";
    case RI_MOUSE_BUTTON_4_UP:        return "XButton1Up";
    case RI_MOUSE_BUTTON_5_DOWN:      return "XButton2Down";
    case RI_MOUSE_BUTTON_5_UP:        return "XButton2Up";
    case RI_MOUSE_WHEEL:              return "Wheel";
    case RI_MOUSE_HWHEEL:             return "HWheel";
    default:                          break;
    }
}