
#include "cpu-memory-binding.h"

#include "../../slang-com-helper.h"

#define SLANG_PRELUDE_NAMESPACE CPPPrelude
#include "../../prelude/slang-cpp-types.h"

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

    m_allBuffers.add(buffer);
    return buffer;
}

CPUMemoryBinding::Buffer CPUMemoryBinding::_allocateBuffer(size_t size, const void* initialData, size_t initialSize)
{
    SLANG_ASSERT(initialSize <= size);
    Buffer buffer = _allocateBuffer(size);

    if (initialData)
    {
        memcpy(buffer.m_data, initialData, initialSize);
    }

    return buffer;
}

SlangResult CPUMemoryBinding::init(slang::ShaderReflection* reflection, int entryPointIndex)
{
    m_reflection = reflection;
    m_rootBuffer = Buffer();

    m_allBuffers.clear();
    m_arena.deallocateAll();
    
    {
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
        SLANG_ASSERT(rootSizeInBytes == globalConstantBuffer);

        if (rootSizeInBytes)
        {
            // Allocate the 'root' buffer
            m_rootBuffer = _allocateBuffer(rootSizeInBytes);

            // Create default empty constant buffers
            uint8_t*const buffer = m_rootBuffer.m_data;
            for (int i = 0; i < parameterCount; ++i)
            {
                auto parameter = reflection->getParameterByIndex(i);
                auto offset = parameter->getOffset();

                auto typeLayout = parameter->getTypeLayout();
                Buffer paramBuffer;
                SLANG_RETURN_ON_FAIL(_add(parameter, typeLayout, buffer + offset, paramBuffer));
            }
        }
    }

    {
        auto entryPointCount = reflection->getEntryPointCount();
        if (entryPointIndex < 0 || entryPointIndex >= entryPointCount)
        {
            SLANG_ASSERT(!"Entry point index out of range");
            return SLANG_FAIL;
        }
        
        m_entryPoint = reflection->getEntryPointByIndex(entryPointIndex);
        size_t entryPointParamsSizeInBytes = 0;

        const int parameterCount = int(m_entryPoint->getParameterCount());
        for (int i = 0 ; i < parameterCount; i++)
        {
            slang::VariableLayoutReflection* parameter = m_entryPoint->getParameterByIndex(i);

            // If has a semantic, then isn't uniform parameter
            if (auto semanticName = parameter->getSemanticName())
            {
                continue;
            }

            auto offset = parameter->getOffset();

            auto typeLayout = parameter->getTypeLayout();
            auto sizeInBytes = typeLayout->getSize();

            size_t endOffset = offset + sizeInBytes;
            entryPointParamsSizeInBytes = (endOffset > entryPointParamsSizeInBytes) ? endOffset : entryPointParamsSizeInBytes; 
        }

        if (entryPointParamsSizeInBytes)
        {
            m_entryPointBuffer = _allocateBuffer(entryPointParamsSizeInBytes);

            uint8_t*const buffer = m_entryPointBuffer.m_data;
            for (int i = 0; i < parameterCount; ++i)
            {
                auto parameter = m_entryPoint->getParameterByIndex(i);
                // If has a semantic, then isn't uniform parameter
                if (auto semanticName = parameter->getSemanticName())
                {
                    continue;
                }

                auto offset = parameter->getOffset();

                auto typeLayout = parameter->getTypeLayout();
                Buffer paramBuffer;
                SLANG_RETURN_ON_FAIL(_add(parameter, typeLayout, buffer + offset, paramBuffer));
            }
        }
    }

    return SLANG_OK;
}

