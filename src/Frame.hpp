#ifndef FRAME_HPP
#define FRAME_HPP

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <winuser.h>
#include <wx/wx.h>

class App;

class Frame : public wxFrame
{
public:
    Frame(std::string title, wxSize size) : wxFrame(nullptr, wxID_ANY, title, wxDefaultPosition, size) {}
    std::function<void(RAWINPUT*)> onKeyboardInput;
    std::function<void(RAWINPUT*)> onMouseInput;

protected:
    WXLRESULT MSWWindowProc(WXUINT msg, WXWPARAM wParam, WXLPARAM lParam) override
    {
        if (msg == WM_INPUT)
        {
            UINT dwSize = 0;
            GetRawInputData((HRAWINPUT)lParam, RID_INPUT, nullptr, &dwSize, sizeof(RAWINPUTHEADER));
            std::vector<BYTE> lpb(dwSize);
            if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb.data(), &dwSize, sizeof(RAWINPUTHEADER)) == dwSize)
            {
                RAWINPUT* raw = (RAWINPUT*)lpb.data();
                if (raw->header.dwType == RIM_TYPEKEYBOARD)
                    onKeyboardInput(raw);
                else if (raw->header.dwType == RIM_TYPEMOUSE)
                    onMouseInput(raw);
            }
            return 0;
        }
        return wxFrame::MSWWindowProc(msg, wParam, lParam);
    }

};

#endif