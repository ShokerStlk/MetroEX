#include "MetroSound.h"
#include <fstream>

#define STB_VORBIS_NO_PUSHDATA_API
#define STB_VORBIS_NO_STDIO
#include "stb_vorbis.h"


MetroSound::MetroSound() {

}
MetroSound::~MetroSound() {

}

bool MetroSound::LoadFromData(MemStream& stream) {
    mData.resize(stream.Remains());
    stream.ReadToBuffer(mData.data(), mData.size());

    return true;
}

bool MetroSound::GetWAVE(BytesArray& waveData) {
    bool result = false;

    waveData.clear();

    int channels, rate;
    int16_t* pcmData = nullptr;
    const int numPcmSamples = stb_vorbis_decode_memory(mData.data(), scast<int>(mData.size()), &channels, &rate, &pcmData);
    if (numPcmSamples > 0) {
        struct {
            char     riff[4];
            uint32_t totalsize;

            char     wave[4];
            char     fmt_[4];
            uint32_t hdrsize;
            uint16_t format;
            uint16_t channels;
            uint32_t freq;
            uint32_t bytesPerSec;
            uint16_t blocksize;
            uint16_t sampleBits;

            char     data[4];
            uint32_t datasize;
        } wavhdr;

        const uint32_t pcmDataSize = scast<uint32_t>(numPcmSamples * channels * 2);

        wavhdr.riff[0] = 'R';
        wavhdr.riff[1] = 'I';
        wavhdr.riff[2] = 'F';
        wavhdr.riff[3] = 'F';
        wavhdr.totalsize = scast<uint32_t>(pcmDataSize + sizeof(wavhdr) - 8);

        wavhdr.wave[0] = 'W';
        wavhdr.wave[1] = 'A';
        wavhdr.wave[2] = 'V';
        wavhdr.wave[3] = 'E';
        wavhdr.fmt_[0] = 'f';
        wavhdr.fmt_[1] = 'm';
        wavhdr.fmt_[2] = 't';
        wavhdr.fmt_[3] = ' ';
        wavhdr.hdrsize = 16;
        wavhdr.format = 1; // PCM
        wavhdr.channels = scast<uint16_t>(channels);
        wavhdr.freq = scast<uint32_t>(rate);
        wavhdr.bytesPerSec = scast<uint32_t>(rate * channels * 2);
        wavhdr.blocksize = scast<uint32_t>(channels * 2);
        wavhdr.sampleBits = 16;

        wavhdr.data[0] = 'd';
        wavhdr.data[1] = 'a';
        wavhdr.data[2] = 't';
        wavhdr.data[3] = 'a';
        wavhdr.datasize = scast<uint32_t>(pcmDataSize);

        waveData.resize(sizeof(wavhdr) + pcmDataSize);
        memcpy(waveData.data(), &wavhdr, sizeof(wavhdr));
        memcpy(waveData.data() + sizeof(wavhdr), pcmData, pcmDataSize);

        result = true;
    }

    if (pcmData) {
        free(pcmData);
    }

    return result;
}

bool MetroSound::SaveAsOGG(const fs::path& outPath) {
    bool result = false;

    std::ofstream file(outPath, std::ofstream::binary);
    if (file.good()) {
        file.write(rcast<const char*>(mData.data()), mData.size());
        file.flush();

        result = true;
    }

    return result;
}

bool MetroSound::SaveAsWAV(const fs::path& outPath) {
    bool result = false;

    BytesArray waveData;
    if (this->GetWAVE(waveData)) {
        std::ofstream file(outPath, std::ofstream::binary);
        if (file.good()) {
            file.write(rcast<const char*>(waveData.data()), waveData.size());
            file.flush();

            result = true;
        }
    }

    return result;
}
