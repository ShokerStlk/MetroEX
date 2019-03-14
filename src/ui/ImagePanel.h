#pragma once

#include "metro/MetroTexture.h"

using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;


namespace MetroEX {

    /// <summary>
    /// Summary for ImagePanel
    /// </summary>
    public ref class ImagePanel : public System::Windows::Forms::UserControl
    {
    public:
        ImagePanel();

        void SetTexture(MetroTexture* texture);

        void EnableTransparency(const bool enable);

    protected:
        ~ImagePanel() {
            if (components) {
                delete components;
            }

            if (mBackgroundBrush) {
                delete mBackgroundBrush;
            }
        }

        void OnPaint(System::Windows::Forms::PaintEventArgs^ e) override;
        void OnScroll(System::Windows::Forms::ScrollEventArgs^ e) override;
        void OnMouseMove(System::Windows::Forms::MouseEventArgs^ e) override;
        void OnMouseUp(System::Windows::Forms::MouseEventArgs^ e) override;

        void SwitchPanning(bool isPanning);
        void UpdateScrollPosition(Point position);
        void AdjustLayout();
        void AdjustSize();
        void AdjustScrolling();

    private:
        System::Drawing::Brush^ mBackgroundBrush;
        System::Drawing::Image^ mImage;
        System::Drawing::Point  mStartMousePos;
        System::Drawing::Point  mStartScrollPosition;
        bool                    mPanning;
        bool                    mTransparencyEnabled;

        System::ComponentModel::Container ^components;

#pragma region Windows Form Designer generated code
        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        void InitializeComponent(void)
        {
            this->SuspendLayout();
            // 
            // ImagePanel
            // 
            this->Name = L"ImagePanel";
            this->Size = System::Drawing::Size(567, 490);
            this->ResumeLayout(false);

        }
#pragma endregion
    };
}
