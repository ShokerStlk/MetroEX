#pragma once

namespace MetroEX {

    using namespace System;
    using namespace System::ComponentModel;
    using namespace System::Collections;
    using namespace System::Windows::Forms;
    using namespace System::Data;
    using namespace System::Drawing;

    /// <summary>
    /// Summary for AboutDlg
    /// </summary>
    public ref class AboutDlg : public System::Windows::Forms::Form
    {
    public:
        AboutDlg(void)
        {
            InitializeComponent();
            //
            //TODO: Add the constructor code here
            //
        }

    protected:
        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        ~AboutDlg()
        {
            if (components)
            {
                delete components;
            }
        }
    private: System::Windows::Forms::PictureBox^  pictureBox1;
    private: System::Windows::Forms::RichTextBox^  richTextBox1;
    private: System::Windows::Forms::Button^  button1;
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
            System::ComponentModel::ComponentResourceManager^  resources = (gcnew System::ComponentModel::ComponentResourceManager(AboutDlg::typeid));
            this->pictureBox1 = (gcnew System::Windows::Forms::PictureBox());
            this->richTextBox1 = (gcnew System::Windows::Forms::RichTextBox());
            this->button1 = (gcnew System::Windows::Forms::Button());
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->pictureBox1))->BeginInit();
            this->SuspendLayout();
            // 
            // pictureBox1
            // 
            this->pictureBox1->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"pictureBox1.Image")));
            this->pictureBox1->Location = System::Drawing::Point(13, 13);
            this->pictureBox1->Name = L"pictureBox1";
            this->pictureBox1->Size = System::Drawing::Size(128, 128);
            this->pictureBox1->TabIndex = 0;
            this->pictureBox1->TabStop = false;
            // 
            // richTextBox1
            // 
            this->richTextBox1->BorderStyle = System::Windows::Forms::BorderStyle::None;
            this->richTextBox1->Location = System::Drawing::Point(148, 13);
            this->richTextBox1->Name = L"richTextBox1";
            this->richTextBox1->ReadOnly = true;
            this->richTextBox1->Size = System::Drawing::Size(414, 128);
            this->richTextBox1->TabIndex = 1;
            this->richTextBox1->Text = L"";
            // 
            // button1
            // 
            this->button1->Location = System::Drawing::Point(486, 148);
            this->button1->Name = L"button1";
            this->button1->Size = System::Drawing::Size(75, 23);
            this->button1->TabIndex = 2;
            this->button1->Text = L"OK";
            this->button1->UseVisualStyleBackColor = true;
            this->button1->Click += gcnew System::EventHandler(this, &AboutDlg::button1_Click);
            // 
            // AboutDlg
            // 
            this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
            this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
            this->ClientSize = System::Drawing::Size(575, 182);
            this->Controls->Add(this->button1);
            this->Controls->Add(this->richTextBox1);
            this->Controls->Add(this->pictureBox1);
            this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedSingle;
            this->MaximizeBox = false;
            this->MinimizeBox = false;
            this->Name = L"AboutDlg";
            this->SizeGripStyle = System::Windows::Forms::SizeGripStyle::Hide;
            this->StartPosition = System::Windows::Forms::FormStartPosition::CenterParent;
            this->Text = L"AboutDlg";
            this->Load += gcnew System::EventHandler(this, &AboutDlg::AboutDlg_Load);
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->pictureBox1))->EndInit();
            this->ResumeLayout(false);

        }
#pragma endregion

        private: System::Void button1_Click(System::Object^  sender, System::EventArgs^  e) {
            this->Close();
        }
        private: System::Void AboutDlg_Load(System::Object^  sender, System::EventArgs^  e) {
            String^ rtfText = LR"({\rtf1\ansi\ansicpg1251\deff0\nouicompat\deflang1049{\fonttbl{\f0\fnil\fcharset204 Segoe UI;}{\f1\fnil\fcharset0 Calibri;}}
{\colortbl ;\red0\green77\blue187;\red243\green164\blue71;}
{\*\generator Riched20 10.0.17763}\viewkind4\uc1 
\pard\qc\cf1\b\f0\fs18 MetroEX\cf0\b0  v0.29a\par
\par
Created by Sergii "\cf2 iOrange\cf0 " Kudlai\par
\par

\pard\sa200\sl276\slmult1\qc 2019\f1\fs22\lang9\par
}
 )";

            array<Byte>^ byteArray = System::Text::Encoding::ASCII->GetBytes(rtfText);
            System::IO::MemoryStream stream(byteArray);

            this->richTextBox1->LoadFile(%stream, RichTextBoxStreamType::RichText);
        }
    };
}
