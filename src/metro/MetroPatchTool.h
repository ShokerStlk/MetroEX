#pragma once
#include "mycommon.h"

class MetroPatchTool {
public:
    MetroPatchTool();
    ~MetroPatchTool();

    bool CreatePatchFromFolder(const fs::path& contentFolder, const fs::path& vfxPath);
};
