#include "ImagePanel.h"

namespace MetroEX {

    ImagePanel::ImagePanel() {
        Bitmap bmp(16, 16);
        for (int y = 0; y < bmp.Height; ++y) {
            for (int x = 0; x < bmp.Width; ++x) {
                const bool isWhite = (x < 8 && y < 8) || (x >= 8 && y >= 8);
                bmp.SetPixel(x, y, isWhite ? Color::LightGray : Color::DarkGray);
            }
        }
        mBackgroundBrush = gcnew TextureBrush(%bmp);

        mPanning = false;
        mTransparencyEnabled = true;
        mImage = nullptr;

        InitializeComponent();

        this->SetStyle(ControlStyles::AllPaintingInWmPaint | ControlStyles::UserPaint | ControlStyles::OptimizedDoubleBuffer | ControlStyles::ResizeRedraw, true);
        this->SetStyle(ControlStyles::Selectable, false);
        this->UpdateStyles();
    }

    void ImagePanel::SetTexture(MetroTexture* texture) {
        if (mImage) {
            delete mImage;
            mImage = nullptr;
        }

        if (texture) {
            BytesArray pixels;
            texture->GetBGRA(pixels);

            const int w = scast<int>(texture->GetWidth());
            const int h = scast<int>(texture->GetHeight());

            Bitmap^ bmp = gcnew Bitmap(w, h, Imaging::PixelFormat::Format32bppArgb);

            Rectangle rc(0, 0, w, h);
            Imaging::BitmapData^ bmpData = bmp->LockBits(rc, Imaging::ImageLockMode::WriteOnly, bmp->PixelFormat);
            memcpy(bmpData->Scan0.ToPointer(), pixels.data(), pixels.size());
            bmp->UnlockBits(bmpData);

            mImage = bmp;
        }

        this->AdjustScrolling();
        this->Invalidate();
    }

    void ImagePanel::EnableTransparency(const bool enable) {
        if (mTransparencyEnabled != enable) {
            mTransparencyEnabled = enable;
            this->Invalidate();
        }
    }


    void ImagePanel::OnPaint(System::Windows::Forms::PaintEventArgs^ e) {
        int left = this->Padding.Left;
        int top = this->Padding.Top;

        // background
        if (mImage != nullptr && mTransparencyEnabled) {
            e->Graphics->FillRectangle(mBackgroundBrush, this->ClientRectangle);
        } else {
            e->Graphics->FillRectangle(Brushes::White, this->ClientRectangle);
        }

        // image
        if (mImage != nullptr) {
            if (this->AutoScroll) {
                left += this->AutoScrollPosition.X;
                top += this->AutoScrollPosition.Y;
            }

            if (!mTransparencyEnabled) {
                e->Graphics->CompositingMode = System::Drawing::Drawing2D::CompositingMode::SourceCopy;
            } else {
                e->Graphics->CompositingMode = System::Drawing::Drawing2D::CompositingMode::SourceOver;
            }

            e->Graphics->DrawImageUnscaled(mImage, left, top);
        }

        // borders
        ControlPaint::DrawBorder(e->Graphics, this->ClientRectangle, this->ForeColor, ButtonBorderStyle::Solid);
    }

    void ImagePanel::OnScroll(System::Windows::Forms::ScrollEventArgs^ e) {
        this->Invalidate();

        UserControl::OnScroll(e);
    }

    void ImagePanel::OnMouseMove(System::Windows::Forms::MouseEventArgs^ e) {
        UserControl::OnMouseMove(e);

        if (e->Button == System::Windows::Forms::MouseButtons::Left) {
            if (!mPanning) {
                mStartMousePos = e->Location;
                this->SwitchPanning(true);
            }

            if (mPanning) {
                const int x = -(mStartScrollPosition.X - (mStartMousePos.X - e->Location.X));
                const int y = -(mStartScrollPosition.Y - (mStartMousePos.Y - e->Location.Y));

                this->UpdateScrollPosition(Point(x, y));
            }
        }
    }

    void ImagePanel::OnMouseUp(System::Windows::Forms::MouseEventArgs^ e) {
        UserControl::OnMouseUp(e);

        if (mPanning) {
            this->SwitchPanning(false);
        }
    }

    void ImagePanel::SwitchPanning(bool isPanning) {
        if (isPanning != mPanning) {
            mPanning = isPanning;
            mStartScrollPosition = this->AutoScrollPosition;
            this->Cursor = isPanning ? Cursors::SizeAll : Cursors::Default;
        }
    }

    void ImagePanel::UpdateScrollPosition(Point position) {
        this->AutoScrollPosition = position;
        this->Invalidate();
        this->OnScroll(gcnew ScrollEventArgs(ScrollEventType::ThumbPosition, 0));
    }

    void ImagePanel::AdjustLayout() {
        if (this->AutoSize) {
            this->AdjustSize();
        }
        else if (this->AutoScroll) {
            this->AdjustScrolling();
        }
    }

    void ImagePanel::AdjustSize() {
        if (this->AutoSize && this->Dock == DockStyle::None) {
            this->Size = this->PreferredSize;
        }
    }

    void ImagePanel::AdjustScrolling() {
        if (this->AutoScroll && mImage != nullptr) {
            this->AutoScrollMinSize = mImage->Size;
        }
    }

}
