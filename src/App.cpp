#include "App.hpp"

bool App::OnInit()
{
    #ifdef _DEBUG
    #include <io.h>
    #include <fcntl.h>
    #include <iostream>
    #include <cstdio>

    AllocConsole();               // create a new console window
    FILE* fp;
    freopen_s(&fp, "CONOUT$", "w", stdout);   // redirect stdout
    freopen_s(&fp, "CONOUT$", "w", stderr);   // redirect stderr
    std::cout.clear();
    std::cerr.clear();

    #endif

    m_frame = new Frame(m_title, wxSize(600, 400));
    m_frame->SetMinSize(wxSize(400, 200));

    std::ifstream layoutsfile("data/layouts.json");
    if (!layoutsfile.is_open()) {
        wxLogError("Error reading JSON layout file.");
        return 0;
    }
    m_layouts = JSON::parse(layoutsfile);
    
    m_frame->onKeyboardInput = [this](RAWINPUT* raw) { this->OnKeyboardInput(raw); };
    m_frame->onMouseInput = [this](RAWINPUT* raw) { this->OnMouseInput(raw); };

    m_frame->Bind(wxEVT_SIZE, [this](wxSizeEvent& event) {
        wxSize newSize = event.GetSize();
        m_frame->SetTitle(m_title + " (" + std::to_string(newSize.x) + ", " + std::to_string(newSize.y) + ")");
        event.Skip();
    });

    RAWINPUTDEVICE rid[2];
    //KEYBOARD
    rid[0].usUsagePage = 0x01; rid[0].usUsage = 0x06; rid[0].dwFlags = RIDEV_INPUTSINK; rid[0].hwndTarget = (HWND)m_frame->GetHandle();
    //MOUSE
    rid[1].usUsagePage = 0x01; rid[1].usUsage = 0x02; rid[1].dwFlags = RIDEV_INPUTSINK; rid[1].hwndTarget = (HWND)m_frame->GetHandle();

    if (!RegisterRawInputDevices(rid, 2, sizeof(RAWINPUTDEVICE)))
        wxLogError("Failed to register raw input devices.");

    wxPanel* panel = new wxPanel(m_frame);

    m_label = new wxStaticText(panel, wxID_ANY, "Hello World");
    wxChoice* choice = new wxChoice(panel, wxID_ANY);
    //m_listBox = new wxListBox(panel, wxID_ANY);

    // Scrollable container
    m_scrollBox = new wxScrolledWindow(panel, wxID_ANY, wxDefaultPosition, wxSize(400, 200), wxVSCROLL);
    m_scrollBox->SetScrollRate(5, 5);

    // Sizer inside scroll box
    m_scrollBoxSizer = new wxBoxSizer(wxVERTICAL);
    m_scrollBox->SetSizer(m_scrollBoxSizer);

    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(m_label, 0, wxALL, 10);
    sizer->Add(choice, 0, wxALL | wxEXPAND, 10);
    sizer->Add(m_scrollBox, 1, wxALL | wxEXPAND, 10);
    panel->SetSizer(sizer);

    m_scrollBox->FitInside();
    m_scrollBox->Layout();

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

    m_frame->Show(true);
    return true;
}

void App::AddEvent(std::string name)
{
    int curPos = m_scrollBox->GetScrollPos(wxVERTICAL);
    int maxPos = m_scrollBox->GetScrollRange(wxVERTICAL);
    int thumbSize = m_scrollBox->GetScrollThumb(wxVERTICAL);
    bool atBottom = (curPos + thumbSize >= maxPos - 1);

    wxButton* btn = new wxButton(m_scrollBox, wxID_ANY, name);
    m_scrollBoxSizer->Add(btn, 0, wxEXPAND | wxALL, 5);

    m_scrollBox->FitInside();
    m_scrollBox->Layout();

    if (atBottom)
    {
        int newMax = m_scrollBox->GetScrollRange(wxVERTICAL);
        if (newMax < 0) newMax = 0;
        m_scrollBox->Scroll(0, newMax);
    }
}

void App::OnKeyboardInput(RAWINPUT* raw)
{
    bool state = raw->data.keyboard.Flags % 2; //0 = down, 1 = up
    bool isExtended = false;
    if (raw->data.keyboard.Flags > 1) isExtended = true;

    //Convert key to unique ID
    uint32_t keyID = (isExtended ? 0xE000 : 0x0000) | raw->data.keyboard.MakeCode;
    //Ghost input
    if (keyID == 0xE02A) return;

    if (state == 1)
    {
        m_pressedKeys.reset(keyID);
        return;
    }

    if (m_pressedKeys.test(keyID)) return;

    if (m_currentLayout.test(keyID))
    {
        const std::string& name = m_keyNames[keyID];
        AddEvent(name);
    }
    else
    {
        std::cout << "Unknown key: " << HexToString(keyID) << std::endl;
        AddEvent(HexToString(keyID));
    }

    m_pressedKeys.set(keyID);
    
}

void App::OnMouseInput(RAWINPUT* raw)
{
    USHORT flag = raw->data.mouse.usButtonFlags;

    //flag == 0 means its probably a movement and not an action
    if (flag != 0)
    {
        std::cout << MouseButtonName(flag) << std::endl;
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