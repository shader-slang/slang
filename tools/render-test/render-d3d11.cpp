﻿// render-d3d11.cpp
#include "render-d3d11.h"

#include "options.h"
#include "render.h"
#include "d3d-util.h"

// In order to use the Slang API, we need to include its header

#include <slang.h>

#include "slang-support.h"

#include "../../source/core/slang-com-ptr.h"

#ifdef _MSC_VER
#pragma warning(disable: 4996)
#endif
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "external/stb/stb_image_write.h"

// We will be rendering with Direct3D 11, so we need to include
// the Windows and D3D11 headers

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#undef WIN32_LEAN_AND_MEAN
#undef NOMINMAX

#include <d3d11_2.h>
#include <d3dcompiler.h>

// We will use the C standard library just for printing error messages.
#include <stdio.h>

#ifdef _MSC_VER
#include <stddef.h>
#if (_MSC_VER < 1900)
#define snprintf sprintf_s
#endif
#endif
//
using namespace Slang;

namespace renderer_test {

class D3D11Renderer : public Renderer, public ShaderCompiler
{
public:
    // Renderer    implementation
    virtual SlangResult initialize(void* inWindowHandle) override;
    virtual void setClearColor(const float color[4]) override;
    virtual void clearFrame() override;
    virtual void presentFrame() override;
    virtual TextureResource* createTextureResource(Resource::Type type, Resource::Usage initialUsage, const TextureResource::Desc& desc, const TextureResource::Data* initData) override;
    virtual BufferResource* createBufferResource(Resource::Usage initialUsage, const BufferResource::Desc& bufferDesc, const void* initData) override;
    virtual SlangResult captureScreenShot(char const* outputPath) override;
    virtual void serializeOutput(BindingState* state, const char* fileName) override;
    virtual InputLayout* createInputLayout( const InputElementDesc* inputElements, UInt inputElementCount) override;
    virtual BindingState * createBindingState(const ShaderInputLayout& layout) override;
    virtual ShaderCompiler* getShaderCompiler() override;
    virtual void* map(BufferResource* buffer, MapFlavor flavor) override;
    virtual void unmap(BufferResource* buffer) override;
    virtual void setInputLayout(InputLayout* inputLayout) override;
    virtual void setPrimitiveTopology(PrimitiveTopology topology) override;
    virtual void setBindingState(BindingState * state);
    virtual void setVertexBuffers(UInt startSlot, UInt slotCount, BufferResource*const* buffers, const UInt* strides,  const UInt* offsets) override;    
    virtual void setShaderProgram(ShaderProgram* inProgram) override;
    virtual void setConstantBuffers(UInt startSlot, UInt slotCount, BufferResource*const* buffers,  const UInt* offsets) override;
    virtual void draw(UInt vertexCount, UInt startVertex) override;
    virtual void dispatchCompute(int x, int y, int z) override;
    virtual void submitGpuWork() override {}
    virtual void waitForGpu() override {}

    // ShaderCompiler implementation
    virtual ShaderProgram* compileProgram(ShaderCompileRequest const& request) override;

    protected:

    struct Binding
    {
        ShaderInputType type;
        InputBufferType bufferType;                        // Only valid if `type` is `Buffer`
        
        ComPtr<ID3D11ShaderResourceView> srv;
        ComPtr<ID3D11UnorderedAccessView> uav;
        ComPtr<ID3D11SamplerState> samplerState;

        RefPtr<Resource> resource;                        /// Can hold texture of buffer

        int binding = 0;
        bool isOutput = false;
    };

    class BindingStateImpl: public BindingState
    {
		public:
        List<Binding> m_bindings;
        int m_numRenderTargets = 0;
    };
    class ShaderProgramImpl: public ShaderProgram
    {
		public:
        ComPtr<ID3D11VertexShader> m_vertexShader;
        ComPtr<ID3D11PixelShader> m_pixelShader;
        ComPtr<ID3D11ComputeShader> m_computeShader;
    };

    class BufferResourceImpl: public BufferResource
    {
		public:
        typedef BufferResource Parent;

        BufferResourceImpl(const Desc& desc, Usage initialUsage):
            Parent(desc),
            m_initialUsage(initialUsage)
        {
        }

        MapFlavor m_mapFlavor;
        Usage m_initialUsage;
        ComPtr<ID3D11Buffer> m_buffer;
        ComPtr<ID3D11Buffer> m_staging;
    };
    class TextureResourceImpl : public TextureResource
    {
    public:
        typedef TextureResource Parent;

        TextureResourceImpl(Type type, const Desc& desc, Usage initialUsage) :
            Parent(type, desc),
            m_initialUsage(initialUsage)
        {
        }
        Usage m_initialUsage;
        ComPtr<ID3D11Resource> m_resource;
    };

	class InputLayoutImpl: public InputLayout
	{	
		public:
		ComPtr<ID3D11InputLayout> m_layout;
	};

        /// Capture a texture to a file
    static HRESULT captureTextureToFile(ID3D11Device* device, ID3D11DeviceContext* context, ID3D11Texture2D* texture, char const* outputPath);

    //void* map(ID3D11Buffer* buffer, MapFlavor flavor);
    //void unmap(ID3D11Buffer* buffer);

    Result createInputBuffer(const InputBufferDesc& bufferDesc, bool isOutput, const List<unsigned int>& bufferData, 
		RefPtr<Resource>& resourceOut, ComPtr<ID3D11UnorderedAccessView>& viewOut, ComPtr<ID3D11ShaderResourceView>& srvOut);

    Result createInputTexture(const InputTextureDesc& inputDesc, RefPtr<Resource>& resourceOut, ComPtr<ID3D11ShaderResourceView>& viewOut);

    Result createInputSampler(const InputSamplerDesc& inputDesc, ComPtr<ID3D11SamplerState>& stateOut);

    void applyBindingState(bool isCompute);

