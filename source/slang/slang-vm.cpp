#include "slang-vm.h"

#include "core/slang-blob.h"
#include "slang-vm-inst-impl.h"

namespace Slang
{

// Our VM insts need to be 8-byte aligned, so we can replace the opcode with function pointers and
// sectionId with data pointers.
static_assert(sizeof(VMOperand) % 8 == 0);
static_assert(sizeof(VMInstHeader) % 8 == 0);
static_assert(sizeof(VMOperand) == sizeof(VMExecOperand));
static_assert(sizeof(VMInstHeader) == sizeof(VMExecInstHeader));

namespace
{
enum class OperandAccess
{
    Read,
    Write,
};

static bool isRangeInBounds(uint64_t offset, uint64_t size, uint64_t limit)
{
    return offset <= limit && size <= limit - offset;
}

static const char* getSectionName(int sectionId)
{
    switch (sectionId)
    {
    case kSlangByteCodeSectionWorkingSet:
        return "working set";
    case kSlangByteCodeSectionConstants:
        return "constants";
    case kSlangByteCodeSectionInsts:
        return "instructions";
    case kSlangByteCodeSectionImmediate:
        return "immediate";
    case kSlangByteCodeSectionFuncs:
        return "functions";
    case kSlangByteCodeSectionStrings:
        return "strings";
    default:
        return "unknown";
    }
}

static size_t getScalarSize(ArithmeticExtCode extCode)
{
    return size_t(1) << extCode.scalarBitWidth;
}

static size_t getElementCount(ArithmeticExtCode extCode)
{
    return extCode.vectorSize ? extCode.vectorSize : 1;
}

static ArithmeticExtCode decodeArithmeticExtCode(uint32_t extCodeValue)
{
    ArithmeticExtCode extCode = {};
    memcpy(&extCode, &extCodeValue, sizeof(extCode));
    return extCode;
}

static bool getArithmeticOperandSizes(
    VMOp op,
    uint32_t extCodeValue,
    size_t& outDstSize,
    size_t& outSrcSize)
{
    auto extCode = decodeArithmeticExtCode(extCodeValue);
    auto elementCount = getElementCount(extCode);
    auto srcSize = getScalarSize(extCode) * elementCount;

    switch (op)
    {
    case VMOp::Less:
    case VMOp::Leq:
    case VMOp::Greater:
    case VMOp::Geq:
    case VMOp::Equal:
    case VMOp::Neq:
        outDstSize = sizeof(uint32_t) * elementCount;
        outSrcSize = srcSize;
        return true;
    default:
        outDstSize = srcSize;
        outSrcSize = srcSize;
        return true;
    }
}

static void getCastOperandSizes(uint32_t extCodeValue, size_t& outDstSize, size_t& outSrcSize)
{
    auto toExtCode = decodeArithmeticExtCode(extCodeValue);
    auto fromExtCode = decodeArithmeticExtCode(extCodeValue >> 16);
    auto elementCount = getElementCount(fromExtCode);
    outDstSize = getScalarSize(toExtCode) * elementCount;
    outSrcSize = getScalarSize(fromExtCode) * elementCount;
}

static bool isBinaryArithmeticOp(VMOp op)
{
    switch (op)
    {
    case VMOp::Add:
    case VMOp::Sub:
    case VMOp::Mul:
    case VMOp::Div:
    case VMOp::Rem:
    case VMOp::And:
    case VMOp::Or:
    case VMOp::BitAnd:
    case VMOp::BitOr:
    case VMOp::BitXor:
    case VMOp::Shl:
    case VMOp::Shr:
    case VMOp::Less:
    case VMOp::Leq:
    case VMOp::Greater:
    case VMOp::Geq:
    case VMOp::Equal:
    case VMOp::Neq:
        return true;
    default:
        return false;
    }
}

static bool isUnaryArithmeticOp(VMOp op)
{
    switch (op)
    {
    case VMOp::Neg:
    case VMOp::Not:
    case VMOp::BitNot:
        return true;
    default:
        return false;
    }
}

static bool isValidInstOffset(const ExecutableFunction& func, uint32_t offset)
{
    for (auto instOffset : func.m_instOffsets)
    {
        if (instOffset == offset)
            return true;
    }
    return false;
}

static Index findInstIndex(const ExecutableFunction& func, uint32_t offset)
{
    for (Index i = 0; i < func.m_instOffsets.getCount(); i++)
    {
        if (func.m_instOffsets[i] == offset)
            return i;
    }
    return -1;
}

static bool validateControlFlowTarget(
    ByteCodeInterpreter* interpreter,
    const ExecutableFunction& exeFunc,
    VMInstHeader instHeader,
    uint8_t* operandBase,
    uint32_t operandIdx)
{
    if (operandIdx >= instHeader.operandCount)
    {
        interpreter->reportError("VM branch instruction is missing a target operand.");
        return false;
    }

    VMOperand operand;
    memcpy(&operand, operandBase + operandIdx * sizeof(VMOperand), sizeof(VMOperand));
    if (operand.sectionId != kSlangByteCodeSectionInsts ||
        !isValidInstOffset(exeFunc, operand.offset))
    {
        interpreter->reportError("VM branch target does not point to an instruction boundary.");
        return false;
    }
    return true;
}
} // namespace

ISlangUnknown* ByteCodeInterpreter::getInterface(const Guid& guid)
{
    if (guid == ISlangUnknown::getTypeGuid() || guid == IByteCodeRunner::getTypeGuid())
        return static_cast<IByteCodeRunner*>(this);

    return nullptr;
}

SlangResult ByteCodeInterpreter::validateFunctionForExecution(
    VMFunctionView func,
    ExecutableFunction& exeFunc)
{
    auto header = func.header;
    if (!header)
    {
        reportError("Invalid function header.");
        return SLANG_FAIL;
    }
    if (header->workingSetSizeInBytes % sizeof(uint64_t) != 0)
    {
        reportError("Function working set size must be 8-byte aligned.");
        return SLANG_FAIL;
    }
    if (header->parameterSizeInBytes > header->workingSetSizeInBytes)
    {
        reportError("Function parameter size exceeds working set size.");
        return SLANG_FAIL;
    }
    uint32_t lastParameterOffset = 0;
    for (uint32_t i = 0; i < header->parameterCount; i++)
    {
        auto parameterOffset = header->getParameterOffset(i);
        if (parameterOffset < lastParameterOffset || parameterOffset > header->parameterSizeInBytes)
        {
            reportError("Invalid parameter offset in function.");
            return SLANG_FAIL;
        }
        lastParameterOffset = parameterOffset;
    }

    exeFunc.m_instOffsets.clear();
    exeFunc.m_opcodes.clear();

    uint8_t* cursor = func.functionCode;
    uint8_t* codeEnd = func.functionCodeEnd;
    while (cursor < codeEnd)
    {
        auto remainingSize = (uint64_t)(codeEnd - cursor);
        if (remainingSize < sizeof(VMInstHeader))
        {
            reportError("Truncated VM instruction header.");
            return SLANG_FAIL;
        }

        // Copy the header into an aligned local so we don't rely on the raw
        // bytecode blob meeting alignof(VMInstHeader).
        VMInstHeader instHeader;
        memcpy(&instHeader, cursor, sizeof(VMInstHeader));
        auto operandBytes = uint64_t(instHeader.operandCount) * sizeof(VMOperand);
        auto instSize = uint64_t(sizeof(VMInstHeader)) + operandBytes;
        if (operandBytes / sizeof(VMOperand) != instHeader.operandCount || instSize > remainingSize)
        {
            reportError("VM instruction operand list exceeds function code size.");
            return SLANG_FAIL;
        }
        if (instHeader.opcode == VMOp::CallExt && instHeader.operandCount < 2)
        {
            reportError("VM CallExt instruction requires at least 2 operands.");
            return SLANG_FAIL;
        }

        auto instOffset = (uint32_t)(cursor - func.functionCode);
        exeFunc.m_instOffsets.add(instOffset);
        exeFunc.m_opcodes.add(instHeader.opcode);

        uint8_t* operandBase = cursor + sizeof(VMInstHeader);
        for (uint32_t operandIdx = 0; operandIdx < instHeader.operandCount; operandIdx++)
        {
            VMOperand operand;
            memcpy(&operand, operandBase + operandIdx * sizeof(VMOperand), sizeof(VMOperand));
            switch (operand.sectionId)
            {
            case kSlangByteCodeSectionWorkingSet:
                if (!isRangeInBounds(operand.offset, operand.size, header->workingSetSizeInBytes))
                {
                    reportError("VM operand points outside the working set.");
                    return SLANG_FAIL;
                }
                break;
            case kSlangByteCodeSectionConstants:
                if (!isRangeInBounds(operand.offset, operand.size, m_moduleView.constantBlobSize))
                {
                    reportError("VM operand points outside the constants section.");
                    return SLANG_FAIL;
                }
                break;
            case kSlangByteCodeSectionInsts:
                if (!isRangeInBounds(operand.offset, operand.size, header->codeSize) ||
                    (operand.size == 0 && operand.offset >= header->codeSize))
                {
                    reportError("VM operand points outside the instruction section.");
                    return SLANG_FAIL;
                }
                break;
            case kSlangByteCodeSectionImmediate:
                break;
            case kSlangByteCodeSectionFuncs:
                if (operand.offset >= m_moduleView.functionCount)
                {
                    reportError("VM operand references an invalid function.");
                    return SLANG_FAIL;
                }
                break;
            case kSlangByteCodeSectionStrings:
                if (operand.offset >= m_moduleView.stringCount)
                {
                    reportError("VM operand references an invalid string.");
                    return SLANG_FAIL;
                }
                if (operand.size != 0 && operand.size < sizeof(const char*))
                {
                    reportError("VM string operand is smaller than a pointer.");
                    return SLANG_FAIL;
                }
                break;
            default:
                reportError("VM operand uses an invalid section id.");
                return SLANG_FAIL;
            }
        }

        cursor += instSize;
    }

    if (cursor != codeEnd)
    {
        reportError("VM function code size does not match instruction stream.");
        return SLANG_FAIL;
    }

    for (auto instOffset : exeFunc.m_instOffsets)
    {
        uint8_t* instPtr = func.functionCode + instOffset;
        VMInstHeader instHeader;
        memcpy(&instHeader, instPtr, sizeof(VMInstHeader));
        uint8_t* operandBase = instPtr + sizeof(VMInstHeader);
        switch (instHeader.opcode)
        {
        case VMOp::Jump:
            if (!validateControlFlowTarget(this, exeFunc, instHeader, operandBase, 0))
                return SLANG_FAIL;
            break;
        case VMOp::JumpIf:
            if (!validateControlFlowTarget(this, exeFunc, instHeader, operandBase, 1) ||
                !validateControlFlowTarget(this, exeFunc, instHeader, operandBase, 2))
                return SLANG_FAIL;
            break;
        default:
            break;
        }
    }

    return SLANG_OK;
}

SlangResult ByteCodeInterpreter::prepareModuleForExecution()
{
    m_stringLits.clear();
    m_stringLits.setCount(m_moduleView.stringCount);
    for (uint32_t i = 0; i < m_moduleView.stringCount; i++)
    {
        auto strOffset = m_moduleView.stringOffsets[i];
        const char* str = (const char*)m_moduleView.constants + strOffset;
        m_stringLits[i] = str;
    }
    m_stringLitsPtr = m_stringLits.getBuffer();

    m_functions.setCount(m_moduleView.functionCount);
    for (uint32_t i = 0; i < m_moduleView.functionCount; i++)
    {
        auto func = m_moduleView.getFunction(i);
        auto& exeFunc = m_functions[i];
        SLANG_RETURN_ON_FAIL(validateFunctionForExecution(func, exeFunc));
        exeFunc.m_codeBuffer.setCount(func.header->codeSize / sizeof(uint64_t));
        exeFunc.m_header = func.header;
        exeFunc.m_parameterOffsets.clear();
        for (uint32_t j = 0; j < func.header->parameterCount; j++)
        {
            exeFunc.m_parameterOffsets.add(func.header->getParameterOffset(j));
        }
        exeFunc.m_parameterOffsets.add(func.header->parameterSizeInBytes);

        // Copy the code into the executable function buffer
        if (func.header->codeSize)
            memcpy(exeFunc.m_codeBuffer.getBuffer(), func.functionCode, func.header->codeSize);

        // Replace the instruction headers with function pointers
        for (auto instOffset : exeFunc.m_instOffsets)
        {
            auto inst = reinterpret_cast<VMExecInstHeader*>(
                (uint8_t*)exeFunc.m_codeBuffer.getBuffer() + instOffset);
            VMInstHeader* instHeader = reinterpret_cast<VMInstHeader*>(inst);
            auto handler = mapInstToFunction(instHeader, &m_moduleView, m_extInstHandlers);
            if (!handler)
            {
                StringBuilder instStr;
                printVMInst(instStr, &m_moduleView, instHeader);
                reportError(
                    "Cannot find execution handler for instruction %s\n",
                    instStr.toString().getBuffer());
                return SLANG_FAIL;
            }
            inst->functionPtr = handler;
            for (uint32_t operandIdx = 0; operandIdx < instHeader->operandCount; operandIdx++)
            {
                auto& operand = instHeader->getOperand(operandIdx);
                auto& execOpernad = inst->getOperand(operandIdx);
                switch (operand.sectionId)
                {
                case kSlangByteCodeSectionConstants:
                    execOpernad.section = &m_moduleView.constants;
                    break;
                case kSlangByteCodeSectionInsts:
                    execOpernad.section = (uint8_t**)&m_currentFuncCode;
                    break;
                case kSlangByteCodeSectionWorkingSet:
                    execOpernad.section = (uint8_t**)&m_currentWorkingSet;
                    break;
                case kSlangByteCodeSectionStrings:
                    execOpernad.section = (uint8_t**)&m_stringLitsPtr;
                    execOpernad.offset *= sizeof(const char*);
                    break;
                }
            }
        }
    }

    return SLANG_OK;
}

static int getExecOperandSectionId(ByteCodeInterpreter* ctx, const VMExecOperand& operand)
{
    if (operand.section == &ctx->m_moduleView.constants)
        return kSlangByteCodeSectionConstants;
    if (operand.section == (uint8_t**)&ctx->m_currentFuncCode)
        return kSlangByteCodeSectionInsts;
    if (operand.section == (uint8_t**)&ctx->m_currentWorkingSet)
        return kSlangByteCodeSectionWorkingSet;
    if (operand.section == (uint8_t**)&ctx->m_stringLitsPtr)
        return kSlangByteCodeSectionStrings;

    auto rawSectionId = (uintptr_t)operand.section;
    if (rawSectionId <= kSlangByteCodeSectionStrings)
        return (int)rawSectionId;

    return -1;
}

bool ByteCodeInterpreter::validateOperandAccess(
    const VMExecOperand& operand,
    size_t size,
    bool isWrite,
    size_t additionalOffset)
{
    uint64_t offset = operand.offset;
    if (additionalOffset > UINT64_MAX - offset)
    {
        return failExecution("VM operand offset overflow.");
    }
    offset += additionalOffset;

    auto sectionId = getExecOperandSectionId(this, operand);
    uint64_t sectionSize = 0;
    switch (sectionId)
    {
    case kSlangByteCodeSectionWorkingSet:
        sectionSize = m_currentWorkingSetSizeInBytes;
        break;
    case kSlangByteCodeSectionConstants:
        if (isWrite)
            return failExecution("VM attempted to write to the constants section.");
        sectionSize = m_moduleView.constantBlobSize;
        break;
    case kSlangByteCodeSectionInsts:
        if (isWrite)
            return failExecution("VM attempted to write to the instruction section.");
        if (!m_currentFunction)
            return failExecution("No VM function is active.");
        sectionSize = m_currentFunction->m_header->codeSize;
        if (size == 0)
        {
            if (offset >= sectionSize || !isValidInstOffset(*m_currentFunction, (uint32_t)offset))
                return failExecution("VM instruction operand has an invalid code offset.");
            return true;
        }
        break;
    case kSlangByteCodeSectionStrings:
        if (isWrite)
            return failExecution("VM attempted to write to the strings section.");
        sectionSize = uint64_t(m_moduleView.stringCount) * sizeof(const char*);
        break;
    default:
        return failExecution(
            "VM instruction used %s operand as memory.",
            getSectionName(sectionId));
    }

    if (!isRangeInBounds(offset, size, sectionSize))
    {
        return failExecution(
            "VM operand access out of bounds in %s section: offset=%llu size=%llu "
            "sectionSize=%llu.",
            getSectionName(sectionId),
            (unsigned long long)offset,
            (unsigned long long)size,
            (unsigned long long)sectionSize);
    }
    return true;
}

static bool getPointerRange(const void* ptr, size_t size, uintptr_t& start, uintptr_t& end)
{
    start = reinterpret_cast<uintptr_t>(ptr);
    if (size > UINTPTR_MAX - start)
        return false;
    end = start + size;
    return true;
}

static bool isPointerInRange(uintptr_t ptr, uintptr_t rangeStart, uintptr_t rangeEnd)
{
    return ptr >= rangeStart && ptr < rangeEnd;
}

static bool isPointerRangeInRange(
    uintptr_t start,
    uintptr_t end,
    uintptr_t rangeStart,
    uintptr_t rangeEnd)
{
    return start >= rangeStart && end <= rangeEnd;
}

bool ByteCodeInterpreter::validatePointerAccess(const void* ptr, size_t size, bool isWrite)
{
    if (!ptr && size != 0)
        return failExecution("VM attempted to access a null pointer.");

    uintptr_t accessStart = 0;
    uintptr_t accessEnd = 0;
    if (!getPointerRange(ptr, size, accessStart, accessEnd))
        return failExecution("VM pointer access overflow.");

    uintptr_t workingSetStart = reinterpret_cast<uintptr_t>(m_currentWorkingSet);
    uintptr_t workingSetEnd = workingSetStart + m_currentWorkingSetSizeInBytes;
    uintptr_t constantsStart = reinterpret_cast<uintptr_t>(m_moduleView.constants);
    uintptr_t constantsEnd = constantsStart + m_moduleView.constantBlobSize;

    if (isPointerInRange(accessStart, workingSetStart, workingSetEnd))
    {
        if (!isPointerRangeInRange(accessStart, accessEnd, workingSetStart, workingSetEnd))
            return failExecution("VM pointer access exceeded the current working set.");
        return true;
    }

    if (isPointerInRange(accessStart, constantsStart, constantsEnd))
    {
        if (isWrite)
            return failExecution("VM attempted to write through a constants pointer.");
        if (!isPointerRangeInRange(accessStart, accessEnd, constantsStart, constantsEnd))
            return failExecution("VM pointer access exceeded the constants section.");
        return true;
    }

    return true;
}

bool ByteCodeInterpreter::validatePointerOffset(
    const void* basePtr,
    int64_t elementOffset,
    uint32_t stride,
    size_t accessSize,
    void** outPtr)
{
    if (stride == 0)
    {
        *outPtr = const_cast<void*>(basePtr);
        return validatePointerAccess(*outPtr, accessSize, false);
    }
    if (elementOffset == INT64_MIN)
        return failExecution("VM pointer offset overflow.");

    int64_t byteOffset = 0;
    if (elementOffset > 0 && uint64_t(elementOffset) > UINT64_MAX / stride)
        return failExecution("VM pointer offset overflow.");
    if (elementOffset < 0 && uint64_t(-elementOffset) > UINT64_MAX / stride)
        return failExecution("VM pointer offset overflow.");

    if (elementOffset >= 0)
    {
        auto unsignedOffset = uint64_t(elementOffset) * uint64_t(stride);
        if (unsignedOffset > uint64_t(INT64_MAX))
            return failExecution("VM pointer offset overflow.");
        byteOffset = int64_t(unsignedOffset);
    }
    else
    {
        auto unsignedOffset = uint64_t(-elementOffset) * uint64_t(stride);
        if (unsignedOffset > uint64_t(INT64_MAX))
            return failExecution("VM pointer offset overflow.");
        byteOffset = -int64_t(unsignedOffset);
    }

    auto base = reinterpret_cast<uintptr_t>(basePtr);
    uintptr_t result = 0;
    if (byteOffset >= 0)
    {
        if (uint64_t(byteOffset) > UINTPTR_MAX - base)
            return failExecution("VM pointer offset overflow.");
        result = base + uintptr_t(byteOffset);
    }
    else
    {
        auto negativeOffset = uintptr_t(-byteOffset);
        if (negativeOffset > base)
            return failExecution("VM pointer offset overflow.");
        result = base - negativeOffset;
    }

    *outPtr = reinterpret_cast<void*>(result);

    uintptr_t workingSetStart = reinterpret_cast<uintptr_t>(m_currentWorkingSet);
    uintptr_t workingSetEnd = workingSetStart + m_currentWorkingSetSizeInBytes;
    uintptr_t constantsStart = reinterpret_cast<uintptr_t>(m_moduleView.constants);
    uintptr_t constantsEnd = constantsStart + m_moduleView.constantBlobSize;
    uintptr_t accessEnd = 0;
    if (accessSize > UINTPTR_MAX - result)
        return failExecution("VM pointer offset access overflow.");
    accessEnd = result + accessSize;

    if (isPointerInRange(base, workingSetStart, workingSetEnd) &&
        !isPointerRangeInRange(result, accessEnd, workingSetStart, workingSetEnd))
    {
        return failExecution("VM working-set pointer arithmetic went out of bounds.");
    }

    if (isPointerInRange(base, constantsStart, constantsEnd) &&
        !isPointerRangeInRange(result, accessEnd, constantsStart, constantsEnd))
    {
        return failExecution("VM constants pointer arithmetic went out of bounds.");
    }

    return true;
}

static bool validateOperandCount(
    ByteCodeInterpreter* ctx,
    VMExecInstHeader* inst,
    uint32_t expectedCount)
{
    if (inst->operandCount != expectedCount)
    {
        return ctx->failExecution(
            "VM instruction has invalid operand count: expected %u, found %u.",
            expectedCount,
            inst->operandCount);
    }
    return true;
}

static bool validateMinOperandCount(
    ByteCodeInterpreter* ctx,
    VMExecInstHeader* inst,
    uint32_t minCount)
{
    if (inst->operandCount < minCount)
    {
        return ctx->failExecution(
            "VM instruction has too few operands: expected at least %u, found %u.",
            minCount,
            inst->operandCount);
    }
    return true;
}

static bool validateSpecialOperandSection(
    ByteCodeInterpreter* ctx,
    const VMExecOperand& operand,
    int expectedSectionId)
{
    auto sectionId = getExecOperandSectionId(ctx, operand);
    if (sectionId != expectedSectionId)
    {
        return ctx->failExecution(
            "VM instruction expected a %s operand, found %s.",
            getSectionName(expectedSectionId),
            getSectionName(sectionId));
    }
    return true;
}

bool ByteCodeInterpreter::validateCurrentInstruction(VMExecInstHeader* inst)
{
    if (!m_currentFunction || !m_currentFuncCode)
        return failExecution("No VM function is active.");

    auto codeStart = reinterpret_cast<uint8_t*>(m_currentFuncCode);
    auto instPtr = reinterpret_cast<uint8_t*>(inst);
    if (instPtr < codeStart)
        return failExecution("VM instruction pointer is outside the current function.");
    auto instOffset = uint32_t(instPtr - codeStart);
    auto instIndex = findInstIndex(*m_currentFunction, instOffset);
    if (instIndex < 0)
        return failExecution("VM instruction pointer does not point to an instruction boundary.");

    auto op = m_currentFunction->m_opcodes[instIndex];
    auto check = [&](uint32_t operandIdx, size_t size, OperandAccess access) -> bool
    {
        if (operandIdx >= inst->operandCount)
            return failExecution("VM instruction is missing an operand.");
        return validateOperandAccess(
            inst->getOperand(operandIdx),
            size,
            access == OperandAccess::Write);
    };

    if (isBinaryArithmeticOp(op))
    {
        size_t dstSize = 0;
        size_t srcSize = 0;
        getArithmeticOperandSizes(op, inst->opcodeExtension, dstSize, srcSize);
        return validateOperandCount(this, inst, 3) && check(0, dstSize, OperandAccess::Write) &&
               check(1, srcSize, OperandAccess::Read) && check(2, srcSize, OperandAccess::Read);
    }

    if (isUnaryArithmeticOp(op))
    {
        size_t dstSize = 0;
        size_t srcSize = 0;
        getArithmeticOperandSizes(op, inst->opcodeExtension, dstSize, srcSize);
        return validateOperandCount(this, inst, 2) && check(0, dstSize, OperandAccess::Write) &&
               check(1, srcSize, OperandAccess::Read);
    }

    switch (op)
    {
    case VMOp::Nop:
        return validateOperandCount(this, inst, 0);
    case VMOp::Ret:
        if (inst->opcodeExtension > m_currentFunction->m_header->returnValueSizeInBytes)
            return failExecution("VM return size exceeds the function return value size.");
        if (inst->opcodeExtension == 0)
            return validateOperandCount(this, inst, 0);
        return validateOperandCount(this, inst, 1) &&
               check(0, inst->opcodeExtension, OperandAccess::Read);
    case VMOp::Jump:
        return validateOperandCount(this, inst, 1) &&
               validateSpecialOperandSection(
                   this,
                   inst->getOperand(0),
                   kSlangByteCodeSectionInsts) &&
               check(0, 0, OperandAccess::Read);
    case VMOp::JumpIf:
        return validateOperandCount(this, inst, 3) &&
               check(0, sizeof(uint32_t), OperandAccess::Read) &&
               validateSpecialOperandSection(
                   this,
                   inst->getOperand(1),
                   kSlangByteCodeSectionInsts) &&
               check(1, 0, OperandAccess::Read) &&
               validateSpecialOperandSection(
                   this,
                   inst->getOperand(2),
                   kSlangByteCodeSectionInsts) &&
               check(2, 0, OperandAccess::Read);
    case VMOp::Load:
        return validateOperandCount(this, inst, 2) &&
               check(0, inst->opcodeExtension, OperandAccess::Write) &&
               check(1, sizeof(void*), OperandAccess::Read);
    case VMOp::Store:
        return validateOperandCount(this, inst, 2) &&
               check(0, sizeof(void*), OperandAccess::Read) &&
               check(1, inst->opcodeExtension, OperandAccess::Read);
    case VMOp::Copy:
        return validateOperandCount(this, inst, 2) &&
               check(0, inst->opcodeExtension, OperandAccess::Write) &&
               check(1, inst->opcodeExtension, OperandAccess::Read);
    case VMOp::GetWorkingSetPtr:
        if (!isRangeInBounds(inst->opcodeExtension, 0, m_currentWorkingSetSizeInBytes))
            return failExecution("VM working-set pointer offset is out of bounds.");
        return validateOperandCount(this, inst, 1) && check(0, sizeof(void*), OperandAccess::Write);
    case VMOp::GetElementPtr:
        return validateOperandCount(this, inst, 3) &&
               check(0, sizeof(void*), OperandAccess::Write) &&
               check(1, sizeof(void*), OperandAccess::Read) &&
               check(2, sizeof(uint32_t), OperandAccess::Read);
    case VMOp::OffsetPtr:
        return validateOperandCount(this, inst, 3) &&
               check(0, sizeof(void*), OperandAccess::Write) &&
               check(1, sizeof(void*), OperandAccess::Read) &&
               check(2, sizeof(int32_t), OperandAccess::Read);
    case VMOp::GetElement:
        return validateOperandCount(this, inst, 3) &&
               check(0, inst->opcodeExtension, OperandAccess::Write) &&
               check(1, 0, OperandAccess::Read) && check(2, sizeof(uint32_t), OperandAccess::Read);
    case VMOp::Swizzle:
        {
            if (!validateMinOperandCount(this, inst, 2))
                return false;
            auto extCode = decodeArithmeticExtCode(inst->opcodeExtension);
            auto elementSize = getScalarSize(extCode);
            if (!check(0, (inst->operandCount - 2) * elementSize, OperandAccess::Write))
                return false;
            for (uint32_t i = 2; i < inst->operandCount; i++)
            {
                if (!validateSpecialOperandSection(
                        this,
                        inst->getOperand(i),
                        kSlangByteCodeSectionImmediate))
                    return false;
                auto index = inst->getOperand(i).offset;
                if (index > SIZE_MAX / elementSize)
                    return failExecution("VM swizzle index overflow.");
                if (!validateOperandAccess(
                        inst->getOperand(1),
                        elementSize,
                        false,
                        size_t(index) * elementSize))
                    return false;
            }
            return true;
        }
    case VMOp::Cast:
        {
            size_t dstSize = 0;
            size_t srcSize = 0;
            getCastOperandSizes(inst->opcodeExtension, dstSize, srcSize);
            return validateOperandCount(this, inst, 2) && check(0, dstSize, OperandAccess::Write) &&
                   check(1, srcSize, OperandAccess::Read);
        }
    case VMOp::CallExt:
        if (!validateMinOperandCount(this, inst, 2))
            return false;
        if (!validateSpecialOperandSection(
                this,
                inst->getOperand(1),
                kSlangByteCodeSectionStrings) ||
            !check(0, inst->getOperand(0).size, OperandAccess::Write) ||
            !check(1, sizeof(const char*), OperandAccess::Read))
            return false;
        for (uint32_t i = 2; i < inst->operandCount; i++)
        {
            auto sectionId = getExecOperandSectionId(this, inst->getOperand(i));
            auto size = sectionId == kSlangByteCodeSectionStrings ? sizeof(const char*)
                                                                  : inst->getOperand(i).size;
            if (!check(i, size, OperandAccess::Read))
                return false;
        }
        return true;
    case VMOp::Call:
        {
            if (!validateMinOperandCount(this, inst, 2) || !validateSpecialOperandSection(
                                                               this,
                                                               inst->getOperand(1),
                                                               kSlangByteCodeSectionFuncs))
                return false;
            auto funcId = inst->getOperand(1).offset;
            if (funcId >= (uint32_t)m_functions.getCount())
                return failExecution("VM call references an invalid function.");
            auto& func = m_functions[funcId];
            if (inst->operandCount != func.m_header->parameterCount + 2)
                return failExecution("VM call argument count does not match callee parameters.");
            if (!check(0, inst->opcodeExtension, OperandAccess::Write))
                return false;
            for (uint32_t i = 0; i < func.m_header->parameterCount; i++)
            {
                auto parameterSize = func.m_parameterOffsets[i + 1] - func.m_parameterOffsets[i];
                if (!check(i + 2, parameterSize, OperandAccess::Read))
                    return false;
            }
            return true;
        }
    case VMOp::Print:
        if (!validateMinOperandCount(this, inst, 1) ||
            !validateSpecialOperandSection(
                this,
                inst->getOperand(0),
                kSlangByteCodeSectionStrings) ||
            !check(0, sizeof(const char*), OperandAccess::Read))
            return false;
        for (uint32_t i = 1; i < inst->operandCount; i++)
        {
            auto sectionId = getExecOperandSectionId(this, inst->getOperand(i));
            auto size = sectionId == kSlangByteCodeSectionStrings ? sizeof(const char*)
                                                                  : inst->getOperand(i).size;
            if (!check(i, size, OperandAccess::Read))
                return false;
        }
        return true;
    case VMOp::Dispatch:
        return failExecution("VM dispatch instruction is not executable.");
    }

    return failExecution("VM instruction has an invalid opcode.");
}

SLANG_NO_THROW SlangResult SLANG_MCALL ByteCodeInterpreter::loadModule(IBlob* moduleBlob)
{
    m_stack.reserve(128);
    m_workingSetBuffer.reserve(1024 * 1024); // Reserve 1MB for working set
    m_currentWorkingSet = m_workingSetBuffer.getBuffer();

    m_errorBuilder.clear();
    m_code.clear();
    m_functions.clear();
    m_stack.clear();
    m_returnRegister.clear();
    m_currentInst = nullptr;
    m_currentFuncCode = nullptr;
    m_currentFunction = nullptr;
    m_currentWorkingSetSizeInBytes = 0;
    m_executionFailed = false;
    m_code.addRange((uint8_t*)(moduleBlob->getBufferPointer()), moduleBlob->getBufferSize());
    SLANG_RETURN_ON_FAIL(initVMModule(
        m_code.getBuffer(),
        (uint32_t)moduleBlob->getBufferSize(),
        &m_moduleView,
        &m_errorBuilder));
    SLANG_RETURN_ON_FAIL(prepareModuleForExecution());
    return SLANG_OK;
}

SLANG_NO_THROW void SLANG_MCALL ByteCodeInterpreter::getErrorString(slang::IBlob** outBlob)
{
    *outBlob = StringBlob::moveCreate(m_errorBuilder.produceString()).detach();
    m_errorBuilder.clear();
}

SLANG_NO_THROW int SLANG_MCALL ByteCodeInterpreter::findFunctionByName(const char* name)
{
    for (uint32_t i = 0; i < m_moduleView.functionCount; i++)
    {
        auto func = m_moduleView.getFunction(i);
        if (UnownedStringSlice(func.name) == name)
        {
            return (int)i;
        }
    }
    return -1; // Function not found
}

SLANG_NO_THROW SlangResult SLANG_MCALL
ByteCodeInterpreter::getFunctionInfo(uint32_t index, slang::ByteCodeFuncInfo* outInfo)
{
    if (index >= m_moduleView.functionCount)
    {
        return SLANG_FAIL;
    }
    auto func = m_moduleView.getFunction(index);
    outInfo->parameterCount = func.header->parameterCount;
    outInfo->returnValueSize = func.header->returnValueSizeInBytes;
    return SLANG_OK;
}

SLANG_NO_THROW SlangResult SLANG_MCALL
ByteCodeInterpreter::selectFunctionByIndex(uint32_t functionIndex)
{
    if (functionIndex >= m_moduleView.functionCount)
    {
        reportError(
            "Function index %u out of range [0, %u)",
            functionIndex,
            m_moduleView.functionCount);
        return SLANG_FAIL;
    }
    auto func = m_moduleView.getFunction(functionIndex);
    m_currentFuncCode = m_functions[functionIndex].m_codeBuffer.getBuffer();
    m_currentInst = reinterpret_cast<VMExecInstHeader*>(m_currentFuncCode);
    m_currentFunction = &m_functions[functionIndex];
    m_workingSetBuffer.setCount(func.header->workingSetSizeInBytes / sizeof(uint64_t));
    m_currentWorkingSet = m_workingSetBuffer.getBuffer();
    m_currentWorkingSetSizeInBytes = func.header->workingSetSizeInBytes;
    return SLANG_OK;
}

SLANG_NO_THROW SlangResult SLANG_MCALL
ByteCodeInterpreter::execute(void* argumentData, size_t argumentSize)
{
    if (!m_currentInst)
    {
        reportError("No function selected for execution");
        return SLANG_FAIL;
    }
    if (!m_currentWorkingSet)
    {
        reportError("No working set allocated for execution");
        return SLANG_FAIL;
    }
    if (!m_currentFunction)
    {
        reportError("No function selected for execution");
        return SLANG_FAIL;
    }
    auto parameterSize = m_currentFunction->m_header->parameterSizeInBytes;
    if (argumentSize > parameterSize)
    {
        reportError("Argument size exceeds function parameter area.");
        return SLANG_FAIL;
    }
    // Copy the arguments into the working set
    if (argumentData && argumentSize > 0)
    {
        memcpy(m_currentWorkingSet, argumentData, argumentSize);
    }
    m_returnValSize = 0;
    m_executionFailed = false;
    while (m_currentInst)
    {
        if (!validateCurrentInstruction(m_currentInst))
            return SLANG_FAIL;
        auto nextInst = m_currentInst->getNextInst();
        auto currentInst = m_currentInst;
        m_currentInst = nextInst;
        currentInst->functionPtr(this, currentInst, m_extInstHandlerUserData);
        if (m_executionFailed)
            return SLANG_FAIL;
    }
    return SLANG_OK;
}

ByteCodeInterpreter::ByteCodeInterpreter()
{
    m_printCallback = defaultPrintCallback;
    m_printCallbackUserData = this;
}

SLANG_NO_THROW SlangResult SLANG_MCALL
ByteCodeInterpreter::setPrintCallback(slang::VMPrintFunc callback, void* userData)
{
    m_printCallback = callback;
    m_printCallbackUserData = userData;
    return SLANG_OK;
}

void ByteCodeInterpreter::defaultPrintCallback(const char* str, void* userData)
{
    SLANG_UNUSED(userData);
    printf("%s", str);
}

ExecutableFunction::InstIterator ExecutableFunction::begin()
{
    ExecutableFunction::InstIterator iter;
    iter.codePtr = (uint8_t*)m_codeBuffer.getBuffer();
    return iter;
}

ExecutableFunction::InstIterator ExecutableFunction::end()
{
    ExecutableFunction::InstIterator iter;
    iter.codePtr = (uint8_t*)(m_codeBuffer.getBuffer() + m_codeBuffer.getCount());
    return iter;
}


} // namespace Slang


SLANG_EXTERN_C SLANG_API SlangResult slang_createByteCodeRunner(
    const slang::ByteCodeRunnerDesc* desc,
    slang::IByteCodeRunner** outByteCodeRunner)
{
    SLANG_UNUSED(desc);
    Slang::RefPtr<Slang::ByteCodeInterpreter> runner = new Slang::ByteCodeInterpreter();
    *outByteCodeRunner = static_cast<slang::IByteCodeRunner*>(runner.detach());
    return SLANG_OK;
}

SLANG_EXTERN_C SLANG_API SlangResult
slang_disassembleByteCode(slang::IBlob* moduleBlob, slang::IBlob** outDisassemblyBlob)
{
    Slang::VMModuleView moduleView;
    SLANG_RETURN_ON_FAIL(Slang::initVMModule(
        (uint8_t*)moduleBlob->getBufferPointer(),
        (uint32_t)moduleBlob->getBufferSize(),
        &moduleView));
    Slang::StringBuilder sb;
    sb << moduleView;
    *outDisassemblyBlob = Slang::StringBlob::moveCreate(sb.produceString()).detach();
    return SLANG_OK;
}
