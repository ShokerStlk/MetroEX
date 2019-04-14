#pragma once

class MetroModel;

namespace MetroEX {
    using namespace System;
    using namespace System::ComponentModel;
    using namespace System::Collections;
    using namespace System::Windows::Forms;
    using namespace System::Data;
    using namespace System::Drawing;

    public ref class DlgModelInfo : public System::Windows::Forms::Form {
        ref struct MeshInfo {
            String^ name;
            String^ texturePath;
            String^ material1;
            String^ material2;
            String^ material3;
            int     numVertices;
            int     numFaces;
            int     numBones;
        };

        ref struct MotionInfo {
            String^ name;
            String^ path;
            float   duration;
        };

        ref struct ModelInfo {
            String^                     skeletonPath;
            int                         numBones;
            Generic::List<MeshInfo^>    meshes;
            Generic::List<MotionInfo^>  motions;
        };

    public:
        DlgModelInfo() {
            mModelInfo = nullptr;
            InitializeComponent();
        }

        void SetModel(MetroModel* model);
        void UpdateUI();

    protected:
        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        ~DlgModelInfo() {
            if (components) {
                delete components;
            }
        }

    private:
        ModelInfo^  mModelInfo;



    private: System::Windows::Forms::TableLayoutPanel^  tableLayoutPanel1;

    private: System::Windows::Forms::Panel^  panel2;

    private: System::Windows::Forms::Panel^  panel3;
    private: System::Windows::Forms::GroupBox^  groupBox1;
    private: System::Windows::Forms::SplitContainer^  splitContainer1;
    private: System::Windows::Forms::ListBox^  lstMeshes;
    private: System::Windows::Forms::TableLayoutPanel^  tableLayoutPanel2;
    private: System::Windows::Forms::Panel^  panel4;
    private: System::Windows::Forms::TextBox^  txtMeshTexture;

    private: System::Windows::Forms::Label^  label1;
    private: System::Windows::Forms::Panel^  panel5;
    private: System::Windows::Forms::Label^  lblMeshVertices;
    private: System::Windows::Forms::Label^  lblMeshFaces;
    private: System::Windows::Forms::Panel^  panel6;
    private: System::Windows::Forms::TextBox^  txtSkeletonPath;
    private: System::Windows::Forms::Label^  label2;
    private: System::Windows::Forms::GroupBox^  groupBox2;
    private: System::Windows::Forms::SplitContainer^  splitContainer2;
    private: System::Windows::Forms::ListBox^  lstMotions;
    private: System::Windows::Forms::TableLayoutPanel^  tableLayoutPanel3;
    private: System::Windows::Forms::Panel^  panel7;
    private: System::Windows::Forms::TextBox^  txtMotionPath;
    private: System::Windows::Forms::Label^  label3;
    private: System::Windows::Forms::Panel^  panel8;
    private: System::Windows::Forms::Label^  lblMotionLength;
    private: System::Windows::Forms::Label^  lblNumBones;
    private: System::Windows::Forms::TextBox^  txtMeshMaterials;

    private: System::Windows::Forms::Label^  label4;


    protected:

    private:
        /// <summary>
        /// Required designer variable.
        /// </summary>
        System::ComponentModel::Container ^components;

#pragma region Windows Form Designer generated code
        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        void InitializeComponent(void)
        {
            this->tableLayoutPanel1 = (gcnew System::Windows::Forms::TableLayoutPanel());
            this->panel2 = (gcnew System::Windows::Forms::Panel());
            this->groupBox1 = (gcnew System::Windows::Forms::GroupBox());
            this->splitContainer1 = (gcnew System::Windows::Forms::SplitContainer());
            this->lstMeshes = (gcnew System::Windows::Forms::ListBox());
            this->tableLayoutPanel2 = (gcnew System::Windows::Forms::TableLayoutPanel());
            this->panel4 = (gcnew System::Windows::Forms::Panel());
            this->txtMeshMaterials = (gcnew System::Windows::Forms::TextBox());
            this->label4 = (gcnew System::Windows::Forms::Label());
            this->txtMeshTexture = (gcnew System::Windows::Forms::TextBox());
            this->label1 = (gcnew System::Windows::Forms::Label());
            this->panel5 = (gcnew System::Windows::Forms::Panel());
            this->lblMeshFaces = (gcnew System::Windows::Forms::Label());
            this->lblMeshVertices = (gcnew System::Windows::Forms::Label());
            this->panel6 = (gcnew System::Windows::Forms::Panel());
            this->lblNumBones = (gcnew System::Windows::Forms::Label());
            this->txtSkeletonPath = (gcnew System::Windows::Forms::TextBox());
            this->label2 = (gcnew System::Windows::Forms::Label());
            this->panel3 = (gcnew System::Windows::Forms::Panel());
            this->groupBox2 = (gcnew System::Windows::Forms::GroupBox());
            this->splitContainer2 = (gcnew System::Windows::Forms::SplitContainer());
            this->lstMotions = (gcnew System::Windows::Forms::ListBox());
            this->tableLayoutPanel3 = (gcnew System::Windows::Forms::TableLayoutPanel());
            this->panel7 = (gcnew System::Windows::Forms::Panel());
            this->txtMotionPath = (gcnew System::Windows::Forms::TextBox());
            this->label3 = (gcnew System::Windows::Forms::Label());
            this->panel8 = (gcnew System::Windows::Forms::Panel());
            this->lblMotionLength = (gcnew System::Windows::Forms::Label());
            this->tableLayoutPanel1->SuspendLayout();
            this->panel2->SuspendLayout();
            this->groupBox1->SuspendLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->splitContainer1))->BeginInit();
            this->splitContainer1->Panel1->SuspendLayout();
            this->splitContainer1->Panel2->SuspendLayout();
            this->splitContainer1->SuspendLayout();
            this->tableLayoutPanel2->SuspendLayout();
            this->panel4->SuspendLayout();
            this->panel5->SuspendLayout();
            this->panel6->SuspendLayout();
            this->panel3->SuspendLayout();
            this->groupBox2->SuspendLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->splitContainer2))->BeginInit();
            this->splitContainer2->Panel1->SuspendLayout();
            this->splitContainer2->Panel2->SuspendLayout();
            this->splitContainer2->SuspendLayout();
            this->tableLayoutPanel3->SuspendLayout();
            this->panel7->SuspendLayout();
            this->panel8->SuspendLayout();
            this->SuspendLayout();
            // 
            // tableLayoutPanel1
            // 
            this->tableLayoutPanel1->ColumnCount = 1;
            this->tableLayoutPanel1->ColumnStyles->Add((gcnew System::Windows::Forms::ColumnStyle(System::Windows::Forms::SizeType::Percent,
                100)));
            this->tableLayoutPanel1->Controls->Add(this->panel2, 0, 0);
            this->tableLayoutPanel1->Controls->Add(this->panel3, 0, 1);
            this->tableLayoutPanel1->Dock = System::Windows::Forms::DockStyle::Fill;
            this->tableLayoutPanel1->Location = System::Drawing::Point(0, 0);
            this->tableLayoutPanel1->Name = L"tableLayoutPanel1";
            this->tableLayoutPanel1->RowCount = 2;
            this->tableLayoutPanel1->RowStyles->Add((gcnew System::Windows::Forms::RowStyle(System::Windows::Forms::SizeType::Percent, 50)));
            this->tableLayoutPanel1->RowStyles->Add((gcnew System::Windows::Forms::RowStyle(System::Windows::Forms::SizeType::Percent, 50)));
            this->tableLayoutPanel1->RowStyles->Add((gcnew System::Windows::Forms::RowStyle(System::Windows::Forms::SizeType::Absolute, 20)));
            this->tableLayoutPanel1->Size = System::Drawing::Size(1036, 732);
            this->tableLayoutPanel1->TabIndex = 0;
            // 
            // panel2
            // 
            this->panel2->Controls->Add(this->groupBox1);
            this->panel2->Dock = System::Windows::Forms::DockStyle::Fill;
            this->panel2->Location = System::Drawing::Point(3, 3);
            this->panel2->Name = L"panel2";
            this->panel2->Size = System::Drawing::Size(1030, 360);
            this->panel2->TabIndex = 1;
            // 
            // groupBox1
            // 
            this->groupBox1->Controls->Add(this->splitContainer1);
            this->groupBox1->Dock = System::Windows::Forms::DockStyle::Fill;
            this->groupBox1->Location = System::Drawing::Point(0, 0);
            this->groupBox1->Margin = System::Windows::Forms::Padding(0);
            this->groupBox1->Name = L"groupBox1";
            this->groupBox1->Size = System::Drawing::Size(1030, 360);
            this->groupBox1->TabIndex = 0;
            this->groupBox1->TabStop = false;
            this->groupBox1->Text = L"Meshes:";
            // 
            // splitContainer1
            // 
            this->splitContainer1->Dock = System::Windows::Forms::DockStyle::Fill;
            this->splitContainer1->Location = System::Drawing::Point(3, 16);
            this->splitContainer1->Margin = System::Windows::Forms::Padding(0);
            this->splitContainer1->Name = L"splitContainer1";
            // 
            // splitContainer1.Panel1
            // 
            this->splitContainer1->Panel1->Controls->Add(this->lstMeshes);
            // 
            // splitContainer1.Panel2
            // 
            this->splitContainer1->Panel2->Controls->Add(this->tableLayoutPanel2);
            this->splitContainer1->Size = System::Drawing::Size(1024, 341);
            this->splitContainer1->SplitterDistance = 341;
            this->splitContainer1->TabIndex = 0;
            // 
            // lstMeshes
            // 
            this->lstMeshes->Dock = System::Windows::Forms::DockStyle::Fill;
            this->lstMeshes->FormattingEnabled = true;
            this->lstMeshes->Location = System::Drawing::Point(0, 0);
            this->lstMeshes->Margin = System::Windows::Forms::Padding(0);
            this->lstMeshes->Name = L"lstMeshes";
            this->lstMeshes->Size = System::Drawing::Size(341, 341);
            this->lstMeshes->TabIndex = 0;
            this->lstMeshes->SelectedIndexChanged += gcnew System::EventHandler(this, &DlgModelInfo::lstMeshes_SelectedIndexChanged);
            // 
            // tableLayoutPanel2
            // 
            this->tableLayoutPanel2->ColumnCount = 1;
            this->tableLayoutPanel2->ColumnStyles->Add((gcnew System::Windows::Forms::ColumnStyle(System::Windows::Forms::SizeType::Percent,
                100)));
            this->tableLayoutPanel2->ColumnStyles->Add((gcnew System::Windows::Forms::ColumnStyle(System::Windows::Forms::SizeType::Absolute,
                20)));
            this->tableLayoutPanel2->Controls->Add(this->panel4, 0, 0);
            this->tableLayoutPanel2->Controls->Add(this->panel5, 0, 1);
            this->tableLayoutPanel2->Controls->Add(this->panel6, 0, 2);
            this->tableLayoutPanel2->Dock = System::Windows::Forms::DockStyle::Fill;
            this->tableLayoutPanel2->Location = System::Drawing::Point(0, 0);
            this->tableLayoutPanel2->Margin = System::Windows::Forms::Padding(0);
            this->tableLayoutPanel2->Name = L"tableLayoutPanel2";
            this->tableLayoutPanel2->RowCount = 4;
            this->tableLayoutPanel2->RowStyles->Add((gcnew System::Windows::Forms::RowStyle(System::Windows::Forms::SizeType::Percent, 25)));
            this->tableLayoutPanel2->RowStyles->Add((gcnew System::Windows::Forms::RowStyle(System::Windows::Forms::SizeType::Percent, 25)));
            this->tableLayoutPanel2->RowStyles->Add((gcnew System::Windows::Forms::RowStyle(System::Windows::Forms::SizeType::Percent, 25)));
            this->tableLayoutPanel2->RowStyles->Add((gcnew System::Windows::Forms::RowStyle(System::Windows::Forms::SizeType::Percent, 25)));
            this->tableLayoutPanel2->Size = System::Drawing::Size(679, 341);
            this->tableLayoutPanel2->TabIndex = 0;
            // 
            // panel4
            // 
            this->panel4->Controls->Add(this->txtMeshMaterials);
            this->panel4->Controls->Add(this->label4);
            this->panel4->Controls->Add(this->txtMeshTexture);
            this->panel4->Controls->Add(this->label1);
            this->panel4->Dock = System::Windows::Forms::DockStyle::Fill;
            this->panel4->Location = System::Drawing::Point(0, 0);
            this->panel4->Margin = System::Windows::Forms::Padding(0);
            this->panel4->Name = L"panel4";
            this->panel4->Size = System::Drawing::Size(679, 85);
            this->panel4->TabIndex = 0;
            // 
            // txtMeshMaterials
            // 
            this->txtMeshMaterials->Dock = System::Windows::Forms::DockStyle::Top;
            this->txtMeshMaterials->Location = System::Drawing::Point(0, 46);
            this->txtMeshMaterials->Name = L"txtMeshMaterials";
            this->txtMeshMaterials->Size = System::Drawing::Size(679, 20);
            this->txtMeshMaterials->TabIndex = 3;
            // 
            // label4
            // 
            this->label4->AutoSize = true;
            this->label4->Dock = System::Windows::Forms::DockStyle::Top;
            this->label4->Location = System::Drawing::Point(0, 33);
            this->label4->Name = L"label4";
            this->label4->Size = System::Drawing::Size(36, 13);
            this->label4->TabIndex = 2;
            this->label4->Text = L"Other:";
            // 
            // txtMeshTexture
            // 
            this->txtMeshTexture->Dock = System::Windows::Forms::DockStyle::Top;
            this->txtMeshTexture->Location = System::Drawing::Point(0, 13);
            this->txtMeshTexture->Name = L"txtMeshTexture";
            this->txtMeshTexture->ReadOnly = true;
            this->txtMeshTexture->Size = System::Drawing::Size(679, 20);
            this->txtMeshTexture->TabIndex = 1;
            // 
            // label1
            // 
            this->label1->AutoSize = true;
            this->label1->Dock = System::Windows::Forms::DockStyle::Top;
            this->label1->Location = System::Drawing::Point(0, 0);
            this->label1->Margin = System::Windows::Forms::Padding(3, 2, 3, 2);
            this->label1->Name = L"label1";
            this->label1->Size = System::Drawing::Size(46, 13);
            this->label1->TabIndex = 0;
            this->label1->Text = L"Texture:";
            // 
            // panel5
            // 
            this->panel5->Controls->Add(this->lblMeshFaces);
            this->panel5->Controls->Add(this->lblMeshVertices);
            this->panel5->Dock = System::Windows::Forms::DockStyle::Fill;
            this->panel5->Location = System::Drawing::Point(0, 85);
            this->panel5->Margin = System::Windows::Forms::Padding(0);
            this->panel5->Name = L"panel5";
            this->panel5->Size = System::Drawing::Size(679, 85);
            this->panel5->TabIndex = 1;
            // 
            // lblMeshFaces
            // 
            this->lblMeshFaces->AutoSize = true;
            this->lblMeshFaces->Location = System::Drawing::Point(4, 20);
            this->lblMeshFaces->Name = L"lblMeshFaces";
            this->lblMeshFaces->Size = System::Drawing::Size(86, 13);
            this->lblMeshFaces->TabIndex = 1;
            this->lblMeshFaces->Text = L"Triangles: 65535";
            // 
            // lblMeshVertices
            // 
            this->lblMeshVertices->AutoSize = true;
            this->lblMeshVertices->Location = System::Drawing::Point(4, 4);
            this->lblMeshVertices->Name = L"lblMeshVertices";
            this->lblMeshVertices->Size = System::Drawing::Size(81, 13);
            this->lblMeshVertices->TabIndex = 0;
            this->lblMeshVertices->Text = L"Vertices: 65535";
            // 
            // panel6
            // 
            this->panel6->Controls->Add(this->lblNumBones);
            this->panel6->Controls->Add(this->txtSkeletonPath);
            this->panel6->Controls->Add(this->label2);
            this->panel6->Dock = System::Windows::Forms::DockStyle::Fill;
            this->panel6->Location = System::Drawing::Point(0, 170);
            this->panel6->Margin = System::Windows::Forms::Padding(0);
            this->panel6->Name = L"panel6";
            this->panel6->Size = System::Drawing::Size(679, 85);
            this->panel6->TabIndex = 2;
            // 
            // lblNumBones
            // 
            this->lblNumBones->AutoSize = true;
            this->lblNumBones->Location = System::Drawing::Point(3, 40);
            this->lblNumBones->Name = L"lblNumBones";
            this->lblNumBones->Size = System::Drawing::Size(49, 13);
            this->lblNumBones->TabIndex = 2;
            this->lblNumBones->Text = L"Bones: 0";
            // 
            // txtSkeletonPath
            // 
            this->txtSkeletonPath->Dock = System::Windows::Forms::DockStyle::Top;
            this->txtSkeletonPath->Location = System::Drawing::Point(0, 13);
            this->txtSkeletonPath->Name = L"txtSkeletonPath";
            this->txtSkeletonPath->ReadOnly = true;
            this->txtSkeletonPath->Size = System::Drawing::Size(679, 20);
            this->txtSkeletonPath->TabIndex = 1;
            // 
            // label2
            // 
            this->label2->AutoSize = true;
            this->label2->Dock = System::Windows::Forms::DockStyle::Top;
            this->label2->Location = System::Drawing::Point(0, 0);
            this->label2->Name = L"label2";
            this->label2->Size = System::Drawing::Size(52, 13);
            this->label2->TabIndex = 0;
            this->label2->Text = L"Skeleton:";
            // 
            // panel3
            // 
            this->panel3->Controls->Add(this->groupBox2);
            this->panel3->Dock = System::Windows::Forms::DockStyle::Fill;
            this->panel3->Location = System::Drawing::Point(3, 369);
            this->panel3->Name = L"panel3";
            this->panel3->Size = System::Drawing::Size(1030, 360);
            this->panel3->TabIndex = 2;
            // 
            // groupBox2
            // 
            this->groupBox2->Controls->Add(this->splitContainer2);
            this->groupBox2->Dock = System::Windows::Forms::DockStyle::Fill;
            this->groupBox2->Location = System::Drawing::Point(0, 0);
            this->groupBox2->Margin = System::Windows::Forms::Padding(0);
            this->groupBox2->Name = L"groupBox2";
            this->groupBox2->Size = System::Drawing::Size(1030, 360);
            this->groupBox2->TabIndex = 0;
            this->groupBox2->TabStop = false;
            this->groupBox2->Text = L"Motions:";
            // 
            // splitContainer2
            // 
            this->splitContainer2->Dock = System::Windows::Forms::DockStyle::Fill;
            this->splitContainer2->Location = System::Drawing::Point(3, 16);
            this->splitContainer2->Margin = System::Windows::Forms::Padding(0);
            this->splitContainer2->Name = L"splitContainer2";
            // 
            // splitContainer2.Panel1
            // 
            this->splitContainer2->Panel1->Controls->Add(this->lstMotions);
            // 
            // splitContainer2.Panel2
            // 
            this->splitContainer2->Panel2->Controls->Add(this->tableLayoutPanel3);
            this->splitContainer2->Size = System::Drawing::Size(1024, 341);
            this->splitContainer2->SplitterDistance = 341;
            this->splitContainer2->TabIndex = 0;
            // 
            // lstMotions
            // 
            this->lstMotions->Dock = System::Windows::Forms::DockStyle::Fill;
            this->lstMotions->FormattingEnabled = true;
            this->lstMotions->Location = System::Drawing::Point(0, 0);
            this->lstMotions->Margin = System::Windows::Forms::Padding(0);
            this->lstMotions->Name = L"lstMotions";
            this->lstMotions->Size = System::Drawing::Size(341, 341);
            this->lstMotions->TabIndex = 0;
            this->lstMotions->SelectedIndexChanged += gcnew System::EventHandler(this, &DlgModelInfo::lstMotions_SelectedIndexChanged);
            // 
            // tableLayoutPanel3
            // 
            this->tableLayoutPanel3->ColumnCount = 1;
            this->tableLayoutPanel3->ColumnStyles->Add((gcnew System::Windows::Forms::ColumnStyle(System::Windows::Forms::SizeType::Percent,
                100)));
            this->tableLayoutPanel3->ColumnStyles->Add((gcnew System::Windows::Forms::ColumnStyle(System::Windows::Forms::SizeType::Absolute,
                20)));
            this->tableLayoutPanel3->Controls->Add(this->panel7, 0, 0);
            this->tableLayoutPanel3->Controls->Add(this->panel8, 0, 1);
            this->tableLayoutPanel3->Dock = System::Windows::Forms::DockStyle::Fill;
            this->tableLayoutPanel3->Location = System::Drawing::Point(0, 0);
            this->tableLayoutPanel3->Margin = System::Windows::Forms::Padding(0);
            this->tableLayoutPanel3->Name = L"tableLayoutPanel3";
            this->tableLayoutPanel3->RowCount = 4;
            this->tableLayoutPanel3->RowStyles->Add((gcnew System::Windows::Forms::RowStyle(System::Windows::Forms::SizeType::Percent, 25)));
            this->tableLayoutPanel3->RowStyles->Add((gcnew System::Windows::Forms::RowStyle(System::Windows::Forms::SizeType::Percent, 25)));
            this->tableLayoutPanel3->RowStyles->Add((gcnew System::Windows::Forms::RowStyle(System::Windows::Forms::SizeType::Percent, 25)));
            this->tableLayoutPanel3->RowStyles->Add((gcnew System::Windows::Forms::RowStyle(System::Windows::Forms::SizeType::Percent, 25)));
            this->tableLayoutPanel3->Size = System::Drawing::Size(679, 341);
            this->tableLayoutPanel3->TabIndex = 0;
            // 
            // panel7
            // 
            this->panel7->Controls->Add(this->txtMotionPath);
            this->panel7->Controls->Add(this->label3);
            this->panel7->Dock = System::Windows::Forms::DockStyle::Fill;
            this->panel7->Location = System::Drawing::Point(0, 0);
            this->panel7->Margin = System::Windows::Forms::Padding(0);
            this->panel7->Name = L"panel7";
            this->panel7->Size = System::Drawing::Size(679, 85);
            this->panel7->TabIndex = 0;
            // 
            // txtMotionPath
            // 
            this->txtMotionPath->Dock = System::Windows::Forms::DockStyle::Top;
            this->txtMotionPath->Location = System::Drawing::Point(0, 13);
            this->txtMotionPath->Name = L"txtMotionPath";
            this->txtMotionPath->ReadOnly = true;
            this->txtMotionPath->Size = System::Drawing::Size(679, 20);
            this->txtMotionPath->TabIndex = 1;
            // 
            // label3
            // 
            this->label3->AutoSize = true;
            this->label3->Dock = System::Windows::Forms::DockStyle::Top;
            this->label3->Location = System::Drawing::Point(0, 0);
            this->label3->Name = L"label3";
            this->label3->Size = System::Drawing::Size(32, 13);
            this->label3->TabIndex = 0;
            this->label3->Text = L"Path:";
            // 
            // panel8
            // 
            this->panel8->Controls->Add(this->lblMotionLength);
            this->panel8->Dock = System::Windows::Forms::DockStyle::Fill;
            this->panel8->Location = System::Drawing::Point(0, 85);
            this->panel8->Margin = System::Windows::Forms::Padding(0);
            this->panel8->Name = L"panel8";
            this->panel8->Size = System::Drawing::Size(679, 85);
            this->panel8->TabIndex = 1;
            // 
            // lblMotionLength
            // 
            this->lblMotionLength->AutoSize = true;
            this->lblMotionLength->Location = System::Drawing::Point(4, 4);
            this->lblMotionLength->Name = L"lblMotionLength";
            this->lblMotionLength->Size = System::Drawing::Size(101, 13);
            this->lblMotionLength->TabIndex = 0;
            this->lblMotionLength->Text = L"Length: 15 seconds";
            // 
            // DlgModelInfo
            // 
            this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
            this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
            this->ClientSize = System::Drawing::Size(1036, 732);
            this->Controls->Add(this->tableLayoutPanel1);
            this->Name = L"DlgModelInfo";
            this->Text = L"Metro model properties";
            this->tableLayoutPanel1->ResumeLayout(false);
            this->panel2->ResumeLayout(false);
            this->groupBox1->ResumeLayout(false);
            this->splitContainer1->Panel1->ResumeLayout(false);
            this->splitContainer1->Panel2->ResumeLayout(false);
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->splitContainer1))->EndInit();
            this->splitContainer1->ResumeLayout(false);
            this->tableLayoutPanel2->ResumeLayout(false);
            this->panel4->ResumeLayout(false);
            this->panel4->PerformLayout();
            this->panel5->ResumeLayout(false);
            this->panel5->PerformLayout();
            this->panel6->ResumeLayout(false);
            this->panel6->PerformLayout();
            this->panel3->ResumeLayout(false);
            this->groupBox2->ResumeLayout(false);
            this->splitContainer2->Panel1->ResumeLayout(false);
            this->splitContainer2->Panel2->ResumeLayout(false);
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->splitContainer2))->EndInit();
            this->splitContainer2->ResumeLayout(false);
            this->tableLayoutPanel3->ResumeLayout(false);
            this->panel7->ResumeLayout(false);
            this->panel7->PerformLayout();
            this->panel8->ResumeLayout(false);
            this->panel8->PerformLayout();
            this->ResumeLayout(false);

        }
#pragma endregion


    private:
        void lstMeshes_SelectedIndexChanged(System::Object^ sender, System::EventArgs^ e);
        void lstMotions_SelectedIndexChanged(System::Object^ sender, System::EventArgs^ e);
    };
}
