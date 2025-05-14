// slang-relative-ptr.h
#ifndef SLANG_RELATIVE_PTR_H
#define SLANG_RELATIVE_PTR_H

// This file implements a smart pointer type `RelativePtr<T>`
// that, rather than storing the actual *address* of a value
// of type `T`, stores the relative offset (in bytes) between
// the target `T*` and the address of the `RelativePtr<T>`
// itself.
//
// This kind of pointer representation can be useful when
// implementing "memory-mappable" data structures that can
// still conveniently represent complicated object graphs.

#include "slang-basic.h"

namespace Slang
{
    namespace detail
    {
        struct RelativePtr32Traits
        {
            using Offset = Int32;
            using UOffset = UInt32;
        };

        struct RelativePtr64Traits
        {
            using Offset = Int64;
            using UOffset = UInt64;
        };
    }

    template<typename T, typename Traits>
    struct RelativePtr
    {
    public:
        using This = RelativePtr<T,Traits>;
        using Value = T;
        using RawPtr = T*;
        using Offset = typename Traits::Offset;
        using UOffset = typename Traits::UOffset;

        RelativePtr() = default;
        RelativePtr(RelativePtr const& ptr)
        {
            set(ptr);
        }
        RelativePtr(RelativePtr&& ptr)
        {
            set(ptr);
        }
        RelativePtr(T* ptr)
        {
            set(ptr);
        }

        T* get() const
        {
            if (_offset == 0)
            {
                return nullptr;
            }

            intptr_t thisAddr = intptr_t(this);
            intptr_t targetAddr = thisAddr + intptr_t(_offset);

            return (T*)(targetAddr);
        }

        operator T*() const { return get(); }
        T* operator->() const { return get(); }

        void set(T* ptr)
        {
            if (ptr == nullptr)
            {
                _offset = 0;
                return;
            }

            intptr_t thisAddr = intptr_t(this);
            intptr_t targetAddr = intptr_t(ptr);
            intptr_t offsetVal = targetAddr - thisAddr;

            _offset = Offset(offsetVal);
            SLANG_ASSERT(intptr_t(_offset) == offsetVal);
        }

    private:
        Offset _offset = 0;
    };

    template<typename T>
    using RelativePtr32 = RelativePtr<T, detail::RelativePtr32Traits>;

    template<typename T>
    using RelativePtr64 = RelativePtr<T, detail::RelativePtr64Traits>;

} // namespace Slang

#endif
