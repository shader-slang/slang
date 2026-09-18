// slang-ir-spirv-snippet.cpp

#include "slang-ir-spirv-snippet.h"

#include "compiler-core/slang-spirv-core-grammar.h"
#include "core/slang-token-reader.h"
#include "slang-lookup-spirv.h"

namespace Slang
{
static SpvStorageClass translateStorageClass(String name)
{
    if (name == "Uniform")
    {
        return SpvStorageClassUniform;
    }
    else if (name == "StorageBuffer")
    {
        return SpvStorageClassStorageBuffer;
    }
    return (SpvStorageClass)-1;
}

SpvSnippet::ASMType parseASMType(Slang::Misc::TokenReader& tokenReader)
{
    auto word = tokenReader.ReadWord();
    if (word == "float")
        return SpvSnippet::ASMType::Float;
    else if (word == "double")
        return SpvSnippet::ASMType::Double;
    else if (word == "uint2")
        return SpvSnippet::ASMType::UInt2;
    else if (word == "uint16_t")
        return SpvSnippet::ASMType::UInt16;
    else if (word == "float2")
        return SpvSnippet::ASMType::Float2;
    else if (word == "int")
        return SpvSnippet::ASMType::Int;
    else if (word == "uint")
        return SpvSnippet::ASMType::UInt;
    else if (word == "_p")
        return SpvSnippet::ASMType::FloatOrDouble;
    else if (word == "half")
        return SpvSnippet::ASMType::Half;
    return SpvSnippet::ASMType::None;
}

bool SpvSnippet::isEmittableASMType(ASMType type)
{
    switch (type)
    {
    case ASMType::Int:
    case ASMType::UInt:
    case ASMType::UInt16:
    case ASMType::Half:
    case ASMType::Float:
    case ASMType::Float2:
    case ASMType::UInt2:
        return true;
    default:
        // None (the unknown-token sentinel), Double, and FloatOrDouble have no lowering in either
        // emitter switch, so an operand of one of these types is diagnosed rather than emitted.
        return false;
    }
}

UnownedStringSlice SpvSnippet::getASMTypeName(ASMType type)
{
    switch (type)
    {
    case ASMType::Int:
        return UnownedStringSlice("int");
    case ASMType::UInt:
        return UnownedStringSlice("uint");
    case ASMType::UInt16:
        return UnownedStringSlice("uint16_t");
    case ASMType::Half:
        return UnownedStringSlice("half");
    case ASMType::Float:
        return UnownedStringSlice("float");
    case ASMType::Double:
        return UnownedStringSlice("double");
    case ASMType::FloatOrDouble:
        return UnownedStringSlice("float-or-double (_p)");
    case ASMType::Float2:
        return UnownedStringSlice("float2");
    case ASMType::UInt2:
        return UnownedStringSlice("uint2");
    case ASMType::None:
        // None is the sentinel parseASMType returns for an unrecognized type token; naming it
        // "unknown" is the spelling the un-emittable-operand diagnostic reports for such a token.
        return UnownedStringSlice("unknown");
    }
    // Every ASMType enumerator is handled above, and omitting `default` lets -Wswitch flag a newly
    // added one; reaching here means an out-of-contract cast to a non-enumerator value.
    SLANG_UNEXPECTED("unhandled ASMType in getASMTypeName");
}

// Read an unsigned integer (a SPIR-V word) or a SPIR-V enum (currently those
// which are coded into this function).
//
// This also 'or's together a list of these words/enums separated by '|'
SpvWord readWordOrWordLiteral(Misc::TokenReader& reader)
{
    SpvWord ret = 0;
    do
    {
        switch (reader.NextToken().Type)
        {
        case Slang::Misc::TokenType::IntLiteral:
            ret = reader.ReadUInt();
            break;
        case Slang::Misc::TokenType::Identifier:
            {
                const auto i = reader.ReadWord();
#define GO(x)    \
    if (i == #x) \
    ret |= Spv##x
                GO(ScopeWorkgroup);
                else GO(ScopeDevice);
                else GO(MemorySemanticsMaskNone);
                else GO(MemorySemanticsAcquireReleaseMask);
                else GO(MemorySemanticsUniformMemoryMask);
                else GO(MemorySemanticsImageMemoryMask);
                else GO(MemorySemanticsAtomicCounterMemoryMask);
                else GO(MemorySemanticsWorkgroupMemoryMask);
#undef GO
                else
                {
                    reader.Back(1);
                    throw Misc::TextFormatException(
                        "Text parsing error: Unrecognized SPIR-V enum: " + i);
                }
            }
            break;
        default:
            throw Misc::TextFormatException("Text parsing error: Expected int or SPIR-V enum");
        }
    } while (reader.AdvanceIf(Misc::TokenType::OpBitOr));
    return ret;
}

RefPtr<SpvSnippet> SpvSnippet::parse(
    const SPIRVCoreGrammarInfo& spirvGrammar,
    UnownedStringSlice definition)
{
    RefPtr<SpvSnippet> snippet = new SpvSnippet();
    try
    {
        Dictionary<String, SpvWord> mapInstNameToIndex;
        Slang::Misc::TokenReader tokenReader(definition);
        // A leading "*" at the beginning of the snip modifies $resultType with
        // a storage class.
        if (tokenReader.AdvanceIf("*"))
        {
            auto storageToken = tokenReader.ReadWord();
            snippet->resultStorageClass = translateStorageClass(storageToken);
        }
        while (!tokenReader.IsEnd())
        {
            SpvSnippet::ASMInst inst;
            if (tokenReader.AdvanceIf("%"))
            {
                String instName = tokenReader.ReadToken().Content;
                mapInstNameToIndex.set(instName, (int)snippet->instructions.getCount());
                inst.resultName = instName;
                tokenReader.Read(Slang::Misc::TokenType::OpAssign);
            }
            SpvOp opCode;
            switch (tokenReader.NextToken().Type)
            {
            case Slang::Misc::TokenType::IntLiteral:
                opCode = (SpvOp)tokenReader.ReadInt();
                break;
            case Slang::Misc::TokenType::Identifier:
                {
                    auto opName = tokenReader.ReadWord();
                    const auto opCodeMaybe = spirvGrammar.opcodes.lookup(opName.getUnownedSlice());
                    if (!opCodeMaybe)
                    {
                        throw Misc::TextFormatException(
                            "Text parsing error: Unrecognized SPIR-V opcode: " + opName);
                    }
                    opCode = *opCodeMaybe;
                    break;
                }
            default:
                throw Misc::TextFormatException("Text parsing error: SPIR-V intrinsics must "
                                                "begin with an integer or opcode name");
            }
            inst.opCode = opCode;
            bool insideOperandList = true;
            const bool isExtInst = inst.opCode == SpvOpExtInst;
            bool isGLSLstd450OpcodeAllowed = false;
            auto readExtInstOpcode = [&]()
            {
                switch (tokenReader.NextToken().Type)
                {
                case Slang::Misc::TokenType::IntLiteral:
                    return (SpvWord)tokenReader.ReadInt();
                    break;
                case Slang::Misc::TokenType::Identifier:
                    {
                        if (isGLSLstd450OpcodeAllowed)
                        {
                            auto opName = tokenReader.ReadWord();
                            GLSLstd450 glslOpcode;
                            if (!lookupGLSLstd450(opName.getUnownedSlice(), glslOpcode))
                            {
                                throw Misc::TextFormatException(
                                    "Text parsing error: Unrecognized SPIR-V GLSLstd450 opcode: " +
                                    opName);
                            }
                            return (SpvWord)glslOpcode;
                        }
                    }
                // fallthrough
                default:
                    throw Misc::TextFormatException(
                        "Text parsing error: Failed to read SPIR-V ExtInst Opcode");
                }
            };
            while (insideOperandList)
            {
                ASMOperand operand = {ASMOperandType::SpvWord, 0, 0, 0};
                switch (tokenReader.NextToken().Type)
                {
                case Slang::Misc::TokenType::Semicolon:
                    insideOperandList = false;
                    tokenReader.ReadToken();
                    break;
                case Slang::Misc::TokenType::IntLiteral:
                    operand.type = SpvSnippet::ASMOperandType::SpvWord;
                    operand.content = tokenReader.ReadInt();
                    inst.operands.add(operand);
                    break;
                case Slang::Misc::TokenType::OpMod:
                    {
                        tokenReader.ReadToken();
                        operand.type = SpvSnippet::ASMOperandType::InstReference;
                        auto refName = tokenReader.ReadToken().Content;
                        if (!mapInstNameToIndex.tryGetValue(refName, operand.content))
                        {
                            // An undefined `%name` reference is malformed user input and must be
                            // diagnosed.
                            throw Misc::TextFormatException(
                                "Text parsing error: SPIR-V snippet references an undefined "
                                "instruction: %" +
                                refName);
                        }
                        inst.operands.add(operand);
                    }
                    break;
                case Slang::Misc::TokenType::Identifier:
                    {
                        auto identifier = tokenReader.ReadToken().Content;
                        if (identifier == "resultType")
                        {
                            operand.type = SpvSnippet::ASMOperandType::ResultTypeId;
                            operand.content = (SpvWord)0xFFFFFFFF;
                            if (tokenReader.AdvanceIf("*"))
                            {
                                // A "*" at operand qualifies the use of `resultType` as
                                // `ptr(resultType, storage class), but does
                                // not modify `resultType` itself.
                                auto storageClass = tokenReader.ReadWord();
                                auto spvStorageClass = translateStorageClass(storageClass);
                                operand.content = spvStorageClass;
                                snippet->usedPtrResultTypeStorageClasses.add(spvStorageClass);
                            }
                            inst.operands.add(operand);
                        }
                        else if (identifier == "resultId")
                        {
                            operand.type = SpvSnippet::ASMOperandType::ResultId;
                            inst.operands.add(operand);
                        }
                        else if (identifier == "glsl450")
                        {
                            operand.type = SpvSnippet::ASMOperandType::GLSL450ExtInstSet;
                            inst.operands.add(operand);
                            // Allow the next token to be parsed as a glslsstd450 opcode
                            isGLSLstd450OpcodeAllowed = isExtInst;
                        }
                        else if (identifier == "fi")
                        {
                            operand.type = SpvSnippet::ASMOperandType::FloatIntegerSelection;
                            tokenReader.Read("(");
                            operand.content = readExtInstOpcode();
                            tokenReader.Read(",");
                            operand.content2 = readExtInstOpcode();
                            tokenReader.Read(")");
                            inst.operands.add(operand);
                        }
                        else if (identifier == "fus")
                        {
                            operand.type = SpvSnippet::ASMOperandType::FloatUnsignedSignedSelection;
                            tokenReader.Read("(");
                            operand.content = readExtInstOpcode();
                            tokenReader.Read(",");
                            operand.content2 = readExtInstOpcode();
                            tokenReader.Read(",");
                            operand.content3 = readExtInstOpcode();
                            tokenReader.Read(")");
                            inst.operands.add(operand);
                        }
                        else if (identifier == "_type")
                        {
                            operand.type = SpvSnippet::ASMOperandType::TypeReference;
                            tokenReader.Read("(");
                            operand.content = (SpvWord)parseASMType(tokenReader);
                            tokenReader.Read(")");
                            inst.operands.add(operand);
                        }
                        else if (identifier.startsWith("_"))
                        {
                            operand.type = SpvSnippet::ASMOperandType::ObjectReference;
                            operand.content = (SpvWord)stringToInt(
                                identifier.subString(1, identifier.getLength() - 1));
                            inst.operands.add(operand);
                        }
                        else if (identifier == "const")
                        {
                            operand.type = SpvSnippet::ASMOperandType::ConstantReference;
                            ASMConstant constant;
                            memset(&constant, 0, sizeof(ASMConstant));
                            tokenReader.Read("(");
                            constant.type = parseASMType(tokenReader);
                            int i = 0;
                            // The value arrays are fixed-size. The bound is tested before
                            // AdvanceIf(","), so an over-long list leaves its extra comma
                            // unconsumed for the closing ')' read below to reject as the same
                            // E29000 parse error the sibling paths produce.
                            while (i < SpvSnippet::kMaxASMConstantValues &&
                                   tokenReader.AdvanceIf(","))
                            {
                                switch (constant.type)
                                {
                                case ASMType::Half:
                                case ASMType::Float:
                                // `Double` is read here even though it is not isEmittableASMType
                                // and is never emitted: reading its fractional literal lets the
                                // snippet parse and reach the legalization-time `E29001`
                                // "un-emittable operand" diagnostic (raised by validateSpvSnippet),
                                // rather than failing as a misleading `E29000` parse error. It
                                // lands in `floatValues` (narrowed to 32-bit) because that is the
                                // field ASMConstant hashes/compares on; the narrowing is harmless
                                // precisely because a `double` constant never reaches emit.
                                case ASMType::Double:
                                case ASMType::Float2:
                                case ASMType::FloatOrDouble:
                                    constant.floatValues[i] = tokenReader.ReadFloat();
                                    ++i;
                                    break;

                                default:
                                    constant.intValues[i] = readWordOrWordLiteral(tokenReader);
                                    ++i;
                                    break;
                                }
                            }
                            tokenReader.Read(")");
                            snippet->constants.add(constant);
                            operand.content = (SpvWord)(snippet->constants.getCount() - 1);
                            inst.operands.add(operand);
                        }
                        else if (isGLSLstd450OpcodeAllowed)
                        {
                            GLSLstd450 glslstd450Opcode;
                            lookupGLSLstd450(identifier.getUnownedSlice(), glslstd450Opcode);
                            operand.type = SpvSnippet::ASMOperandType::SpvWord;
                            operand.content = (SpvWord)glslstd450Opcode;
                            inst.operands.add(operand);
                        }
                        else
                        {
                            // An unrecognized operand identifier is malformed user input and must
                            // be diagnosed.
                            throw Misc::TextFormatException(
                                "Text parsing error: Invalid SPIR-V ASM operand: \"" + identifier +
                                "\"");
                        }
                    }
                    break;
                default:
                    insideOperandList = false;
                    break;
                }
            }
            snippet->instructions.add(inst);
        }
    }
    // Any parse step throws Misc::TextFormatException on malformed snippet text; returning null
    // here lets the caller (getParsedSpvSnippet) report it once as snippet-parsing-failed (E29000).
    catch (const Slang::Misc::TextFormatException&)
    {
        return nullptr;
    }
    return snippet;
}


} // namespace Slang
