#ifndef CPU_MEMORY_BINDING_H
#define CPU_MEMORY_BINDING_H

#include "core/slang-basic.h"

#include "core/slang-memory-arena.h"

namespace renderer_test {

struct CPUMemoryBinding
{
    struct Buffer
    {
        Buffer() : m_data(nullptr), m_sizeInBytes(0) {}
        uint8_t* m_data;
        size_t m_sizeInBytes;
    };

    struct Location
    {
        bool isValid() const { return m_cur != nullptr; }
        bool isInvalid() const { return m_cur == nullptr; }

        Location toField(const char* name) const;
        Location toIndex(int index) const;

        slang::TypeLayoutReflection* getTypeLayout() const { return m_typeLayout; }
        uint8_t* getPtr() const { return m_cur; }

        SLANG_FORCE_INLINE Location():m_typeLayout(nullptr), m_cur(nullptr) {}
      
        SLANG_FORCE_INLINE Location(slang::TypeLayoutReflection* typeLayout, uint8_t* ptr):
            m_typeLayout(typeLayout),
            m_cur(ptr)
        {
        }
        
    protected:
        slang::TypeLayoutReflection* m_typeLayout;
        uint8_t* m_cur;
    };
    
    slang::VariableLayoutReflection* getParameterByName(const char* name);
    slang::VariableLayoutReflection* getEntryPointParameterByName(const char* name);

        /// Finds which buffer starts at the ptr index
    Slang::Index findBufferIndex(const void* ptr) const;

    Location find(const char* name);

    SlangResult setBufferContents(const Location& location, const void* initialData, size_t sizeInBytes);
    SlangResult setNewBuffer(const Location& location, const void* initialData, size_t sizeInBytes, Buffer& outBuffer);
    SlangResult setObject(const Location& location, void* object);
    SlangResult setInplace(const Location& location, const void* data, size_t sizeInBytes);
        /// Initialize memory with a 'sensible' value based on type. Pointer types become null.
    SlangResult initValue(slang::TypeLayoutReflection* typeLayout, void* dst);
    SlangResult initValue(const Location& location) { return initValue(location.getTypeLayout(), location.getPtr()); }

        /// Set the size of a *non fixed size* array at location.
        /// A non fixed size array is reflected as having a count of 0 elements.
        /// Only returns a buffer in outBuffer if a new buffer is created. 
    SlangResult setArrayCount(const Location& location, int count, Buffer& outBuffer);
    
    SlangResult init(slang::ShaderReflection* reflection, int entryPointIndex);
    CPUMemoryBinding();

    Buffer _allocateBuffer(size_t size);
    Buffer _allocateBuffer(size_t size, const void* initialData, size_t initialSize);

    SlangResult _add(slang::VariableLayoutReflection* var, slang::TypeLayoutReflection* type, void* dst, Buffer& outBuffer);

    Slang::MemoryArena m_arena;    ///< Storage for buffers

    Buffer m_rootBuffer;
    Buffer m_entryPointBuffer;

    slang::ShaderReflection* m_reflection;
    
    // All buffers
    Slang::List<Buffer> m_allBuffers;

    slang::EntryPointReflection* m_entryPoint;
    int m_entryPointIndex;
};

} // renderer_test

#endif //CPU_MEMORY_BINDING_H
