#pragma once

#include "mycommon.h"
#include "metro/MetroTypes.h"

#include <shlobj_core.h>

#include "RenderPanel.h"
#include "ImagePanel.h"
#include "SoundPanel.h"

class VFXReader;
class MetroTexturesDatabase;
class MetroConfigsDatabase;

namespace MetroEX {

    enum class FileType : size_t {
        Unknown,
        Folder,
        FolderBin,
        Bin,
        BinArchive,
        BinEditable,
        Texture,
        Model,
        Level,
        Sound
    };

    struct FileExtractionCtx {
        size_t      fileIdx;
        FileType    type;
        // models
        bool        mdlSaveAsObj;
        bool        mdlSaveAsFbx;
        // textures
        bool        txUseBC3;
        bool        txSaveAsDds;
        bool        txSaveAsTga;
        bool        txSaveAsPng;
        // sounds
        bool        sndSaveAsOgg;
        bool        sndSaveAsWav;

        // batch
        bool        batch;
        bool        raw;
        size_t      numFilesTotal;
        size_t      progress;
    };

    using namespace System;
    using namespace System::ComponentModel;
    using namespace System::Collections;
    using namespace System::Windows::Forms;
    using namespace System::Data;
    using namespace System::Drawing;


    public ref class MainForm : public System::Windows::Forms::Form
    {
    public:
        MainForm() {
            mImagePanel = nullptr;
            mRenderPanel = nullptr;
            mSoundPanel = nullptr;
            mVFXReader = nullptr;

            mExtractionCtx = new FileExtractionCtx;
            mExtractionProgressDlg = nullptr;

            mOriginalRootNode = nullptr;

            InitializeComponent();
        }

    protected:
        ~MainForm() {
            if (components) {
                delete components;
            }
        }

    private:
        MetroEX::ImagePanel^        mImagePanel;
        MetroEX::RenderPanel^       mRenderPanel;
        MetroEX::SoundPanel^        mSoundPanel;

        //
        VFXReader*                  mVFXReader;

        FileExtractionCtx*          mExtractionCtx;
        System::Threading::Thread^  mExtractionThread;
        IProgressDialog*            mExtractionProgressDlg;

        TreeNode^                   mOriginalRootNode;

    public:
        MetroTexturesDatabase*      mTexturesDatabase;
        MetroConfigsDatabase*       mConfigsDatabase;

    protected:

    private: System::Windows::Forms::StatusStrip^  statusStrip1;
    private: System::Windows::Forms::SplitContainer^  splitContainer1;




    private: System::Windows::Forms::ImageList^  imageListMain;
    private: System::Windows::Forms::ToolStripStatusLabel^  statusLabel1;
    private: System::Windows::Forms::ToolStripStatusLabel^  statusLabel2;
    private: System::Windows::Forms::ToolStripStatusLabel^  statusLabel3;
    private: System::Windows::Forms::ToolStripStatusLabel^  statusLabel4;
    private: System::Windows::Forms::ContextMenuStrip^  ctxMenuExportTexture;
    private: System::Windows::Forms::ContextMenuStrip^  ctxMenuExportModel;
    private: System::Windows::Forms::ContextMenuStrip^  ctxMenuExportSound;
    private: System::Windows::Forms::ContextMenuStrip^  ctxMenuExportRaw;
    private: System::Windows::Forms::ContextMenuStrip^  ctxMenuExportFolder;
    private: System::Windows::Forms::ToolStripMenuItem^  saveAsDDSToolStripMenuItem;
    private: System::Windows::Forms::ToolStripMenuItem^  saveAsLegacyDDSToolStripMenuItem;
    private: System::Windows::Forms::ToolStripMenuItem^  saveAsTGAToolStripMenuItem;
    private: System::Windows::Forms::ToolStripMenuItem^  saveAsPNGToolStripMenuItem;
    private: System::Windows::Forms::ToolStripMenuItem^  extractFileToolStripMenuItem;
    private: System::Windows::Forms::ToolStripMenuItem^  saveAsOBJToolStripMenuItem;
    private: System::Windows::Forms::ToolStripMenuItem^  saveAsFBXToolStripMenuItem;
    private: System::Windows::Forms::ToolStripMenuItem^  saveAsOGGToolStripMenuItem;
    private: System::Windows::Forms::ToolStripMenuItem^  saveAsWAVToolStripMenuItem;
    private: System::Windows::Forms::ToolStripMenuItem^  extractFolderToolStripMenuItem;
    private: System::Windows::Forms::ToolStripMenuItem^  extractFolderWithConversionToolStripMenuItem;


    private: System::Windows::Forms::ToolStrip^  toolStrip1;
    private: System::Windows::Forms::ToolStripButton^  toolBtnFileOpen;
    private: System::Windows::Forms::ToolStripButton^  toolBtnAbout;
    private: System::Windows::Forms::ToolStripSeparator^  toolStripSeparator2;
    private: System::Windows::Forms::ToolStripButton^  toolBtnImgEnableAlpha;
    private: System::Windows::Forms::ToolStripSeparator^  toolStripSeparator1;

    private: System::Windows::Forms::Timer^  filterTimer;
    private: System::Windows::Forms::TableLayoutPanel^  tableLayoutPanel1;
    private: System::Windows::Forms::TreeView^  treeView1;
    private: System::Windows::Forms::TextBox^  txtTreeSearch;



    private: System::ComponentModel::IContainer^  components;

    private:
        /// <summary>
        /// Required designer variable.
        /// </summary>


#pragma region Windows Form Designer generated code
        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        void InitializeComponent(void)
        {
            this->components = (gcnew System::ComponentModel::Container());
            System::ComponentModel::ComponentResourceManager^  resources = (gcnew System::ComponentModel::ComponentResourceManager(MainForm::typeid));
            this->statusStrip1 = (gcnew System::Windows::Forms::StatusStrip());
            this->statusLabel1 = (gcnew System::Windows::Forms::ToolStripStatusLabel());
            this->statusLabel2 = (gcnew System::Windows::Forms::ToolStripStatusLabel());
            this->statusLabel3 = (gcnew System::Windows::Forms::ToolStripStatusLabel());
            this->statusLabel4 = (gcnew System::Windows::Forms::ToolStripStatusLabel());
            this->splitContainer1 = (gcnew System::Windows::Forms::SplitContainer());
            this->tableLayoutPanel1 = (gcnew System::Windows::Forms::TableLayoutPanel());
            this->treeView1 = (gcnew System::Windows::Forms::TreeView());
            this->imageListMain = (gcnew System::Windows::Forms::ImageList(this->components));
            this->txtTreeSearch = (gcnew System::Windows::Forms::TextBox());
            this->ctxMenuExportTexture = (gcnew System::Windows::Forms::ContextMenuStrip(this->components));
            this->saveAsDDSToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
            this->saveAsLegacyDDSToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
            this->saveAsTGAToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
            this->saveAsPNGToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
            this->ctxMenuExportModel = (gcnew System::Windows::Forms::ContextMenuStrip(this->components));
            this->saveAsOBJToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
            this->saveAsFBXToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
            this->ctxMenuExportSound = (gcnew System::Windows::Forms::ContextMenuStrip(this->components));
            this->saveAsOGGToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
            this->saveAsWAVToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
            this->ctxMenuExportRaw = (gcnew System::Windows::Forms::ContextMenuStrip(this->components));
            this->extractFileToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
            this->ctxMenuExportFolder = (gcnew System::Windows::Forms::ContextMenuStrip(this->components));
            this->extractFolderToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
            this->extractFolderWithConversionToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
            this->toolStrip1 = (gcnew System::Windows::Forms::ToolStrip());
            this->toolBtnFileOpen = (gcnew System::Windows::Forms::ToolStripButton());
            this->toolStripSeparator1 = (gcnew System::Windows::Forms::ToolStripSeparator());
            this->toolBtnAbout = (gcnew System::Windows::Forms::ToolStripButton());
            this->toolStripSeparator2 = (gcnew System::Windows::Forms::ToolStripSeparator());
            this->toolBtnImgEnableAlpha = (gcnew System::Windows::Forms::ToolStripButton());
            this->filterTimer = (gcnew System::Windows::Forms::Timer(this->components));
            this->statusStrip1->SuspendLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->splitContainer1))->BeginInit();
            this->splitContainer1->Panel1->SuspendLayout();
            this->splitContainer1->SuspendLayout();
            this->tableLayoutPanel1->SuspendLayout();
            this->ctxMenuExportTexture->SuspendLayout();
            this->ctxMenuExportModel->SuspendLayout();
            this->ctxMenuExportSound->SuspendLayout();
            this->ctxMenuExportRaw->SuspendLayout();
            this->ctxMenuExportFolder->SuspendLayout();
            this->toolStrip1->SuspendLayout();
            this->SuspendLayout();
            // 
            // statusStrip1
            // 
            this->statusStrip1->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(4) {
                this->statusLabel1, this->statusLabel2,
                    this->statusLabel3, this->statusLabel4
            });
            this->statusStrip1->Location = System::Drawing::Point(0, 697);
            this->statusStrip1->Name = L"statusStrip1";
            this->statusStrip1->Size = System::Drawing::Size(1159, 22);
            this->statusStrip1->TabIndex = 1;
            this->statusStrip1->Text = L"statusStrip1";
            // 
            // statusLabel1
            // 
            this->statusLabel1->Name = L"statusLabel1";
            this->statusLabel1->Size = System::Drawing::Size(286, 17);
            this->statusLabel1->Spring = true;
            this->statusLabel1->TextAlign = System::Drawing::ContentAlignment::MiddleLeft;
            // 
            // statusLabel2
            // 
            this->statusLabel2->Name = L"statusLabel2";
            this->statusLabel2->Size = System::Drawing::Size(286, 17);
            this->statusLabel2->Spring = true;
            this->statusLabel2->TextAlign = System::Drawing::ContentAlignment::MiddleLeft;
            // 
            // statusLabel3
            // 
            this->statusLabel3->Name = L"statusLabel3";
            this->statusLabel3->Size = System::Drawing::Size(286, 17);
            this->statusLabel3->Spring = true;
            this->statusLabel3->TextAlign = System::Drawing::ContentAlignment::MiddleLeft;
            // 
            // statusLabel4
            // 
            this->statusLabel4->BorderStyle = System::Windows::Forms::Border3DStyle::Raised;
            this->statusLabel4->DisplayStyle = System::Windows::Forms::ToolStripItemDisplayStyle::Text;
            this->statusLabel4->Name = L"statusLabel4";
            this->statusLabel4->Size = System::Drawing::Size(286, 17);
            this->statusLabel4->Spring = true;
            this->statusLabel4->TextAlign = System::Drawing::ContentAlignment::MiddleLeft;
            // 
            // splitContainer1
            // 
            this->splitContainer1->Dock = System::Windows::Forms::DockStyle::Fill;
            this->splitContainer1->Location = System::Drawing::Point(0, 25);
            this->splitContainer1->Name = L"splitContainer1";
            // 
            // splitContainer1.Panel1
            // 
            this->splitContainer1->Panel1->Controls->Add(this->tableLayoutPanel1);
            this->splitContainer1->Size = System::Drawing::Size(1159, 672);
            this->splitContainer1->SplitterDistance = 301;
            this->splitContainer1->TabIndex = 2;
            // 
            // tableLayoutPanel1
            // 
            this->tableLayoutPanel1->ColumnCount = 1;
            this->tableLayoutPanel1->ColumnStyles->Add((gcnew System::Windows::Forms::ColumnStyle(System::Windows::Forms::SizeType::Percent,
                100)));
            this->tableLayoutPanel1->Controls->Add(this->treeView1, 0, 1);
            this->tableLayoutPanel1->Controls->Add(this->txtTreeSearch, 0, 0);
            this->tableLayoutPanel1->Dock = System::Windows::Forms::DockStyle::Fill;
            this->tableLayoutPanel1->Location = System::Drawing::Point(0, 0);
            this->tableLayoutPanel1->Margin = System::Windows::Forms::Padding(0);
            this->tableLayoutPanel1->Name = L"tableLayoutPanel1";
            this->tableLayoutPanel1->RowCount = 2;
            this->tableLayoutPanel1->RowStyles->Add((gcnew System::Windows::Forms::RowStyle(System::Windows::Forms::SizeType::Absolute, 22)));
            this->tableLayoutPanel1->RowStyles->Add((gcnew System::Windows::Forms::RowStyle()));
            this->tableLayoutPanel1->Size = System::Drawing::Size(301, 672);
            this->tableLayoutPanel1->TabIndex = 2;
            // 
            // treeView1
            // 
            this->treeView1->Dock = System::Windows::Forms::DockStyle::Fill;
            this->treeView1->ImageIndex = 0;
            this->treeView1->ImageList = this->imageListMain;
            this->treeView1->Location = System::Drawing::Point(0, 22);
            this->treeView1->Margin = System::Windows::Forms::Padding(0);
            this->treeView1->Name = L"treeView1";
            this->treeView1->SelectedImageIndex = 0;
            this->treeView1->Size = System::Drawing::Size(301, 650);
            this->treeView1->TabIndex = 0;
            this->treeView1->AfterCollapse += gcnew System::Windows::Forms::TreeViewEventHandler(this, &MainForm::treeView1_AfterCollapse);
            this->treeView1->AfterExpand += gcnew System::Windows::Forms::TreeViewEventHandler(this, &MainForm::treeView1_AfterExpand);
            this->treeView1->AfterSelect += gcnew System::Windows::Forms::TreeViewEventHandler(this, &MainForm::treeView1_AfterSelect);
            this->treeView1->NodeMouseClick += gcnew System::Windows::Forms::TreeNodeMouseClickEventHandler(this, &MainForm::treeView1_NodeMouseClick);
            // 
            // imageListMain
            // 
            this->imageListMain->ImageStream = (cli::safe_cast<System::Windows::Forms::ImageListStreamer^>(resources->GetObject(L"imageListMain.ImageStream")));
            this->imageListMain->TransparentColor = System::Drawing::Color::Transparent;
            this->imageListMain->Images->SetKeyName(0, L"document.png");
            this->imageListMain->Images->SetKeyName(1, L"folder_closed.png");
            this->imageListMain->Images->SetKeyName(2, L"folder_opened.png");
            // 
            // txtTreeSearch
            // 
            this->txtTreeSearch->Dock = System::Windows::Forms::DockStyle::Top;
            this->txtTreeSearch->Location = System::Drawing::Point(0, 0);
            this->txtTreeSearch->Margin = System::Windows::Forms::Padding(0);
            this->txtTreeSearch->Name = L"txtTreeSearch";
            this->txtTreeSearch->Size = System::Drawing::Size(301, 20);
            this->txtTreeSearch->TabIndex = 1;
            this->txtTreeSearch->TextChanged += gcnew System::EventHandler(this, &MainForm::txtTreeSearch_TextChanged);
            // 
            // ctxMenuExportTexture
            // 
            this->ctxMenuExportTexture->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(4) {
                this->saveAsDDSToolStripMenuItem,
                    this->saveAsLegacyDDSToolStripMenuItem, this->saveAsTGAToolStripMenuItem, this->saveAsPNGToolStripMenuItem
            });
            this->ctxMenuExportTexture->Name = L"ctxMenuExportTexture";
            this->ctxMenuExportTexture->Size = System::Drawing::Size(184, 92);
            // 
            // saveAsDDSToolStripMenuItem
            // 
            this->saveAsDDSToolStripMenuItem->Name = L"saveAsDDSToolStripMenuItem";
            this->saveAsDDSToolStripMenuItem->Size = System::Drawing::Size(183, 22);
            this->saveAsDDSToolStripMenuItem->Text = L"Save as DDS...";
            this->saveAsDDSToolStripMenuItem->Click += gcnew System::EventHandler(this, &MainForm::saveAsDDSToolStripMenuItem_Click);
            // 
            // saveAsLegacyDDSToolStripMenuItem
            // 
            this->saveAsLegacyDDSToolStripMenuItem->Name = L"saveAsLegacyDDSToolStripMenuItem";
            this->saveAsLegacyDDSToolStripMenuItem->Size = System::Drawing::Size(183, 22);
            this->saveAsLegacyDDSToolStripMenuItem->Text = L"Save as legacy DDS...";
            this->saveAsLegacyDDSToolStripMenuItem->Click += gcnew System::EventHandler(this, &MainForm::saveAsLegacyDDSToolStripMenuItem_Click);
            // 
            // saveAsTGAToolStripMenuItem
            // 
            this->saveAsTGAToolStripMenuItem->Name = L"saveAsTGAToolStripMenuItem";
            this->saveAsTGAToolStripMenuItem->Size = System::Drawing::Size(183, 22);
            this->saveAsTGAToolStripMenuItem->Text = L"Save as TGA...";
            this->saveAsTGAToolStripMenuItem->Click += gcnew System::EventHandler(this, &MainForm::saveAsTGAToolStripMenuItem_Click);
            // 
            // saveAsPNGToolStripMenuItem
            // 
            this->saveAsPNGToolStripMenuItem->Name = L"saveAsPNGToolStripMenuItem";
            this->saveAsPNGToolStripMenuItem->Size = System::Drawing::Size(183, 22);
            this->saveAsPNGToolStripMenuItem->Text = L"Save as PNG...";
            this->saveAsPNGToolStripMenuItem->Click += gcnew System::EventHandler(this, &MainForm::saveAsPNGToolStripMenuItem_Click);
            // 
            // ctxMenuExportModel
            // 
            this->ctxMenuExportModel->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(2) {
                this->saveAsOBJToolStripMenuItem,
                    this->saveAsFBXToolStripMenuItem
            });
            this->ctxMenuExportModel->Name = L"ctxMenuExportModel";
            this->ctxMenuExportModel->Size = System::Drawing::Size(145, 48);
            // 
            // saveAsOBJToolStripMenuItem
            // 
            this->saveAsOBJToolStripMenuItem->Name = L"saveAsOBJToolStripMenuItem";
            this->saveAsOBJToolStripMenuItem->Size = System::Drawing::Size(144, 22);
            this->saveAsOBJToolStripMenuItem->Text = L"Save as OBJ...";
            this->saveAsOBJToolStripMenuItem->Click += gcnew System::EventHandler(this, &MainForm::saveAsOBJToolStripMenuItem_Click);
            // 
            // saveAsFBXToolStripMenuItem
            // 
            this->saveAsFBXToolStripMenuItem->Name = L"saveAsFBXToolStripMenuItem";
            this->saveAsFBXToolStripMenuItem->Size = System::Drawing::Size(144, 22);
            this->saveAsFBXToolStripMenuItem->Text = L"Save as FBX...";
            this->saveAsFBXToolStripMenuItem->Click += gcnew System::EventHandler(this, &MainForm::saveAsFBXToolStripMenuItem_Click);
            // 
            // ctxMenuExportSound
            // 
            this->ctxMenuExportSound->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(2) {
                this->saveAsOGGToolStripMenuItem,
                    this->saveAsWAVToolStripMenuItem
            });
            this->ctxMenuExportSound->Name = L"ctxMenuExportSound";
            this->ctxMenuExportSound->Size = System::Drawing::Size(150, 48);
            // 
            // saveAsOGGToolStripMenuItem
            // 
            this->saveAsOGGToolStripMenuItem->Name = L"saveAsOGGToolStripMenuItem";
            this->saveAsOGGToolStripMenuItem->Size = System::Drawing::Size(149, 22);
            this->saveAsOGGToolStripMenuItem->Text = L"Save as OGG...";
            this->saveAsOGGToolStripMenuItem->Click += gcnew System::EventHandler(this, &MainForm::saveAsOGGToolStripMenuItem_Click);
            // 
            // saveAsWAVToolStripMenuItem
            // 
            this->saveAsWAVToolStripMenuItem->Name = L"saveAsWAVToolStripMenuItem";
            this->saveAsWAVToolStripMenuItem->Size = System::Drawing::Size(149, 22);
            this->saveAsWAVToolStripMenuItem->Text = L"Save as WAV...";
            this->saveAsWAVToolStripMenuItem->Click += gcnew System::EventHandler(this, &MainForm::saveAsWAVToolStripMenuItem_Click);
            // 
            // ctxMenuExportRaw
            // 
            this->ctxMenuExportRaw->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(1) { this->extractFileToolStripMenuItem });
            this->ctxMenuExportRaw->Name = L"ctxMenuExportRaw";
            this->ctxMenuExportRaw->Size = System::Drawing::Size(138, 26);
            // 
            // extractFileToolStripMenuItem
            // 
            this->extractFileToolStripMenuItem->Name = L"extractFileToolStripMenuItem";
            this->extractFileToolStripMenuItem->Size = System::Drawing::Size(137, 22);
            this->extractFileToolStripMenuItem->Text = L"Extract file...";
            this->extractFileToolStripMenuItem->Click += gcnew System::EventHandler(this, &MainForm::extractFileToolStripMenuItem_Click);
            // 
            // ctxMenuExportFolder
            // 
            this->ctxMenuExportFolder->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(2) {
                this->extractFolderToolStripMenuItem,
                    this->extractFolderWithConversionToolStripMenuItem
            });
            this->ctxMenuExportFolder->Name = L"ctxMenuExportFolder";
            this->ctxMenuExportFolder->Size = System::Drawing::Size(240, 48);
            // 
            // extractFolderToolStripMenuItem
            // 
            this->extractFolderToolStripMenuItem->Name = L"extractFolderToolStripMenuItem";
            this->extractFolderToolStripMenuItem->Size = System::Drawing::Size(239, 22);
            this->extractFolderToolStripMenuItem->Text = L"Extract folder...";
            this->extractFolderToolStripMenuItem->Click += gcnew System::EventHandler(this, &MainForm::extractFolderToolStripMenuItem_Click);
            // 
            // extractFolderWithConversionToolStripMenuItem
            // 
            this->extractFolderWithConversionToolStripMenuItem->Name = L"extractFolderWithConversionToolStripMenuItem";
            this->extractFolderWithConversionToolStripMenuItem->Size = System::Drawing::Size(239, 22);
            this->extractFolderWithConversionToolStripMenuItem->Text = L"Extract folder with conversion...";
            this->extractFolderWithConversionToolStripMenuItem->Click += gcnew System::EventHandler(this, &MainForm::extractFolderWithConversionToolStripMenuItem_Click);
            // 
            // toolStrip1
            // 
            this->toolStrip1->GripStyle = System::Windows::Forms::ToolStripGripStyle::Hidden;
            this->toolStrip1->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(5) {
                this->toolBtnFileOpen,
                    this->toolStripSeparator1, this->toolBtnAbout, this->toolStripSeparator2, this->toolBtnImgEnableAlpha
            });
            this->toolStrip1->Location = System::Drawing::Point(0, 0);
            this->toolStrip1->Name = L"toolStrip1";
            this->toolStrip1->Size = System::Drawing::Size(1159, 25);
            this->toolStrip1->TabIndex = 5;
            this->toolStrip1->Text = L"toolStrip1";
            // 
            // toolBtnFileOpen
            // 
            this->toolBtnFileOpen->DisplayStyle = System::Windows::Forms::ToolStripItemDisplayStyle::Image;
            this->toolBtnFileOpen->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"toolBtnFileOpen.Image")));
            this->toolBtnFileOpen->ImageTransparentColor = System::Drawing::Color::Magenta;
            this->toolBtnFileOpen->Name = L"toolBtnFileOpen";
            this->toolBtnFileOpen->Size = System::Drawing::Size(23, 22);
            this->toolBtnFileOpen->ToolTipText = L"Open Metro Exodus archive...";
            this->toolBtnFileOpen->Click += gcnew System::EventHandler(this, &MainForm::toolBtnFileOpen_Click);
            // 
            // toolStripSeparator1
            // 
            this->toolStripSeparator1->Name = L"toolStripSeparator1";
            this->toolStripSeparator1->Size = System::Drawing::Size(6, 25);
            // 
            // toolBtnAbout
            // 
            this->toolBtnAbout->DisplayStyle = System::Windows::Forms::ToolStripItemDisplayStyle::Image;
            this->toolBtnAbout->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"toolBtnAbout.Image")));
            this->toolBtnAbout->ImageTransparentColor = System::Drawing::Color::Magenta;
            this->toolBtnAbout->Name = L"toolBtnAbout";
            this->toolBtnAbout->Size = System::Drawing::Size(23, 22);
            this->toolBtnAbout->ToolTipText = L"About";
            this->toolBtnAbout->Click += gcnew System::EventHandler(this, &MainForm::toolBtnAbout_Click);
            // 
            // toolStripSeparator2
            // 
            this->toolStripSeparator2->Name = L"toolStripSeparator2";
            this->toolStripSeparator2->Size = System::Drawing::Size(6, 25);
            // 
            // toolBtnImgEnableAlpha
            // 
            this->toolBtnImgEnableAlpha->Checked = true;
            this->toolBtnImgEnableAlpha->CheckState = System::Windows::Forms::CheckState::Checked;
            this->toolBtnImgEnableAlpha->DisplayStyle = System::Windows::Forms::ToolStripItemDisplayStyle::Text;
            this->toolBtnImgEnableAlpha->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9, static_cast<System::Drawing::FontStyle>((System::Drawing::FontStyle::Bold | System::Drawing::FontStyle::Underline))));
            this->toolBtnImgEnableAlpha->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"toolBtnImgEnableAlpha.Image")));
            this->toolBtnImgEnableAlpha->ImageTransparentColor = System::Drawing::Color::Magenta;
            this->toolBtnImgEnableAlpha->Name = L"toolBtnImgEnableAlpha";
            this->toolBtnImgEnableAlpha->Size = System::Drawing::Size(23, 22);
            this->toolBtnImgEnableAlpha->Text = L"A";
            this->toolBtnImgEnableAlpha->ToolTipText = L"Enable alpha";
            this->toolBtnImgEnableAlpha->Click += gcnew System::EventHandler(this, &MainForm::toolBtnImgEnableAlpha_Click);
            // 
            // filterTimer
            // 
            this->filterTimer->Interval = 1000;
            this->filterTimer->Tick += gcnew System::EventHandler(this, &MainForm::filterTimer_Tick);
            // 
            // MainForm
            // 
            this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
            this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
            this->ClientSize = System::Drawing::Size(1159, 719);
            this->Controls->Add(this->splitContainer1);
            this->Controls->Add(this->toolStrip1);
            this->Controls->Add(this->statusStrip1);
            this->Name = L"MainForm";
            this->StartPosition = System::Windows::Forms::FormStartPosition::CenterScreen;
            this->Text = L"MetroEX - \"Metro Exodus\" files explorer (by iOrange)";
            this->Load += gcnew System::EventHandler(this, &MainForm::MainForm_Load);
            this->statusStrip1->ResumeLayout(false);
            this->statusStrip1->PerformLayout();
            this->splitContainer1->Panel1->ResumeLayout(false);
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->splitContainer1))->EndInit();
            this->splitContainer1->ResumeLayout(false);
            this->tableLayoutPanel1->ResumeLayout(false);
            this->tableLayoutPanel1->PerformLayout();
            this->ctxMenuExportTexture->ResumeLayout(false);
            this->ctxMenuExportModel->ResumeLayout(false);
            this->ctxMenuExportSound->ResumeLayout(false);
            this->ctxMenuExportRaw->ResumeLayout(false);
            this->ctxMenuExportFolder->ResumeLayout(false);
            this->toolStrip1->ResumeLayout(false);
            this->toolStrip1->PerformLayout();
            this->ResumeLayout(false);
            this->PerformLayout();

        }
