#include "MetroTexture.h"
#include "lz4.h"
#include "dds_utils.h"

#define STBI_WRITE_NO_STDIO
#define STB_IMAGE_WRITE_IMPLEMENTATION
#pragma warning (disable : 4793) // `anonymous namespace'::stbiw__outfile': function compiled as native
#include "stb_image_write.h"
#pragma warning (default : 4793)

#include <fstream>


MetroTexture::MetroTexture()
    : mIsCubemap(false)
    , mWidth(0)
    , mHeight(0)
    , mDepth(0)
    , mNumMips(0)
    , mFormat(TextureFormat::BC7)
{
}
MetroTexture::~MetroTexture() {
}


bool MetroTexture::LoadFromData(MemStream& stream, const CharString& name) {
    bool result = false;

    const uint8_t* data = stream.GetDataAtCursor();
    const size_t length = stream.Remains();

    if (*rcast<const uint32_t*>(data) == cDDSFileSignature) {
        // this is a plain DDS file
        MemStream stream(data, length);
        stream.SkipBytes(4); // skip DDS magic

        DDSURFACEDESC2 ddsHdr;
        DDS_HEADER_DXT10 dx10Hdr;

        stream.ReadStruct(ddsHdr);
        if (ddsHdr.ddpfPixelFormat.dwFourCC == PIXEL_FMT_FOURCC('D', 'X', '1', '0')) {
            stream.ReadStruct(dx10Hdr);

            switch (dx10Hdr.dxgiFormat) {
                case DXGI_FORMAT_BC7_TYPELESS:
                case DXGI_FORMAT_BC7_UNORM:
                case DXGI_FORMAT_BC7_UNORM_SRGB: {
                    mFormat = TextureFormat::BC7;
                } break;

                case DXGI_FORMAT_BC6H_TYPELESS:
                case DXGI_FORMAT_BC6H_SF16:
                case DXGI_FORMAT_BC6H_UF16: {
                    mFormat = TextureFormat::BC6H;
                    mIsCubemap = true;
                } break;

                default:
                    return false;
            }
        } else {
            return false;
        }

        mWidth = ddsHdr.dwWidth;
        mHeight = ddsHdr.dwHeight;
        mDepth = 1;
        mNumMips = (ddsHdr.dwMipMapCount > 1) ? ddsHdr.dwMipMapCount : 1;

        mData.resize(stream.Remains());
        stream.ReadToBuffer(mData.data(), mData.size());

        result = true;
    } else {
        // LZ4-compressed BC7 texture
        CharString extension = fs::path(name).extension().string();
        size_t dimension = 0, numMips = 0;
        if (extension == ".512") {
            dimension = 512;
            numMips = 10;
        } else if (extension == ".1024") {
            dimension = 1024;
            numMips = 1;
        } else if (extension == ".2048") {
            dimension = 2048;
            numMips = 1;
        }

        if (dimension > 0) {
            const size_t bc7size = DDS_GetCompressedSizeBC7(dimension, dimension, numMips);
            mData.resize(bc7size);
            const int lz4Result = LZ4_decompress_safe(rcast<const char*>(data), rcast<char*>(mData.data()), scast<int>(length), scast<int>(bc7size));
            if (lz4Result != scast<int>(bc7size)) {
                mData.resize(0);
            } else {
                mWidth = dimension;
                mHeight = dimension;
                mDepth = 1;
                mNumMips = numMips;
                mFormat = TextureFormat::BC7;

                result = true;
            }
        }
    }

    return result;
}

bool MetroTexture::SaveAsDDS(const fs::path& filePath) {
    bool result = false;

    DDSURFACEDESC2 ddsHdr;
    DDS_HEADER_DXT10 dx10Hdr;
    DDS_MakeDX10Headers(ddsHdr, dx10Hdr, mWidth, mHeight, mNumMips, mIsCubemap);

    std::ofstream file(filePath, std::ofstream::binary);
    if (file.good()) {
        file.write(rcast<const char*>(&cDDSFileSignature), sizeof(cDDSFileSignature));
        file.write(rcast<const char*>(&ddsHdr), sizeof(ddsHdr));
        file.write(rcast<const char*>(&dx10Hdr), sizeof(dx10Hdr));
        file.write(rcast<const char*>(mData.data()), mData.size());

        result = true;
    }

    return result;
}

