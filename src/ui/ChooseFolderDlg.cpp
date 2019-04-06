#include "ChooseFolderDlg.h"

//#NOTE_SK: let's use fancy Vista-style folder picker dialog instead of .NET built-in crap
#include <shobjidl_core.h>

fs::path  ChooseFolderDialog::ChooseFolder(const CharString& title, void* parentHwnd) {
    fs::path result;

    IFileDialog* pfd = nullptr;
    HRESULT hr = ::CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));
    if (SUCCEEDED(hr)) {
        DWORD dwOptions = 0;
        hr = pfd->GetOptions(&dwOptions);
        if (SUCCEEDED(hr)) {
            pfd->SetOptions(dwOptions | FOS_PICKFOLDERS);
        }

        hr = pfd->Show(rcast<HWND>(parentHwnd));
        if (SUCCEEDED(hr)) {
            IShellItem* psi = nullptr;
            hr = pfd->GetResult(&psi);
            if (SUCCEEDED(hr)) {
                LPWSTR path = nullptr;
                hr = psi->GetDisplayName(SIGDN_FILESYSPATH, &path);
                if (SUCCEEDED(hr)) {
                    result = path;
                    ::CoTaskMemFree(path);
                }

                MySafeRelease(psi);
            }
        }

        MySafeRelease(pfd);
    }

    return result;
}
