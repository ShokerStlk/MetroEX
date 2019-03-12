#include "metro/VFXReader.h"
#include "metro/MetroTexturesDatabase.h"
#include "metro/MetroTexture.h"
#include "metro/MetroModel.h"
#include "metro/MetroSound.h"

#include <fstream>

#include "ChooseFolderDlg.h"

#include "MainForm.h"
#include "ExtractionOptionsDgl.h"
#include "AboutDlg.h"

// String to std::string wrapper
#include <msclr/marshal_cppstd.h>
using namespace msclr::interop;

static const int kImageIdxFile          = 0;
static const int kImageIdxFoldeClosed   = 1;
static const int kImageIdxFoldeOpen     = 2;


namespace MetroEX {
    static FileType DetectFileType(const MetroFile& mf) {
        FileType result = FileType::Unknown;

        String^ name = marshal_as<String^>(mf.name.c_str());

        if (name->EndsWith(L".dds") ||
            name->EndsWith(L".512") ||
            name->EndsWith(L".1024") ||
            name->EndsWith(L".2048")) {
            result = FileType::Texture;
        } else if (name->EndsWith(L".model")) {
            result = FileType::Model;
        } else if (name->EndsWith(L".vba")) {
            result = FileType::Sound;
        } else if (name->EndsWith(L"lightmaps")) {
            result = FileType::Level;
        }

        return result;
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
        this->splitContainer1->Panel2->Controls->Add(mImagePanel);
        mImagePanel->Dock = System::Windows::Forms::DockStyle::Fill;
        mImagePanel->Location = System::Drawing::Point(0, 0);
        mImagePanel->Name = L"mImagePanel";
        mImagePanel->Size = System::Drawing::Size(528, 386);
        mImagePanel->AutoScroll = true;


        mRenderPanel = gcnew RenderPanel();
        this->splitContainer1->Panel2->Controls->Add(mRenderPanel);

        mRenderPanel->Dock = System::Windows::Forms::DockStyle::Fill;
        mRenderPanel->Location = System::Drawing::Point(0, 0);
        mRenderPanel->Name = L"mRenderPanel";
        mRenderPanel->Size = System::Drawing::Size(528, 386);

        if (!mRenderPanel->InitGraphics()) {
            this->ShowErrorMessage("Failed to initialize DirectX 11 graphics!\n3D viewer will be unavailable.");
        }

        mRenderPanel->Hide();
    }

    void MainForm::openToolStripMenuItem_Click(System::Object^, System::EventArgs^) {
        OpenFileDialog ofd;
        ofd.Title = L"Open Metro Exodus vfx file...";
        ofd.Filter = L"VFX files (*.vfx)|*.vfx";
        ofd.FilterIndex = 0;
        ofd.RestoreDirectory = true;
        if (ofd.ShowDialog(this) == System::Windows::Forms::DialogResult::OK) {
            if (mVFXReader) {
                delete mVFXReader;
            }

            mVFXReader = new VFXReader();
            if (mVFXReader->LoadFromFile(marshal_as<CharString>(ofd.FileName))) {
                this->UpdateFilesList();

                size_t fileIdx = mVFXReader->FindFile("content\\textures_handles_storage.bin");
                if (MetroFile::InvalidFileIdx != fileIdx) {
                    BytesArray content;
                    if (mVFXReader->ExtractFile(fileIdx, content)) {
                        mTexturesDatabase = new MetroTexturesDatabase();
                        mTexturesDatabase->LoadFromData(content.data(), content.size());

                        fileIdx = mVFXReader->FindFile("content\\scripts\\texture_aliases.bin");
                        if (MetroFile::InvalidFileIdx != fileIdx) {
                            if (mVFXReader->ExtractFile(fileIdx, content)) {
                                mTexturesDatabase->LoadAliasesFromData(content.data(), content.size());
                            }
                        }
                    }
                }
            }
        }
    }

    void MainForm::closeToolStripMenuItem_Click(System::Object^, System::EventArgs^) {
        this->Close();
    }

    void MainForm::aboutToolStripMenuItem_Click(System::Object^, System::EventArgs^) {
        AboutDlg dlg;
        dlg.Icon = this->Icon;
        dlg.Text = this->Text;
        dlg.ShowDialog(this);
    }

