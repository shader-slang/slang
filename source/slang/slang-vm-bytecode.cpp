#include "slang-vm-bytecode.h"

#include "core/slang-blob.h"
#include "core/slang-stream.h"
#include "core/slang-string-escape-util.h"

#include <string.h>

using namespace slang;

namespace Slang
{
static SlangResult consumeFourCC(MemoryStreamBase& stream, uint32_t expected)
{
    uint32_t fourCC = 0;
    size_t bytesRead = 0;
    SLANG_RETURN_ON_FAIL(stream.read(&fourCC, sizeof(fourCC), bytesRead));
    if (bytesRead != sizeof(fourCC) || fourCC != expected)
    {
        return SLANG_FAIL;
    }
    return SLANG_OK;
}

template<typename T>
static SlangResult readValue(MemoryStreamBase& stream, T& value)
{
    size_t bytesRead = 0;
    SLANG_RETURN_ON_FAIL(stream.read(&value, sizeof(T), bytesRead));
    if (bytesRead != sizeof(T))
    {
        return SLANG_FAIL; // Not enough data
    }
    return SLANG_OK;
}

static SlangResult readUInt32(MemoryStreamBase& stream, uint32_t& value)
{
    return readValue(stream, value);
}

static SlangResult calcArrayByteSize(uint32_t count, size_t elementSize, size_t& outSize)
{
    if (elementSize != 0 && size_t(count) > (~size_t(0)) / elementSize)
    {
        return SLANG_FAIL;
    }

    outSize = size_t(count) * elementSize;
    return SLANG_OK;
}

static SlangResult checkByteRangeAvailable(
    MemoryStreamBase& stream,
    uint32_t codeSize,
    size_t byteCount)
{
    Int64 position = stream.getPosition();
    if (position < 0 || UInt64(position) > UInt64(codeSize))
    {
        return SLANG_FAIL;
    }

    UInt64 remainingBytes = UInt64(codeSize) - UInt64(position);
    if (UInt64(byteCount) > remainingBytes)
    {
        return SLANG_FAIL;
    }

    return SLANG_OK;
}

SlangResult initVMModule(uint8_t* code, uint32_t codeSize, VMModuleView* moduleView)
{
    MemoryStreamBase stream(FileAccess::Read, code, codeSize);
    moduleView->code = code;

    // Check the FourCC
    SLANG_RETURN_ON_FAIL(consumeFourCC(stream, kSlangByteCodeFourCC));

    // Check the version
    uint32_t version = 0;
    SLANG_RETURN_ON_FAIL(readUInt32(stream, version));
    if (version > kSlangByteCodeVersion)
    {
        return SLANG_FAIL; // Unsupported version
    }

    // Read the function section
    SLANG_RETURN_ON_FAIL(consumeFourCC(stream, kSlangByteCodeFunctionsFourCC));
    uint32_t functionSectionSize = 0;
    SLANG_RETURN_ON_FAIL(readUInt32(stream, functionSectionSize));
    auto funcDataStart = stream.getPosition();
    // The entire function section bytes must be in-bounds.
    SLANG_RETURN_ON_FAIL(checkByteRangeAvailable(stream, codeSize, functionSectionSize));
    if (functionSectionSize < sizeof(uint32_t)) // At least the function count
    {
        return SLANG_FAIL; // Invalid section size
    }

    SLANG_RETURN_ON_FAIL(readUInt32(stream, moduleView->functionCount));
    // Bound-check the functionOffsets array: its byte size must fit in the section bytes
    // remaining after the functionCount field, computed with subtraction to avoid overflow.
    size_t functionOffsetsSize = 0;
    SLANG_RETURN_ON_FAIL(
        calcArrayByteSize(moduleView->functionCount, sizeof(uint32_t), functionOffsetsSize));
    if (functionOffsetsSize > size_t(functionSectionSize) - sizeof(uint32_t))
    {
        return SLANG_FAIL;
    }
    // The functionOffsets array is accessed as uint32_t[]; require the underlying byte position
    // to be 4-byte aligned so that the reinterpret_cast does not produce an unaligned pointer.
    if (stream.getPosition() % sizeof(uint32_t) != 0)
    {
        return SLANG_FAIL;
    }
    moduleView->functionOffsets = reinterpret_cast<uint32_t*>(code + stream.getPosition());

    stream.seek(SeekOrigin::Start, funcDataStart + functionSectionSize);

    // Read the kernel blob section
    SLANG_RETURN_ON_FAIL(consumeFourCC(stream, kSlangByteCodeKernelBlobFourCC));
    SLANG_RETURN_ON_FAIL(readUInt32(stream, moduleView->kernelBlobSize));
    SLANG_RETURN_ON_FAIL(checkByteRangeAvailable(stream, codeSize, moduleView->kernelBlobSize));
    moduleView->kernelBlob = code + stream.getPosition();
    stream.seek(SeekOrigin::Current, moduleView->kernelBlobSize);

    // Read the constants section. The writer stores `constantBlobSize` as the size of the
    // constants blob only (the bytes that follow the stringCount field and stringOffsets array),
    // so use it as that bound directly.
    SLANG_RETURN_ON_FAIL(consumeFourCC(stream, kSlangByteCodeConstantsFourCC));
    SLANG_RETURN_ON_FAIL(readUInt32(stream, moduleView->constantBlobSize));
    SLANG_RETURN_ON_FAIL(readUInt32(stream, moduleView->stringCount));
    size_t stringOffsetsSize = 0;
    SLANG_RETURN_ON_FAIL(
        calcArrayByteSize(moduleView->stringCount, sizeof(uint32_t), stringOffsetsSize));
    SLANG_RETURN_ON_FAIL(checkByteRangeAvailable(stream, codeSize, stringOffsetsSize));
    // The stringOffsets array is accessed as uint32_t[]; require the underlying byte position
    // to be 4-byte aligned so that the reinterpret_cast does not produce an unaligned pointer.
    if (stream.getPosition() % sizeof(uint32_t) != 0)
    {
        return SLANG_FAIL;
    }
    moduleView->stringOffsets = reinterpret_cast<uint32_t*>(code + stream.getPosition());
    SLANG_RETURN_ON_FAIL(stream.seek(SeekOrigin::Current, Int64(stringOffsetsSize)));
    SLANG_RETURN_ON_FAIL(checkByteRangeAvailable(stream, codeSize, moduleView->constantBlobSize));
    moduleView->constants = code + stream.getPosition();

    for (uint32_t i = 0; i < moduleView->functionCount; i++)
    {
        // Validate the function offset: must be 4-byte aligned for the VMFuncHeader / uint32_t
        // paramOffsets cast, and must leave room for at least a VMFuncHeader.
        uint32_t funcOff = moduleView->functionOffsets[i];
        if (funcOff % sizeof(uint32_t) != 0)
        {
            return SLANG_FAIL;
        }
        if (funcOff > codeSize || size_t(codeSize) - funcOff < sizeof(VMFuncHeader))
        {
            return SLANG_FAIL;
        }
        auto functionStart = code + funcOff;
        auto header = (VMFuncHeader*)(functionStart);

        // Validate that the parameter-offsets array plus the function code body fit within
        // the remaining bytes of the buffer after the header.
        size_t paramBytes = 0;
        SLANG_RETURN_ON_FAIL(
            calcArrayByteSize(header->parameterCount, sizeof(uint32_t), paramBytes));
        size_t remainingAfterHeader = size_t(codeSize) - funcOff - sizeof(VMFuncHeader);
        if (paramBytes > remainingAfterHeader)
        {
            return SLANG_FAIL;
        }
        if (size_t(header->codeSize) > remainingAfterHeader - paramBytes)
        {
            return SLANG_FAIL;
        }

        // Validate the function name reference: name.offset must index into stringOffsets, and
        // the resulting byte offset must point at a NUL-terminated string inside the constants
        // blob so downstream strlen-style access does not read out of bounds.
        if (header->name.offset >= moduleView->stringCount)
        {
            return SLANG_FAIL;
        }
        uint32_t strOff = moduleView->stringOffsets[header->name.offset];
        if (strOff >= moduleView->constantBlobSize)
        {
            return SLANG_FAIL;
        }
        const char* nameStr = (const char*)moduleView->constants + strOff;
        if (memchr(nameStr, 0, size_t(moduleView->constantBlobSize) - strOff) == nullptr)
        {
            return SLANG_FAIL;
        }

        VMFunctionView functionView;
        functionView.moduleView = moduleView;
        functionView.header = header;
        functionView.paramOffsets = (uint32_t*)(functionStart + sizeof(VMFuncHeader));
        functionView.name = nameStr;
        functionView.functionCode = (uint8_t*)functionView.paramOffsets + paramBytes;
        functionView.functionCodeEnd = functionView.functionCode + header->codeSize;
        moduleView->functionViews.add(functionView);
    }
    return SLANG_OK;
}

StringBuilder& operator<<(StringBuilder& sb, VMOp op)
{
    switch (op)
    {
    case VMOp::Add:
        sb << "add";
        break;
    case VMOp::Sub:
        sb << "sub";
        break;
    case VMOp::Mul:
        sb << "mul";
        break;
    case VMOp::Div:
        sb << "div";
        break;
    case VMOp::Rem:
        sb << "rem";
        break;
    case VMOp::And:
        sb << "and";
        break;
    case VMOp::Or:
        sb << "or";
        break;
    case VMOp::BitXor:
        sb << "bitxor";
        break;
    case VMOp::BitNot:
        sb << "bitnot";
        break;
    case VMOp::Shl:
        sb << "shl";
        break;
    case VMOp::Shr:
        sb << "shr";
        break;
    case VMOp::Equal:
        sb << "equal";
        break;
    case VMOp::Neq:
        sb << "neq";
        break;
    case VMOp::Less:
        sb << "less";
        break;
    case VMOp::Leq:
        sb << "leq";
        break;
    case VMOp::Greater:
        sb << "greater";
        break;
    case VMOp::Geq:
        sb << "geq";
        break;
    case VMOp::Nop:
        sb << "nop";
        break;
    case VMOp::Neg:
        sb << "neg";
        break;
    case VMOp::Not:
        sb << "not";
        break;
    case VMOp::Jump:
        sb << "jump";
        break;
    case VMOp::JumpIf:
        sb << "jumpif";
        break;
    case VMOp::Dispatch:
        sb << "dispatch";
        break;
    case VMOp::Load:
        sb << "load";
        break;
    case VMOp::Store:
        sb << "store";
        break;
    case VMOp::Copy:
        sb << "copy";
        break;
    case VMOp::GetWorkingSetPtr:
        sb << "get_working_set_ptr";
        break;
    case VMOp::GetElementPtr:
        sb << "get_element_ptr";
        break;
    case VMOp::OffsetPtr:
        sb << "offset_ptr";
        break;
    case VMOp::GetElement:
        sb << "get_element";
        break;
    case VMOp::Cast:
        sb << "cast";
        break;
    case VMOp::CallExt:
        sb << "call_ext";
        break;
    case VMOp::Call:
        sb << "call";
        break;
    case VMOp::Swizzle:
        sb << "swizzle";
        break;
    case VMOp::Ret:
        sb << "ret";
        break;
    case VMOp::Print:
        sb << "print";
        break;
    default:
        sb << "unknown_op(" << static_cast<uint32_t>(op) << ")";
        break;
    }
    return sb;
}

StringBuilder& operator<<(StringBuilder& sb, ArithmeticExtCode extCode)
{
    switch (extCode.scalarType)
    {
    case kSlangByteCodeScalarTypeSignedInt:
        sb << "i";
        break;
    case kSlangByteCodeScalarTypeUnsignedInt:
        sb << "u";
        break;
    case kSlangByteCodeScalarTypeFloat:
        sb << "f";
        break;
    default:
        sb << "x";
        break;
    }
    sb << (8 << extCode.scalarBitWidth);
    if (extCode.vectorSize > 1)
    {
        sb << "v" << extCode.vectorSize;
    }
    return sb;
}

void printVMInst(StringBuilder& sb, VMModuleView* moduleView, VMInstHeader* inst)
{
    auto lenBeforeOpCode = sb.getLength();
    sb << inst->opcode;
    if (inst->opcodeExtension != 0)
    {
        switch (inst->opcode)
        {
        case VMOp::Add:
        case VMOp::Sub:
        case VMOp::Mul:
        case VMOp::Div:
        case VMOp::Rem:
        case VMOp::And:
        case VMOp::Or:
        case VMOp::BitXor:
        case VMOp::BitNot:
        case VMOp::BitAnd:
        case VMOp::BitOr:
        case VMOp::Neg:
        case VMOp::Not:
        case VMOp::Shl:
        case VMOp::Shr:
        case VMOp::Equal:
        case VMOp::Neq:
        case VMOp::Less:
        case VMOp::Leq:
        case VMOp::Greater:
        case VMOp::Geq:
            {
                ArithmeticExtCode extCode;
                memcpy(&extCode, &inst->opcodeExtension, sizeof(extCode));
                sb << "." << extCode;
            }
            break;
        case VMOp::Cast:
            {
                ArithmeticExtCode extCode;
                memcpy(&extCode, &inst->opcodeExtension, sizeof(extCode));
                sb << "." << extCode;
                uint32_t fromCode = inst->opcodeExtension >> 16;
                memcpy(&extCode, &fromCode, sizeof(extCode));
                sb << "." << extCode;
            }
            break;
        default:
            sb << "." << inst->opcodeExtension;
            break;
        }
    }
    auto opCodeLength = (int)(sb.getLength() - lenBeforeOpCode);
    static const int kOpCodeColumnWidth = 20;
    if (opCodeLength < kOpCodeColumnWidth)
    {
        for (int i = 0; i < kOpCodeColumnWidth - opCodeLength; i++)
        {
            sb << " ";
        }
    }
    else
    {
        sb << " ";
    }
    for (uint32_t i = 0; i < inst->operandCount; i++)
    {
        if (i > 0)
            sb << ", ";
        auto operand = inst->getOperand(i);
        switch (operand.sectionId)
        {
        case kSlangByteCodeSectionConstants:
            switch (operand.getType())
            {
            case OperandDataType::Int32:
                {
                    int32_t val = 0;
                    moduleView->getConstant<int32_t>(operand, val);
                    sb << "i32(" << val << ")";
                    continue;
                }
            case OperandDataType::Int64:
                {
                    int64_t val = 0;
                    moduleView->getConstant<int64_t>(operand, val);
                    sb << "i64(" << val << ")";
                    continue;
                }
            case OperandDataType::Float32:
                {
                    float val = 0.f;
                    moduleView->getConstant<float>(operand, val);
                    sb << "f32(" << val << ")";
                    continue;
                }
            case OperandDataType::Float64:
                {
                    double val = 0.0;
                    moduleView->getConstant<double>(operand, val);
                    sb << "f32(" << val << ")";
                    continue;
                }
            }
            sb << "const:";
            break;
        case kSlangByteCodeSectionInsts:
            sb << "inst:";
            break;
        case kSlangByteCodeSectionWorkingSet:
            sb << "ws:";
            break;
        case kSlangByteCodeSectionImmediate:
            sb << "!";
            break;
        case kSlangByteCodeSectionFuncs:
            sb << moduleView->getFunction(operand.offset).name;
            continue;
        case kSlangByteCodeSectionStrings:
            sb << "str:";
            if (operand.offset < moduleView->stringCount)
            {
                auto str = StringEscapeUtil::escapeString(UnownedStringSlice(
                    ((char*)moduleView->constants + moduleView->stringOffsets[operand.offset])));
                sb << str;
            }
            else
            {
                sb << "<invalid string index>";
            }
            continue;
        default:
            sb << "section(" << operand.sectionId << ")@";
            break;
        }
        sb << String(inst->getOperand(i).offset, 16);
    }
}

StringBuilder& operator<<(StringBuilder& sb, VMModuleView& module)
{
    static const int addrColumnSize = 6;
    for (uint32_t i = 0; i < module.functionCount; i++)
    {
        auto f = module.getFunction(i);
        sb << "func " << f.name << ":\n";
        for (auto inst : f)
        {
            sb << "  ";
            auto loc = ((uint8_t*)inst - f.functionCode);
            auto pos = sb.getLength();
            sb << String((uint32_t)loc, 16) << ": ";
            auto addrLength = (int)(sb.getLength() - pos);
            for (int j = 0; j < addrColumnSize - addrLength; j++)
            {
                sb << " ";
            }
            printVMInst(sb, &module, inst);
            sb << "\n";
        }
    }
    return sb;
}

VMFunctionView VMModuleView::getFunction(Index index) const
{
    if (index >= functionCount)
        return {};
    return functionViews[index];
}
} // namespace Slang
