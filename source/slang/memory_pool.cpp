#include "memory_pool.h"

namespace Slang
{
    const size_t kPoolSegmentSize = 4 << 20; // use 4MB segments

    struct MemoryPoolSegment
    {
        unsigned char* data = nullptr;
        size_t allocPtr = 0;
        MemoryPoolSegment* nextSegment = nullptr;
    };

    MemoryPool::~MemoryPool()
    {
        while (curSegment)
        {
            auto nxtSegment = curSegment->nextSegment;
            free(curSegment->data);
            delete curSegment;
            curSegment = nxtSegment;
        }
    }

    void newSegment(MemoryPool* pool)
    {
        auto seg = new MemoryPoolSegment();
        seg->nextSegment = pool->curSegment;
        seg->data = (unsigned char*)malloc(kPoolSegmentSize);
        pool->curSegment = seg;
    }

    void * MemoryPool::alloc(size_t size)
    {
        assert(size < kPoolSegmentSize);
        // ensure there is a segment available
        if (!curSegment)
            newSegment(this);
        if (curSegment->allocPtr + size > kPoolSegmentSize)
            newSegment(this);
        // alloc memory from current segment
        void* rs = curSegment->data + curSegment->allocPtr;
        curSegment->allocPtr += size;
        return rs;
    }
    void * MemoryPool::allocZero(size_t size)
    {
        auto rs = alloc(size);
        memset(rs, 0, size);
        return rs;
    }
}
