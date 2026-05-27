#ifndef SLANG_VM_H
#define SLANG_VM_H

#include "core/slang-string-util.h"
#include "slang-vm-bytecode.h"

#ifndef SLANG_ENABLE_VALIDATION_VM_BYTECODE
#define SLANG_ENABLE_VALIDATION_VM_BYTECODE 1
#endif

using namespace slang;

namespace Slang
{

struct ByteCodeExecutionContext
{
    void* currentWorkingSet;
    uint32_t currentWorkingSetSizeInBytes;
};

class ByteCodeInterpreter;

// Represents a relocated function code ready for execution.
// Relocated functions are VMInsts allocated in a 8-byte aligned buffer, and instruction headers
// Replaced with actual function pointers that can execute the instruction.
class ExecutableFunction
{
public:
    typedef VMInstIterator<VMExecOperand, VMExecInstHeader> InstIterator;
    List<uint64_t> m_codeBuffer;
    VMFuncHeader* m_header;
    List<uint32_t> m_parameterOffsets;
    List<uint32_t> m_instOffsets;
    List<VMOp> m_opcodes;

    InstIterator begin();
    InstIterator end();
};

struct StackFrame
{
    VMExecInstHeader* m_currentInst = nullptr;
    void* m_currentFuncCode = nullptr;
    ExecutableFunction* m_currentFunction = nullptr;
    size_t m_workingSetOffset = 0;
    uint32_t m_currentWorkingSetSizeInBytes = 0;
};

class ByteCodeInterpreter : public RefObject, public IByteCodeRunner
{
public:
    SLANG_REF_OBJECT_IUNKNOWN_ALL
    ISlangUnknown* getInterface(const Guid& guid);

public:
    VMModuleView m_moduleView;
    List<uint8_t> m_code;
    StringBuilder m_errorBuilder;
    List<ExecutableFunction> m_functions;
    Dictionary<String, VMExtFunction> m_extInstHandlers;
    SlangResult prepareModuleForExecution();
#if SLANG_ENABLE_VALIDATION_VM_BYTECODE
    SlangResult validateFunctionForExecution(VMFunctionView func, ExecutableFunction& exeFunc);
#endif
    void* m_extInstHandlerUserData = nullptr;
    List<uint8_t> m_returnRegister;
    List<uint64_t> m_workingSetBuffer;
    List<StackFrame> m_stack;
    List<const char*> m_stringLits;
    const char** m_stringLitsPtr = nullptr;

    size_t m_returnValSize = 0;
    bool m_executionFailed = false;

    static size_t getWorkingSetWordCount(uint32_t byteSize)
    {
        return (size_t(byteSize) + sizeof(uint64_t) - 1) / sizeof(uint64_t);
    }

    void pushFrame(uint32_t size)
    {
        StackFrame frame;
        frame.m_currentFunction = m_currentFunction;
        frame.m_currentWorkingSetSizeInBytes = m_currentWorkingSetSizeInBytes;
        frame.m_workingSetOffset =
            (uint32_t)((uint64_t*)m_currentWorkingSet - m_workingSetBuffer.getBuffer());
        m_stack.add(frame);
        auto stackBufferCount = m_workingSetBuffer.getCount();
        m_workingSetBuffer.setCount(m_workingSetBuffer.getCount() + getWorkingSetWordCount(size));
        m_currentWorkingSet = m_workingSetBuffer.getBuffer() + stackBufferCount;
        m_currentWorkingSetSizeInBytes = size;
    }
    void popFrame()
    {
        auto& stackFrame = m_stack.getLast();
        auto lastWorkingSetBufferCount =
            (uint32_t)((uint64_t*)m_currentWorkingSet - m_workingSetBuffer.getBuffer());
        m_workingSetBuffer.setCount(lastWorkingSetBufferCount);
        m_currentInst = stackFrame.m_currentInst->getNextInst();
        m_currentFuncCode = stackFrame.m_currentFuncCode;
        m_currentFunction = stackFrame.m_currentFunction;
        m_currentWorkingSetSizeInBytes = stackFrame.m_currentWorkingSetSizeInBytes;
        m_currentWorkingSet = m_workingSetBuffer.getBuffer() + stackFrame.m_workingSetOffset;
        m_stack.removeLast();
    }

