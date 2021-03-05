#pragma once

#include "slang-gfx.h"
#include "slang-com-ptr.h"
#include "core/slang-basic.h"

namespace gfx
{

enum class CommandName
{
    SetPipelineState,
    SetDescriptorSet,
    BindRootShaderObject,
    SetFramebuffer,
    ClearFrame,
    SetViewports,
    SetScissorRects,
    SetPrimitiveTopology,
    SetVertexBuffers,
    SetIndexBuffer,
    Draw,
    DrawIndexed,
    SetStencilReference,
    DispatchCompute,
    UploadBufferData,
    CopyBuffer
};

const uint8_t kMaxCommandOperands = 5;

struct Command
{
    CommandName name;
    uint32_t operands[kMaxCommandOperands];
    Command() = default;
    Command(CommandName inName, uint32_t op)
        : name(inName)
    {
        operands[0] = op;
    }
    Command(CommandName inName, uint32_t op1, uint32_t op2)
        : name(inName)
    {
        operands[0] = op1;
        operands[1] = op2;
    }
    Command(CommandName inName, uint32_t op1, uint32_t op2, uint32_t op3)
        : name(inName)
    {
        operands[0] = op1;
        operands[1] = op2;
        operands[2] = op3;
    }
    Command(CommandName inName, uint32_t op1, uint32_t op2, uint32_t op3, uint32_t op4)
        : name(inName)
    {
        operands[0] = op1;
        operands[1] = op2;
        operands[2] = op3;
        operands[3] = op4;
    }
    Command(
        CommandName inName,
        uint32_t op1,
        uint32_t op2,
        uint32_t op3,
        uint32_t op4,
        uint32_t op5)
        : name(inName)
    {
        operands[0] = op1;
        operands[1] = op2;
        operands[2] = op3;
        operands[3] = op4;
        operands[4] = op5;
    }
};

class CommandWriter
{
public:
    Slang::List<Command> m_commands;
    Slang::List<Slang::ComPtr<ISlangUnknown>> m_objects;
    Slang::List<uint8_t> m_data;

public:
    void clear()
    {
        m_commands.clear();
        for (auto& obj : m_objects)
            obj = nullptr;
        m_objects.clear();
        m_data.clear();
    }

    // Copies user data into `m_data` buffer and returns the offset to retrieve the data.
    uint32_t encodeData(const void* data, size_t size)
    {
        uint32_t offset = (uint32_t)m_data.getCount();
        m_data.setCount(m_data.getCount() + (Slang::Index)size);
        memcpy(m_data.getBuffer() + offset, data, size);
        return offset;
    }

    uint32_t encodeObject(ISlangUnknown* obj)
    {
        uint32_t offset = (uint32_t)m_objects.getCount();
        ComPtr<ISlangUnknown> ptr;
        ptr = obj;
        m_objects.add(ptr);
        return offset;
    }

    template <typename T> T* getObject(uint32_t offset)
    {
        return static_cast<T*>(m_objects[offset].get());
    }

    template <typename T> T* getData(uint32_t offset)
    {
        return reinterpret_cast<T*>(m_data.getBuffer() + offset);
    }

    void setPipelineState(IPipelineState* state)
    {
        auto offset = encodeObject(state);
        m_commands.add(Command(CommandName::SetPipelineState, offset));
    }

    void setDescriptorSet(
        PipelineType pipelineType,
        IPipelineLayout* layout,
        UInt index,
        IDescriptorSet* descriptorSet)
    {
        uint32_t pipelineLayoutOffset = encodeObject(layout);
        uint32_t descSetOffset = encodeObject(descriptorSet);
        m_commands.add(Command(
            CommandName::SetDescriptorSet,
            (uint32_t)pipelineType,
            pipelineLayoutOffset,
            (uint32_t)index,
            descSetOffset));
    }

    void bindRootShaderObject(PipelineType pipelineType, IShaderObject* object)
    {
        auto rootOffset = encodeObject(object);
        m_commands.add(Command(
            CommandName::BindRootShaderObject,
            (uint32_t)pipelineType, rootOffset));
    }

