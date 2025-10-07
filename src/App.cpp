#include "App.hpp"

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    double time = NowMs();
    if (nCode == HC_ACTION && App::s_instance)
    {
        KBDLLHOOKSTRUCT* kb = (KBDLLHOOKSTRUCT*)lParam;

        // Determine key state
        bool keyDown = (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN);
        bool keyUp = (wParam == WM_KEYUP || wParam == WM_SYSKEYUP);
        bool extended = (kb->flags & LLKHF_EXTENDED);

        if (keyDown || keyUp)
        {
            RAWINPUT raw = {};

            raw.header.dwType = RIM_TYPEKEYBOARD;
            raw.header.dwSize = sizeof(RAWINPUT);
            raw.header.hDevice = nullptr;
            raw.header.wParam = 0;
            
            raw.data.keyboard.MakeCode = kb->scanCode;
            raw.data.keyboard.Flags = (keyUp ? RI_KEY_BREAK : 0) | (extended ? RI_KEY_E0 : 0);
            raw.data.keyboard.VKey = kb->vkCode;
            raw.data.keyboard.Reserved = 0;
            raw.data.keyboard.ExtraInformation = 0;

            Input input = { raw, time };

            {
                std::lock_guard<std::mutex> lock(App::s_instance->m_queueMutex);
                App::s_instance->m_inputQueue.push(input);
            }
            App::s_instance->m_queueCV.notify_one();
        }
    }

    return CallNextHookEx(nullptr, nCode, wParam, lParam);
}

LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    double time = NowMs();
    if (nCode == HC_ACTION && App::s_instance)
    {
        MSLLHOOKSTRUCT* ms = (MSLLHOOKSTRUCT*)(lParam);
        
        RAWINPUT raw = {};
        raw.header.dwType = RIM_TYPEMOUSE;

        switch (wParam)
        {
        case WM_LBUTTONDOWN: raw.data.mouse.usButtonFlags = RI_MOUSE_LEFT_BUTTON_DOWN; break;
        case WM_LBUTTONUP:   raw.data.mouse.usButtonFlags = RI_MOUSE_LEFT_BUTTON_UP; break;

        case WM_RBUTTONDOWN: raw.data.mouse.usButtonFlags = RI_MOUSE_RIGHT_BUTTON_DOWN; break;
        case WM_RBUTTONUP:   raw.data.mouse.usButtonFlags = RI_MOUSE_RIGHT_BUTTON_UP; break;

        case WM_MBUTTONDOWN: raw.data.mouse.usButtonFlags = RI_MOUSE_MIDDLE_BUTTON_DOWN; break;
        case WM_MBUTTONUP:   raw.data.mouse.usButtonFlags = RI_MOUSE_MIDDLE_BUTTON_UP; break;

        case WM_XBUTTONDOWN:
        {
            WORD button = HIWORD(ms->mouseData);
            if (button == XBUTTON1)
                raw.data.mouse.usButtonFlags = RI_MOUSE_BUTTON_4_DOWN;
            else if (button == XBUTTON2)
                raw.data.mouse.usButtonFlags = RI_MOUSE_BUTTON_5_DOWN;
            break;
        }

        case WM_XBUTTONUP:
        {
            WORD button = HIWORD(ms->mouseData);
            if (button == XBUTTON1)
                raw.data.mouse.usButtonFlags = RI_MOUSE_BUTTON_4_UP;
            else if (button == XBUTTON2)
                raw.data.mouse.usButtonFlags = RI_MOUSE_BUTTON_5_UP;
            break;
        }

		default: time = 0.0; break;
        }

        if (time > 0.0)
        {
            Input input = { raw, time };

            {
                std::lock_guard<std::mutex> lock(App::s_instance->m_queueMutex);
                App::s_instance->m_inputQueue.push(input);
            }
            App::s_instance->m_queueCV.notify_one();
        }

    }

    return CallNextHookEx(nullptr, nCode, wParam, lParam);
}



