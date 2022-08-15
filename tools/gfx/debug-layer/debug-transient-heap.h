// debug-transient-heap.h
#pragma once
#include "debug-base.h"

namespace gfx
{
using namespace Slang;

namespace debug
{

class DebugTransientResourceHeap : public DebugObject<ITransientResourceHeap>
{
public:
    SLANG_COM_OBJECT_IUNKNOWN_ALL;

public:
    ITransientResourceHeap* getInterface(const Slang::Guid& guid);
    virtual SLANG_NO_THROW Result SLANG_MCALL synchronizeAndReset() override;
    virtual SLANG_NO_THROW Result SLANG_MCALL finish() override;
    virtual SLANG_NO_THROW Result SLANG_MCALL
        createCommandBuffer(ICommandBuffer** outCommandBuffer) override;
};

} // namespace debug
} // namespace gfx
