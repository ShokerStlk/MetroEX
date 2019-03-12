#include "ui/MainForm.h"

//#NOTE_SK: CLR issues - have to undef this bullshit so I can use the proper static function of Icon
//Thanks Microsoft !!!
#ifdef ExtractAssociatedIcon
#undef ExtractAssociatedIcon
#endif

using namespace System;
using namespace System::Drawing;
using namespace System::Windows::Forms;

[STAThreadAttribute]
void Main(array<String^>^ args) {
    Application::EnableVisualStyles();
    Application::SetCompatibleTextRenderingDefault(false);

    //#NOTE_SK: CLR winforms is no easy shit nowadays, and I don't like the idead of duplicating
    //          same icon in Forms's resx, so I just grab it from app and re-use ;)
    Icon^ appIcon = Icon::ExtractAssociatedIcon(System::Reflection::Assembly::GetExecutingAssembly()->Location);

    MetroEX::MainForm form;
    form.Icon = appIcon;

    Application::Run(%form);
}
