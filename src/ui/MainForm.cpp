#include "metro/VFXReader.h"
#include "metro/MetroDatabases.h"
#include "metro/MetroTexture.h"
#include "metro/MetroModel.h"
#include "metro/MetroSound.h"
#include "metro/MetroSkeleton.h"
#include "metro/MetroMotion.h"

#include <fstream>

#include "ChooseFolderDlg.h"

#include "MainForm.h"
#include "ExtractionOptionsDgl.h"
#include "AboutDlg.h"

// String to std::string wrapper
#include <msclr/marshal_cppstd.h>
using namespace msclr::interop;

enum class eNodeEventType : size_t {
    Default,
    Open,
    Close
};

static const size_t kEmptyIdx               = ~0;
static const size_t kEmptyCustomValue       = ~0;

static const int    kImageIdxFolderClosed   = 0;
static const int    kImageIdxFolderOpen     = 1;
static const int    kImageIdxFile           = 2;
static const int    kImageIdxBinUnkn        = 3;
static const int    kImageIdxBinArchive     = 4;
static const int    kImageIdxBinEditable    = 5;
static const int    kImageIdxTexture        = 6;
static const int    kImageIdxMotion         = 7;
static const int    kImageIdxSound          = 8;
static const int    kImageIdxModel          = 9;

static const size_t kFileIdxMask            = size_t(~0) >> 1;
static const size_t kFolderSortedFlag       = size_t(1) << ((sizeof(size_t) * 8) - 1);

namespace MetroEX {
    ref struct FileTagData {
        FileType fileType; // type of file
        size_t   fileIdx; // index inside .vfx
        size_t   subFileIdx; // index inside .bin database

        FileTagData(const FileType _fileType, const size_t _fileIdx, const size_t _subFileIdx) {
            fileType = _fileType;
            fileIdx = _fileIdx;
            subFileIdx = _subFileIdx;
        }
    };

    String^ PathToString(const fs::path& p) {
        return marshal_as<String^>(p.wstring());
    }

    fs::path StringToPath(String^ s) {
        return marshal_as<std::wstring>(s);
    }

    ref class NodeSorter : public System::Collections::IComparer {
    public:
        virtual int Compare(Object^ x, Object^ y) {
            System::Windows::Forms::TreeNode^ left = safe_cast<System::Windows::Forms::TreeNode^>(x);
            System::Windows::Forms::TreeNode^ right = safe_cast<System::Windows::Forms::TreeNode^>(y);

            if (left->Nodes->Count) {
                return (right->Nodes->Count > 0) ? left->Text->CompareTo(right->Text) : -1;
            } else {
                return (right->Nodes->Count > 0) ? 1 : left->Text->CompareTo(right->Text);
            }
        }
    };

    static FileType DetectFileType(const MetroFile& mf) {
        FileType result = FileType::Unknown;

        String^ name = marshal_as<String^>(mf.name.c_str());

        if (name->EndsWith(L".dds") ||
            name->EndsWith(L".512") ||
            name->EndsWith(L".1024") ||
            name->EndsWith(L".2048")) {
            result = FileType::Texture;
        } else if (name->EndsWith(L".bin")) {
            result = FileType::Bin;
        } else if (name->EndsWith(L".model")) {
            result = FileType::Model;
        } else if (name->EndsWith(L".m2")) {
            result = FileType::Motion;
        } else if (name->EndsWith(L".vba")) {
            result = FileType::Sound;
        } else if (name->EndsWith(L"lightmaps")) {
            result = FileType::Level;
        }

        return result;
    }

    static void UpdateNodeIcon(TreeNode^ Node, eNodeEventType eventType = eNodeEventType::Default) {
        FileTagData^ fileData = safe_cast<FileTagData^>(Node->Tag);
        FileType fileType = fileData->fileType;

        Node->ImageIndex = kImageIdxFile;
        Node->SelectedImageIndex = kImageIdxFile;

        switch (fileType)
        {
            case FileType::Unknown: {
            } break;

            case FileType::Folder:
            case FileType::FolderBin: {
                if (eventType == eNodeEventType::Open) {
                    Node->ImageIndex = kImageIdxFolderOpen;
                    Node->SelectedImageIndex = kImageIdxFolderOpen;
                } else {
                    Node->ImageIndex = kImageIdxFolderClosed;
                    Node->SelectedImageIndex = kImageIdxFolderClosed;
                }
            } break;

            case FileType::Bin: {
                Node->ImageIndex = kImageIdxBinUnkn;
                Node->SelectedImageIndex = kImageIdxBinUnkn;
            } break;

            case FileType::BinArchive: {
                Node->ImageIndex = kImageIdxBinArchive;
                Node->SelectedImageIndex = kImageIdxBinArchive;
            } break;

            case FileType::BinEditable: {
                Node->ImageIndex = kImageIdxBinEditable;
                Node->SelectedImageIndex = kImageIdxBinEditable;
            } break;

            case FileType::Model: {
                Node->ImageIndex = kImageIdxModel;
                Node->SelectedImageIndex = kImageIdxModel;
            } break;

            case FileType::Texture: {
                Node->ImageIndex = kImageIdxTexture;
                Node->SelectedImageIndex = kImageIdxTexture;
            } break;

            case FileType::Sound: {
                Node->ImageIndex = kImageIdxSound;
                Node->SelectedImageIndex = kImageIdxSound;
            } break;
        }
    }

