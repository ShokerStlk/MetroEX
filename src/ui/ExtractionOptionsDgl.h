#pragma once

namespace MetroEX {

    using namespace System;
    using namespace System::ComponentModel;
    using namespace System::Collections;
    using namespace System::Windows::Forms;
    using namespace System::Data;
    using namespace System::Drawing;

    public ref class ExtractionOptionsDgl : public System::Windows::Forms::Form
    {
    public:
        ExtractionOptionsDgl() {
            InitializeComponent();
        }

        // textures
        bool IsTexturesAsDds() {
            return radioTexAsDDS->Checked;
        }
        bool IsTexturesAsLegacyDds() {
            return radioTexAsLegacyDDS->Checked;
        }
        bool IsTexturesAsTga() {
            return radioTexAsTGA->Checked;
        }
        bool IsTexturesAsPng() {
            return radioTexAsPNG->Checked;
        }
        // models
        bool IsModelsAsObj() {
            return radioMdlAsOBJ->Checked;
        }
        bool IsModelsAsFbx() {
            return radioMdlAsFBX->Checked;
        }
        // sounds
        bool IsSoundsAsOgg() {
            return radioSndAsOGG->Checked;
        }
        bool IsSoundsAsWav() {
            return radioSndAsWAV->Checked;
        }

    protected:
        ~ExtractionOptionsDgl() {
            if (components) {
                delete components;
            }
        }
    private: System::Windows::Forms::GroupBox^  groupBox1;
    private: System::Windows::Forms::RadioButton^  radioTexAsDDS;
    private: System::Windows::Forms::RadioButton^  radioTexAsLegacyDDS;
    private: System::Windows::Forms::RadioButton^  radioTexAsPNG;
    private: System::Windows::Forms::RadioButton^  radioTexAsTGA;
    private: System::Windows::Forms::GroupBox^  groupBox2;
    private: System::Windows::Forms::RadioButton^  radioMdlAsFBX;
    private: System::Windows::Forms::RadioButton^  radioMdlAsOBJ;
    private: System::Windows::Forms::GroupBox^  groupBox3;
    private: System::Windows::Forms::RadioButton^  radioSndAsOGG;
    private: System::Windows::Forms::RadioButton^  radioSndAsWAV;
    private: System::Windows::Forms::Button^  btnCancel;
    private: System::Windows::Forms::Button^  btnOK;
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
            this->groupBox1 = (gcnew System::Windows::Forms::GroupBox());
            this->radioTexAsPNG = (gcnew System::Windows::Forms::RadioButton());
            this->radioTexAsTGA = (gcnew System::Windows::Forms::RadioButton());
            this->radioTexAsLegacyDDS = (gcnew System::Windows::Forms::RadioButton());
            this->radioTexAsDDS = (gcnew System::Windows::Forms::RadioButton());
            this->groupBox2 = (gcnew System::Windows::Forms::GroupBox());
            this->radioMdlAsFBX = (gcnew System::Windows::Forms::RadioButton());
            this->radioMdlAsOBJ = (gcnew System::Windows::Forms::RadioButton());
            this->groupBox3 = (gcnew System::Windows::Forms::GroupBox());
            this->radioSndAsWAV = (gcnew System::Windows::Forms::RadioButton());
            this->radioSndAsOGG = (gcnew System::Windows::Forms::RadioButton());
            this->btnCancel = (gcnew System::Windows::Forms::Button());
            this->btnOK = (gcnew System::Windows::Forms::Button());
            this->groupBox1->SuspendLayout();
            this->groupBox2->SuspendLayout();
            this->groupBox3->SuspendLayout();
            this->SuspendLayout();
            // 
            // groupBox1
            // 
            this->groupBox1->Controls->Add(this->radioTexAsPNG);
            this->groupBox1->Controls->Add(this->radioTexAsTGA);
            this->groupBox1->Controls->Add(this->radioTexAsLegacyDDS);
            this->groupBox1->Controls->Add(this->radioTexAsDDS);
            this->groupBox1->Location = System::Drawing::Point(12, 12);
            this->groupBox1->Name = L"groupBox1";
            this->groupBox1->Size = System::Drawing::Size(468, 117);
            this->groupBox1->TabIndex = 0;
            this->groupBox1->TabStop = false;
            this->groupBox1->Text = L"Textures options:";
            // 
            // radioTexAsPNG
            // 
            this->radioTexAsPNG->AutoSize = true;
            this->radioTexAsPNG->Location = System::Drawing::Point(7, 92);
            this->radioTexAsPNG->Name = L"radioTexAsPNG";
            this->radioTexAsPNG->Size = System::Drawing::Size(95, 17);
            this->radioTexAsPNG->TabIndex = 3;
            this->radioTexAsPNG->Text = L"Export as PNG";
            this->radioTexAsPNG->UseVisualStyleBackColor = true;
            // 
            // radioTexAsTGA
            // 
            this->radioTexAsTGA->AutoSize = true;
            this->radioTexAsTGA->Location = System::Drawing::Point(7, 68);
            this->radioTexAsTGA->Name = L"radioTexAsTGA";
            this->radioTexAsTGA->Size = System::Drawing::Size(94, 17);
            this->radioTexAsTGA->TabIndex = 2;
            this->radioTexAsTGA->Text = L"Export as TGA";
            this->radioTexAsTGA->UseVisualStyleBackColor = true;
            // 
            // radioTexAsLegacyDDS
            // 
            this->radioTexAsLegacyDDS->AutoSize = true;
            this->radioTexAsLegacyDDS->Location = System::Drawing::Point(7, 44);
            this->radioTexAsLegacyDDS->Name = L"radioTexAsLegacyDDS";
            this->radioTexAsLegacyDDS->Size = System::Drawing::Size(129, 17);
            this->radioTexAsLegacyDDS->TabIndex = 1;
            this->radioTexAsLegacyDDS->Text = L"Export as legacy DDS";
            this->radioTexAsLegacyDDS->UseVisualStyleBackColor = true;
            // 
            // radioTexAsDDS
            // 
            this->radioTexAsDDS->AutoSize = true;
            this->radioTexAsDDS->Checked = true;
            this->radioTexAsDDS->Location = System::Drawing::Point(7, 20);
            this->radioTexAsDDS->Name = L"radioTexAsDDS";
            this->radioTexAsDDS->Size = System::Drawing::Size(95, 17);
            this->radioTexAsDDS->TabIndex = 0;
            this->radioTexAsDDS->TabStop = true;
            this->radioTexAsDDS->Text = L"Export as DDS";
            this->radioTexAsDDS->UseVisualStyleBackColor = true;
            // 
            // groupBox2
            // 
            this->groupBox2->Controls->Add(this->radioMdlAsFBX);
            this->groupBox2->Controls->Add(this->radioMdlAsOBJ);
            this->groupBox2->Location = System::Drawing::Point(13, 136);
            this->groupBox2->Name = L"groupBox2";
            this->groupBox2->Size = System::Drawing::Size(467, 70);
            this->groupBox2->TabIndex = 1;
            this->groupBox2->TabStop = false;
            this->groupBox2->Text = L"Models options:";
            // 
            // radioMdlAsFBX
            // 
            this->radioMdlAsFBX->AutoSize = true;
            this->radioMdlAsFBX->Location = System::Drawing::Point(7, 44);
            this->radioMdlAsFBX->Name = L"radioMdlAsFBX";
            this->radioMdlAsFBX->Size = System::Drawing::Size(92, 17);
            this->radioMdlAsFBX->TabIndex = 1;
            this->radioMdlAsFBX->TabStop = true;
            this->radioMdlAsFBX->Text = L"Export as FBX";
            this->radioMdlAsFBX->UseVisualStyleBackColor = true;
            // 
            // radioMdlAsOBJ
            // 
            this->radioMdlAsOBJ->AutoSize = true;
            this->radioMdlAsOBJ->Checked = true;
            this->radioMdlAsOBJ->Location = System::Drawing::Point(7, 20);
            this->radioMdlAsOBJ->Name = L"radioMdlAsOBJ";
            this->radioMdlAsOBJ->Size = System::Drawing::Size(92, 17);
            this->radioMdlAsOBJ->TabIndex = 0;
            this->radioMdlAsOBJ->TabStop = true;
            this->radioMdlAsOBJ->Text = L"Export as OBJ";
            this->radioMdlAsOBJ->UseVisualStyleBackColor = true;
            // 
            // groupBox3
            // 
            this->groupBox3->Controls->Add(this->radioSndAsWAV);
            this->groupBox3->Controls->Add(this->radioSndAsOGG);
            this->groupBox3->Location = System::Drawing::Point(13, 213);
            this->groupBox3->Name = L"groupBox3";
            this->groupBox3->Size = System::Drawing::Size(467, 71);
            this->groupBox3->TabIndex = 2;
            this->groupBox3->TabStop = false;
            this->groupBox3->Text = L"Sounds options:";
            // 
            // radioSndAsWAV
            // 
            this->radioSndAsWAV->AutoSize = true;
            this->radioSndAsWAV->Location = System::Drawing::Point(7, 44);
            this->radioSndAsWAV->Name = L"radioSndAsWAV";
            this->radioSndAsWAV->Size = System::Drawing::Size(97, 17);
            this->radioSndAsWAV->TabIndex = 1;
            this->radioSndAsWAV->TabStop = true;
            this->radioSndAsWAV->Text = L"Export as WAV";
            this->radioSndAsWAV->UseVisualStyleBackColor = true;
            // 
            // radioSndAsOGG
            // 
            this->radioSndAsOGG->AutoSize = true;
            this->radioSndAsOGG->Checked = true;
            this->radioSndAsOGG->Location = System::Drawing::Point(7, 20);
            this->radioSndAsOGG->Name = L"radioSndAsOGG";
            this->radioSndAsOGG->Size = System::Drawing::Size(96, 17);
            this->radioSndAsOGG->TabIndex = 0;
            this->radioSndAsOGG->TabStop = true;
            this->radioSndAsOGG->Text = L"Export as OGG";
            this->radioSndAsOGG->UseVisualStyleBackColor = true;
            // 
            // btnCancel
            // 
            this->btnCancel->Location = System::Drawing::Point(404, 294);
            this->btnCancel->Name = L"btnCancel";
            this->btnCancel->Size = System::Drawing::Size(75, 23);
            this->btnCancel->TabIndex = 3;
            this->btnCancel->Text = L"Cancel";
            this->btnCancel->UseVisualStyleBackColor = true;
            this->btnCancel->Click += gcnew System::EventHandler(this, &ExtractionOptionsDgl::btnCancel_Click);
            // 
            // btnOK
            // 
            this->btnOK->Location = System::Drawing::Point(323, 294);
            this->btnOK->Name = L"btnOK";
            this->btnOK->Size = System::Drawing::Size(75, 23);
            this->btnOK->TabIndex = 4;
            this->btnOK->Text = L"OK";
            this->btnOK->UseVisualStyleBackColor = true;
            this->btnOK->Click += gcnew System::EventHandler(this, &ExtractionOptionsDgl::btnOK_Click);
            // 
            // ExtractionOptionsDgl
            // 
            this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
            this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
            this->ClientSize = System::Drawing::Size(494, 329);
            this->Controls->Add(this->btnOK);
            this->Controls->Add(this->btnCancel);
            this->Controls->Add(this->groupBox3);
            this->Controls->Add(this->groupBox2);
            this->Controls->Add(this->groupBox1);
            this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedSingle;
            this->MaximizeBox = false;
            this->MinimizeBox = false;
            this->Name = L"ExtractionOptionsDgl";
            this->ShowInTaskbar = false;
            this->SizeGripStyle = System::Windows::Forms::SizeGripStyle::Hide;
            this->StartPosition = System::Windows::Forms::FormStartPosition::CenterParent;
            this->Text = L"Extraction options";
            this->groupBox1->ResumeLayout(false);
            this->groupBox1->PerformLayout();
            this->groupBox2->ResumeLayout(false);
            this->groupBox2->PerformLayout();
            this->groupBox3->ResumeLayout(false);
            this->groupBox3->PerformLayout();
            this->ResumeLayout(false);

        }
#pragma endregion
    private: System::Void btnCancel_Click(System::Object^  sender, System::EventArgs^  e) {
        this->DialogResult = System::Windows::Forms::DialogResult::Cancel;
        this->Close();
    }
    private: System::Void btnOK_Click(System::Object^  sender, System::EventArgs^  e) {
        this->DialogResult = System::Windows::Forms::DialogResult::OK;
        this->Close();
    }
};
}