//#TODO: also mips ?
bool MetroTexture::SaveAsLegacyDDS(const fs::path& filePath) {
    bool result = false;

    //#TODO: add support!
    if (this->IsCubemap()) {
        return false;
    }

    std::ofstream file(filePath, std::ofstream::binary);
    if (file.good()) {
        DDSURFACEDESC2 ddsHdr;
        DDS_MakeDX9Header(ddsHdr, mWidth, mHeight, 1);

        BytesArray rgbaPixels;
        this->GetRGBA(rgbaPixels);

        BytesArray bc3Blocks(DDS_GetCompressedSizeBC7(mWidth, mHeight, 1));
        DDS_CompressBC3(rgbaPixels.data(), bc3Blocks.data(), mWidth, mHeight);

        file.write(rcast<const char*>(&cDDSFileSignature), sizeof(cDDSFileSignature));
        file.write(rcast<const char*>(&ddsHdr), sizeof(ddsHdr));
        file.write(rcast<const char*>(bc3Blocks.data()), bc3Blocks.size());

        result = true;
    }

    return result;
}

bool MetroTexture::SaveAsTGA(const fs::path& filePath) {
    bool result = false;

    std::ofstream file(filePath, std::ofstream::binary);
    if (file.good()) {
        BytesArray bgraPixels;
        if (this->GetBGRA(bgraPixels)) {
            uint16_t hdr[9] = { 0 };
            hdr[1] = 2;
            hdr[6] = scast<uint16_t>(mWidth);
            hdr[7] = scast<uint16_t>(mHeight);
            hdr[8] = 32;
            file.write(rcast<const char*>(hdr), sizeof(hdr));

            const size_t pitch = mWidth * 4;
            const char* pixels = rcast<const char*>(bgraPixels.data()) + (pitch * (mHeight - 1));
            for (size_t y = 0; y < mHeight; ++y) {
                file.write(pixels, pitch);
                pixels -= pitch;
            }

            result = true;
        }
    }

    return result;
}

bool MetroTexture::SaveAsPNG(const fs::path& filePath) {
    bool result = false;

    std::ofstream file(filePath, std::ofstream::binary);
    if (file.good()) {
        BytesArray rgbaPixels;
        if (this->GetRGBA(rgbaPixels)) {
            const int success = stbi_write_png_to_func([](void* ptr, void* data, int size) {
                std::ofstream* filePtr = rcast<std::ofstream*>(ptr);
                filePtr->write(rcast<const char*>(data), size);
            }, &file, scast<int>(mWidth), scast<int>(mHeight), 4, rgbaPixels.data(), 0);

            result = (success > 0);
        }
    }

    return result;
}

bool MetroTexture::IsCubemap() const {
    return mIsCubemap;
}

size_t MetroTexture::GetWidth() const {
    return mWidth;
}

size_t MetroTexture::GetHeight() const {
    return mHeight;
}

size_t MetroTexture::GetDepth() const {
    return mDepth;
}

size_t MetroTexture::GetNumMips() const {
    return mNumMips;
}

MetroTexture::TextureFormat MetroTexture::GetFormat() const {
    return mFormat;
}

bool MetroTexture::GetRGBA(BytesArray& imagePixels) const {
    bool result = false;

    if (!mData.empty()) {
        //#TODO: add support for other formats!
        if (mFormat == TextureFormat::BC7) {
            imagePixels.resize(mWidth * mHeight * 4);
            DDS_DecompressBC7(mData.data(), imagePixels.data(), mWidth, mHeight);
            result = true;
        }
    }

    return result;
}

bool MetroTexture::GetBGRA(BytesArray& imagePixels) const {
    const bool result = this->GetRGBA(imagePixels);
    if (result) {
        uint8_t* rgba = imagePixels.data();
        for (size_t i = 0; i < mWidth * mHeight; ++i) {
            std::swap(rgba[0], rgba[2]);
            rgba += 4;
        }
    }
    return result;
}

const uint8_t* MetroTexture::GetRawData() const {
    return mData.data();
}
