#include "SoundPanel.h"

using namespace System::IO;
using namespace System::Media;

namespace MetroEX {

    SoundPanel::SoundPanel()
        : mSound(nullptr)
        , mPlayer(nullptr)
    {
        InitializeComponent();
    }

    void SoundPanel::SetSound(MetroSound* sound) {
        if (sound != mSound) {
            if (mPlayer) {
                mPlayer->Stop();
            }

            MySafeDelete(mSound);
            MySafeDelete(mPlayer);

            mSound = sound;

            if (mSound) {
                BytesArray waveData;
                if (mSound->GetWAVE(waveData)) {
                    array<Byte>^ managedArray = gcnew array<Byte>(scast<int>(waveData.size()));
                    pin_ptr<Byte> managedArrayPtr = &managedArray[0];

                    memcpy(managedArrayPtr, waveData.data(), waveData.size());

                    MemoryStream^ memStream = gcnew MemoryStream(managedArray);
                    mPlayer = gcnew SoundPlayer(memStream);
                }
            }
        }
    }

    void SoundPanel::OnVisibleChanged(System::EventArgs^ e) {
        System::Windows::Forms::UserControl::OnVisibleChanged(e);

        if (!this->Visible) {
            if (mPlayer) {
                mPlayer->Stop();
            }

            MySafeDelete(mSound);
            MySafeDelete(mPlayer);
        }
    }

    void SoundPanel::btnPlay_Click(System::Object^ sender, System::EventArgs^ e) {
        if (mPlayer) {
            mPlayer->Play();
        }
    }

    void SoundPanel::btnStop_Click(System::Object^ sender, System::EventArgs^ e) {
        if (mPlayer) {
            mPlayer->Stop();
        }
    }

}
