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

#include "Frame.hpp"
#include "tools/json.hpp"
#include "tools/helpers.hpp"

using JSON = nlohmann::json;

class App : public wxApp
{
public:
    bool OnInit() override;

private:
    void OnKeyboardInput(RAWINPUT* raw);
    void OnMouseInput(RAWINPUT* raw);
    void ChangeLayout(wxString layout);
    void AddEvent(std::string name);

    std::string m_title = "Keyboard Logger";
    Frame* m_frame;
    JSON m_layouts;
    std::string m_currentLayoutName;
    std::array<std::string, 61440> m_keyNames;
    std::bitset<61440> m_currentLayout;
    std::bitset<61440> m_pressedKeys;

    wxScrolledWindow* m_scrollBox;
    wxBoxSizer* m_scrollBoxSizer;

    wxListBox* m_listBox;

    //TEMP
    wxStaticText* m_label;
};

#endif