#pragma once

#include "slang-hgvm/slang-hgvm.h"

#include <emscripten/val.h>

namespace Slang::HGVM
{
/// Implements Window and Surface services using DOM canvases and the existing WebGPU RHI.
class BrowserHostServices : public IHGVMHostServices
{
public:
    BrowserHostServices(rhi::IDevice* device, emscripten::val host);
    ~BrowserHostServices();

    /// Wraps a canvas registered by the JavaScript package as a typed HGVM Window.
    SlangResult adoptWindow(uint32_t id, RefPtr<HGVMHostObject>& outWindow, String& outError);

    SlangResult createWindow(
        std::string_view title,
        uint32_t width,
        uint32_t height,
        RefPtr<HGVMHostObject>& outWindow,
        String& outError) override;
    SlangResult processWindowEvents(HGVMHostObject*, bool& outOpen, String& outError) override;
    SlangResult pollInputEvent(HGVMHostObject*, HGVMInputEvent&, String& outError) override;
    SlangResult closeWindow(HGVMHostObject*, String& outError) override;
    SlangResult setWindowTitle(HGVMHostObject*, std::string_view, String& outError) override;
    SlangResult createSurface(
        HGVMHostObject*,
        rhi::Format,
        uint32_t imageCount,
        RefPtr<HGVMHostObject>& outSurface,
        String& outError) override;
    SlangResult resizeSurface(HGVMHostObject*, uint32_t, uint32_t, String& outError) override;
    SlangResult acquireSurfaceTexture(
        HGVMHostObject*,
        ComPtr<rhi::ITexture>& outTexture,
        String& outError) override;
    SlangResult presentSurface(HGVMHostObject*, String& outError) override;
    SlangResult blitTexture2D(rhi::ITexture*, rhi::ITexture*, String& outError) override;

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};
} // namespace Slang::HGVM
