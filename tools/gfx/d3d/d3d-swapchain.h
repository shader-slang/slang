#pragma once

#include "slang-gfx.h"
#include "core/slang-basic.h"
#include <dxgi1_4.h>
#include "../renderer-shared.h"
#include "d3d-util.h"

namespace gfx
{
class D3DSwapchainBase
    : public ISwapchain
    , public Slang::RefObject
{
public:
    SLANG_REF_OBJECT_IUNKNOWN_ALL
    ISwapchain* getInterface(const Slang::Guid& guid)
    {
        if (guid == GfxGUID::IID_ISlangUnknown || guid == GfxGUID::IID_ISwapchain)
            return static_cast<ISwapchain*>(this);
        return nullptr;
    }

public:
    Result init(const ISwapchain::Desc& desc, WindowHandle window, DXGI_SWAP_EFFECT swapEffect)
    {
        // Return fail on non-supported platforms.
        switch (window.type)
        {
        case WindowHandle::Type::Win32Handle:
            break;
        default:
            return SLANG_FAIL;
        }

        m_desc = desc;

        // Describe the swap chain.
        DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
        swapChainDesc.BufferCount = desc.imageCount;
        swapChainDesc.BufferDesc.Width = desc.width;
        swapChainDesc.BufferDesc.Height = desc.height;
        swapChainDesc.BufferDesc.Format = D3DUtil::getMapFormat(desc.format);
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.SwapEffect = swapEffect;
        swapChainDesc.OutputWindow = (HWND)window.handleValues[0];
        swapChainDesc.SampleDesc.Count = 1;
        swapChainDesc.Windowed = TRUE;

        if (!desc.enableVSync)
        {
            swapChainDesc.Flags |= DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
        }

        // Swap chain needs the queue so that it can force a flush on it.
        ComPtr<IDXGISwapChain> swapChain;
        SLANG_RETURN_ON_FAIL(
            getDXGIFactory()->CreateSwapChain(getOwningDevice(), &swapChainDesc, swapChain.writeRef()));
        SLANG_RETURN_ON_FAIL(swapChain->QueryInterface(m_swapChain.writeRef()));

        if (!desc.enableVSync)
        {
            m_swapChainWaitableObject = m_swapChain->GetFrameLatencyWaitableObject();

            int maxLatency = desc.imageCount - 2;

            // Make sure the maximum latency is in the range required by dx runtime
            maxLatency = (maxLatency < 1) ? 1 : maxLatency;
            maxLatency = (maxLatency > DXGI_MAX_SWAP_CHAIN_BUFFERS) ? DXGI_MAX_SWAP_CHAIN_BUFFERS
                                                                    : maxLatency;

            m_swapChain->SetMaximumFrameLatency(maxLatency);
        }

        SLANG_RETURN_ON_FAIL(getDXGIFactory()->MakeWindowAssociation(
            (HWND)window.handleValues[0], DXGI_MWA_NO_ALT_ENTER));

        createSwapchainBufferImages();
        return SLANG_OK;
    }
    virtual SLANG_NO_THROW const Desc& SLANG_MCALL getDesc() override { return m_desc; }
    virtual SLANG_NO_THROW Result SLANG_MCALL
        getImage(uint32_t index, ITextureResource** outResource) override
    {
        m_images[index]->addRef();
        *outResource = m_images[index].get();
        return SLANG_OK;
    }
    virtual SLANG_NO_THROW Result SLANG_MCALL present() override
    {
        if (m_swapChainWaitableObject)
        {
            // check if now is good time to present
            // This doesn't wait - because the wait time is 0. If it returns WAIT_TIMEOUT it
            // means that no frame is waiting to be be displayed so there is no point doing a
            // present.
            const bool shouldPresent =
                (WaitForSingleObjectEx(m_swapChainWaitableObject, 0, TRUE) != WAIT_TIMEOUT);
            if (shouldPresent)
            {
                m_swapChain->Present(0, 0);
            }
        }
        else
        {
            if (SLANG_FAILED(m_swapChain->Present(1, 0)))
            {
                return SLANG_FAIL;
            }
        }
        return SLANG_OK;
    }

    virtual SLANG_NO_THROW int SLANG_MCALL acquireNextImage() override
    {
        uint32_t count;
        m_swapChain->GetLastPresentCount(&count);
        return (int)(count % m_desc.imageCount);
    }


    virtual SLANG_NO_THROW Result SLANG_MCALL resize(uint32_t width, uint32_t height) override
    {
        if (width == m_desc.width && height == m_desc.height)
            return SLANG_OK;
        
        m_desc.width = width;
        m_desc.height = height;
        for (auto& image : m_images)
            image = nullptr;
        m_images.clear();
        auto result = m_swapChain->ResizeBuffers(
                m_desc.imageCount,
                width,
                height,
                D3DUtil::getMapFormat(m_desc.format),
            m_desc.enableVSync ? 0 : DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT);
        if (result != 0)
            return SLANG_FAIL;
        createSwapchainBufferImages();
        return SLANG_OK;
    }

public:
    virtual void createSwapchainBufferImages() = 0;
    virtual IDXGIFactory* getDXGIFactory() = 0;
    virtual IUnknown* getOwningDevice() = 0;
    ISwapchain::Desc m_desc;
    HANDLE m_swapChainWaitableObject = nullptr;
    ComPtr<IDXGISwapChain2> m_swapChain;
    Slang::ShortList<ComPtr<ITextureResource>> m_images;
};

}
