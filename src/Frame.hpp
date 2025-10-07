#ifndef FRAME_HPP
#define FRAME_HPP

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <winuser.h>
#include <wx/wx.h>

#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>

#include "tools/helpers.hpp"
#include "tools/types.hpp"

class Frame : public wxFrame
{
public:
    Frame(std::string title, wxSize size) : wxFrame(nullptr, wxID_ANY, title, wxDefaultPosition, size) {}
};

#endif