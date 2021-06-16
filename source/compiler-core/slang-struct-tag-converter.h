#ifndef SLANG_COMPILER_CORE_STRUCT_TAG_CONVERTER_H
#define SLANG_COMPILER_CORE_STRUCT_TAG_CONVERTER_H

#include "slang-struct-tag-system.h"
#include "slang-diagnostic-sink.h"

namespace Slang {

class StructTagConverter
{
public:

    const void* maybeConvertCurrent(const void* in);
    const void* maybeConvertCurrent(slang::StructTag tag, const void* in);

    const void* maybeConvertCurrentArray(const void* in, Index count);
    const void*const* maybeConvertCurrentPtrArray(const void*const* in, Index count);

        /// Convert to current form
    SlangResult convertCurrent(const StructTagType* type, const void* src, void* dst);

        /// Convert all the referenced items starting at in.
        /// Returns -1 if there was an error, else returns number converted.
    Index convertCurrentContained(const StructTagType* structType, const void* in);

    void setContained(Index stackIndex, const StructTagType* structType, void* dst);

        /// Make a copy of the in structure (in the arena) such that it conforms to current versions, and return the copy
    void* clone(const void* in);

        /// Allocates of type from src
    void* allocateAndCopy(const StructTagType* type, const void* src);
    void copy(const StructTagType* type, const void* src, void* dst);

    template <typename T>
    const T* maybeConvertCurrent(const void* in) { return (const T*)maybeConvertCurrent(T::kStructTag, in); }

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

    List<const void*> m_stack;

    StructTagSystem* m_system;
    DiagnosticSink* m_sink;
    MemoryArena* m_arena;
};

} // namespace Slang

#endif // SLANG_COMPILER_CORE_STRUCT_TAG_CONVERTER_H
