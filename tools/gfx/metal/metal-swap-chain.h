// metal-swap-chain.h
#pragma once

#include "metal-base.h"
#include "metal-command-queue.h"
#include "metal-device.h"
#include "metal-texture.h"

#include <Metal/Metal.hpp>
#include <MetalKit/MetalKit.hpp>

namespace gfx
{

using namespace Slang;

namespace metal
{

class SwapchainImpl
    : public ISwapchain
    , public ComObject
{
public:
    SLANG_COM_OBJECT_IUNKNOWN_ALL
    ISwapchain* getInterface(const Guid& guid);

public:
    ISwapchain::Desc m_desc;
    RefPtr<CommandQueueImpl> m_queue;
    ShortList<RefPtr<TextureResourceImpl>> m_images;
    ShortList<MTL::Drawable*> m_drawables;
    RefPtr<DeviceImpl> m_renderer;
    uint32_t m_currentImageIndex = 0;
    WindowHandle m_windowHandle;
    MTL::PixelFormat m_metalFormat = MTL::PixelFormat::PixelFormatInvalid;

    void destroySwapchainAndImages();

    void getWindowSize(int& widthOut, int& heightOut) const;

    Result createSwapchainAndImages();

    MetalApi* m_api = nullptr;

public:
    ~SwapchainImpl();

    Result init(DeviceImpl* renderer, const ISwapchain::Desc& desc, WindowHandle window);

    virtual SLANG_NO_THROW const Desc& SLANG_MCALL getDesc() override { return m_desc; }
    virtual SLANG_NO_THROW Result SLANG_MCALL
        getImage(GfxIndex index, ITextureResource** outResource) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL resize(GfxCount width, GfxCount height) override;
    virtual SLANG_NO_THROW Result SLANG_MCALL present() override;
    virtual SLANG_NO_THROW int SLANG_MCALL acquireNextImage() override;
    virtual SLANG_NO_THROW bool SLANG_MCALL isOccluded() override { return false; }
    virtual SLANG_NO_THROW Result SLANG_MCALL setFullScreenMode(bool mode) override;
};

} // namespace metal
} // namespace gfx
