#include "log.h"

// Print to VS Output window
void Log(System::String^ message, ... array<System::Object^>^ args) {
#ifdef _DEBUG
    using namespace System::Diagnostics;
    Debug::WriteLine(message, args); // slow perfomance
#endif
}
