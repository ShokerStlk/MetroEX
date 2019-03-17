#pragma once
#include "mycommon.h"

class MetroTexture {
public:
    enum class TextureFormat : size_t {
        BC7,
        BC6H,
        RGBA
    };

public:
    MetroTexture();
    ~MetroTexture();

    bool            LoadFromData(const uint8_t* data, const size_t length, const CharString& name);
    bool            SaveAsDDS(const fs::path& filePath);
    bool            SaveAsLegacyDDS(const fs::path& filePath);
    bool            SaveAsTGA(const fs::path& filePath);
    bool            SaveAsPNG(const fs::path& filePath);

    bool            IsCubemap() const;
    size_t          GetWidth() const;
    size_t          GetHeight() const;
    size_t          GetDepth() const;
    size_t          GetNumMips() const;
    TextureFormat   GetFormat() const;

    bool            GetRGBA(BytesArray& imagePixels) const;
    bool            GetBGRA(BytesArray& imagePixels) const;
    const uint8_t*  GetRawData() const;

private:
    BytesArray      mData;
    bool            mIsCubemap;
    size_t          mWidth;
    size_t          mHeight;
    size_t          mDepth;
    size_t          mNumMips;
    TextureFormat   mFormat;
};
