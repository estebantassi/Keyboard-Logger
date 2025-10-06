#ifndef HELPERS_HPP
#define HELPERS_HPP

#include <string>
#include <sstream>
#include <cstdint>
#include <iomanip>
#include <windows.h>

#include "./types.hpp"


static double NowMs() {
    static LARGE_INTEGER freq = [] {
        LARGE_INTEGER f;
        QueryPerformanceFrequency(&f);
        return f;
        }();
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    // Convert to milliseconds
    return (now.QuadPart * 1000.0) / freq.QuadPart;
}

static std::string ShortenDouble(double time, int size) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(size) << time;
    return oss.str();
}

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

inline void GetMouseInput(MouseInput* mouseInput)
{
    switch (mouseInput->flag)
    {
    //LMB
    case RI_MOUSE_LEFT_BUTTON_DOWN: {
		mouseInput->name = "LButton";
        mouseInput->state = 0;
        return;
    }
    case RI_MOUSE_LEFT_BUTTON_UP: {
        mouseInput->name = "LButton";
        mouseInput->state = 1;
        return;
    }

    //RMB
    case RI_MOUSE_RIGHT_BUTTON_DOWN: {
        mouseInput->name = "RButton";
        mouseInput->state = 0;
        return;
    }
    case RI_MOUSE_RIGHT_BUTTON_UP: {
        mouseInput->name = "RButton";
        mouseInput->state = 1;
        return;
    }

	//MMB
    case RI_MOUSE_MIDDLE_BUTTON_DOWN: {
        mouseInput->name = "MButton";
        mouseInput->state = 0;
        return;
    }
    case RI_MOUSE_MIDDLE_BUTTON_UP: {
        mouseInput->name = "MButton";
        mouseInput->state = 1;
        return;
    }

    //X1
    case RI_MOUSE_BUTTON_4_DOWN: {
        mouseInput->name = "XButton1";
        mouseInput->state = 0;
        return;
    }
    case RI_MOUSE_BUTTON_4_UP: {
        mouseInput->name = "XButton1";
        mouseInput->state = 1;
        return;
    }

    //X2
    case RI_MOUSE_BUTTON_5_DOWN: {
        mouseInput->name = "XButton2";
        mouseInput->state = 0;
        return;
    }
    case RI_MOUSE_BUTTON_5_UP: {
        mouseInput->name = "XButton2";
        mouseInput->state = 1;
        return;
    }

    //case RI_MOUSE_WHEEL:              return "Wheel";
    //case RI_MOUSE_HWHEEL:             return "HWheel";

    default:    return;
    }
}

#endif