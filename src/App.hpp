#ifndef APP_HPP
#define APP_HPP

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <winuser.h>
#include <wx/wx.h>

#include <iostream>
#include <string>
#include <fstream>
#include <bitset>
#include <unordered_map>

#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>

#include "Frame.hpp"
#include "tools/json.hpp"
#include "tools/helpers.hpp"
#include "tools/types.hpp"

using JSON = nlohmann::json;

class App : public wxApp
{
public:
    static App* s_instance;
    std::queue<Input> m_inputQueue;
    std::mutex m_queueMutex;
    std::condition_variable m_queueCV;
    HHOOK m_keyboardHook = nullptr;
    HHOOK m_mouseHook = nullptr;
    bool OnInit() override;
    int OnExit() override;

private:
    std::thread m_inputWorker;
    bool m_isRunning = true;

private:
    void OnKeyboardInput(Input* input);
    void OnMouseInput(Input* input);
    void ChangeLayout(wxString layout);
    wxButton* AddButtonEvent(std::string name);
    wxStaticText* AddTextEvent(std::string name);

    std::string m_title = "Keyboard Logger";
    Frame* m_frame;
    JSON m_layouts;
    std::string m_currentLayoutName;
    std::array<std::string, 61440> m_keyNames;
    std::bitset<61440> m_currentLayout;
    std::bitset<61440> m_pressedKeys;
    std::unordered_map<uint32_t, double> m_keyPressTime;
    std::unordered_map<char*, double> m_mousePressTime;

	double m_lastEventTime = 0.0;

    std::unordered_map<uint32_t, wxStaticText*> m_keyButtons;
    std::unordered_map<char*, wxStaticText*> m_mouseButtons;

    wxScrolledWindow* m_scrollBox;
    wxBoxSizer* m_scrollBoxSizer;

	bool m_isListening = true;
};

#endif