    void MainForm::MainForm_Load(System::Object^, System::EventArgs^) {
//#ifdef _DEBUG
//        //#NOTE_SK: for debugging purposes we might want to extract raw files
//        this->ctxMenuExportModel->Items->Add(this->extractFileToolStripMenuItem);
//        this->ctxMenuExportModel->Size.Height += this->extractFileToolStripMenuItem->Size.Height;
//        this->ctxMenuExportTexture->Items->Add(this->extractFileToolStripMenuItem);
//        this->ctxMenuExportTexture->Size.Height += this->extractFileToolStripMenuItem->Size.Height;
//        this->ctxMenuExportSound->Items->Add(this->extractFileToolStripMenuItem);
//        this->ctxMenuExportSound->Size.Height += this->extractFileToolStripMenuItem->Size.Height;
//#endif

        mImagePanel = gcnew ImagePanel();
        this->pnlViewers->Controls->Add(mImagePanel);
        mImagePanel->Dock = System::Windows::Forms::DockStyle::Fill;
        mImagePanel->Location = System::Drawing::Point(0, 0);
        mImagePanel->Name = L"mImagePanel";
        mImagePanel->Size = System::Drawing::Size(528, 386);
        mImagePanel->AutoScroll = true;


        mSoundPanel = gcnew SoundPanel();
        this->pnlViewers->Controls->Add(mSoundPanel);
        mSoundPanel->Dock = System::Windows::Forms::DockStyle::Fill;
        mSoundPanel->Location = System::Drawing::Point(0, 0);
        mSoundPanel->Name = L"mSoundPanel";
        mSoundPanel->Size = System::Drawing::Size(528, 386);


        mRenderPanel = gcnew RenderPanel();
        this->pnlViewers->Controls->Add(mRenderPanel);
        mRenderPanel->Dock = System::Windows::Forms::DockStyle::Fill;
        mRenderPanel->Location = System::Drawing::Point(0, 0);
        mRenderPanel->Name = L"mRenderPanel";
        mRenderPanel->Size = System::Drawing::Size(528, 386);

        if (!mRenderPanel->InitGraphics()) {
            this->ShowErrorMessage("Failed to initialize DirectX 11 graphics!\n3D viewer will be unavailable.");
        }

        this->SwitchViewPanel(PanelType::Texture);
        this->SwitchInfoPanel(PanelType::Sound);
    }

    // toolstrip buttons
    void MainForm::toolBtnFileOpen_Click(System::Object^, System::EventArgs^) {
        OpenFileDialog ofd;
        ofd.Title = L"Open Metro Exodus vfx file...";
        ofd.Filter = L"VFX files (*.vfx)|*.vfx";
        ofd.FilterIndex = 0;
        ofd.RestoreDirectory = true;
        if (ofd.ShowDialog(this) == System::Windows::Forms::DialogResult::OK) {
            if (mVFXReader) {
                delete mVFXReader;
            }

            System::Windows::Forms::Cursor::Current = System::Windows::Forms::Cursors::WaitCursor;

            mVFXReader = new VFXReader();
            if (mVFXReader->LoadFromFile(StringToPath(ofd.FileName))) {
                MetroTexturesDatabase* texDb;
                MetroConfigsDatabase* cfgDb;
                LoadDatabasesFromFile(mVFXReader, texDb, cfgDb);
                mTexturesDatabase = texDb;
                mConfigsDatabase = cfgDb;

                this->UpdateFilesList();
            }

            System::Windows::Forms::Cursor::Current = System::Windows::Forms::Cursors::Arrow;
        }
    }

    void MainForm::toolBtnAbout_Click(System::Object^, System::EventArgs^) {
        AboutDlg dlg;
        dlg.Icon = this->Icon;
        dlg.Text = this->Text;
        dlg.ShowDialog(this);
    }

    void MainForm::toolBtnImgEnableAlpha_Click(System::Object^, System::EventArgs^) {
        if (mImagePanel) {
            toolBtnImgEnableAlpha->Checked = !toolBtnImgEnableAlpha->Checked;
            mImagePanel->EnableTransparency(toolBtnImgEnableAlpha->Checked);
        }
    }

    // treeview
    void MainForm::treeView1_AfterSelect(System::Object^, System::Windows::Forms::TreeViewEventArgs^ e) {
        FileTagData^ fileData = safe_cast<FileTagData^>(e->Node->Tag);
        const size_t fileIdx = fileData->fileIdx & kFileIdxMask;
        const bool isSubFile = fileData->subFileIdx != kEmptyIdx;

        if (mVFXReader) {
            if (isSubFile) {
                const MetroConfigsDatabase::ConfigInfo& ci = mConfigsDatabase->GetFileByIdx(fileData->subFileIdx);

                this->statusLabel1->Text = L"config.bin";
                this->statusLabel2->Text = fileData->subFileIdx.ToString();
                this->statusLabel3->Text = ci.offset.ToString();
                this->statusLabel4->Text = ci.length.ToString();
            } else {
                const MetroFile& mf = mVFXReader->GetFile(fileIdx);
                if (mf.IsFile()) {
                    this->statusLabel1->Text = mf.pakIdx.ToString();
                    this->statusLabel2->Text = mf.offset.ToString();
                    this->statusLabel3->Text = mf.sizeCompressed.ToString();
                    this->statusLabel4->Text = mf.sizeUncompressed.ToString();

                    this->DetectFileAndShow(fileIdx);
                } else {
                    this->statusLabel1->Text = String::Empty;
                    this->statusLabel2->Text = String::Empty;
                    this->statusLabel3->Text = String::Empty;
                    this->statusLabel4->Text = String::Empty;
                }
            }
        }
    }

    void MainForm::treeView1_AfterCollapse(System::Object^, System::Windows::Forms::TreeViewEventArgs^ e) {
        UpdateNodeIcon(e->Node, eNodeEventType::Close);
    }

    void MainForm::treeView1_AfterExpand(System::Object^, System::Windows::Forms::TreeViewEventArgs^ e) {
        UpdateNodeIcon(e->Node, eNodeEventType::Open);

        FileTagData^ fileData = safe_cast<FileTagData^>(e->Node->Tag);
        if (0 == (fileData->fileIdx & kFolderSortedFlag)) {
            const size_t fileIdx = fileData->fileIdx & kFileIdxMask;

            if (e->Node->Nodes->Count > 1) {
                System::Windows::Forms::Cursor::Current = System::Windows::Forms::Cursors::WaitCursor;

                //#NOTE_SK: somehow, BeginUpdate/BeginUpdate makes it even slower, so commented out for the moment
                //this->treeView1->BeginUpdate();
                this->treeView1->SuspendLayout();
                array<TreeNode^>^ nodes = gcnew array<TreeNode^>(e->Node->Nodes->Count);
                e->Node->Nodes->CopyTo(nodes, 0);
                NodeSorter^ sorter = gcnew NodeSorter();
                System::Array::Sort(nodes, sorter);
                e->Node->Nodes->Clear();
                e->Node->Nodes->AddRange(nodes);
                delete sorter;
                delete nodes;
                //this->treeView1->EndUpdate();
                this->treeView1->ResumeLayout(false);

                System::Windows::Forms::Cursor::Current = System::Windows::Forms::Cursors::Arrow;
            }

            fileData->fileIdx = kFolderSortedFlag | fileIdx;
        }
    }

