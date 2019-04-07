#pragma once

#ifndef CS2MStr
// Convert CharString to String^
#define CS2MStr(value) marshal_as<String^>(value)
#endif

void Log(System::String^ message, ... array<System::Object^>^ args);