
#include "cpu-memory-binding.h"

#include "../../slang-com-helper.h"

namespace renderer_test {
using namespace Slang;

CPUMemoryBinding::CPUMemoryBinding()
{
    m_arena.init(1024, 16);
}

CPUMemoryBinding::Buffer CPUMemoryBinding::_allocateBuffer(size_t size)
{
    Buffer buffer;

    // Use 16 byte alignment so works for all common types including typical simd
    void* data = m_arena.allocateAligned(size, 16);
    ::memset(data, 0, size);

    buffer.m_data = (uint8_t*)data;
    buffer.m_sizeInBytes = size;

    return buffer;
}

SlangResult CPUMemoryBinding::init(slang::ShaderReflection* reflection)
{
    m_buffers.clear();
    m_arena.deallocateAll();

    size_t globalConstantBuffer = reflection->getGlobalConstantBufferSize();

    size_t rootSizeInBytes = 0;
    const int parameterCount = reflection->getParameterCount();
    for (int i = 0; i < parameterCount; ++i)
    {
        auto parameter = reflection->getParameterByIndex(i);

        auto offset = parameter->getOffset();

        auto typeLayout = parameter->getTypeLayout();
        auto sizeInBytes = typeLayout->getSize();

        size_t endOffset = offset + sizeInBytes;

        rootSizeInBytes = (endOffset > rootSizeInBytes) ? endOffset : rootSizeInBytes;        
    }

    // Allocate the root (0 is the root)
    m_buffers.add(_allocateBuffer(rootSizeInBytes));

    {
        uint8_t*const buffer = m_buffers[0].m_data;

        for (int i = 0; i < parameterCount; ++i)
        {
            auto parameter = reflection->getParameterByIndex(i);
            auto offset = parameter->getOffset();

            auto typeLayout = parameter->getTypeLayout();

            SLANG_RETURN_ON_FAIL(_add(parameter, typeLayout, buffer + offset));
        }
    }

    return SLANG_OK;
}


int CPUMemoryBinding::_addBuffer(slang::VariableLayoutReflection* varLayout, const Buffer& buffer)
{
    if (varLayout)
    {
        m_bufferMap.Add(varLayout, int(m_buffers.getCount()));
    }

    m_buffers.add(buffer);
    return int(m_buffers.getCount() - 1);
}

SlangResult CPUMemoryBinding::_add(slang::VariableLayoutReflection* varLayout, slang::TypeLayoutReflection* typeLayout, void* dst)
{
    switch (typeLayout->getKind())
    {
        case slang::TypeReflection::Kind::Array:
        {
            auto elementTypeLayout = typeLayout->getElementTypeLayout();
            auto elementCount = int(typeLayout->getElementCount());
            const size_t stride = elementTypeLayout->getSize();
            uint8_t* cur = (uint8_t*)dst;
            for (int i = 0; i < elementCount; ++i)
            {
                _add(nullptr, elementTypeLayout, cur);
                cur += stride;
            }
            break;
        }
        case slang::TypeReflection::Kind::Struct:
        {
            auto structTypeLayout = typeLayout;
            //auto name = structTypeLayout->getName();
            //SLANG_UNUSED(name);

            auto fieldCount = structTypeLayout->getFieldCount();
            for (uint32_t ff = 0; ff < fieldCount; ++ff)
            {
                auto field = structTypeLayout->getFieldByIndex(ff);
                auto offset = field->getOffset();

                SLANG_RETURN_ON_FAIL(_add(nullptr, field->getTypeLayout(), ((uint8_t*)dst) + offset));
            }
            break;
        }
        case slang::TypeReflection::Kind::ConstantBuffer:
        {
            SLANG_ASSERT(typeLayout->getSize() == sizeof(void*));

            auto elementTypeLayout = typeLayout->getElementTypeLayout();
            const size_t elementSize = elementTypeLayout->getSize();

            Buffer buffer = _allocateBuffer(elementSize);
            _addBuffer(varLayout, buffer);

            SLANG_RETURN_ON_FAIL(_add(nullptr, elementTypeLayout, buffer.m_data));

            // Constant buffers map to a pointer
            *(void**)dst = buffer.m_data;
            break;
        }
        default: break;
    }
    return SLANG_OK;
}

slang::VariableLayoutReflection* CPUMemoryBinding::getParameterByName(const char* name)
{
    const int parameterCount = m_reflection->getParameterCount();
    for (int i = 0; i < parameterCount; ++i)
    {
        auto parameter = m_reflection->getParameterByIndex(i);
        const char* paramName = parameter->getName();
        if (strcmp(name, paramName) == 0)
        {
            return parameter;
        }
    }
    return nullptr;
}

} // renderer_test
