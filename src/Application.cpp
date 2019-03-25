#include "ui/MainForm.h"
#include <VersionHelpers.h>

//#NOTE_SK: CLR issues - have to undef this bullshit so I can use the proper static function of Icon
//Thanks Microsoft !!!
#ifdef ExtractAssociatedIcon
#undef ExtractAssociatedIcon
#endif

using namespace System;
using namespace System::Drawing;
using namespace System::Windows::Forms;

ref class WheelFilter : public System::Windows::Forms::IMessageFilter
{
public:
    // Inherited via IMessageFilter
    virtual bool PreFilterMessage(System::Windows::Forms::Message % m)
    {
        if (m.Msg == WM_MOUSEWHEEL) {
            // WM_MOUSEWHEEL, find the control at screen position m.LParam
            POINT pos { m.LParam.ToInt32() & 0xffff, m.LParam.ToInt32() >> 16 };
            HWND hWnd = WindowFromPoint(pos);
            if (hWnd && hWnd != m.HWnd.ToPointer()) {
                SendMessage(hWnd, m.Msg, (WPARAM)m.WParam.ToPointer(), (LPARAM)m.LParam.ToPointer());
                return true;
            }
        }
        return false;
    }
};

[STAThreadAttribute]
void Main(array<String^>^ args) {
    Application::EnableVisualStyles();
    Application::SetCompatibleTextRenderingDefault(false);

    //#NOTE_SK: CLR winforms is no easy shit nowadays, and I don't like the idead of duplicating
    //          same icon in Forms's resx, so I just grab it from app and re-use ;)
    Icon^ appIcon = Icon::ExtractAssociatedIcon(System::Reflection::Assembly::GetExecutingAssembly()->Location);

    MetroEX::MainForm form;
    form.Icon = appIcon;

    if (IsWindows7OrGreater() && !IsWindows8OrGreater()) {
        Application::AddMessageFilter(gcnew WheelFilter());
    }

    Application::Run(%form);
}