    void MainForm::treeView1_AfterSelect(System::Object^, System::Windows::Forms::TreeViewEventArgs^ e) {
        const size_t fileIdx = safe_cast<size_t>(e->Node->Tag);

        if (mVFXReader) {
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

    void MainForm::treeView1_AfterCollapse(System::Object^, System::Windows::Forms::TreeViewEventArgs^ e) {
        e->Node->ImageIndex = kImageIdxFoldeClosed;
        e->Node->SelectedImageIndex = kImageIdxFoldeClosed;
    }

    void MainForm::treeView1_AfterExpand(System::Object^, System::Windows::Forms::TreeViewEventArgs^ e) {
        e->Node->ImageIndex = kImageIdxFoldeOpen;
        e->Node->SelectedImageIndex = kImageIdxFoldeOpen;
    }

    void MainForm::treeView1_NodeMouseClick(System::Object^, System::Windows::Forms::TreeNodeMouseClickEventArgs^ e) {
        if (e->Button == System::Windows::Forms::MouseButtons::Right) {
            const size_t fileIdx = safe_cast<size_t>(e->Node->Tag);

            const MetroFile& mf = mVFXReader->GetFile(fileIdx);
            const FileType fileType = DetectFileType(mf);

            memset(mExtractionCtx, 0, sizeof(FileExtractionCtx));
            mExtractionCtx->fileIdx = fileIdx;
            mExtractionCtx->type = fileType;

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
            mExtractionThread->Start(marshal_as<String^>(folderPath.u8string()));
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
                mExtractionThread->Start(marshal_as<String^>(folderPath.u8string()));
            }
        }
    }


    void MainForm::ShowErrorMessage(String^ message) {
        System::Windows::Forms::MessageBoxButtons buttons = System::Windows::Forms::MessageBoxButtons::OK;
        System::Windows::Forms::MessageBoxIcon mbicon = System::Windows::Forms::MessageBoxIcon::Error;
        System::Windows::Forms::MessageBox::Show(message, this->Text, buttons, mbicon);
    }

    void MainForm::UpdateFilesList() {
        this->treeView1->Nodes->Clear();

        if (mVFXReader) {
            String^ rootName = marshal_as<String^>(mVFXReader->mFileName);
            TreeNode^ rootNode = this->treeView1->Nodes->Add(rootName);
            size_t rootIdx = 0;

            rootNode->ImageIndex = kImageIdxFoldeClosed;
            rootNode->SelectedImageIndex = kImageIdxFoldeClosed;
            rootNode->Tag = rootIdx;

            MetroFile& rootDir = mVFXReader->mFiles.front();

            for (size_t idx = rootDir.firstFile; idx < rootDir.firstFile + rootDir.numFiles; ++idx) {
                const MetroFile& mf = mVFXReader->mFiles[idx];

                if (mf.IsFile()) {
                    TreeNode^ fileNode = rootNode->Nodes->Add(marshal_as<String^>(mf.name));
                    fileNode->Tag = idx;
                    fileNode->ImageIndex = kImageIdxFile;
                    fileNode->SelectedImageIndex = kImageIdxFile;
                } else {
                    this->AddFoldersRecursive(mf, idx, rootNode);
                }
            }
        }
    }

    void MainForm::AddFoldersRecursive(const MetroFile& dir, const size_t folderIdx, TreeNode^ rootItem) {
        TreeNode^ dirLeafNode = rootItem->Nodes->Add(marshal_as<String^>(dir.name));

        dirLeafNode->ImageIndex = kImageIdxFoldeClosed;
        dirLeafNode->SelectedImageIndex = kImageIdxFoldeClosed;
        dirLeafNode->Tag = folderIdx;

        for (size_t idx = dir.firstFile; idx < dir.firstFile + dir.numFiles; ++idx) {
            const MetroFile& mf = mVFXReader->mFiles[idx];

            if (mf.IsFile()) {
                TreeNode^ fileNode = dirLeafNode->Nodes->Add(marshal_as<String^>(mf.name));
                fileNode->Tag = idx;
                fileNode->ImageIndex = kImageIdxFile;
                fileNode->SelectedImageIndex = kImageIdxFile;
            } else {
                this->AddFoldersRecursive(mf, idx, dirLeafNode);
            }
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

        //case FileType::Level: {
        //    this->ShowLevel(fileIdx);
        //} break;
        }
    }

    void MainForm::ShowTexture(const size_t fileIdx) {
        mRenderPanel->Hide();
        mImagePanel->Show();

        const MetroFile& mf = mVFXReader->GetFile(fileIdx);

        BytesArray content;
        if (mVFXReader->ExtractFile(fileIdx, content)) {
            MetroTexture texture;
            if (texture.LoadFromData(content.data(), content.size(), mf.name)) {
                mImagePanel->SetTexture(&texture);
            }
        }
    }

    void MainForm::ShowModel(const size_t fileIdx) {
        mImagePanel->Hide();
        mRenderPanel->Show();

        const MetroFile& mf = mVFXReader->GetFile(fileIdx);
        BytesArray modelData;
        if (mVFXReader->ExtractFile(fileIdx, modelData)) {
            MetroModel* mdl = new MetroModel();
            if (mdl->LoadFromData(modelData.data(), modelData.size(), mVFXReader, fileIdx)) {
                mRenderPanel->SetModel(mdl, mVFXReader, mTexturesDatabase);
            } else {
                delete mdl;
            }
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
        String^ name = marshal_as<String^>(mf.name);

        fs::path resultPath = outPath;
        if (resultPath.empty()) {
            SaveFileDialog sfd;
            sfd.Title = L"Save file...";
            sfd.Filter = L"All files (*.*)|*.*";
            sfd.FileName = name;
            sfd.RestoreDirectory = true;
            sfd.OverwritePrompt = true;

            if (sfd.ShowDialog(this) == System::Windows::Forms::DialogResult::OK) {
                resultPath = marshal_as<CharString>(sfd.FileName);
            } else {
                return true;
            }
        }

        if (!resultPath.empty()) {
            BytesArray content;
            if (mVFXReader->ExtractFile(ctx.fileIdx, content)) {
                std::ofstream file(resultPath, std::ofstream::binary);
                if (file.good()) {
                    file.write(rcast<const char*>(content.data()), content.size());
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
                resultPath = marshal_as<CharString>(sfd.FileName);
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
                resultPath = marshal_as<CharString>(sfd.FileName);
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
                resultPath = marshal_as<CharString>(sfd.FileName);
            } else {
                return true;
            }
        }

        if (!resultPath.empty()) {
            BytesArray content;
            if (mVFXReader->ExtractFile(ctx.fileIdx, content)) {
                if (ctx.sndSaveAsOgg) {
                    result = MetroSound::SaveDataAsOGG(content.data(), content.size(), resultPath);
                } else {
                    result = MetroSound::SaveDataAsWAV(content.data(), content.size(), resultPath);
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
        this->ExtractFolderComplete(*mExtractionCtx, marshal_as<CharString>(folderPath->ToString()));

        if (mExtractionProgressDlg) {
            mExtractionProgressDlg->StopProgressDialog();
            SAFE_RELEASE(mExtractionProgressDlg);
        }
    }
}
