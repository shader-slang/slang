#ifndef SLANG_COMPILER_CORE_STRUCT_TAG_CONVERTER_H
#define SLANG_COMPILER_CORE_STRUCT_TAG_CONVERTER_H

#include "slang-struct-tag-system.h"
#include "slang-diagnostic-sink.h"

namespace Slang {

class StructTagConverter
{
public:
    typedef StructTagField Field;
    typedef Field::Type FieldType;

    typedef uint32_t BitField;

    struct ScopeStack
    {
        ScopeStack(StructTagConverter* converter):
            m_stack(converter->m_convertStack),
            m_startIndex(converter->m_convertStack.getCount())
        {
        }
        ~ScopeStack()
        {
            m_stack.setCount(m_startIndex);
        }

        Index getStartIndex() const { return m_startIndex; }
        operator Index() const { return m_startIndex; }

    protected:
        List<const void*>& m_stack;
        Index m_startIndex;
    };

        /// Convert all the referenced items starting at in.
        /// Items that are converted are stored on the m_convertStack.
        /// The BitField records a bit for every 'field' (where exts is the 0 field) where there is something converted.
        /// If the BitField has no bits set -> then nothing was converted and can be used as is.
        /// To write the converted data, use setContainedConverted.
        ///
        /// NOTE! This method adds items to the end of the m_convertStack, it is the responsibility of the caller to clean up
        /// This can be made simpler by just using ScopeStack.
    SlangResult maybeConvertCurrentContained(const StructTagType* structType, const void* in, BitField* outFieldsSet);

        /// For every fieldSet bit set, copys over the data held in the m_convertStack (indexed from stackStartIndex).
    void setContainedConverted(const StructTagType* structType, Index stackIndex, BitField fieldsSet, void* dst);

    SlangResult maybeConvertCurrent(const void* in, const void*& out);
    SlangResult maybeConvertCurrent(slang::StructTag tag, const void* in, const void*& out);
    SlangResult maybeConvertCurrentArray(const void* in, Index count, const void*& out);
    SlangResult maybeConvertCurrentPtrArray(const void*const* in, Index count, const void*const*& out);

    template <typename T>
    const T* maybeConvertCurrent(const void* in)
    {
        const void* dst;
        return SLANG_SUCCEEDED(maybeConvertCurrent(T::kStructTag, in, dst)) ? (const T*)dst : nullptr;
    }

        /// Convert an array (always copies). 
    SlangResult convertCurrentArray(const void* in, Index count, void*& out);
        /// Convert a pointer array (always copies)
    SlangResult convertCurrentPtrArray(const void*const* in, Index count, void**& out );
        /// Convert a single tagged object (always copies)
    SlangResult convertCurrent(const void* in, void*& out );
        /// Convert the items contained in inout
    SlangResult convertCurrentContained(const StructTagType* structType, void* inout);

        /// Allocates of type and copies src to dst
    void* allocateAndCopy(const StructTagType* type, const void* src);
        /// Copy from src to dst, zero extending or shrinking however structType requires
    void copy(const StructTagType* structType, const void* src, void* dst);

        /// Returns true if it's possible to convert tag to current type
    bool canConvertToCurrent(slang::StructTag tag, StructTagType* type) const;

        /// Ctor. The sink and arena are optional. If the arena isn't set then it is not possible to copy convert anything
        /// and so if a copy convert is required, it will fail.
        /// The sink is optional - if it's set failures will occur silently.
    StructTagConverter(StructTagSystem* system, MemoryArena* arena, DiagnosticSink* sink):
        m_system(system),
        m_arena(arena),
        m_sink(sink)
    {
    }

protected:

    SlangResult _requireArena();
    SlangResult _diagnoseCantConvert(slang::StructTag tag, StructTagType* type);
    SlangResult _diagnoseUnknownType(slang::StructTag tag);
    SlangResult _diagnoseDifferentTypes(slang::StructTag tagA, slang::StructTag tagB);

        /// Used to hold pointers to things that have been converted.
    List<const void*> m_convertStack;

    StructTagSystem* m_system;
    DiagnosticSink* m_sink;
    MemoryArena* m_arena;
};

} // namespace Slang

#endif // SLANG_COMPILER_CORE_STRUCT_TAG_CONVERTER_H
