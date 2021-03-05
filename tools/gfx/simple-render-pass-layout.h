// simple-render-pass-layout.h
#pragma once

// Implementation of a dummy render pass layout object that stores and holds its
// desc value. Used by targets that does not expose an API object for the render pass
// concept.

#include "slang-gfx.h"
#include "slang-com-helper.h"
#include "core/slang-basic.h"

namespace gfx
{

class SimpleRenderPassLayout
    : public IRenderPassLayout
    , public Slang::RefObject
{
public:
    SLANG_REF_OBJECT_IUNKNOWN_ALL
    IRenderPassLayout* getInterface(const Slang::Guid& guid);

public:
    Slang::ShortList<AttachmentAccessDesc> m_renderTargetAccesses;
    AttachmentAccessDesc m_depthStencilAccess;
    bool m_hasDepthStencil;
    void init(const IRenderPassLayout::Desc& desc);
};

}