    void MainForm::treeView1_NodeMouseClick(System::Object^, System::Windows::Forms::TreeNodeMouseClickEventArgs^ e) {
        if (e->Button == System::Windows::Forms::MouseButtons::Right) {
            FileTagData^ fileData = safe_cast<FileTagData^>(e->Node->Tag);
            const bool isSubFile = fileData->subFileIdx != kEmptyIdx;

            const size_t fileIdx = fileData->fileIdx & kFileIdxMask;
            const MetroFile& mf = mVFXReader->GetFile(fileIdx);

            const FileType fileType = isSubFile ? fileData->fileType : DetectFileType(mf);

            memset(mExtractionCtx, 0, sizeof(FileExtractionCtx));
            mExtractionCtx->fileIdx = fileIdx;
            mExtractionCtx->type = fileType;
            mExtractionCtx->customOffset = kEmptyCustomValue;
            mExtractionCtx->customLength = kEmptyCustomValue;
            mExtractionCtx->customFileName = "";

            if (mf.IsFile()) {
                switch (fileType) {
                    case FileType::Texture: {
                        this->ctxMenuExportTexture->Show(this->treeView1, e->X, e->Y);
                    } break;

                    case FileType::Model: {
                        this->ctxMenuExportModel->Show(this->treeView1, e->X, e->Y);
                    } break;

                    case FileType::Sound: {
                        this->ctxMenuExportSound->Show(this->treeView1, e->X, e->Y);
                    } break;

                    case FileType::Bin: {
                        if (isSubFile) {
                            const MetroConfigsDatabase::ConfigInfo& ci = mConfigsDatabase->GetFileByIdx(fileData->subFileIdx);

                            mExtractionCtx->customOffset = ci.offset;
                            mExtractionCtx->customLength = ci.length;
                            mExtractionCtx->customFileName = marshal_as<CharString>(e->Node->Text);
                            this->ctxMenuExportBin->Show(this->treeView1, e->X, e->Y);
                        } else {
                            this->ctxMenuExportRaw->Show(this->treeView1, e->X, e->Y);
                        }
                    } break;

                    case FileType::FolderBin: {
                    } break;

                    default: {
                        this->ctxMenuExportRaw->Show(this->treeView1, e->X, e->Y);
                    } break;
                }
            } else {
                this->ctxMenuExportFolder->Show(this->treeView1, e->X, e->Y);
            }
        }
    }

    //
    void MainForm::extractFileToolStripMenuItem_Click(System::Object^, System::EventArgs^) {
        if (!this->ExtractFile(*mExtractionCtx, fs::path())) {
            this->ShowErrorMessage("Failed to extract file!");
        }
    }

    void MainForm::saveAsDDSToolStripMenuItem_Click(System::Object^, System::EventArgs^) {
        mExtractionCtx->txSaveAsDds = true;
        mExtractionCtx->txUseBC3 = false;

        if (!this->ExtractTexture(*mExtractionCtx, fs::path())) {
            this->ShowErrorMessage("Failed to extract texture!");
        }
    }

    void MainForm::saveAsLegacyDDSToolStripMenuItem_Click(System::Object^, System::EventArgs^) {
        mExtractionCtx->txSaveAsDds = true;
        mExtractionCtx->txUseBC3 = true;

        if (!this->ExtractTexture(*mExtractionCtx, fs::path())) {
            this->ShowErrorMessage("Failed to extract texture!");
        }
    }

    void MainForm::saveAsTGAToolStripMenuItem_Click(System::Object^, System::EventArgs^) {
        mExtractionCtx->txSaveAsTga = true;

        if (!this->ExtractTexture(*mExtractionCtx, fs::path())) {
            this->ShowErrorMessage("Failed to extract texture!");
        }
    }

    void MainForm::saveAsPNGToolStripMenuItem_Click(System::Object^, System::EventArgs^) {
        mExtractionCtx->txSaveAsPng = true;

        if (!this->ExtractTexture(*mExtractionCtx, fs::path())) {
            this->ShowErrorMessage("Failed to extract texture!");
        }
    }

    void MainForm::saveAsOBJToolStripMenuItem_Click(System::Object^, System::EventArgs^) {
        mExtractionCtx->mdlSaveAsObj = true;

        if (!this->ExtractModel(*mExtractionCtx, fs::path())) {
            this->ShowErrorMessage("Failed to extract model!");
        }
    }

    void MainForm::saveAsFBXToolStripMenuItem_Click(System::Object^ sender, System::EventArgs^ e) {
        mExtractionCtx->mdlSaveAsFbx = true;

        if (!this->ExtractModel(*mExtractionCtx, fs::path())) {
            this->ShowErrorMessage("Failed to extract model!");
        }
    }

    void MainForm::saveAsOGGToolStripMenuItem_Click(System::Object^ sender, System::EventArgs^ e) {
        mExtractionCtx->sndSaveAsOgg = true;

        if (!this->ExtractSound(*mExtractionCtx, fs::path())) {
            this->ShowErrorMessage("Failed to extract sound!");
        }
    }

    void MainForm::saveAsWAVToolStripMenuItem_Click(System::Object^ sender, System::EventArgs^ e) {
        mExtractionCtx->sndSaveAsWav = true;

        if (!this->ExtractSound(*mExtractionCtx, fs::path())) {
            this->ShowErrorMessage("Failed to extract sound!");
        }
    }

    void MainForm::extractBinRootToolStripMenuItem_Click(System::Object^ sender, System::EventArgs^ e) {
        mExtractionCtx->customOffset = kEmptyCustomValue;
        mExtractionCtx->customLength = kEmptyCustomValue;
        mExtractionCtx->customFileName = "";

        this->extractFileToolStripMenuItem_Click(sender, e);
    }