#pragma endregion

    private:
        void MainForm_Load(System::Object^ sender, System::EventArgs^ e);
        // toolstrip buttons
        void toolBtnFileOpen_Click(System::Object^ sender, System::EventArgs^ e);
        void toolBtnAbout_Click(System::Object^ sender, System::EventArgs^ e);
        void toolBtnImgEnableAlpha_Click(System::Object^  sender, System::EventArgs^ e);
        // treeview
        void treeView1_AfterSelect(System::Object^ sender, System::Windows::Forms::TreeViewEventArgs^ e);
        void treeView1_AfterCollapse(System::Object^ sender, System::Windows::Forms::TreeViewEventArgs^ e);
        void treeView1_AfterExpand(System::Object^ sender, System::Windows::Forms::TreeViewEventArgs^ e);
        void treeView1_NodeMouseClick(System::Object^ sender, System::Windows::Forms::TreeNodeMouseClickEventArgs^ e);
        // context menu
        void extractFileToolStripMenuItem_Click(System::Object^ sender, System::EventArgs^ e);
        void saveAsDDSToolStripMenuItem_Click(System::Object^ sender, System::EventArgs^ e);
        void saveAsLegacyDDSToolStripMenuItem_Click(System::Object^ sender, System::EventArgs^ e);
        void saveAsTGAToolStripMenuItem_Click(System::Object^ sender, System::EventArgs^ e);
        void saveAsPNGToolStripMenuItem_Click(System::Object^ sender, System::EventArgs^ e);
        void saveAsOBJToolStripMenuItem_Click(System::Object^ sender, System::EventArgs^ e);
        void saveAsFBXToolStripMenuItem_Click(System::Object^ sender, System::EventArgs^ e);
        void saveAsOGGToolStripMenuItem_Click(System::Object^ sender, System::EventArgs^ e);
        void saveAsWAVToolStripMenuItem_Click(System::Object^ sender, System::EventArgs^ e);
        void extractFolderToolStripMenuItem_Click(System::Object^ sender, System::EventArgs^ e);
        void extractFolderWithConversionToolStripMenuItem_Click(System::Object^ sender, System::EventArgs^ e);

    private:
        void ShowErrorMessage(String^ message);
        //
        void UpdateFilesList();
        void AddFoldersRecursive(const MetroFile& dir, const size_t folderIdx, TreeNode^ rootItem);
        void DetectFileAndShow(const size_t fileIdx);
        void ShowTexture(const size_t fileIdx);
        void ShowModel(const size_t fileIdx);
        void ShowSound(const size_t fileIdx);

        // extraction
        CharString MakeFileOutputName(const MetroFile& mf, const FileExtractionCtx& ctx);
        void TextureSaveHelper(const fs::path& folderPath, const FileExtractionCtx& ctx, const CharString& name);
        bool ExtractFile(const FileExtractionCtx& ctx, const fs::path& outPath);
        bool ExtractTexture(const FileExtractionCtx& ctx, const fs::path& outPath);
        bool ExtractModel(const FileExtractionCtx& ctx, const fs::path& outPath);
        bool ExtractSound(const FileExtractionCtx& ctx, const fs::path& outPath);
        bool ExtractFolderComplete(const FileExtractionCtx& ctx, const fs::path& outPath);
        void ExtractionProcessFunc(Object^ folderPath);

        // filter
        void txtTreeSearch_TextChanged(System::Object^ sender, System::EventArgs^ e);
        bool FilterTreeView(TreeNode^ node, String^ text);
        void filterTimer_Tick(System::Object^ sender, System::EventArgs^ e);
};
}
