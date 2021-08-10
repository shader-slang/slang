// slang-ir-spirv-snippet.cpp

#include"slang-ir-spirv-snippet.h"
#include "../core/slang-token-reader.h"

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

RefPtr<SpvSnippet> SpvSnippet::parse(UnownedStringSlice definition)
{
    RefPtr<SpvSnippet> snippet = new SpvSnippet();
    try
    {
        Dictionary<String, int> mapInstNameToIndex;
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
                mapInstNameToIndex[instName] = (int)snippet->instructions.getCount();
                tokenReader.Read(Slang::Misc::TokenType::OpAssign);
            }
            inst.opCode = (SpvWord)tokenReader.ReadInt();
            bool insideOperandList = true;
            while (insideOperandList)
            {
                ASMOperand operand = {};
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
                        operand.type = SpvSnippet::ASMOperandType::InstReference;
                        auto refName = tokenReader.ReadToken().Content;
                        if (!mapInstNameToIndex.TryGetValue(refName, operand.content))
                        {
                            SLANG_ASSERT(!"Invalid SPV ASM: referenced inst is not defined.");
                        }
                        inst.operands.add(operand);
                    }
                    break;
                case Slang::Misc::TokenType::Identifier:
                    {
                        auto identifier = tokenReader.ReadToken().Content;
                        if (identifier.startsWith("_"))
                        {
                            operand.type = SpvSnippet::ASMOperandType::ObjectReference;
                            operand.content =
                                StringToInt(identifier.subString(1, identifier.getLength() - 1));
                            inst.operands.add(operand);
                        }
                        else if (identifier == "resultType")
                        {
                            operand.type = SpvSnippet::ASMOperandType::ResultTypeId;
                            operand.content = -1;
                            if (tokenReader.AdvanceIf("*"))
                            {
                                // A "*" at operand qualifies the use of `resultType` with
                                // a storage class, but does not modify `resultType` itself.
                                auto storageClass = tokenReader.ReadWord();
                                auto spvStorageClass = translateStorageClass(storageClass);
                                operand.content = spvStorageClass;
                                snippet->usedResultTypeStorageClasses.add(spvStorageClass);
                            }
                            inst.operands.add(operand);
                        }
                        else if (identifier == "resultId")
                        {
                            operand.type = SpvSnippet::ASMOperandType::ResultId;
                            inst.operands.add(operand);
                        }
                        else
                        {
                            SLANG_ASSERT(!"Invalid SPV ASM operand.");
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
    catch (const Slang::Misc::TextFormatException&)
    {
        SLANG_ASSERT(!"Invalid ASM format.");
    }
    return snippet;
}


}