    void MainForm::extractBinChunkToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e) {
        if (!this->ExtractFile(*mExtractionCtx, fs::path())) {
            this->ShowErrorMessage("Failed to extract bin file chunk!");
        }
    }

    void MainForm::extractFolderToolStripMenuItem_Click(System::Object^ sender, System::EventArgs^ e) {
        fs::path folderPath = ChooseFolderDialog::ChooseFolder("Choose output directory...", this->Handle.ToPointer());
        if (!folderPath.empty()) {
            mExtractionCtx->batch = true;
            mExtractionCtx->raw = true;
            mExtractionCtx->numFilesTotal = mVFXReader->CountFilesInFolder(mExtractionCtx->fileIdx);
            mExtractionCtx->progress = 0;

            pin_ptr<IProgressDialog*> ipdPtr(&mExtractionProgressDlg);
            HRESULT hr = ::CoCreateInstance(CLSID_ProgressDialog, NULL, CLSCTX_INPROC_SERVER, __uuidof(IProgressDialog), (void**)ipdPtr);
            if (SUCCEEDED(hr)) {
                mExtractionProgressDlg->SetTitle(L"Extracting files...");
                mExtractionProgressDlg->SetLine(0, L"Please wait while your files are being extracted...", FALSE, nullptr);
                mExtractionProgressDlg->StartProgressDialog(rcast<HWND>(this->Handle.ToPointer()), nullptr,
                                                            PROGDLG_NORMAL | PROGDLG_MODAL | PROGDLG_AUTOTIME | PROGDLG_NOMINIMIZE,
                                                            nullptr);
            }

            mExtractionThread = gcnew System::Threading::Thread(gcnew System::Threading::ParameterizedThreadStart(this, &MainForm::ExtractionProcessFunc));
            mExtractionThread->Start(PathToString(folderPath));
        }
    }

    void MainForm::extractFolderWithConversionToolStripMenuItem_Click(System::Object^ sender, System::EventArgs^ e) {
        fs::path folderPath = ChooseFolderDialog::ChooseFolder("Choose output directory...", this->Handle.ToPointer());
        if (!folderPath.empty()) {
            ExtractionOptionsDgl dlgOptions;
            dlgOptions.Icon = this->Icon;
            if (dlgOptions.ShowDialog(this) == System::Windows::Forms::DialogResult::OK) {
                mExtractionCtx->batch = true;
                mExtractionCtx->raw = false;

                // textures
                mExtractionCtx->txSaveAsDds = dlgOptions.IsTexturesAsDds();
                mExtractionCtx->txSaveAsTga = dlgOptions.IsTexturesAsTga();
                mExtractionCtx->txSaveAsPng = dlgOptions.IsTexturesAsPng();
                if (dlgOptions.IsTexturesAsLegacyDds()) {
                    mExtractionCtx->txSaveAsDds = true;
                    mExtractionCtx->txUseBC3 = true;
                }
                // models
                mExtractionCtx->mdlSaveAsObj = dlgOptions.IsModelsAsObj();
                mExtractionCtx->mdlSaveAsFbx = dlgOptions.IsModelsAsFbx();
                // sounds
                mExtractionCtx->sndSaveAsOgg = dlgOptions.IsSoundsAsOgg();
                mExtractionCtx->sndSaveAsWav = dlgOptions.IsSoundsAsWav();

                mExtractionCtx->numFilesTotal = mVFXReader->CountFilesInFolder(mExtractionCtx->fileIdx);
                mExtractionCtx->progress = 0;

                pin_ptr<IProgressDialog*> ipdPtr(&mExtractionProgressDlg);
                HRESULT hr = ::CoCreateInstance(CLSID_ProgressDialog, NULL, CLSCTX_INPROC_SERVER, __uuidof(IProgressDialog), (void**)ipdPtr);
                if (SUCCEEDED(hr)) {
                    mExtractionProgressDlg->SetTitle(L"Extracting files...");
                    mExtractionProgressDlg->SetLine(0, L"Please wait while your files are being extracted...", FALSE, nullptr);
                    mExtractionProgressDlg->StartProgressDialog(rcast<HWND>(this->Handle.ToPointer()), nullptr,
                        PROGDLG_NORMAL | PROGDLG_MODAL | PROGDLG_AUTOTIME | PROGDLG_NOMINIMIZE | PROGDLG_NOCANCEL,
                        nullptr);
                }

                mExtractionThread = gcnew System::Threading::Thread(gcnew System::Threading::ParameterizedThreadStart(this, &MainForm::ExtractionProcessFunc));
                mExtractionThread->Start(PathToString(folderPath));
            }
        }
    }

    void MainForm::ShowErrorMessage(String^ message) {
        System::Windows::Forms::MessageBoxButtons buttons = System::Windows::Forms::MessageBoxButtons::OK;
        System::Windows::Forms::MessageBoxIcon mbicon = System::Windows::Forms::MessageBoxIcon::Error;
        System::Windows::Forms::MessageBox::Show(message, this->Text, buttons, mbicon);
    }

    void MainForm::UpdateFilesList() {
        this->treeView1->BeginUpdate();
        this->treeView1->Nodes->Clear();

        if (mVFXReader) {
            this->txtTreeSearch->Text = String::Empty;

            // Get idx of config.bin
            const size_t configBinIdx = mVFXReader->FindFile("content\\config.bin");

            String^ rootName = marshal_as<String^>(mVFXReader->GetSelfName());
            TreeNode^ rootNode = this->treeView1->Nodes->Add(rootName);
            size_t rootIdx = 0;

            mOriginalRootNode = rootNode;

            rootNode->Tag = gcnew FileTagData(FileType::Folder, rootIdx, kEmptyIdx);
            UpdateNodeIcon(rootNode);

            const MetroFile& rootDir = mVFXReader->GetRootFolder();
            for (size_t idx = rootDir.firstFile; idx < rootDir.firstFile + rootDir.numFiles; ++idx) {
                const MetroFile& mf = mVFXReader->GetFile(idx);

                if (mf.IsFile()) {
                    const FileType fileType = DetectFileType(mf);
                    TreeNode^ fileNode = rootNode->Nodes->Add(marshal_as<String^>(mf.name));
                    fileNode->Tag = gcnew FileTagData(fileType, idx, kEmptyIdx);
                    UpdateNodeIcon(fileNode);
                } else {
                    this->AddFoldersRecursive(mf, idx, rootNode, configBinIdx);
                }
            }
        }

        this->treeView1->EndUpdate();
    }

    void MainForm::AddFoldersRecursive(const MetroFile& dir, const size_t folderIdx, TreeNode^ rootItem, const size_t configBinIdx) {
        // Add root folder
        TreeNode^ dirLeafNode = rootItem->Nodes->Add(marshal_as<String^>(dir.name));

        dirLeafNode->Tag = gcnew FileTagData(FileType::Folder, folderIdx, kEmptyIdx);
        UpdateNodeIcon(dirLeafNode);

        // Add files and folders inside
        for (size_t idx = dir.firstFile; idx < dir.firstFile + dir.numFiles; ++idx) {
            const MetroFile& mf = mVFXReader->GetFile(idx);

            if (mf.IsFile()) {
                //==> Add file to list
                if (idx == configBinIdx) {
                    //====> config.bin
                    this->AddBinaryArchive(mf, idx, dirLeafNode);
                } else {
                    //====> any other file
                    const FileType fileType = DetectFileType(mf);
                    TreeNode^ fileNode = dirLeafNode->Nodes->Add(marshal_as<String^>(mf.name));
                    fileNode->Tag = gcnew FileTagData(fileType, idx, kEmptyIdx);
                    UpdateNodeIcon(fileNode);
                }
            } else {
                //==> Add folder to list
                this->AddFoldersRecursive(mf, idx, dirLeafNode, configBinIdx);
            }
        }
    }

    void MainForm::AddBinaryArchive(const MetroFile& mf, const size_t fileIdx, TreeNode^ rootItem) {
        TreeNode^ fileNode = rootItem->Nodes->Add(marshal_as<String^>(mf.name));
        fileNode->Tag = gcnew FileTagData(FileType::BinArchive, fileIdx, kEmptyIdx);
        UpdateNodeIcon(fileNode);

        for (size_t idx = 0, numFiles = mConfigsDatabase->GetNumFiles(); idx < numFiles; ++idx) {
            const MetroConfigsDatabase::ConfigInfo& ci = mConfigsDatabase->GetFileByIdx(idx);

            const bool isNameDecrypted = !ci.nameStr.empty();

            String^ fileName = (isNameDecrypted ?
                marshal_as<String^>(ci.nameStr) :
                String::Format("unknCRC32_0x{0:X}.bin", ci.nameCRC)
            );

            TreeNode^ lastNode = fileNode; // folder to add file
            if (isNameDecrypted) {
                array<String^>^ pathArray = fileName->Split('\\');
                fileName = pathArray[pathArray->Length - 1];

                // Add all sub-folders
                String^ curPath = pathArray[0];
                for (int i = 0; i < (pathArray->Length - 1); ++i) {
                    array<TreeNode^>^ folderNodes = lastNode->Nodes->Find(curPath, false);
                    if (folderNodes->Length == 0) {
                        // Create new folder node
                        String^ folderName = pathArray[i];

                        lastNode = lastNode->Nodes->Add(folderName);
                        lastNode->Tag = gcnew FileTagData(FileType::FolderBin, fileIdx, 0);
                        lastNode->Name = curPath; // for Find()
                        UpdateNodeIcon(lastNode);
                    }
                    else {
                        // Use existing node folder
                        lastNode = folderNodes[0];
                    }

                    curPath += "\\" + pathArray[i + 1];
                }
            }

            // Add binary file
            TreeNode^ chunkNode = lastNode->Nodes->Add(fileName);
            chunkNode->Tag = gcnew FileTagData(FileType::Bin, fileIdx, idx);
            UpdateNodeIcon(chunkNode);
        }
    }

    void MainForm::DetectFileAndShow(const size_t fileIdx) {
        const MetroFile& mf = mVFXReader->GetFile(fileIdx);
        if (!mf.IsFile()) {
            return;
        }

        const FileType fileType = DetectFileType(mf);

        switch (fileType) {
            case FileType::Texture: {
                this->ShowTexture(fileIdx);
            } break;

            case FileType::Model: {
                this->ShowModel(fileIdx);
            } break;

            case FileType::Motion: {
                BytesArray content;
                if (mVFXReader->ExtractFile(fileIdx, content)) {
                    MetroMotion motion;
                    if (motion.LoadFromData(content.data(), content.size())) {
                    }
                }
            } break;

            case FileType::Sound: {
                this->ShowSound(fileIdx);
            } break;

        //case FileType::Level: {
        //    this->ShowLevel(fileIdx);
        //} break;
        }
    }

    void MainForm::ShowTexture(const size_t fileIdx) {
        const MetroFile& mf = mVFXReader->GetFile(fileIdx);

        BytesArray content;
        if (mVFXReader->ExtractFile(fileIdx, content)) {
            MetroTexture texture;
            if (texture.LoadFromData(content.data(), content.size(), mf.name)) {
                if (texture.IsCubemap()) {
                    this->SwitchViewPanel(PanelType::Model);
                    mRenderPanel->SetCubemap(&texture);
                } else {
                    this->SwitchViewPanel(PanelType::Texture);
                    mImagePanel->SetTexture(&texture);
                }

                this->SwitchInfoPanel(PanelType::Texture);

                this->lblImgPropCompression->Text = texture.IsCubemap() ? L"BC6H" : L"BC7";
                this->lblImgPropWidth->Text = texture.GetWidth().ToString();
                this->lblImgPropHeight->Text = texture.GetHeight().ToString();
                this->lblImgPropMips->Text = texture.GetNumMips().ToString();
            }
        }
    }

    void MainForm::ShowModel(const size_t fileIdx) {
        this->SwitchViewPanel(PanelType::Model);
        this->SwitchInfoPanel(PanelType::Model);

        const MetroFile& mf = mVFXReader->GetFile(fileIdx);
        BytesArray modelData;
        if (mVFXReader->ExtractFile(fileIdx, modelData)) {
            MetroModel* mdl = new MetroModel();
            if (mdl->LoadFromData(modelData.data(), modelData.size(), mVFXReader, fileIdx)) {
                mRenderPanel->SetModel(mdl, mVFXReader, mTexturesDatabase);

                this->lstMdlPropMotions->Items->Clear();
                if (mdl->IsAnimated()) {
                    const size_t numMotions = mdl->GetNumMotions();
                    for (size_t i = 0; i < numMotions; ++i) {
                        const MetroMotion* motion = mdl->GetMotion(i);
                        this->lstMdlPropMotions->Items->Add(marshal_as<String^>(motion->GetName()));
                    }

                    this->lblMdlPropType->Text = L"Animated";
                    this->lblMdlPropJoints->Text = mdl->GetSkeleton()->GetNumBones().ToString();
                } else {
                    this->lblMdlPropType->Text = L"Static";
                    this->lblMdlPropJoints->Text = L"0";
                }

                size_t numVertices = 0, numTriangles = 0;
                const size_t numMeshes = mdl->GetNumMeshes();
                for (size_t i = 0; i < numMeshes; ++i) {
                    const MetroMesh* mesh = mdl->GetMesh(i);
                    numVertices += mesh->vertices.size();
                    numTriangles += mesh->faces.size();
                }

                this->lblMdlPropVertices->Text = numVertices.ToString();
                this->lblMdlPropTriangles->Text = numTriangles.ToString();

                this->btnMdlPropPlayStopAnim->Text = L"Play";
            } else {
                MySafeDelete(mdl);
            }
        }
    }

    void MainForm::ShowSound(const size_t fileIdx) {
        mImagePanel->Hide();
        mRenderPanel->Hide();
        mSoundPanel->Show();

        const MetroFile& mf = mVFXReader->GetFile(fileIdx);
        BytesArray modelData;
        if (mVFXReader->ExtractFile(fileIdx, modelData)) {
            MetroSound* snd = new MetroSound();
            if (snd->LoadFromData(modelData.data(), modelData.size())) {
                mSoundPanel->SetSound(snd);
            } else {
                MySafeDelete(snd);
            }
        }
    }

    void MainForm::SwitchViewPanel(PanelType t) {
        switch (t) {
            case PanelType::Texture: {
                mRenderPanel->Hide();
                mSoundPanel->Hide();
                mImagePanel->Show();
            } break;

            case PanelType::Model: {
                mImagePanel->Hide();
                mSoundPanel->Hide();
                mRenderPanel->Show();
            } break;

            case PanelType::Sound: {
                mImagePanel->Hide();
                mRenderPanel->Hide();
                mSoundPanel->Show();
            } break;
        }
    }

    void MainForm::SwitchInfoPanel(PanelType t) {
        switch (t) {
            case PanelType::Texture: {
                this->pnlMdlProps->Dock = System::Windows::Forms::DockStyle::None;
                this->pnlMdlProps->Hide();

                this->pnlImageProps->Location = System::Drawing::Point(0, 0);
                this->pnlImageProps->Dock = System::Windows::Forms::DockStyle::Fill;
                this->pnlImageProps->Show();
            } break;

            case PanelType::Model: {
                this->pnlImageProps->Dock = System::Windows::Forms::DockStyle::None;
                this->pnlImageProps->Hide();

                this->pnlMdlProps->Location = System::Drawing::Point(0, 0);
                this->pnlMdlProps->Dock = System::Windows::Forms::DockStyle::Fill;
                this->pnlMdlProps->Show();
            } break;

            case PanelType::Sound: {
                this->pnlMdlProps->Dock = System::Windows::Forms::DockStyle::None;
                this->pnlMdlProps->Hide();

                this->pnlImageProps->Dock = System::Windows::Forms::DockStyle::None;
                this->pnlImageProps->Hide();
            } break;
        }
    }

    // extraction
    CharString MainForm::MakeFileOutputName(const MetroFile& mf, const FileExtractionCtx& ctx) {
        CharString name = mf.name;

        switch (ctx.type) {
            case FileType::Texture: {
                const CharString::size_type dotPos = name.find_last_of('.');
                const size_t replaceLen = name.size() - dotPos;

                if (ctx.txSaveAsDds) {
                    name = name.replace(dotPos, replaceLen, ".dds");
                } else if (ctx.txSaveAsTga) {
                    name = name.replace(dotPos, replaceLen, ".tga");
                } else if (ctx.txSaveAsPng) {
                    name = name.replace(dotPos, replaceLen, ".png");
                }
            } break;

            case FileType::Model: {
                const CharString::size_type dotPos = name.find_last_of('.');
                const size_t replaceLen = name.size() - dotPos;

                if (ctx.mdlSaveAsObj) {
                    name = name.replace(dotPos, replaceLen, ".obj");
                } else {
                    name = name.replace(dotPos, replaceLen, ".fbx");
                }
            } break;

            case FileType::Sound: {
                if (ctx.sndSaveAsOgg) {
                    name[name.size() - 3] = 'o';
                    name[name.size() - 2] = 'g';
                    name[name.size() - 1] = 'g';
                } else {
                    name[name.size() - 3] = 'w';
                    name[name.size() - 2] = 'a';
                    name[name.size() - 1] = 'v';
                }
            } break;
        }

        return name;
    }

    void MainForm::TextureSaveHelper(const fs::path& folderPath, const FileExtractionCtx& ctx, const CharString& name) {
        CharString textureName = CharString("content\\textures\\") + name;

        CharString textureNameSrc = textureName + ".2048";
        size_t textureIdx = mVFXReader->FindFile(textureNameSrc);
        if (textureIdx == MetroFile::InvalidFileIdx) {
            textureNameSrc = textureName + ".1024";
            textureIdx = mVFXReader->FindFile(textureNameSrc);
        }
        if (textureIdx == MetroFile::InvalidFileIdx) {
            textureNameSrc = textureName + ".512";
            textureIdx = mVFXReader->FindFile(textureNameSrc);
        }

        if (textureIdx != MetroFile::InvalidFileIdx) {
            const MetroFile& txMf = mVFXReader->GetFile(textureIdx);
            FileExtractionCtx tmpCtx = ctx;
            tmpCtx.type = FileType::Texture;
            tmpCtx.fileIdx = textureIdx;
            tmpCtx.txSaveAsDds = false;
            tmpCtx.txSaveAsTga = true;
            tmpCtx.txSaveAsPng = false;

            CharString outName = this->MakeFileOutputName(txMf, tmpCtx);
            this->ExtractTexture(tmpCtx, folderPath / outName);
        }
    }

    bool MainForm::ExtractFile(const FileExtractionCtx& ctx, const fs::path& outPath) {
        bool result = false;

        const MetroFile& mf = mVFXReader->GetFile(ctx.fileIdx);
        String^ name = ctx.customFileName.empty() ?
            marshal_as<String^>(mf.name) :
            marshal_as<String^>(ctx.customFileName);

        fs::path resultPath = outPath;
        if (resultPath.empty()) {
            SaveFileDialog sfd;
            sfd.Title = L"Save file...";
            sfd.Filter = L"All files (*.*)|*.*";
            sfd.FileName = name;
            sfd.RestoreDirectory = true;
            sfd.OverwritePrompt = true;

            if (sfd.ShowDialog(this) == System::Windows::Forms::DialogResult::OK) {
                resultPath = StringToPath(sfd.FileName);
            } else {
                return true;
            }
        }

        if (!resultPath.empty()) {
            BytesArray content;
            if (mVFXReader->ExtractFile(ctx.fileIdx, content)) {
                std::ofstream file(resultPath, std::ofstream::binary);
                if (file.good()) {
                    const void* data = content.data();
                    size_t dataSize = content.size();

                    bool hasCustomLength = ctx.customLength != kEmptyCustomValue;
                    bool hasCustomOffset = ctx.customOffset != kEmptyCustomValue;

                    size_t lengthToWrite = hasCustomLength ?
                        ctx.customLength :
                        dataSize;

                    if (hasCustomOffset) {
                        MemStream stream(data, dataSize);
                        stream.SetCursor(ctx.customOffset);
                        data = stream.GetDataAtCursor();

                        if (hasCustomLength == false) {
                            lengthToWrite = dataSize - ctx.customOffset;
                        }
                    }

                    file.write(rcast<const char*>(data), lengthToWrite);
                    file.flush();

                    result = true;
                }
            }
        }

        return result;
    }

    bool MainForm::ExtractTexture(const FileExtractionCtx& ctx, const fs::path& outPath) {
        bool result = false;

        const MetroFile& mf = mVFXReader->GetFile(ctx.fileIdx);

        fs::path resultPath = outPath;
        if (resultPath.empty()) {
            String^ title;
            String^ filter;
            if (ctx.txSaveAsDds) {
                title = L"Save DDS texture...";
                filter = L"DirectDraw Surface (*.dds)|*.dds";
            } else if (ctx.txSaveAsTga) {
                title = L"Save TGA texture...";
                filter = L"Targa images (*.tga)|*.tga";
            } else {
                title = L"Save PNG texture...";
                filter = L"PNG images (*.png)|*.png";
            }

            CharString nameWithExt = this->MakeFileOutputName(mf, ctx);

            SaveFileDialog sfd;
            sfd.Title = title;
            sfd.Filter = filter;
            sfd.FileName = marshal_as<String^>(nameWithExt);
            sfd.RestoreDirectory = true;
            sfd.OverwritePrompt = true;

            if (sfd.ShowDialog(this) == System::Windows::Forms::DialogResult::OK) {
                resultPath = StringToPath(sfd.FileName);
            } else {
                return true;
            }
        }

        if (!resultPath.empty()) {
            BytesArray content;
            if (mVFXReader->ExtractFile(ctx.fileIdx, content)) {
                MetroTexture texture;
                if (texture.LoadFromData(content.data(), content.size(), mf.name)) {
                    if (ctx.txSaveAsDds) {
                        if (ctx.txUseBC3) {
                            result = texture.SaveAsLegacyDDS(resultPath);
                        } else {
                            result = texture.SaveAsDDS(resultPath);
                        }
                    } else if (ctx.txSaveAsTga) {
                        result = texture.SaveAsTGA(resultPath);
                    } else {
                        result = texture.SaveAsPNG(resultPath);
                    }
                }
            }
        }

        return result;
    }

    bool MainForm::ExtractModel(const FileExtractionCtx& ctx, const fs::path& outPath) {
        bool result = false;

        const MetroFile& mf = mVFXReader->GetFile(ctx.fileIdx);

        fs::path resultPath = outPath;
        if (resultPath.empty()) {
            String^ title;
            String^ filter;
            if (ctx.mdlSaveAsObj) {
                title = L"Save OBJ model...";
                filter = L"OBJ model (*.obj)|*.obj";
            } else {
                title = L"Save FBX nodel...";
                filter = L"FBX model (*.fbx)|*.fbx";
            }

            CharString nameWithExt = this->MakeFileOutputName(mf, ctx);

            SaveFileDialog sfd;
            sfd.Title = title;
            sfd.Filter = filter;
            sfd.FileName = marshal_as<String^>(nameWithExt);
            sfd.RestoreDirectory = true;
            sfd.OverwritePrompt = true;

            if (sfd.ShowDialog(this) == System::Windows::Forms::DialogResult::OK) {
                resultPath = StringToPath(sfd.FileName);
            } else {
                return true;
            }
        }

        if (!resultPath.empty()) {
            BytesArray modelData;
            if (mVFXReader->ExtractFile(ctx.fileIdx, modelData)) {
                MetroModel mdl;
                if (mdl.LoadFromData(modelData.data(), modelData.size(), mVFXReader, ctx.fileIdx)) {
                    if (ctx.mdlSaveAsObj) {
                        mdl.SaveAsOBJ(resultPath, mVFXReader, mTexturesDatabase);
                    } else {
                        mdl.SaveAsFBX(resultPath, mVFXReader, mTexturesDatabase);
                    }

                    if (!ctx.batch) {
                        fs::path folderPath = resultPath.parent_path();
                        for (size_t i = 0; i < mdl.GetNumMeshes(); ++i) {
                            const MetroMesh* mesh = mdl.GetMesh(i);
                            if (!mesh->materials.empty()) {
                                const CharString& textureName = mesh->materials.front();

                                const CharString& sourceName = mTexturesDatabase->GetSourceName(textureName);
                                const CharString& bumpName = mTexturesDatabase->GetSourceName(textureName);

                                this->TextureSaveHelper(folderPath, ctx, sourceName);
                                if (!bumpName.empty()) {
                                    this->TextureSaveHelper(folderPath, ctx, bumpName + "_nm");
                                }
                            }
                        }
                    }

                    result = true;
                }
            }
        }

        return result;
    }

    bool MainForm::ExtractSound(const FileExtractionCtx& ctx, const fs::path& outPath) {
        bool result = false;

        const MetroFile& mf = mVFXReader->GetFile(ctx.fileIdx);

        fs::path resultPath = outPath;
        if (resultPath.empty()) {
            String^ title;
            String^ filter;
            if (ctx.sndSaveAsOgg) {
                title = L"Save Ogg sound...";
                filter = L"Ogg Vorbis (*.ogg)|*.ogg";
            } else {
                title = L"Save WAV sound...";
                filter = L"Wave sounds (*.wav)|*.wav";
            }

            CharString nameWithExt = this->MakeFileOutputName(mf, ctx);

            SaveFileDialog sfd;
            sfd.Title = title;
            sfd.Filter = filter;
            sfd.FileName = marshal_as<String^>(nameWithExt);
            sfd.RestoreDirectory = true;
            sfd.OverwritePrompt = true;

            if (sfd.ShowDialog(this) == System::Windows::Forms::DialogResult::OK) {
                resultPath = StringToPath(sfd.FileName);
            } else {
                return true;
            }
        }

        if (!resultPath.empty()) {
            BytesArray content;
            if (mVFXReader->ExtractFile(ctx.fileIdx, content)) {
                MetroSound sound;
                if (sound.LoadFromData(content.data(), content.size())) {
                    if (ctx.sndSaveAsOgg) {
                        result = sound.SaveAsOGG(resultPath);
                    } else {
                        result = sound.SaveAsWAV(resultPath);
                    }
                }
            }
        }

        return result;
    }

    bool MainForm::ExtractFolderComplete(const FileExtractionCtx& ctx, const fs::path& outPath) {
        bool result = false;

        const MetroFile& folder = mVFXReader->GetFile(ctx.fileIdx);

        fs::path curPath = outPath / folder.name;
        fs::create_directories(curPath);

        FileExtractionCtx tmpCtx = ctx;
        for (size_t idx = folder.firstFile; idx < (folder.firstFile + folder.numFiles); ++idx) {
            const MetroFile& mf = mVFXReader->GetFile(idx);

            tmpCtx.fileIdx = idx;
            tmpCtx.type = DetectFileType(mf);

            if (mf.IsFile()) {
                if (ctx.raw) {
                    fs::path filePath = curPath / mf.name;
                    this->ExtractFile(tmpCtx, filePath);
                } else {
                    fs::path filePath = curPath / this->MakeFileOutputName(mf, tmpCtx);
                    switch (tmpCtx.type) {
                    case FileType::Texture: {
                        this->ExtractTexture(tmpCtx, filePath);
                    } break;

                    case FileType::Model: {
                        this->ExtractModel(tmpCtx, filePath);
                    } break;

                    case FileType::Sound: {
                        this->ExtractSound(tmpCtx, filePath);
                    } break;

                    default: {
                        this->ExtractFile(tmpCtx, filePath);
                    } break;
                    }
                }

                mExtractionCtx->progress++;
                if (mExtractionProgressDlg) {
                    mExtractionProgressDlg->SetProgress64(mExtractionCtx->progress, mExtractionCtx->numFilesTotal);
                }

            } else {
                this->ExtractFolderComplete(tmpCtx, curPath);
            }
        }

        result = true;

        return result;
    }

    void MainForm::ExtractionProcessFunc(Object^ folderPath) {
        this->ExtractFolderComplete(*mExtractionCtx, StringToPath(folderPath->ToString()));

        if (mExtractionProgressDlg) {
            mExtractionProgressDlg->StopProgressDialog();
            MySafeRelease(mExtractionProgressDlg);
        }
    }

    // filter
    void MainForm::txtTreeSearch_TextChanged(System::Object^ sender, System::EventArgs^ e) {
        if (mOriginalRootNode != nullptr) {
            this->filterTimer->Stop();
            this->filterTimer->Start();
        }
    }

    void MainForm::filterTimer_Tick(System::Object^ sender, System::EventArgs^ e) {
        this->filterTimer->Stop();

        System::Windows::Forms::Cursor::Current = System::Windows::Forms::Cursors::WaitCursor;

        this->treeView1->BeginUpdate();
        this->treeView1->Nodes->Clear();

        if (String::IsNullOrWhiteSpace(this->txtTreeSearch->Text)) {
            this->treeView1->Nodes->Add(mOriginalRootNode);
        } else {
            TreeNode^ root = safe_cast<TreeNode^>(mOriginalRootNode->Clone());
            this->FilterTreeView(root, this->txtTreeSearch->Text);
            this->treeView1->Nodes->Add(root);

            if (this->txtTreeSearch->Text->Length > 2) {
                root->ExpandAll();
            }
        }

        this->treeView1->EndUpdate();

        System::Windows::Forms::Cursor::Current = System::Windows::Forms::Cursors::Arrow;
    }

    bool MainForm::FilterTreeView(TreeNode^ node, String^ text) {
        System::Collections::Generic::List<TreeNode^>^ nodesToRemove = gcnew System::Collections::Generic::List<TreeNode^>();

        for (int i = 0; i < node->Nodes->Count; i++) {
            if (node->Nodes[i]->Nodes->Count > 0) {
                if (!this->FilterTreeView(node->Nodes[i], this->txtTreeSearch->Text)) {
                    nodesToRemove->Add(node->Nodes[i]);
                }
            } else if (!node->Nodes[i]->Text->Contains(this->txtTreeSearch->Text)) {
                nodesToRemove->Add(node->Nodes[i]);
            }
        }

        for (int i = 0; i < nodesToRemove->Count; i++) {
            node->Nodes->Remove(nodesToRemove[i]);
        }

        MySafeDelete(nodesToRemove);

        return node->Nodes->Count != 0;
    }

    // property panels
    // model props
    void MainForm::lstMdlPropMotions_SelectedIndexChanged(System::Object^, System::EventArgs^) {
        const int idx = lstMdlPropMotions->SelectedIndex;
        if (idx >= 0) {
            mRenderPanel->SwitchMotion(idx);
        }
    }

    void MainForm::btnMdlPropPlayStopAnim_Click(System::Object^, System::EventArgs^) {
        mRenderPanel->PlayAnim(!mRenderPanel->IsPlayingAnim());

        this->btnMdlPropPlayStopAnim->Text = mRenderPanel->IsPlayingAnim() ? L"Stop" : L"Play";
    }
}