    VMExecInstHeader* m_currentInst = nullptr;
    void* m_currentFuncCode = nullptr;
    ExecutableFunction* m_currentFunction = nullptr;
    void* m_currentWorkingSet = nullptr;
    uint32_t m_currentWorkingSetSizeInBytes = 0;

    VMPrintFunc m_printCallback = nullptr;
    void* m_printCallbackUserData = nullptr;

    template<typename... Args>
    void reportError(const char* format, Args... args)
    {
        m_errorBuilder.append(StringUtil::makeStringWithFormat(format, args...));
        m_errorBuilder.append("\n");
    }

    template<typename... Args>
    bool failExecution(const char* format, Args... args)
    {
        reportError(format, args...);
        m_executionFailed = true;
        m_currentInst = nullptr;
        return false;
    }

#if SLANG_ENABLE_VALIDATION_VM_BYTECODE
    bool validateCurrentInstruction(VMExecInstHeader* inst);
    bool validateOperandAccess(
        const VMExecOperand& operand,
        size_t size,
        bool isWrite,
        uint64_t additionalOffset = 0);
    bool validatePointerAccess(const void* ptr, size_t size, bool isWrite);
    bool validatePointerOffset(
        const void* basePtr,
        int64_t elementOffset,
        uint32_t stride,
        size_t accessSize,
        void** outPtr);
#else
    bool validateCurrentInstruction(VMExecInstHeader*) { return true; }
    bool validateOperandAccess(const VMExecOperand&, size_t, bool, uint64_t = 0) { return true; }
    bool validatePointerAccess(const void*, size_t, bool) { return true; }
    bool validatePointerOffset(
        const void* basePtr,
        int64_t elementOffset,
        uint32_t stride,
        size_t accessSize,
        void** outPtr)
    {
        SLANG_UNUSED(accessSize);
        uint64_t elementCount =
            elementOffset < 0 ? uint64_t(-(elementOffset + 1)) + 1 : uint64_t(elementOffset);
        auto byteOffset = elementCount * uint64_t(stride);
        *outPtr =
            elementOffset < 0 ? (uint8_t*)basePtr - byteOffset : (uint8_t*)basePtr + byteOffset;
        return true;
    }
#endif

    static void defaultPrintCallback(const char* message, void* userData);
    ByteCodeInterpreter();

public:
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL loadModule(IBlob* moduleBlob) override;
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    selectFunctionByIndex(uint32_t functionIndex) override;
    virtual SLANG_NO_THROW int SLANG_MCALL findFunctionByName(const char* name) override;

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    getFunctionInfo(uint32_t index, ByteCodeFuncInfo* outInfo) override;
    virtual SLANG_NO_THROW void* SLANG_MCALL getCurrentWorkingSet() override
    {
        return m_currentWorkingSet;
    }
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    execute(void* argumentData, size_t argumentSize) override;
    virtual SLANG_NO_THROW void SLANG_MCALL getErrorString(slang::IBlob** outBlob) override;
    virtual SLANG_NO_THROW void* SLANG_MCALL getReturnValue(size_t* outValueSize) override
    {
        *outValueSize = m_returnValSize;
        return m_returnRegister.getBuffer();
    }
    virtual SLANG_NO_THROW void SLANG_MCALL setExtInstHandlerUserData(void* userData) override
    {
        m_extInstHandlerUserData = userData;
    }
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    registerExtCall(const char* name, VMExtFunction functionPtr) override
    {
        m_extInstHandlers[name] = functionPtr;
        return SLANG_OK;
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    setPrintCallback(VMPrintFunc callback, void* userData) override;
};

} // namespace Slang

#endif
