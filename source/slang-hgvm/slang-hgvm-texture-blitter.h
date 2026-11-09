#pragma once

#include "core/slang-basic.h"

#include <memory>
#include <slang-rhi.h>

namespace Slang::HGVM
{
/// Converts a graph render target into a host presentation surface's format.
class TextureBlitter
{
public:
    explicit TextureBlitter(rhi::IDevice* device);
    ~TextureBlitter();

    /// Compiles the pipeline before a host acquires a short-lived browser canvas texture.
    SlangResult prepare(rhi::Format format, String& outError);
    SlangResult blit(rhi::ITexture* destination, rhi::ITexture* source, String& outError);

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};
} // namespace Slang::HGVM
