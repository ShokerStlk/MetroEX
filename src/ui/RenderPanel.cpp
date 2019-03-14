#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")

#include "metro/MetroModel.h"
#include "metro/MetroLevel.h"
#include "metro/MetroTexture.h"
#include "metro/VFXReader.h"
#include "metro/MetroTexturesDatabase.h"

#include "RenderPanel.h"

#include "shaders/ModelViewerVS.hlsl.h"
#include "shaders/ModelViewerPS.hlsl.h"

// String to std::string wrapper
#include <msclr/marshal_cppstd.h>

namespace MetroEX {

    RenderPanel::RenderPanel()
        : Panel()
        , mSwapChain(nullptr)
        , mDevice(nullptr)
        , mDeviceContext(nullptr)
        , mRenderTargetView(nullptr)
        , mDepthStencilBuffer(nullptr)
        , mDepthStencilState(nullptr)
        , mDepthStencilView(nullptr)
        , mRasterState(nullptr)
        // model viewer
        , mModelViewerVS(nullptr)
        , mModelViewerPS(nullptr)
        , mModelInputLayout(nullptr)
        , mModelConstantBuffer(nullptr)
        , mModelGeometries(nullptr)
        , mModelTextures(nullptr)
        // model viewer stuff
        , mModel(nullptr)
        , mLevel(nullptr)
        , mVFXReader(nullptr)
        , mDatabase(nullptr)
        //
        , mViewingParams(nullptr)
        , mConstantBufferData(nullptr)
    {
        mModelTextures = gcnew System::Collections::Generic::Dictionary<String^, IntPtr>(0);
    }

    bool RenderPanel::InitGraphics() {
        HRESULT result;
        ID3D11Texture2D* backBuffer = nullptr;

        DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
        D3D11_TEXTURE2D_DESC depthBufferDesc = {};
        D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};
        D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {};
        D3D11_RASTERIZER_DESC rasterDesc = {};
        D3D11_VIEWPORT viewport = {};

        swapChainDesc.BufferCount = 1;
        swapChainDesc.BufferDesc.Width = this->Size.Width;
        swapChainDesc.BufferDesc.Height = this->Size.Height;
        swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
        swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
        swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
        swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.OutputWindow = (HWND)this->Handle.ToPointer();
        swapChainDesc.SampleDesc.Count = 1;
        swapChainDesc.SampleDesc.Quality = 0;
        swapChainDesc.Windowed = true;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