bool App::OnInit()
{
    #ifdef _DEBUG
    #include <io.h>
    #include <fcntl.h>
    #include <iostream>
    #include <cstdio>

    AllocConsole();
    FILE* fp;
    freopen_s(&fp, "CONOUT$", "w", stdout);
    freopen_s(&fp, "CONOUT$", "w", stderr);
    std::cout.clear();
    std::cerr.clear();

    #endif

    App::s_instance = this;

    m_frame = new Frame(m_title, wxSize(600, 400));
    m_frame->SetMinSize(wxSize(400, 200));

    std::ifstream layoutsfile("data/layouts.json");
    if (!layoutsfile.is_open()) {
        wxLogError("Error reading JSON layout file.");
        return 0;
    }
    m_layouts = JSON::parse(layoutsfile);

    m_frame->Bind(wxEVT_SIZE, [this](wxSizeEvent& event) {
        wxSize newSize = event.GetSize();
        m_frame->SetTitle(m_title + " (" + std::to_string(newSize.x) + ", " + std::to_string(newSize.y) + ")");
        event.Skip();
    });

    wxPanel* panel = new wxPanel(m_frame);

    wxChoice* choice = new wxChoice(panel, wxID_ANY);
    wxButton* button_Toggle_Recording = new wxButton(panel, wxID_ANY, m_isListening ? "Stop" : "Start");

    // Scrollable container
    m_scrollBox = new wxScrolledWindow(panel, wxID_ANY, wxDefaultPosition, wxSize(400, 200), wxVSCROLL);
    m_scrollBox->SetScrollRate(5, 5);

    // Sizer inside scroll box
    m_scrollBoxSizer = new wxBoxSizer(wxVERTICAL);
    m_scrollBox->SetSizer(m_scrollBoxSizer);

    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(button_Toggle_Recording, 0, wxALL | wxEXPAND, 10);
    sizer->Add(choice, 0, wxALL | wxEXPAND, 10);
    sizer->Add(m_scrollBox, 1, wxALL | wxEXPAND, 10);
    panel->SetSizer(sizer);

    m_scrollBox->FitInside();
    m_scrollBox->Layout();

    button_Toggle_Recording->Bind(wxEVT_BUTTON, [this, button_Toggle_Recording](wxCommandEvent& event) {
        this->m_isListening = !this->m_isListening;
        button_Toggle_Recording->SetLabel(m_isListening ? "Stop" : "Start");
    });

    choice->Bind(wxEVT_CHOICE, [this, choice](wxCommandEvent& event) {
        int selection = choice->GetSelection();
        wxString name = choice->GetString(selection);
        this->ChangeLayout(name);
    });

    for (auto& item : m_layouts.items()) {
        std::string layoutName = item.key();
        choice->Append(layoutName);
    }

    if (choice->GetCount() > 0)
    {
        choice->SetSelection(0);
        ChangeLayout(choice->GetString(0));
    }

    m_inputWorker = std::thread([this]() {
        while (m_isRunning) {
            Input input;
            {
                std::unique_lock<std::mutex> lock(m_queueMutex);
                m_queueCV.wait(lock, [this]() { return !m_inputQueue.empty() || !m_isRunning; });

                if (!m_isRunning && m_inputQueue.empty())
                    break;

                input = m_inputQueue.front();
                m_inputQueue.pop();
            }

            if (input.raw.header.dwType == RIM_TYPEKEYBOARD)
                this->OnKeyboardInput(&input);
            else if (input.raw.header.dwType == RIM_TYPEMOUSE)
                this->OnMouseInput(&input);
        }
    });

    m_keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, GetModuleHandle(nullptr), 0);
    if (!m_keyboardHook)
        wxLogError("Failed to install keyboard hook");

    m_mouseHook = SetWindowsHookEx(WH_MOUSE_LL, LowLevelMouseProc, GetModuleHandle(nullptr), 0);
    if (!m_mouseHook)
        wxLogError("Failed to install mouse hook");

    m_frame->Show(true);
    return true;
}

wxButton* App::AddButtonEvent(std::string name)
{
    int curPos = m_scrollBox->GetScrollPos(wxVERTICAL);
    int maxPos = m_scrollBox->GetScrollRange(wxVERTICAL);
    int thumbSize = m_scrollBox->GetScrollThumb(wxVERTICAL);
    bool atBottom = (curPos + thumbSize >= maxPos - 1);

    wxButton* btn = new wxButton(m_scrollBox, wxID_ANY, wxString::FromUTF8(name));
    m_scrollBoxSizer->Add(btn, 0, wxEXPAND | wxALL, 5);

    m_scrollBox->FitInside();
    m_scrollBox->Layout();

    if (atBottom)
    {
        int newMax = m_scrollBox->GetScrollRange(wxVERTICAL);
        if (newMax < 0) newMax = 0;
        m_scrollBox->Scroll(0, newMax);
    }

    return btn;
}

wxStaticText* App::AddTextEvent(std::string name)
{
    int curPos = m_scrollBox->GetScrollPos(wxVERTICAL);
    int maxPos = m_scrollBox->GetScrollRange(wxVERTICAL);
    int thumbSize = m_scrollBox->GetScrollThumb(wxVERTICAL);
    bool atBottom = (curPos + thumbSize >= maxPos - 1);

    wxStaticText* label = new wxStaticText(m_scrollBox, wxID_ANY, name);
    m_scrollBoxSizer->Add(label, 0, wxEXPAND | wxALL, 5);

    m_scrollBox->FitInside();
    m_scrollBox->Layout();

    if (atBottom)
    {
        int newMax = m_scrollBox->GetScrollRange(wxVERTICAL);
        if (newMax < 0) newMax = 0;
        m_scrollBox->Scroll(0, newMax);
    }

    return label;
}

