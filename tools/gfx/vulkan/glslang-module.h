// glslang-module.h
#pragma once

#include "core/slang-list.h"
#include "slang-com-helper.h"
#include "slang-com-ptr.h"
#include "slang-glslang/slang-glslang.h"
#include "slang.h"
#include "spirv-tools/include/spirv-tools/linker.hpp"

namespace gfx
{

struct GlslangModule
{
    /// true if has been initialized
    SLANG_FORCE_INLINE bool isInitialized() const { return m_module != nullptr; }

    /// Initialize
    Slang::Result init();

    /// Destroy
    void destroy();

    /// Dtor
    ~GlslangModule() { destroy(); }

    Slang::ComPtr<ISlangBlob> linkSPIRV(Slang::List<Slang::ComPtr<ISlangBlob>> spirvModules);

protected:
    void* m_module = nullptr;

    glslang_LinkSPIRVFunc m_linkSPIRVFunc = nullptr;
};

} // namespace gfx
