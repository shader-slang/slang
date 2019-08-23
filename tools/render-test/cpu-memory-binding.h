#ifndef CPU_MEMORY_BINDING_H
#define CPU_MEMORY_BINDING_H

#include "core/slang-basic.h"

#include "core/slang-memory-arena.h"
#include "core/slang-dictionary.h"

namespace renderer_test {


struct CPUMemoryBinding
{
    struct Buffer
    {
        Buffer() : m_data(nullptr), m_sizeInBytes(0) {}
        uint8_t* m_data;
        size_t m_sizeInBytes;
    };

    
    slang::VariableLayoutReflection* getParameterByName(const char* name);

    SlangResult init(slang::ShaderReflection* reflection);
    CPUMemoryBinding();

    int _addBuffer(slang::VariableLayoutReflection* var, const Buffer& buffer);
    Buffer _allocateBuffer(size_t size);

    SlangResult _add(slang::VariableLayoutReflection* var, slang::TypeLayoutReflection* type, void* dst);

    Slang::MemoryArena m_arena;    ///< Storage for buffers

    slang::ShaderReflection* m_reflection;
    Slang::Dictionary<slang::VariableLayoutReflection*, int> m_bufferMap;
    Slang::List<Buffer> m_buffers;
};

} // renderer_test

#endif //CPU_MEMORY_BINDING_H