void App::OnKeyboardInput(Input* input)
{
    if (!m_isListening) return;

    RAWINPUT* raw = &input->raw;
	double time = input->time;

    bool state = raw->data.keyboard.Flags % 2; //0 = down, 1 = up
    bool isExtended = false;
    if (raw->data.keyboard.Flags > 1) isExtended = true;

    //Convert key to unique ID
    uint32_t keyID = (isExtended ? 0xE000 : 0x0000) | raw->data.keyboard.MakeCode;
    //Ghost input
    if (keyID == 0xE02A) return;

    bool isPressed = m_pressedKeys.test(keyID);
    bool isKnown = m_currentLayout.test(keyID);
    std::string name = isKnown ? m_keyNames[keyID] : HexToString(keyID);

	double deltaTimePress = state ? 0.0 : time - m_lastEventTime;
	if (!state) m_lastEventTime = time;
    
    wxTheApp->CallAfter([this, state, isKnown, keyID, isPressed, name, time, deltaTimePress]() {
		if (state == 1)
        {
            if (isKnown)
            {
                double duration = 0.0;
                if (m_keyPressTime.find(keyID) != m_keyPressTime.end())
                {
                    duration = time - m_keyPressTime[keyID];
                    m_keyPressTime.erase(keyID);
                }
				const std::string durationString = ShortenDouble(duration, 2);
                if (m_keyButtons.find(keyID) != m_keyButtons.end())
                {
                    const wxString oldname = m_keyButtons[keyID]->GetLabel();
                    m_keyButtons[keyID]->SetLabel(wxString::FromUTF8(oldname + " | " + durationString + " | " + name + " (up)"));
                    m_keyButtons.erase(keyID);
                }
            }
            m_pressedKeys.reset(keyID);
            return;
        }

        if (isPressed) return;

        if (deltaTimePress > 0.0)
        {
            const std::string deltaTimeString = ShortenDouble(deltaTimePress, 2);
            AddTextEvent(deltaTimeString);
        }

        wxButton* btn = AddButtonEvent(name + " (down)");
        m_pressedKeys.set(keyID);
        m_keyButtons[keyID] = btn;
        m_keyPressTime[keyID] = time;
    });
}

void App::OnMouseInput(Input* input)
{
    if (!m_isListening) return;

	RAWINPUT* raw = &input->raw;
	double time = input->time;

    MouseInput mouseInput = {
        raw->data.mouse.usButtonFlags,
        false,
        nullptr
    };

    //flag == 0 means its probably a movement and not an action
	GetMouseInput(&mouseInput);

    double deltaTimePress = mouseInput.state ? 0.0 : time - m_lastEventTime;
    if (!mouseInput.state) m_lastEventTime = time;

    if (mouseInput.name != nullptr)
    {
        wxTheApp->CallAfter([this, mouseInput, time, deltaTimePress]() {
            if (mouseInput.state)
            {
                double duration = 0.0;
                if (m_mousePressTime.find(mouseInput.name) != m_mousePressTime.end())
                {
                    duration = time - m_mousePressTime[mouseInput.name];
                    m_mousePressTime.erase(mouseInput.name);
                }
                const std::string durationString = ShortenDouble(duration, 2);
                if (m_mouseButtons.find(mouseInput.name) != m_mouseButtons.end())
                {
                    const wxString oldname = m_mouseButtons[mouseInput.name]->GetLabel();
                    m_mouseButtons[mouseInput.name]->SetLabel(wxString::FromUTF8(oldname + " | " + durationString + " | " + mouseInput.name + " (up)"));
                    m_mouseButtons.erase(mouseInput.name);
                }

                return;
            }
  
            if (deltaTimePress > 0.0)
            {
                const std::string deltaTimeString = ShortenDouble(deltaTimePress, 2);
                AddTextEvent(deltaTimeString);
            }

            const std::string& name = mouseInput.name;
            wxButton* btn = AddButtonEvent(name + " (down)");
            m_mouseButtons[mouseInput.name] = btn;
            m_mousePressTime[mouseInput.name] = time;
        });
    }
}

void App::ChangeLayout(wxString layout)
{
    m_currentLayout.reset();
    m_currentLayoutName = layout;   
    for (auto& item : m_layouts[(std::string)layout].items()) {
        m_keyNames[StringToHex(item.key())] = item.value();
        m_currentLayout.set(StringToHex(item.key()));
    }
}

int App::OnExit() {
    m_isRunning = false;
    m_queueCV.notify_one();
    if (m_inputWorker.joinable())
        m_inputWorker.join();
    if (m_mouseHook)
        UnhookWindowsHookEx(m_mouseHook);
	if (m_keyboardHook)
		UnhookWindowsHookEx(m_keyboardHook);
    return 0;
}

App* App::s_instance = nullptr;