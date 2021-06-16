#ifndef SLANG_COMPILER_CORE_STRUCT_TAG_CONVERTER_H
#define SLANG_COMPILER_CORE_STRUCT_TAG_CONVERTER_H

#include "slang-struct-tag-system.h"
#include "slang-diagnostic-sink.h"

namespace Slang {

class StructTagConverter
{
public:

    SlangResult getCurrent(const void* in, const void** out);
    SlangResult getCurrentArray(const void* in, Index count, const void** out);
    SlangResult getCurrentPtrArray(const void*const* in, Index count, const void*const** out);

        /// Copies type
    SlangResult copy(const StructTagType* type, void* dst, const void* src);

        /// Make a copy of the in structure (in the arena) such that it conforms to current versions, and return the copy
    void* clone(const void* in);

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

    StructTagSystem* m_system;
    DiagnosticSink* m_sink;
    MemoryArena* m_arena;
};

} // namespace Slang

#endif // SLANG_COMPILER_CORE_STRUCT_TAG_CONVERTER_H