        D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
        pin_ptr<IDXGISwapChain*> swapChainPtr(&mSwapChain);
        pin_ptr<ID3D11Device*> devicePtr(&mDevice);
        pin_ptr<ID3D11DeviceContext*> contextPtr(&mDeviceContext);
        result = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE,
                                               nullptr, 0, &featureLevel, 1,
                                               D3D11_SDK_VERSION, &swapChainDesc,
                                               swapChainPtr, devicePtr, nullptr, contextPtr);
        if (FAILED(result)) {
            return false;
        }

        if (!this->CreateRenderTargets()) {
            return false;
        }

        rasterDesc.AntialiasedLineEnable = false;
        rasterDesc.CullMode = D3D11_CULL_NONE;
        rasterDesc.DepthBias = 0;
        rasterDesc.DepthBiasClamp = 0.0f;
        rasterDesc.DepthClipEnable = true;
        rasterDesc.FillMode = D3D11_FILL_SOLID;
        rasterDesc.FrontCounterClockwise = false;
        rasterDesc.MultisampleEnable = false;
        rasterDesc.ScissorEnable = false;
        rasterDesc.SlopeScaledDepthBias = 0.0f;

        pin_ptr<ID3D11RasterizerState*> rsPtr(&mRasterState);
        result = mDevice->CreateRasterizerState(&rasterDesc, rsPtr);
        if (FAILED(result)) {
            return false;
        }

        mDeviceContext->RSSetState(mRasterState);

        depthStencilDesc.DepthEnable = true;
        depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
        depthStencilDesc.StencilEnable = false;

        pin_ptr<ID3D11DepthStencilState*> dssPtr(&mDepthStencilState);
        result = mDevice->CreateDepthStencilState(&depthStencilDesc, dssPtr);
        if (FAILED(result)) {
            return false;
        }

        mDeviceContext->OMSetDepthStencilState(mDepthStencilState, 1);

        pin_ptr<ID3D11VertexShader*> vsPtr(&mModelViewerVS);
        result = mDevice->CreateVertexShader(sModelViewerVSData, sizeof(sModelViewerVSData), nullptr, vsPtr);
        if (FAILED(result)) {
            return false;
        }

        pin_ptr<ID3D11PixelShader*> psPtr(&mModelViewerPS);
        result = mDevice->CreatePixelShader(sModelViewerPSData, sizeof(sModelViewerPSData), nullptr, psPtr);
        if (FAILED(result)) {
            return false;
        }

        D3D11_INPUT_ELEMENT_DESC vsDesc[] = {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(MetroVertex, pos),    D3D11_INPUT_PER_VERTEX_DATA, 0},
            { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(MetroVertex, normal), D3D11_INPUT_PER_VERTEX_DATA, 0},
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, offsetof(MetroVertex, uv0),    D3D11_INPUT_PER_VERTEX_DATA, 0}
        };
        pin_ptr<ID3D11InputLayout*> ilPtr(&mModelInputLayout);
        result = mDevice->CreateInputLayout(vsDesc, 3, sModelViewerVSData, sizeof(sModelViewerVSData), ilPtr);
        if (FAILED(result)) {
            return false;
        }

        mViewingParams = new ViewingParams;
        mViewingParams->rotation = vec2(0.0f);
        mViewingParams->offset = vec2(0.0f);
        mViewingParams->nearFar = vec2(0.001f, 50.0f);
        mViewingParams->zoom = 1.0f;

        mConstantBufferData = new ConstantBufferData;
        mConstantBufferData->modelBSphere = vec4(1.0f);
        mConstantBufferData->matModel = MatIdentity;
        mConstantBufferData->matView = MatIdentity;
        mConstantBufferData->matProjection = MatIdentity;

        pin_ptr<ID3D11Buffer*> cbPtr(&mModelConstantBuffer);
        D3D11_BUFFER_DESC desc = {};
        D3D11_SUBRESOURCE_DATA subData = {};

        desc.ByteWidth = sizeof(ConstantBufferData);
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        subData.pSysMem = mConstantBufferData;
        result = mDevice->CreateBuffer(&desc, &subData, cbPtr);
        if (FAILED(result)) {
            return false;
        }

        D3D11_SAMPLER_DESC samplerDesc = {};
        samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
        samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
        samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
        samplerDesc.MaxAnisotropy = 1;
        samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;

        pin_ptr<ID3D11SamplerState*> samplerStatePtr(&mModelTextureSampler);
        result = mDevice->CreateSamplerState(&samplerDesc, samplerStatePtr);
        if (FAILED(result)) {
            return false;
        }

        return true;
    }

    void RenderPanel::SetModel(MetroModel* model, VFXReader* vfxReader, MetroTexturesDatabase* database) {
        if (mModel != model) {
            SAFE_DELETE(mModel);

            mModel = model;
            mVFXReader = vfxReader;
            mDatabase = database;

            this->CreateModelGeometries();
            this->CreateModelTextures();
            this->UpdateProjectionAndReset();
            this->Render();
        }
    }

    bool RenderPanel::CreateRenderTargets() {
        if (!mDevice || !mDeviceContext || !mSwapChain) {
            return false;
        }

        SAFE_RELEASE(mRenderTargetView);
        SAFE_RELEASE(mDepthStencilBuffer);
        SAFE_RELEASE(mDepthStencilView);

        ID3D11Texture2D* backBuffer = nullptr;
        pin_ptr<ID3D11Texture2D*> backBufferPtr(&backBuffer);
        HRESULT hr = mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)backBufferPtr);
        if (FAILED(hr)) {
            return false;
        }
        pin_ptr<ID3D11RenderTargetView*> rtvPtr(&mRenderTargetView);
        hr = mDevice->CreateRenderTargetView(backBuffer, nullptr, rtvPtr);
        backBuffer->Release();

        D3D11_TEXTURE2D_DESC depthBufferDesc = {};
        D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};
        D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {};

        depthBufferDesc.Width = this->Size.Width;
        depthBufferDesc.Height = this->Size.Height;
        depthBufferDesc.MipLevels = 1;
        depthBufferDesc.ArraySize = 1;
        depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        depthBufferDesc.SampleDesc.Count = 1;
        depthBufferDesc.SampleDesc.Quality = 0;
        depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
        depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;


        pin_ptr<ID3D11Texture2D*> dsbPtr(&mDepthStencilBuffer);
        hr = mDevice->CreateTexture2D(&depthBufferDesc, nullptr, dsbPtr);
        if (FAILED(hr)) {
            return false;
        }

        depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

        pin_ptr<ID3D11DepthStencilView*> dsvPtr(&mDepthStencilView);
        hr = mDevice->CreateDepthStencilView(mDepthStencilBuffer, &depthStencilViewDesc, dsvPtr);
        if (FAILED(hr)) {
            return false;
        }

        mDeviceContext->OMSetRenderTargets(1, rtvPtr, mDepthStencilView);

        D3D11_VIEWPORT viewport = {};
        viewport.Width = (float)this->Size.Width;
        viewport.Height = (float)this->Size.Height;
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;
        viewport.TopLeftX = 0.0f;
        viewport.TopLeftY = 0.0f;

        mDeviceContext->RSSetViewports(1, &viewport);

        return true;
    }

    void RenderPanel::UpdateModelMatrix() {
        mConstantBufferData->matModel = MatRotate(Deg2Rad(mViewingParams->rotation.x), 0.0f, 1.0f, 0.0f) * MatRotate(-Deg2Rad(mViewingParams->rotation.y), 0.0f, 0.0f, 1.0f);
        mConstantBufferData->matModel[3] = vec4(mViewingParams->offset.x, mViewingParams->offset.y, 0.0f, 1.0f);
    }

    void RenderPanel::UpdateViewMatrix() {
        if (mModel) {
            const float r = mConstantBufferData->modelBSphere.w * mViewingParams->zoom;
            mConstantBufferData->matView = MatLookAt(vec3(-r, r, r), vec3(0.0f), vec3(0.0f, 1.0f, 0.0f));
        }
    }

    void RenderPanel::UpdateProjectionAndReset() {
        mViewingParams->zoom = 1.0f;

        const float r = mConstantBufferData->modelBSphere.w;
        this->UpdateViewMatrix();

        mViewingParams->nearFar = vec2(r * 0.001f, r * 100.0f);

        const int w = this->Width;
        const int h = this->Height;
        mConstantBufferData->matProjection = MatPerspective(Deg2Rad(60.0f), scast<float>(w) / scast<float>(h), mViewingParams->nearFar.x, mViewingParams->nearFar.y);

        mConstantBufferData->matModel = MatIdentity;
        mViewingParams->rotation = vec2(0.0f);
        mViewingParams->offset = vec2(0.0f);
    }

    void RenderPanel::CreateModelGeometries() {
        if (!mModel || !mDevice) {
            return;
        }

        if (mModelGeometries) {
            for each (RenderGeometry* rg in mModelGeometries) {
                if (rg) {
                    SAFE_RELEASE(rg->vb);
                    SAFE_RELEASE(rg->ib);
                    delete rg;
                }
            }

            SAFE_DELETE(mModelGeometries);
        }

        const size_t numMeshes = mModel->GetNumMeshes();
        mModelGeometries = gcnew array<RenderGeometry*>(scast<int>(numMeshes));

        mConstantBufferData->modelBSphere = vec4(0.0f, 0.0f, 0.0f, 1.0f);

        AABBox modelBBox;
        modelBBox.Reset();
        for (size_t i = 0; i < numMeshes; ++i) {
            const MetroMesh* mesh = mModel->GetMesh(i);
            if (!mesh->vertices.empty() && !mesh->faces.empty()) {
                RenderGeometry* rg = new RenderGeometry;

                AABBox bbox;
                bbox.Reset();

                rg->texture = nullptr;
                rg->numFaces = mesh->faces.size();

                D3D11_BUFFER_DESC desc = {};
                D3D11_SUBRESOURCE_DATA subData = {};

                //vb
                desc.ByteWidth = scast<UINT>(mesh->vertices.size() * sizeof(MetroVertex));
                desc.Usage = D3D11_USAGE_DEFAULT;
                desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
                subData.pSysMem = mesh->vertices.data();
                mDevice->CreateBuffer(&desc, &subData, &rg->vb);

                //ib
                desc.ByteWidth = scast<UINT>(mesh->faces.size() * sizeof(MetroFace));
                desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
                subData.pSysMem = mesh->faces.data();
                mDevice->CreateBuffer(&desc, &subData, &rg->ib);

                for (const MetroVertex& v : mesh->vertices) {
                    bbox.Absorb(v.pos);
                }

                modelBBox.Absorb(bbox.minimum);
                modelBBox.Absorb(bbox.maximum);

                mConstantBufferData->modelBSphere = vec4(modelBBox.Center(), Length(modelBBox.Extent()));

                mModelGeometries[scast<int>(i)] = rg;
            } else {
                mModelGeometries[scast<int>(i)] = nullptr;
            }
        }
    }

    void RenderPanel::CreateModelTextures() {
        if (!mModel || !mVFXReader) {
            return;
        }

        for each (IntPtr ptr in mModelTextures->Values) {
            RenderTexture* rt = rcast<RenderTexture*>(ptr.ToPointer());
            SAFE_RELEASE(rt->srv);
            SAFE_RELEASE(rt->tex);
        }
        mModelTextures->Clear();

        const size_t numMeshes = mModel->GetNumMeshes();
        for (size_t i = 0; i < numMeshes; ++i) {
            const MetroMesh* mesh = mModel->GetMesh(i);
            RenderGeometry* rg = mModelGeometries[scast<int>(i)];
            if (rg) {
                const CharString& textureName = mesh->materials.front();
                const CharString& sourceName = mDatabase->GetSourceName(textureName);

                String^ texNameManaged = msclr::interop::marshal_as<String^>(textureName);

                const bool contains = mModelTextures->ContainsKey(texNameManaged);
                if (!contains) {
                    CharString texturePath = CharString("content\\textures\\") + (sourceName.empty() ? textureName : sourceName);

                    CharString texturePathBest = texturePath + ".512";
                    size_t textureIdx = mVFXReader->FindFile(texturePathBest);

                    if (textureIdx != MetroFile::InvalidFileIdx) {
                        BytesArray content;
                        if (mVFXReader->ExtractFile(textureIdx, content)) {
                            const MetroFile& mf = mVFXReader->GetFile(textureIdx);

                            MetroTexture texture;
                            if (texture.LoadFromData(content.data(), content.size(), mf.name)) {
                                if (texture.GetFormat() == MetroTexture::TextureFormat::BC7) {
                                    D3D11_TEXTURE2D_DESC desc = {};
                                    D3D11_SUBRESOURCE_DATA subDesc = {};

                                    desc.Width = scast<UINT>(texture.GetWidth());
                                    desc.Height = scast<UINT>(texture.GetHeight());
                                    desc.MipLevels = 1;
                                    desc.ArraySize = 1;
                                    desc.Format = DXGI_FORMAT_BC7_TYPELESS;
                                    desc.SampleDesc.Count = 1;
                                    desc.Usage = D3D11_USAGE_IMMUTABLE;
                                    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

                                    const UINT numBlocksW = (desc.Width + 3) / 4;
                                    const UINT numBlocksH = (desc.Height + 3) / 4;

                                    subDesc.pSysMem = texture.GetRawData();
                                    subDesc.SysMemPitch = numBlocksW * 16;
                                    subDesc.SysMemSlicePitch = subDesc.SysMemPitch * numBlocksH;

                                    ID3D11Texture2D* texture = nullptr;
                                    pin_ptr<ID3D11Texture2D*> texPtr(&texture);
                                    HRESULT hr = mDevice->CreateTexture2D(&desc, &subDesc, texPtr);
                                    if (SUCCEEDED(hr)) {
                                        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
                                        srvDesc.Format = DXGI_FORMAT_BC7_UNORM;
                                        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
                                        srvDesc.Texture2D.MipLevels = 1;
                                        srvDesc.Texture2D.MostDetailedMip = 0;

                                        ID3D11ShaderResourceView* textureSRV = nullptr;
                                        pin_ptr<ID3D11ShaderResourceView*> srvPtr(&textureSRV);
                                        hr = mDevice->CreateShaderResourceView(texture, &srvDesc, srvPtr);
                                        if (FAILED(hr)) {
                                            SAFE_RELEASE(texture);
                                        } else {
                                            RenderTexture* rt = new RenderTexture;
                                            rt->tex = texture;
                                            rt->srv = textureSRV;

                                            rg->texture = rt;
                                            mModelTextures->Add(texNameManaged, IntPtr(rt));
                                        }
                                    }
                                }
                            }
                        }
                    }
                } else {
                    rg->texture = rcast<RenderTexture*>(mModelTextures[texNameManaged].ToPointer());
                }
            }
        }
    }

    void RenderPanel::Render() {
        static const float clearColor[4] = { 40.0f / 255.0f, 113.0f / 255.0f, 134.0f / 255.0f, 1.0f };

        if (mDeviceContext && mSwapChain) {
            mDeviceContext->ClearRenderTargetView(mRenderTargetView, clearColor);
            mDeviceContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

            mat4 modelView = mConstantBufferData->matView * mConstantBufferData->matModel;
            mConstantBufferData->matModelViewProj = mConstantBufferData->matProjection * modelView;

            D3D11_MAPPED_SUBRESOURCE subRes = {};
            mDeviceContext->Map(mModelConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &subRes);
            memcpy(subRes.pData, mConstantBufferData, sizeof(ConstantBufferData));
            mDeviceContext->Unmap(mModelConstantBuffer, 0);

            pin_ptr<ID3D11Buffer*> cbPtr(&mModelConstantBuffer);
            mDeviceContext->VSSetConstantBuffers(0, 1, cbPtr);

            mDeviceContext->VSSetShader(mModelViewerVS, nullptr, 0);
            mDeviceContext->PSSetShader(mModelViewerPS, nullptr, 0);

            pin_ptr<ID3D11SamplerState*> samplerPtr(&mModelTextureSampler);
            mDeviceContext->PSSetSamplers(0, 1, samplerPtr);

            mDeviceContext->IASetInputLayout(mModelInputLayout);
            mDeviceContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

            if (mModelGeometries) {
                for each (RenderGeometry* rg in mModelGeometries) {
                    if (rg) {
                        ID3D11ShaderResourceView* texSRV = (rg->texture) ? rg->texture->srv : nullptr;
                        pin_ptr<ID3D11ShaderResourceView*> srvPtr(&texSRV);
                        mDeviceContext->PSSetShaderResources(0, 1, srvPtr);

                        const UINT stride = sizeof(MetroVertex);
                        const UINT offset = 0;
                        mDeviceContext->IASetVertexBuffers(0, 1, &rg->vb, &stride, &offset);
                        mDeviceContext->IASetIndexBuffer(rg->ib, DXGI_FORMAT_R16_UINT, 0);

                        mDeviceContext->DrawIndexed(scast<UINT>(rg->numFaces * 3), 0, 0);
                    }
                }
            }

            mSwapChain->Present(0, 0);
        }
    }

    void RenderPanel::OnResize(System::EventArgs^ e) {
        Panel::OnResize(e);

        if (mSwapChain) {
            mRenderTargetView->Release();
            mRenderTargetView = nullptr;

            const int w = this->Size.Width;
            const int h = this->Size.Height;

            HRESULT hr = mSwapChain->ResizeBuffers(1, w, h, DXGI_FORMAT_UNKNOWN, 0);
            if (SUCCEEDED(hr)) {
                this->CreateRenderTargets();
            }

            mConstantBufferData->matProjection = MatPerspective(Deg2Rad(60.0f), scast<float>(w) / scast<float>(h), mViewingParams->nearFar.x, mViewingParams->nearFar.y);
        }
    }

    void RenderPanel::OnPaint(System::Windows::Forms::PaintEventArgs^ e) {
        this->Render();
    }

    void RenderPanel::OnMouseDown(System::Windows::Forms::MouseEventArgs^ e) {
        vec2 mp(scast<float>(e->X), scast<float>(e->Y));

        if (e->Button == System::Windows::Forms::MouseButtons::Left) {
            mViewingParams->lastLMPos = mp;
        } else if (e->Button == System::Windows::Forms::MouseButtons::Right) {
            mViewingParams->lastRMPos = mp;
        }

        Panel::OnMouseDown(e);
    }

    void RenderPanel::OnMouseMove(System::Windows::Forms::MouseEventArgs^ e) {
        vec2 mp(scast<float>(e->X), scast<float>(e->Y));

        if (e->Button == System::Windows::Forms::MouseButtons::Left) {
            vec2 delta = mp - mViewingParams->lastLMPos;
            mViewingParams->lastLMPos = mp;

            mViewingParams->rotation += delta * 0.3f;

            if (mViewingParams->rotation.x < 0.0f) {
                mViewingParams->rotation.x += 360.0f;
            } else if (mViewingParams->rotation.x > 360.0f) {
                mViewingParams->rotation.x -= 360.0f;
            }

            if (mViewingParams->rotation.y < 0.0f) {
                mViewingParams->rotation.y += 360.0f;
            } else if (mViewingParams->rotation.y > 360.0f) {
                mViewingParams->rotation.y -= 360.0f;
            }

            this->UpdateModelMatrix();
            this->Render();

        }

        Panel::OnMouseMove(e);
    }

    void RenderPanel::OnMouseWheel(System::Windows::Forms::MouseEventArgs^ e) {
        if (e->Delta > 0) {
            mViewingParams->zoom = std::min<float>(mViewingParams->zoom + 0.1f, 5.0f);
        } else if (e->Delta < 0) {
            mViewingParams->zoom = std::max<float>(mViewingParams->zoom - 0.1f, 0.1f);
        }

        this->UpdateViewMatrix();
        this->Render();
    }
}
