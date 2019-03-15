#pragma once

#include "metro/MetroSound.h"

using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;


namespace MetroEX {

    public ref class SoundPanel : public System::Windows::Forms::UserControl
    {
    public:
        SoundPanel();

        void SetSound(MetroSound* sound);

    protected:
        ~SoundPanel() {
            if (components) {
                delete components;
            }
        }

        virtual void OnVisibleChanged(System::EventArgs^ e) override;

    private:
        MetroSound*                 mSound;
        System::Media::SoundPlayer^ mPlayer;
    private: System::Windows::Forms::Panel^  panel1;
    private: System::Windows::Forms::Button^  btnStop;
    private: System::Windows::Forms::Button^  btnPlay;

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
            this->panel1 = (gcnew System::Windows::Forms::Panel());
            this->btnPlay = (gcnew System::Windows::Forms::Button());
            this->btnStop = (gcnew System::Windows::Forms::Button());
            this->panel1->SuspendLayout();
            this->SuspendLayout();
            // 
            // panel1
            // 
            this->panel1->Controls->Add(this->btnStop);
            this->panel1->Controls->Add(this->btnPlay);
            this->panel1->Location = System::Drawing::Point(140, 161);
            this->panel1->Name = L"panel1";
            this->panel1->Size = System::Drawing::Size(307, 100);
            this->panel1->TabIndex = 0;
            // 
            // btnPlay
            // 
            this->btnPlay->Location = System::Drawing::Point(40, 37);
            this->btnPlay->Name = L"btnPlay";
            this->btnPlay->Size = System::Drawing::Size(75, 23);
            this->btnPlay->TabIndex = 0;
            this->btnPlay->Text = L"Play";
            this->btnPlay->UseVisualStyleBackColor = true;
            this->btnPlay->Click += gcnew System::EventHandler(this, &SoundPanel::btnPlay_Click);
            // 
            // btnStop
            // 
            this->btnStop->Location = System::Drawing::Point(151, 37);
            this->btnStop->Name = L"btnStop";
            this->btnStop->Size = System::Drawing::Size(75, 23);
            this->btnStop->TabIndex = 1;
            this->btnStop->Text = L"Stop";
            this->btnStop->UseVisualStyleBackColor = true;
            this->btnStop->Click += gcnew System::EventHandler(this, &SoundPanel::btnStop_Click);
            // 
            // SoundPanel
            // 
            this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
            this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
            this->Controls->Add(this->panel1);
            this->Name = L"SoundPanel";
            this->Size = System::Drawing::Size(595, 436);
            this->panel1->ResumeLayout(false);
            this->ResumeLayout(false);

        }
#pragma endregion
        private:
            void btnPlay_Click(System::Object^ sender, System::EventArgs^ e);
            void btnStop_Click(System::Object^ sender, System::EventArgs^ e);
    };
}
