#ifndef SLANG_MEMORY_POOL_H
#define SLANG_MEMORY_POOL_H

#include "../core/basic.h"

namespace Slang
{
    struct MemoryPoolSegment;

    struct MemoryPool : public RefObject
    {
        MemoryPoolSegment* curSegment = nullptr;
        ~MemoryPool();
        void* alloc(size_t size);
        void* allocZero(size_t size);
    };
}

#endif