    void uploadBufferData(IBufferResource* buffer, size_t offset, size_t size, void* data)
    {
        auto bufferOffset = encodeObject(buffer);
        auto dataOffset = encodeData(data, size);
        m_commands.add(Command(
            CommandName::UploadBufferData,
            bufferOffset,
            (uint32_t)offset,
            (uint32_t)size,
            dataOffset));
    }

    void copyBuffer(
        IBufferResource* dst,
        size_t dstOffset,
        IBufferResource* src,
        size_t srcOffset,
        size_t size)
    {
        auto dstBuffer = encodeObject(dst);
        auto srcBuffer = encodeObject(src);
        m_commands.add(Command(
            CommandName::CopyBuffer,
            dstBuffer,
            (uint32_t)dstOffset,
            srcBuffer,
            (uint32_t)srcOffset,
            (uint32_t)size));
    }

    void setFramebuffer(IFramebuffer* frameBuffer)
    {
        uint32_t framebufferOffset = encodeObject(frameBuffer);
        m_commands.add(Command(CommandName::SetFramebuffer, framebufferOffset));
    }

    void clearFrame(uint32_t colorBufferMask, bool clearDepth, bool clearStencil)
    {
        m_commands.add(Command(
            CommandName::ClearFrame, colorBufferMask, clearDepth ? 1 : 0, clearStencil ? 1 : 0));
    }

    void setViewports(UInt count, const Viewport* viewports)
    {
        auto offset = encodeData(viewports, sizeof(Viewport) * count);
        m_commands.add(Command(CommandName::SetViewports, (uint32_t)count, offset));
    }

    void setScissorRects(UInt count, const ScissorRect* scissors)
    {
        auto offset = encodeData(scissors, sizeof(ScissorRect) * count);
        m_commands.add(Command(CommandName::SetScissorRects, (uint32_t)count, offset));
    }

    void setPrimitiveTopology(PrimitiveTopology topology)
    {
        m_commands.add(Command(CommandName::SetPrimitiveTopology, (uint32_t)topology));
    }

    void setVertexBuffers(
        UInt startSlot,
        UInt slotCount,
        IBufferResource* const* buffers,
        const UInt* strides,
        const UInt* offsets)
    {
        uint32_t bufferOffset = 0;
        for (UInt i = 0; i < slotCount; i++)
        {
            auto offset = encodeObject(buffers[i]);
            if (i == 0)
                bufferOffset = offset;
        }
        uint32_t stridesOffset = encodeData(strides, sizeof(UInt) * slotCount);
        uint32_t offsetsOffset = encodeData(offsets, sizeof(UInt) * slotCount);
        m_commands.add(Command(
            CommandName::SetVertexBuffers,
            (uint32_t)startSlot,
            (uint32_t)slotCount,
            bufferOffset,
            stridesOffset,
            offsetsOffset));
    }

    void setIndexBuffer(IBufferResource* buffer, Format indexFormat, UInt offset)
    {
        auto bufferOffset = encodeObject(buffer);
        m_commands.add(Command(
            CommandName::SetIndexBuffer, bufferOffset, (uint32_t)indexFormat, (uint32_t)offset));
    }

    void draw(UInt vertexCount, UInt startVertex)
    {
        m_commands.add(Command(CommandName::Draw, (uint32_t)vertexCount, (uint32_t)startVertex));
    }

    void drawIndexed(UInt indexCount, UInt startIndex, UInt baseVertex)
    {
        m_commands.add(Command(
            CommandName::DrawIndexed,
            (uint32_t)indexCount,
            (uint32_t)startIndex,
            (uint32_t)baseVertex));
    }

    void setStencilReference(uint32_t referenceValue)
    {
        m_commands.add(Command(CommandName::SetStencilReference, referenceValue));
    }

    void dispatchCompute(int x, int y, int z)
    {
        m_commands.add(
            Command(CommandName::DispatchCompute, (uint32_t)x, (uint32_t)y, (uint32_t)z));
    }
};
}