    ComPtr<IDXGISwapChain> m_swapChain;
    ComPtr<ID3D11Device> m_device;
    ComPtr<ID3D11DeviceContext> m_immediateContext;
    ComPtr<ID3D11Texture2D> m_backBufferTexture;

    List<ComPtr<ID3D11RenderTargetView> > m_renderTargetViews;
    List<ComPtr<ID3D11Texture2D> > m_renderTargetTextures;

    RefPtr<BindingStateImpl> m_currentBindings;

    float m_clearColor[4] = { 0, 0, 0, 0 };
};

Renderer* createD3D11Renderer()
{
    return new D3D11Renderer();
}

/* static */HRESULT D3D11Renderer::captureTextureToFile(ID3D11Device* device, ID3D11DeviceContext* context,
    ID3D11Texture2D* texture, char const* outputPath)
{
    if (!context) return E_INVALIDARG;
    if (!texture) return E_INVALIDARG;

    D3D11_TEXTURE2D_DESC textureDesc;
    texture->GetDesc(&textureDesc);

    // Don't bother supporting MSAA for right now
    if (textureDesc.SampleDesc.Count > 1)
    {
        fprintf(stderr, "ERROR: cannot capture multi-sample texture\n");
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    ComPtr<ID3D11Texture2D> stagingTexture;
	
    if (textureDesc.Usage == D3D11_USAGE_STAGING && (textureDesc.CPUAccessFlags & D3D11_CPU_ACCESS_READ))
    {
        stagingTexture = texture;
    }
    else
    {
        // Modify the descriptor to give us a staging texture
        textureDesc.BindFlags = 0;
        textureDesc.MiscFlags &= ~D3D11_RESOURCE_MISC_TEXTURECUBE;
        textureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
        textureDesc.Usage = D3D11_USAGE_STAGING;

        hr = device->CreateTexture2D(&textureDesc, 0, stagingTexture.writeRef());
        if (FAILED(hr))
        {
            fprintf(stderr, "ERROR: failed to create staging texture\n");
            return hr;
        }

        context->CopyResource(stagingTexture, texture);
    }

    // Now just read back texels from the staging textures

    D3D11_MAPPED_SUBRESOURCE mappedResource;
    hr = context->Map(stagingTexture, 0, D3D11_MAP_READ, 0, &mappedResource);
    if (FAILED(hr))
    {
        fprintf(stderr, "ERROR: failed to map texture for read\n");
        return hr;
    }

    int stbResult = stbi_write_png(outputPath, textureDesc.Width, textureDesc.Height, 4,
        mappedResource.pData, mappedResource.RowPitch);

    // Make sure to unmap
    context->Unmap(stagingTexture, 0);

    if (!stbResult)
    {
        fprintf(stderr, "ERROR: failed to write texture to file\n");
        return E_UNEXPECTED;
    }

    return S_OK;
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!! Renderer interface !!!!!!!!!!!!!!!!!!!!!!!!!!

SlangResult D3D11Renderer::initialize(void* inWindowHandle)
{
    auto windowHandle = (HWND)inWindowHandle;

    // Rather than statically link against D3D, we load it dynamically.
    HMODULE d3dModule = LoadLibraryA("d3d11.dll");
    if (!d3dModule)
    {
        fprintf(stderr, "error: failed load 'd3d11.dll'\n");
        return SLANG_FAIL;
    }

    PFN_D3D11_CREATE_DEVICE_AND_SWAP_CHAIN D3D11CreateDeviceAndSwapChain_ =
        (PFN_D3D11_CREATE_DEVICE_AND_SWAP_CHAIN)GetProcAddress(d3dModule, "D3D11CreateDeviceAndSwapChain");
    if (!D3D11CreateDeviceAndSwapChain_)
    {
        fprintf(stderr,
            "error: failed load symbol 'D3D11CreateDeviceAndSwapChain'\n");
        return SLANG_FAIL;
    }

    // We create our device in debug mode, just so that we can check that the
    // example doesn't trigger warnings.
    UINT deviceFlags = 0;
    deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;

    // Our swap chain uses RGBA8 with sRGB, with double buffering.
    DXGI_SWAP_CHAIN_DESC swapChainDesc = { 0 };
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

    // Note(tfoley): Disabling sRGB for DX back buffer for now, so that we
    // can get consistent output with OpenGL, where setting up sRGB will
    // probably be more involved.
    // swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.BufferCount = 2;
    swapChainDesc.OutputWindow = windowHandle;
    swapChainDesc.Windowed = TRUE;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    swapChainDesc.Flags = 0;

    // We will ask for the highest feature level that can be supported.
    const D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
        D3D_FEATURE_LEVEL_9_3,
        D3D_FEATURE_LEVEL_9_2,
        D3D_FEATURE_LEVEL_9_1,
    };
    D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_9_1;
    const int totalNumFeatureLevels = sizeof(featureLevels) / sizeof(featureLevels[0]);

    // On a machine that does not have an up-to-date version of D3D installed,
    // the `D3D11CreateDeviceAndSwapChain` call will fail with `E_INVALIDARG`
    // if you ask for featuer level 11_1. The workaround is to call
    // `D3D11CreateDeviceAndSwapChain` up to twice: the first time with 11_1
    // at the start of the list of requested feature levels, and the second
    // time without it.

    for (int ii = 0; ii < 2; ++ii)
    {
        const HRESULT hr = D3D11CreateDeviceAndSwapChain_(
            nullptr,                    // adapter (use default)
            D3D_DRIVER_TYPE_REFERENCE,
            //D3D_DRIVER_TYPE_HARDWARE,
			nullptr,                    // software
            deviceFlags,
            &featureLevels[ii],
            totalNumFeatureLevels - ii,
            D3D11_SDK_VERSION,
            &swapChainDesc,
            m_swapChain.writeRef(),
            m_device.writeRef(),
            &featureLevel,
            m_immediateContext.writeRef());

        // Failures with `E_INVALIDARG` might be due to feature level 11_1
        // not being supported. 
        if (hr == E_INVALIDARG)
        {
            continue;
        }

        // Other failures are real, though.
        SLANG_RETURN_ON_FAIL(hr);
        // We must have a swap chain
        break;
    }

    // After we've created the swap chain, we can request a pointer to the
    // back buffer as a D3D11 texture, and create a render-target view from it.

    static const IID kIID_ID3D11Texture2D = {
        0x6f15aaf2, 0xd208, 0x4e89, 0x9a, 0xb4, 0x48,
        0x95, 0x35, 0xd3, 0x4f, 0x9c };

    SLANG_RETURN_ON_FAIL(m_swapChain->GetBuffer(0, kIID_ID3D11Texture2D, (void**)m_backBufferTexture.writeRef()));

    for (int i = 0; i < 8; i++)
    {
        ComPtr<ID3D11Texture2D> texture;
        D3D11_TEXTURE2D_DESC textureDesc;
        m_backBufferTexture->GetDesc(&textureDesc);
        SLANG_RETURN_ON_FAIL(m_device->CreateTexture2D(&textureDesc, nullptr, texture.writeRef()));

        ComPtr<ID3D11RenderTargetView> rtv;
        D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
        rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        rtvDesc.Texture2D.MipSlice = 0;
        rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
        SLANG_RETURN_ON_FAIL(m_device->CreateRenderTargetView(texture, &rtvDesc, rtv.writeRef()));

        m_renderTargetViews.Add(rtv);
        m_renderTargetTextures.Add(texture);
    }

    m_immediateContext->OMSetRenderTargets((UINT)m_renderTargetViews.Count(), m_renderTargetViews.Buffer()->readRef(), nullptr);

    // Similarly, we are going to set up a viewport once, and then never
    // switch, since this is a simple test app.
    D3D11_VIEWPORT viewport;
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = (float)gWindowWidth;
    viewport.Height = (float)gWindowHeight;
    viewport.MaxDepth = 1; // TODO(tfoley): use reversed depth
    viewport.MinDepth = 0;
    m_immediateContext->RSSetViewports(1, &viewport);

    return SLANG_OK;
}

void D3D11Renderer::setClearColor(const float color[4])
{
    memcpy(m_clearColor, color, sizeof(m_clearColor));
}

void D3D11Renderer::clearFrame()
{
    for (auto i = 0u; i < m_renderTargetViews.Count(); i++)
	{
        m_immediateContext->ClearRenderTargetView(m_renderTargetViews[i], m_clearColor);
	}
}

void D3D11Renderer::presentFrame()
{
    m_immediateContext->CopyResource(m_backBufferTexture, m_renderTargetTextures[0]);
    m_swapChain->Present(0, 0);
}

SlangResult D3D11Renderer::captureScreenShot(const char* outputPath)
{
    HRESULT hr = captureTextureToFile(m_device, m_immediateContext, m_renderTargetTextures[0], outputPath);
    if (FAILED(hr))
    {
        fprintf(stderr, "error: could not capture screen-shot to '%s'\n", outputPath);
        SLANG_RETURN_ON_FAIL(hr);
    }
    return SLANG_OK;
}

ShaderCompiler* D3D11Renderer::getShaderCompiler()
{
    return this;
}

static D3D11_BIND_FLAG _calcResourceFlag(Resource::BindFlag::Enum bindFlag)
{
    typedef Resource::BindFlag BindFlag;
    switch (bindFlag)
    {
        case BindFlag::VertexBuffer:            return D3D11_BIND_VERTEX_BUFFER;
        case BindFlag::IndexBuffer:             return D3D11_BIND_INDEX_BUFFER;
        case BindFlag::ConstantBuffer:          return D3D11_BIND_CONSTANT_BUFFER;
        case BindFlag::StreamOutput:            return D3D11_BIND_STREAM_OUTPUT;
        case BindFlag::RenderTarget:            return D3D11_BIND_RENDER_TARGET;
        case BindFlag::DepthStencil:            return D3D11_BIND_DEPTH_STENCIL;
        case BindFlag::UnorderedAccess:         return D3D11_BIND_UNORDERED_ACCESS;
        case BindFlag::PixelShaderResource:     return D3D11_BIND_SHADER_RESOURCE;
        case BindFlag::NonPixelShaderResource:  return D3D11_BIND_SHADER_RESOURCE;
        default:                                return D3D11_BIND_FLAG(0);
    }
}

static int _calcResourceBindFlags(int bindFlags)
{
    int dstFlags = 0;
    while (bindFlags)
    {
        int lsb = bindFlags & -bindFlags;

        dstFlags |= _calcResourceFlag(Resource::BindFlag::Enum(lsb));
        bindFlags &= ~lsb;
    }
    return dstFlags;
}

static int _calcResourceAccessFlags(int accessFlags)
{
    switch (accessFlags)
    {
        case 0:         return 0;
        case Resource::AccessFlag::Read:            return D3D11_CPU_ACCESS_READ;
        case Resource::AccessFlag::Write:           return D3D11_CPU_ACCESS_WRITE;
        case Resource::AccessFlag::Read |
             Resource::AccessFlag::Write:           return D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
        default: assert(!"Invalid flags"); return 0;
    }
}

TextureResource* D3D11Renderer::createTextureResource(Resource::Type type, Resource::Usage initialUsage, const TextureResource::Desc& descIn, const TextureResource::Data* initData)
{
    TextureResource::Desc srcDesc(descIn);
    srcDesc.setDefaults(type, initialUsage);
 
    const int effectiveArraySize = srcDesc.calcEffectiveArraySize(type);
    
    assert(initData);
    assert(initData->numSubResources == srcDesc.numMipLevels * effectiveArraySize * srcDesc.size.depth);

    const DXGI_FORMAT format = D3DUtil::getMapFormat(srcDesc.format);
    if (format == DXGI_FORMAT_UNKNOWN)
    {
        return nullptr;
    }

    const int bindFlags = _calcResourceBindFlags(srcDesc.bindFlags);

    // Set up the initialize data
    List<D3D11_SUBRESOURCE_DATA> subRes;
    subRes.SetSize(srcDesc.numMipLevels * effectiveArraySize);
    {
        int subResourceIndex = 0;
        for (int i = 0; i < effectiveArraySize; i++)
        {
            for (int j = 0; j < srcDesc.numMipLevels; j++)
            {
                const int mipHeight = TextureResource::calcMipSize(srcDesc.size.height, j);

                D3D11_SUBRESOURCE_DATA& data = subRes[subResourceIndex];

                data.pSysMem = initData->subResources[subResourceIndex];

                data.SysMemPitch = UINT(initData->mipRowStrides[j]);
                data.SysMemSlicePitch = UINT(initData->mipRowStrides[j] * mipHeight);

                subResourceIndex++;
            }
        }
    }

    const int accessFlags = _calcResourceAccessFlags(srcDesc.cpuAccessFlags);

    RefPtr<TextureResourceImpl> texture(new TextureResourceImpl(type, srcDesc, initialUsage));

    switch (type)
    {
        case Resource::Type::Texture1D:
        {
            D3D11_TEXTURE1D_DESC desc = { 0 };
            desc.BindFlags = bindFlags;
            desc.CPUAccessFlags = accessFlags;
            desc.Format = format;
            desc.MiscFlags = 0;
            desc.MipLevels = srcDesc.numMipLevels;
            desc.ArraySize = effectiveArraySize; 
            desc.Width = srcDesc.size.width; 
            desc.Usage = D3D11_USAGE_DEFAULT;
            
            ComPtr<ID3D11Texture1D> texture1D;
            SLANG_RETURN_NULL_ON_FAIL(m_device->CreateTexture1D(&desc, subRes.Buffer(), texture1D.writeRef()));

            texture->m_resource = texture1D;
            break;
        }
        case Resource::Type::TextureCube:
        case Resource::Type::Texture2D:
        {
            D3D11_TEXTURE2D_DESC desc = { 0 };
            desc.BindFlags = bindFlags;
            desc.CPUAccessFlags = accessFlags;
            desc.Format = format;
            desc.MiscFlags = 0;
            desc.MipLevels = srcDesc.numMipLevels;
            desc.ArraySize = effectiveArraySize; 
       
            desc.Width = srcDesc.size.width;
            desc.Height = srcDesc.size.height;
            desc.Usage = D3D11_USAGE_DEFAULT;
            desc.SampleDesc.Count = srcDesc.sampleDesc.numSamples;
            desc.SampleDesc.Quality = srcDesc.sampleDesc.quality;

            if (type == Resource::Type::TextureCube)
            {
                desc.MiscFlags |= D3D11_RESOURCE_MISC_TEXTURECUBE;
            }

            ComPtr<ID3D11Texture2D> texture2D;
            SLANG_RETURN_NULL_ON_FAIL(m_device->CreateTexture2D(&desc, subRes.Buffer(), texture2D.writeRef()));

            texture->m_resource = texture2D;
            break;
        }
        case Resource::Type::Texture3D:
        {
            D3D11_TEXTURE3D_DESC desc = { 0 };
            desc.BindFlags = bindFlags;
            desc.CPUAccessFlags = accessFlags;
            desc.Format = format;
            desc.MiscFlags = 0;
            desc.MipLevels = srcDesc.numMipLevels;
            desc.Width = srcDesc.size.width; 
            desc.Height = srcDesc.size.height;
            desc.Depth = srcDesc.size.depth;
            desc.Usage = D3D11_USAGE_DEFAULT;
            
            ComPtr<ID3D11Texture3D> texture3D;
            SLANG_RETURN_NULL_ON_FAIL(m_device->CreateTexture3D(&desc, subRes.Buffer(), texture3D.writeRef()));

            texture->m_resource = texture3D;
            break;
        }
        default: return nullptr;
    }

    return texture.detach();
}

BufferResource* D3D11Renderer::createBufferResource(Resource::Usage initialUsage, const BufferResource::Desc& descIn, const void* initData)
{    
    BufferResource::Desc srcDesc(descIn);
    srcDesc.setDefaults(initialUsage);

    // Make aligned to 256 bytes... not sure why, but if you remove this the tests do fail.
    const size_t alignedSizeInBytes = D3DUtil::calcAligned(srcDesc.sizeInBytes, 256);
        
    // Hack to make the initialization never read from out of bounds memory, by copying into a buffer
    List<uint8_t> initDataBuffer;
    if (initData && alignedSizeInBytes > srcDesc.sizeInBytes)
    {
        initDataBuffer.SetSize(alignedSizeInBytes);
        ::memcpy(initDataBuffer.Buffer(), initData, srcDesc.sizeInBytes);
        initData = initDataBuffer.Buffer();
    }

    D3D11_BUFFER_DESC bufferDesc = { 0 };
    bufferDesc.ByteWidth = UINT(alignedSizeInBytes);
    bufferDesc.BindFlags = _calcResourceBindFlags(srcDesc.bindFlags);
    // For read we'll need to do some staging
    bufferDesc.CPUAccessFlags = _calcResourceAccessFlags(descIn.cpuAccessFlags & Resource::AccessFlag::Write);
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;

    // If written by CPU, make it dynamic
    if (descIn.cpuAccessFlags & Resource::AccessFlag::Write)
    {
        bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    }

    switch (initialUsage)
    {
        case Resource::Usage::ConstantBuffer:
        {
            // We'll just assume ConstantBuffers are dynamic for now
            bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
            break;
        }
        default: break;
    }

    if (bufferDesc.BindFlags & (D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE))
    {
        //desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
        if (srcDesc.elementSize != 0)
        {
            bufferDesc.StructureByteStride = srcDesc.elementSize;
            bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
        }
        else
        {
            bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
        }
    }

    D3D11_SUBRESOURCE_DATA subResourceData = { 0 };
    subResourceData.pSysMem = initData;

    RefPtr<BufferResourceImpl> buffer(new BufferResourceImpl(srcDesc, initialUsage));

	SLANG_RETURN_NULL_ON_FAIL(m_device->CreateBuffer(&bufferDesc, initData ? &subResourceData : nullptr, buffer->m_buffer.writeRef()));
    
    if (srcDesc.cpuAccessFlags & Resource::AccessFlag::Read)
    {
        D3D11_BUFFER_DESC bufDesc = {};
        bufDesc.BindFlags = 0;
        bufDesc.ByteWidth = (UINT)alignedSizeInBytes;
        bufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
        bufDesc.Usage = D3D11_USAGE_STAGING;

        SLANG_RETURN_NULL_ON_FAIL(m_device->CreateBuffer(&bufDesc, nullptr, buffer->m_staging.writeRef()));
    }

    return buffer.detach();
}

InputLayout* D3D11Renderer::createInputLayout(const InputElementDesc* inputElementsIn, UInt inputElementCount)
{
    D3D11_INPUT_ELEMENT_DESC inputElements[16] = {};

    char hlslBuffer[1024];
    char* hlslCursor = &hlslBuffer[0];

    hlslCursor += sprintf(hlslCursor, "float4 main(\n");

    for (UInt ii = 0; ii < inputElementCount; ++ii)
    {
        inputElements[ii].SemanticName = inputElementsIn[ii].semanticName;
        inputElements[ii].SemanticIndex = (UINT)inputElementsIn[ii].semanticIndex;
        inputElements[ii].Format = D3DUtil::getMapFormat(inputElementsIn[ii].format);
        inputElements[ii].InputSlot = 0;
        inputElements[ii].AlignedByteOffset = (UINT)inputElementsIn[ii].offset;
        inputElements[ii].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
        inputElements[ii].InstanceDataStepRate = 0;

        if (ii != 0)
        {
            hlslCursor += sprintf(hlslCursor, ",\n");
        }

        char const* typeName = "Unknown";
        switch (inputElementsIn[ii].format)
        {
            case Format::RGB_Float32:
                typeName = "float3";
                break;
            case Format::RG_Float32:
                typeName = "float2";
                break;
            case Format::R_Float32:
                typeName = "float";
                break;
            default:
                return nullptr;
        }

        hlslCursor += sprintf(hlslCursor, "%s a%d : %s%d",
            typeName,
            (int)ii,
            inputElementsIn[ii].semanticName,
            (int)inputElementsIn[ii].semanticIndex);
    }

    hlslCursor += sprintf(hlslCursor, "\n) : SV_Position { return 0; }");

	ComPtr<ID3DBlob> vertexShaderBlob;
	SLANG_RETURN_NULL_ON_FAIL(D3DUtil::compileHLSLShader("inputLayout", hlslBuffer, "main", "vs_5_0", vertexShaderBlob));

    ComPtr<ID3D11InputLayout> inputLayout;
	SLANG_RETURN_NULL_ON_FAIL(m_device->CreateInputLayout(&inputElements[0], (UINT)inputElementCount, vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(),
        inputLayout.writeRef()));

	InputLayoutImpl* impl = new InputLayoutImpl;
	impl->m_layout.swap(inputLayout);

	return impl;
}

void* D3D11Renderer::map(BufferResource* bufferIn, MapFlavor flavor)
{
    BufferResourceImpl* bufferResource = static_cast<BufferResourceImpl*>(bufferIn);

    D3D11_MAP mapType;
    ID3D11Buffer* buffer = bufferResource->m_buffer;

    switch (flavor)
    {
        case MapFlavor::WriteDiscard:
            mapType = D3D11_MAP_WRITE_DISCARD;
            break;
        case MapFlavor::HostWrite:
            mapType = D3D11_MAP_WRITE;
            break;
        case MapFlavor::HostRead:
            mapType = D3D11_MAP_READ;
    
            buffer = bufferResource->m_staging;
            if (!buffer)
            {
                return nullptr;
            }
          
            // Okay copy the data over
            m_immediateContext->CopyResource(buffer, bufferResource->m_buffer);

            break;
        default:
            return nullptr;
    }

    // We update our constant buffer per-frame, just for the purposes
    // of the example, but we don't actually load different data
    // per-frame (we always use an identity projection).
    D3D11_MAPPED_SUBRESOURCE mappedSub;
    SLANG_RETURN_NULL_ON_FAIL(m_immediateContext->Map(buffer, 0, mapType, 0, &mappedSub));

    bufferResource->m_mapFlavor = flavor;

    return mappedSub.pData;
}

void D3D11Renderer::unmap(BufferResource* bufferIn)
{
    BufferResourceImpl* bufferResource = static_cast<BufferResourceImpl*>(bufferIn);
    ID3D11Buffer* buffer = (bufferResource->m_mapFlavor == MapFlavor::HostRead) ? bufferResource->m_staging : bufferResource->m_buffer;
    m_immediateContext->Unmap(buffer, 0);
}

void D3D11Renderer::setInputLayout(InputLayout* inputLayoutIn)
{
    auto inputLayout = static_cast<InputLayoutImpl*>(inputLayoutIn);
    m_immediateContext->IASetInputLayout(inputLayout->m_layout);
}

void D3D11Renderer::setPrimitiveTopology(PrimitiveTopology topology)
{
    m_immediateContext->IASetPrimitiveTopology(D3DUtil::getPrimitiveTopology(topology)); 
}

void D3D11Renderer::setVertexBuffers(UInt startSlot, UInt slotCount, BufferResource*const* buffersIn, const UInt* stridesIn, const UInt* offsetsIn)
{
    static const int kMaxVertexBuffers = 16;
	assert(slotCount <= kMaxVertexBuffers);

    UINT vertexStrides[kMaxVertexBuffers];
    UINT vertexOffsets[kMaxVertexBuffers];
	ID3D11Buffer* dxBuffers[kMaxVertexBuffers];

	auto buffers = (BufferResourceImpl*const*)buffersIn;

    for (UInt ii = 0; ii < slotCount; ++ii)
    {
        vertexStrides[ii] = (UINT)stridesIn[ii];
        vertexOffsets[ii] = (UINT)offsetsIn[ii];
		dxBuffers[ii] = buffers[ii]->m_buffer;
	}

    m_immediateContext->IASetVertexBuffers((UINT)startSlot, (UINT)slotCount, dxBuffers, &vertexStrides[0], &vertexOffsets[0]);
}

void D3D11Renderer::setShaderProgram(ShaderProgram* programIn)
{
    auto program = (ShaderProgramImpl*)programIn;
    m_immediateContext->CSSetShader(program->m_computeShader, nullptr, 0);
    m_immediateContext->VSSetShader(program->m_vertexShader, nullptr, 0);
    m_immediateContext->PSSetShader(program->m_pixelShader, nullptr, 0);
}

void D3D11Renderer::setConstantBuffers(UInt startSlot, UInt slotCount, BufferResource*const* buffersIn, const UInt* offsetsIn)
{
	static const int kMaxConstantBuffers = 16;
	assert(slotCount <= kMaxConstantBuffers);

    // TODO: actually use those offsets

    auto buffers = (BufferResourceImpl*const*)buffersIn;

	// Copy out the actual dx buffers
	ID3D11Buffer* dxBuffers[kMaxConstantBuffers];
	for (UInt i = 0; i < slotCount; i++)
	{
		dxBuffers[i] = buffers[i]->m_buffer;
	}

    m_immediateContext->VSSetConstantBuffers((UINT)startSlot, (UINT)slotCount, dxBuffers);
    m_immediateContext->VSSetConstantBuffers((UINT)startSlot, (UINT)slotCount, dxBuffers);
}

void D3D11Renderer::draw(UInt vertexCount, UInt startVertex)
{
    applyBindingState(false);
    m_immediateContext->Draw((UINT)vertexCount, (UINT)startVertex);
}

ShaderProgram* D3D11Renderer::compileProgram(const ShaderCompileRequest& request)
{
    if (request.computeShader.name)
    {
		ComPtr<ID3DBlob> computeShaderBlob;
		SLANG_RETURN_NULL_ON_FAIL(D3DUtil::compileHLSLShader(request.computeShader.source.path, request.computeShader.source.dataBegin, request.computeShader.name, request.computeShader.profile, computeShaderBlob));

        ComPtr<ID3D11ComputeShader> computeShader;
        SLANG_RETURN_NULL_ON_FAIL(m_device->CreateComputeShader(computeShaderBlob->GetBufferPointer(), computeShaderBlob->GetBufferSize(), nullptr, computeShader.writeRef()));

        ShaderProgramImpl* shaderProgram = new ShaderProgramImpl();
        shaderProgram->m_computeShader.swap(computeShader);
        return shaderProgram;
    }
    else
    {
		ComPtr<ID3DBlob> vertexShaderBlob, fragmentShaderBlob;
        SLANG_RETURN_NULL_ON_FAIL(D3DUtil::compileHLSLShader(request.vertexShader.source.path, request.vertexShader.source.dataBegin, request.vertexShader.name, request.vertexShader.profile, vertexShaderBlob));
        SLANG_RETURN_NULL_ON_FAIL(D3DUtil::compileHLSLShader(request.fragmentShader.source.path, request.fragmentShader.source.dataBegin, request.fragmentShader.name, request.fragmentShader.profile, fragmentShaderBlob));
        
        ComPtr<ID3D11VertexShader> vertexShader;
        ComPtr<ID3D11PixelShader> pixelShader;

        SLANG_RETURN_NULL_ON_FAIL(m_device->CreateVertexShader(vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(), nullptr, vertexShader.writeRef()));
        SLANG_RETURN_NULL_ON_FAIL(m_device->CreatePixelShader(fragmentShaderBlob->GetBufferPointer(), fragmentShaderBlob->GetBufferSize(), nullptr, pixelShader.writeRef()));

        ShaderProgramImpl* shaderProgram = new ShaderProgramImpl();
        shaderProgram->m_vertexShader.swap(vertexShader);
        shaderProgram->m_pixelShader.swap(pixelShader);
        return shaderProgram;
    }
}

void D3D11Renderer::dispatchCompute(int x, int y, int z)
{
    applyBindingState(true);
    m_immediateContext->Dispatch(x, y, z);
}

Result D3D11Renderer::createInputBuffer(const InputBufferDesc& bufferDesc, bool isOutput, const List<unsigned int>& bufferData, 
    RefPtr<Resource>& resourceOut, ComPtr<ID3D11UnorderedAccessView>& viewOut, ComPtr<ID3D11ShaderResourceView>& srvOut)
{
    const size_t bufferSize = bufferData.Count() * sizeof(unsigned int);
    RefPtr<BufferResource> bufferResource;
    SLANG_RETURN_ON_FAIL(createInputBufferResource(bufferDesc, isOutput, bufferSize, bufferData.Buffer(), this, bufferResource));

    BufferResourceImpl* bufferImpl = static_cast<BufferResourceImpl*>(bufferResource.Ptr());

    const int elemSize = bufferDesc.stride <= 0 ? 1 : bufferDesc.stride;
    if (bufferDesc.type == InputBufferType::StorageBuffer)
    {
        D3D11_UNORDERED_ACCESS_VIEW_DESC viewDesc;
        memset(&viewDesc, 0, sizeof(viewDesc));
        viewDesc.Buffer.FirstElement = 0;
        viewDesc.Buffer.NumElements = (UINT)(bufferData.Count() * sizeof(unsigned int) / elemSize);
        viewDesc.Buffer.Flags = 0;
        viewDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
        viewDesc.Format = DXGI_FORMAT_UNKNOWN;

        if (bufferDesc.stride == 0)
        {
            // TODO: are there UAV cases we need to handle that are neither
            // raw nor structured? RWBuffer<T> would be one...

            viewDesc.Buffer.Flags |= D3D11_BUFFER_UAV_FLAG_RAW;
            viewDesc.Format = DXGI_FORMAT_R32_TYPELESS;
        }

        SLANG_RETURN_ON_FAIL(m_device->CreateUnorderedAccessView(bufferImpl->m_buffer, &viewDesc, viewOut.writeRef()));
    }
    if (bufferDesc.type != InputBufferType::ConstantBuffer)
    {
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
        memset(&srvDesc, 0, sizeof(srvDesc));
        srvDesc.Buffer.FirstElement = 0;
        srvDesc.Buffer.ElementWidth = elemSize;
        srvDesc.Buffer.NumElements = (UINT)(bufferData.Count() * sizeof(unsigned int) / elemSize);
        srvDesc.Buffer.ElementOffset = 0;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
        srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	
		if (bufferDesc.stride == 0)
		{
			srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
		}

        SLANG_RETURN_ON_FAIL(m_device->CreateShaderResourceView(bufferImpl->m_buffer, &srvDesc, srvOut.writeRef()));
    }

    resourceOut = bufferResource;

	return SLANG_OK;
}

Result D3D11Renderer::createInputTexture(const InputTextureDesc& inputDesc, RefPtr<Resource>& resourceOut, ComPtr<ID3D11ShaderResourceView>& viewOut)
{
	ComPtr<ID3D11ShaderResourceView> view;

    int bindFlags = 0;

    RefPtr<TextureResource> texture;
    SLANG_RETURN_ON_FAIL(generateTextureResource(inputDesc, bindFlags, this, texture));

    //DXGI_FORMAT format = textureImpl->m_resource->GetD
    const TextureResource::Desc& textureDesc = texture->getDesc();

    DXGI_FORMAT format = D3DUtil::getMapFormat(textureDesc.format);

    TextureResourceImpl* textureImpl = static_cast<TextureResourceImpl*>(texture.Ptr());

    if (inputDesc.dimension == 1)
    {
        D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
        viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1D;
        if (inputDesc.arrayLength != 0)
            viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1DARRAY;
        viewDesc.Texture1D.MipLevels = textureDesc.numMipLevels;
        viewDesc.Texture1D.MostDetailedMip = 0;
        viewDesc.Texture1DArray.ArraySize = textureDesc.arraySize;
        viewDesc.Texture1DArray.FirstArraySlice = 0;
        viewDesc.Texture1DArray.MipLevels = textureDesc.numMipLevels;
        viewDesc.Texture1DArray.MostDetailedMip = 0;
        viewDesc.Format = format; 
        m_device->CreateShaderResourceView(textureImpl->m_resource, &viewDesc, view.writeRef());
    }
    else if (inputDesc.dimension == 2)
    {
        D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;

        if (inputDesc.isCube)
        {
            viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
            viewDesc.TextureCube.MipLevels = textureDesc.numMipLevels;
            viewDesc.TextureCube.MostDetailedMip = 0;
            viewDesc.TextureCubeArray.MipLevels = textureDesc.numMipLevels;
            viewDesc.TextureCubeArray.MostDetailedMip = 0;
            viewDesc.TextureCubeArray.First2DArrayFace = 0;
            viewDesc.TextureCubeArray.NumCubes = inputDesc.arrayLength;
        }
        else
        {
            viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
            viewDesc.Texture2D.MipLevels = textureDesc.numMipLevels;
            viewDesc.Texture2D.MostDetailedMip = 0;
            viewDesc.Texture2DArray.ArraySize = textureDesc.arraySize;
            viewDesc.Texture2DArray.FirstArraySlice = 0;
            viewDesc.Texture2DArray.MipLevels = textureDesc.numMipLevels;
            viewDesc.Texture2DArray.MostDetailedMip = 0;
        }
        if (inputDesc.arrayLength != 0)
            viewDesc.ViewDimension = (D3D11_SRV_DIMENSION)(int)(viewDesc.ViewDimension + 1);
        
        viewDesc.Format = format; 
        
        SLANG_RETURN_ON_FAIL(m_device->CreateShaderResourceView(textureImpl->m_resource, &viewDesc, view.writeRef()));
    }
    else if (inputDesc.dimension == 3)
    {
        D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
        viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
		viewDesc.Texture3D.MipLevels = 1;
        viewDesc.Texture3D.MostDetailedMip = 0;
        viewDesc.Format = format;
        m_device->CreateShaderResourceView(textureImpl->m_resource, &viewDesc, view.writeRef());
    }

    resourceOut = texture;
	viewOut.swap(view);
	return SLANG_OK;
}

Result D3D11Renderer::createInputSampler(const InputSamplerDesc& inputDesc, ComPtr<ID3D11SamplerState>& stateOut)
{
    D3D11_SAMPLER_DESC desc;
    memset(&desc, 0, sizeof(desc));
    desc.AddressU = desc.AddressV = desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    if (inputDesc.isCompareSampler)
    {
        desc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
        desc.Filter = D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;
        desc.MinLOD = desc.MaxLOD = 0.0f;
    }
    else
    {
        desc.Filter = D3D11_FILTER_ANISOTROPIC;
        desc.MaxAnisotropy = 8;
        desc.MinLOD = 0.0f;
        desc.MaxLOD = 100.0f;
    }
    return m_device->CreateSamplerState(&desc, stateOut.writeRef());
}

BindingState* D3D11Renderer::createBindingState(const ShaderInputLayout& layout)
{
    RefPtr<BindingStateImpl> bindingState(new BindingStateImpl);

    const List<ShaderInputLayoutEntry>& srcBindings = layout.entries;
    const int numBindings = int(srcBindings.Count());

    List<Binding>& dstBindings = bindingState->m_bindings;
    dstBindings.SetSize(numBindings);

    bindingState->m_numRenderTargets = layout.numRenderTargets;
	
    for (int i = 0; i < numBindings; ++i)
    {
        Binding& dstBinding = dstBindings[i];
        const ShaderInputLayoutEntry& srcBinding = srcBindings[i];

        dstBinding.type = srcBinding.type;
        dstBinding.binding = srcBinding.hlslBinding;
        dstBinding.isOutput = srcBinding.isOutput;
        switch (srcBinding.type)
        {
            case ShaderInputType::Buffer:
            {
                SLANG_RETURN_NULL_ON_FAIL(createInputBuffer(srcBinding.bufferDesc, srcBinding.isOutput, srcBinding.bufferData, dstBinding.resource, dstBinding.uav, dstBinding.srv));
                dstBinding.bufferType = srcBinding.bufferDesc.type;
				break;
			}
            case ShaderInputType::Texture:
            {
                SLANG_RETURN_NULL_ON_FAIL(createInputTexture(srcBinding.textureDesc, dstBinding.resource, dstBinding.srv));
				break;
			}
            case ShaderInputType::Sampler:
            {
                SLANG_RETURN_NULL_ON_FAIL(createInputSampler(srcBinding.samplerDesc, dstBinding.samplerState));
				break;
			}
            case ShaderInputType::CombinedTextureSampler:
            {
				assert(!"Not implemented");
                //throw "not implemented";
				return nullptr;
				break;
			}
        }
    }

    return bindingState.detach();
}

void D3D11Renderer::applyBindingState(bool isCompute)
{
    auto context = m_immediateContext.get();
    for (auto & binding : m_currentBindings->m_bindings)
    {
        if (binding.type == ShaderInputType::Buffer)
        {
            
            if (binding.bufferType == InputBufferType::ConstantBuffer)
            {
                BufferResourceImpl* bufferResource = static_cast<BufferResourceImpl*>(binding.resource.Ptr());
                ID3D11Buffer* buffer = bufferResource->m_buffer;

                if (isCompute)
                    context->CSSetConstantBuffers(binding.binding, 1, &buffer); 
                else
                {
                    context->VSSetConstantBuffers(binding.binding, 1, &buffer);
                    context->PSSetConstantBuffers(binding.binding, 1, &buffer);
                }
            }
            else if (binding.uav)
            {
                if (isCompute)
                    context->CSSetUnorderedAccessViews(binding.binding, 1, binding.uav.readRef(), nullptr);
                else
                    context->OMSetRenderTargetsAndUnorderedAccessViews(m_currentBindings->m_numRenderTargets,
                        m_renderTargetViews.Buffer()->readRef(), nullptr, binding.binding, 1, binding.uav.readRef(), nullptr);
            }
            else
            {
                if (isCompute)
                    context->CSSetShaderResources(binding.binding, 1, binding.srv.readRef());
                else
                {
                    context->PSSetShaderResources(binding.binding, 1, binding.srv.readRef());
                    context->VSSetShaderResources(binding.binding, 1, binding.srv.readRef());
                }
            }
        }
        else if (binding.type == ShaderInputType::Texture)
        {
            if (binding.uav)
            {
                if (isCompute)
                    context->CSSetUnorderedAccessViews(binding.binding, 1, binding.uav.readRef(), nullptr);
                else
                    context->OMSetRenderTargetsAndUnorderedAccessViews(D3D11_KEEP_RENDER_TARGETS_AND_DEPTH_STENCIL,
                        nullptr, nullptr, binding.binding, 1, binding.uav.readRef(), nullptr);
            }
            else
            {
                if (isCompute)
                    context->CSSetShaderResources(binding.binding, 1, binding.srv.readRef());
                else
                {
                    context->PSSetShaderResources(binding.binding, 1, binding.srv.readRef());
                    context->VSSetShaderResources(binding.binding, 1, binding.srv.readRef());
                }
            }
        }
        else if (binding.type == ShaderInputType::Sampler)
        {
            if (isCompute)
                context->CSSetSamplers(binding.binding, 1, binding.samplerState.readRef());
            else
            {
                context->PSSetSamplers(binding.binding, 1, binding.samplerState.readRef());
                context->VSSetSamplers(binding.binding, 1, binding.samplerState.readRef());
            }
        }
        else
            throw "not implemented";
    }
}

void D3D11Renderer::setBindingState(BindingState* state)
{
    m_currentBindings = static_cast<BindingStateImpl*>(state);
}

void D3D11Renderer::serializeOutput(BindingState* stateIn, const char* fileName)
{
    auto bindingState = static_cast<BindingStateImpl*>(stateIn);
    FILE * f = fopen(fileName, "wb");
    int id = 0;
    for (auto & binding : bindingState->m_bindings)
    {
        if (binding.isOutput)
        {
            if (binding.resource && binding.resource->isBuffer())
            {
                BufferResource* bufferResource = static_cast<BufferResource*>(binding.resource.Ptr());
                const size_t bufferSize = bufferResource->getDesc().sizeInBytes;
                
                unsigned int* ptr = (unsigned int*)map(bufferResource, MapFlavor::HostRead);
                for (auto i = 0u; i < bufferSize / sizeof(unsigned int); i++)
                {
                    fprintf(f, "%X\n", ptr[i]);
                }
                unmap(bufferResource);
            }
            else
            {
                printf("invalid output type at %d.\n", id);
            }
        }
        id++;
    }
    fclose(f);
}

} // renderer_test
