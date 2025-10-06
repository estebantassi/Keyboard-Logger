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
    inline Frame(std::string title, wxSize size) : wxFrame(nullptr, wxID_ANY, title, wxDefaultPosition, size) {}

    std::queue<Input>* m_inputQueue;
    std::condition_variable* m_queueCV;
    std::mutex* m_queueMutex;

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
				RAWINPUT rawCopy = *raw;
                double time = NowMs();
				Input input = { rawCopy, time };
                {
                    std::lock_guard<std::mutex> lock(*m_queueMutex);
                    m_inputQueue->push(input);
                }
                m_queueCV->notify_one();
            }
            return 0;
        }
        return wxFrame::MSWWindowProc(msg, wParam, lParam);
    }

};

#endif