SlangResult CPUMemoryBinding::_add(slang::VariableLayoutReflection* varLayout, slang::TypeLayoutReflection* typeLayout, void* dst, Buffer& outBuffer)
{
    const auto kind = typeLayout->getKind();
    switch (kind)
    {
        case slang::TypeReflection::Kind::Array:
        {
            auto elementTypeLayout = typeLayout->getElementTypeLayout();
            auto elementCount = int(typeLayout->getElementCount());
            const size_t stride = elementTypeLayout->getSize();
            uint8_t* cur = (uint8_t*)dst;
            for (int i = 0; i < elementCount; ++i)
            {
                Buffer elementBuffer;
                _add(nullptr, elementTypeLayout, cur, elementBuffer);
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

                Buffer fieldBuffer;
                SLANG_RETURN_ON_FAIL(_add(nullptr, field->getTypeLayout(), ((uint8_t*)dst) + offset, fieldBuffer));
            }
            break;
        }
        case slang::TypeReflection::Kind::ConstantBuffer:
        {
            SLANG_ASSERT(typeLayout->getSize() == sizeof(void*));

            auto elementTypeLayout = typeLayout->getElementTypeLayout();
            const size_t elementSize = elementTypeLayout->getSize();

            outBuffer = _allocateBuffer(elementSize);
            
            // Constant buffers map to a pointer
            *(void**)dst = outBuffer.m_data;

            // On CPU constant buffers can contain pointers to other resources (including constant buffers)
            Buffer innerBuffer;
            SLANG_RETURN_ON_FAIL(_add(nullptr, elementTypeLayout, outBuffer.m_data, innerBuffer));
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

slang::VariableLayoutReflection* CPUMemoryBinding::getEntryPointParameterByName(const char* name)
{
    const int parameterCount = int(m_entryPoint->getParameterCount());
    for (int i = 0; i < parameterCount; ++i)
    {
        auto parameter = m_entryPoint->getParameterByIndex(i);
        // If has a semantic we will ignore
        if (parameter->getSemanticName())
        {
            continue;
        }
        if (strcmp(parameter->getName(), name) == 0)
        {
            return parameter;
        }
    }
    return nullptr;   
}

CPUMemoryBinding::Location CPUMemoryBinding::find(const char* name)
{
    auto varLayout = getParameterByName(name);
    if (varLayout)
    {
        return Location(varLayout->getTypeLayout(), m_rootBuffer.m_data + varLayout->getOffset());
    }

    varLayout = getEntryPointParameterByName(name);
    if (varLayout)
    {
        return Location(varLayout->getTypeLayout(), m_entryPointBuffer.m_data + varLayout->getOffset());
    }
    return Location();
}

CPUMemoryBinding::Location CPUMemoryBinding::Location::toField(const char* name) const
{
    if (!isValid())
    {
        return *this;
    }

    auto typeLayout = m_typeLayout;
    uint8_t* cur = m_cur;

    // Strip constantBuffer wrapping
    {
        const auto kind = typeLayout->getKind();
        if (kind == slang::TypeReflection::Kind::ConstantBuffer)
        {
            // Follow the pointer
            cur = *(uint8_t**)cur;
            typeLayout = typeLayout->getElementTypeLayout();
        }
    }

    {
        const auto kind = typeLayout->getKind();
        if (kind == slang::TypeReflection::Kind::Struct)
        {
            slang::VariableLayoutReflection* varLayout = nullptr;
            auto fieldCount = typeLayout->getFieldCount();
            for (uint32_t ff = 0; ff < fieldCount; ++ff)
            {
                auto field = typeLayout->getFieldByIndex(ff);
                if (strcmp(field->getName(), name) == 0)
                {
                    return Location(field->getTypeLayout(), cur + field->getOffset());
                }
            }
        }
    }

    return Location();
}

CPUMemoryBinding::Location CPUMemoryBinding::Location::toIndex(int index) const
{
    if (!isValid())
    {
        return *this;
    }

    auto typeLayout = m_typeLayout;
    uint8_t* cur = m_cur;

    const auto kind = typeLayout->getKind();
    switch (kind)
    {
        case slang::TypeReflection::Kind::Array:
        {
            auto elementTypeLayout = typeLayout->getElementTypeLayout();
            const auto elementCount = int(typeLayout->getElementCount());
            const auto elementStride = typeLayout->getElementStride(SLANG_PARAMETER_CATEGORY_UNIFORM);

            if (index < 0 || index >= elementCount)
            {
                SLANG_ASSERT(index < elementCount);
                return Location();
            }

            return Location(elementTypeLayout, cur + elementStride * index);
        }
        default: break;
    }

    return Location();
}


SlangResult CPUMemoryBinding::setBufferContents(const Location& location, const void* initialData, size_t sizeInBytes)
{
    if (!location.isValid())
    {
        return SLANG_FAIL;
    }

    auto typeLayout = location.getTypeLayout();
    uint8_t* cur = location.getPtr();

    const auto kind = typeLayout->getKind();
    switch (kind)
    {
        case slang::TypeReflection::Kind::ConstantBuffer:
        {
            typeLayout = typeLayout->getElementTypeLayout();

            size_t bufferSize = typeLayout->getSize();
            sizeInBytes = (sizeInBytes > bufferSize) ? bufferSize : sizeInBytes;

            void* buffer = *(void**)cur;
            memcpy(buffer, initialData, sizeInBytes);
            return SLANG_OK;
        }
        default: break;
    }
    return SLANG_FAIL;
}

SlangResult CPUMemoryBinding::setNewBuffer(const Location& location, const void* initialData, size_t sizeInBytes, Buffer& outBuffer)
{
    if (!location.isValid())
    {
        return SLANG_FAIL;
    }

    auto typeLayout = location.getTypeLayout();
    uint8_t* cur = location.getPtr();

    const auto kind = typeLayout->getKind();
    switch (kind)
    {
        case slang::TypeReflection::Kind::ConstantBuffer:
        {
            // All should be allocated (!)
            SLANG_ASSERT(typeLayout->getSize() == sizeof(void*));
            SLANG_ASSERT(*(void**)cur);
            return SLANG_FAIL;
        }
        case slang::TypeReflection::Kind::Resource:
        {
            auto type = typeLayout->getType();
            auto shape = type->getResourceShape();

            switch (shape & SLANG_RESOURCE_BASE_SHAPE_MASK)
            {
                case SLANG_STRUCTURED_BUFFER:
                {
                    auto elementTypeLayout = typeLayout->getElementTypeLayout();
                    size_t elementSize = elementTypeLayout->getSize(SLANG_PARAMETER_CATEGORY_UNIFORM);

                    // We don't know the size of the buffer, but we can work it out, based on what is initialized
                    const int numElements = int((sizeInBytes + elementSize - 1) / elementSize);

                    const size_t bufferSize = numElements * elementSize;
                    SLANG_ASSERT(bufferSize >= sizeInBytes);

                    outBuffer = _allocateBuffer(bufferSize, initialData, sizeInBytes);

                    CPPPrelude::StructuredBuffer<uint8_t>& dstBuf = *(CPPPrelude::StructuredBuffer<uint8_t>*)cur;
                    dstBuf.data = (uint8_t*)outBuffer.m_data;
                    dstBuf.count = numElements;
                    return SLANG_OK;
                }
                case SLANG_BYTE_ADDRESS_BUFFER:
                {
                    const size_t bufferSize = (sizeInBytes + 3) & ~size_t(3);

                    outBuffer = _allocateBuffer(bufferSize, initialData, sizeInBytes);

                    CPPPrelude::ByteAddressBuffer& dstBuf = *(CPPPrelude::ByteAddressBuffer*)cur;
                    dstBuf.data = (uint32_t*)outBuffer.m_data;
                    dstBuf.sizeInBytes = bufferSize;
                    return SLANG_OK;
                }
            }
            break;
        }
        default: break;
    }

    return SLANG_FAIL;
}

SlangResult CPUMemoryBinding::setObject(const Location& location, void* object)
{
    if (!location.isValid())
    {
        return SLANG_FAIL;
    }

    auto typeLayout = location.getTypeLayout();
    uint8_t* cur = location.getPtr();

    const auto kind = typeLayout->getKind();
    switch (kind)
    {
        default:
            break;

        case slang::TypeReflection::Kind::ParameterBlock:
        {
            auto elementTypeLayout = typeLayout->getElementTypeLayout();
            SLANG_UNUSED(elementTypeLayout);
            break;
        }
        case slang::TypeReflection::Kind::TextureBuffer:
        {
            auto elementTypeLayout = typeLayout->getElementTypeLayout();
            SLANG_UNUSED(elementTypeLayout);
            break;
        }
        case slang::TypeReflection::Kind::ShaderStorageBuffer:
        {
            auto elementTypeLayout = typeLayout->getElementTypeLayout();
            SLANG_UNUSED(elementTypeLayout);
            break;
        }
        case slang::TypeReflection::Kind::GenericTypeParameter:
        {
            const char* name = typeLayout->getName();
            SLANG_UNUSED(name);
            break;
        }
        case slang::TypeReflection::Kind::Interface:
        {
            const char* name = typeLayout->getName();
            SLANG_UNUSED(name);
            break;
        }
        case slang::TypeReflection::Kind::Resource:
        {
            auto type = typeLayout->getType();
            auto shape = type->getResourceShape();

            //auto access = type->getResourceAccess();

            switch (shape & SLANG_RESOURCE_BASE_SHAPE_MASK)
            {
                default:
                    assert(!"unhandled case");
                    break;
                case SLANG_TEXTURE_1D:
                case SLANG_TEXTURE_2D:
                case SLANG_TEXTURE_3D:
                case SLANG_TEXTURE_CUBE:
                case SLANG_TEXTURE_BUFFER:
                {
                    *(void**)cur = object;
                    return SLANG_OK;
                }
            }
            if (shape & SLANG_TEXTURE_ARRAY_FLAG)
            {

            }
            if (shape & SLANG_TEXTURE_MULTISAMPLE_FLAG)
            {

            }

            break;
        }
    }

    return SLANG_FAIL;
}

SlangResult CPUMemoryBinding::setInplace(const Location& location, const void* data, size_t sizeInBytes)
{
    if (!location.isValid())
    {
        return SLANG_FAIL;
    }

    size_t dstSize = location.getTypeLayout()->getSize();
    sizeInBytes = (sizeInBytes > dstSize) ? dstSize : sizeInBytes;
    memcpy(location.getPtr(), data, sizeInBytes);
    return SLANG_OK;
}

} // renderer_test
