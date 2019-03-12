#pragma once
#include "mycommon.h"

// Vista-style dialog
struct ChooseFolderDialog {
    static fs::path ChooseFolder(const CharString& title, void* parentHwnd = nullptr);
};
