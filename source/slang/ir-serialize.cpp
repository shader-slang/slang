// ir-serialize.cpp
#include "ir-serialize.h"

#include "../core/text-io.h"

#include "ir-insts.h"

namespace Slang {

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! IRSerialInfo !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

UnownedStringSlice IRSerialData::getStringSlice(StringIndex index) const
{
    if (index == StringIndex(0))
    {
        return UnownedStringSlice(m_strings.begin() + 1, UInt(0));
    }

    const char* prefix = m_strings.begin() + int(index);

    struct Reader
    {
        char operator()(int pos) const { SLANG_UNUSED(pos); return *m_pos++; } 
        Reader(const char* pos):m_pos(pos) {}
        mutable const char* m_pos;
    };

    Reader reader(prefix);
    const int len = GetUnicodePointFromUTF8(reader);
    return UnownedStringSlice(reader.m_pos, len);
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! IRSerialWriter !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

void IRSerialWriter::_addInstruction(IRInst* inst)
{
    // It cannot already be in the map
    SLANG_ASSERT(!m_instMap.ContainsKey(inst));

    // Add to the map
    m_instMap.Add(inst, Ser::InstIndex(m_insts.Count()));
    m_insts.Add(inst);

    // Add all the decorations, to the list. 
    // We don't add to the decoration map, as we only want to do this once all the instructions have been hit
    if (inst->firstDecoration)
    {
        m_instWithFirstDecoration.Add(inst);

        IRDecoration* decor = inst->firstDecoration;
  
        const int initialNumDecor = int(m_decorations.Count());
        while (decor)
        {
            m_decorations.Add(decor);
            decor = decor->next;
        }
        
        const Ser::SizeType numDecor = Ser::SizeType(int(m_decorations.Count()) - initialNumDecor);
        
        Ser::InstRun run;
        run.m_parentIndex = m_instMap[inst];

        // NOTE! This isn't quite correct, as we need to correct for when all the instructions are added, this is done at the end
        run.m_startInstIndex = Ser::InstIndex(initialNumDecor);
        run.m_numChildren = numDecor;

        m_serialData->m_decorationRuns.Add(run);
    }
}

IRSerialData::StringIndex IRSerialWriter::getStringIndex(Name* name) 
{ 
    return name ? getStringIndex(name->text.getUnownedSlice()) : Ser::kNullStringIndex; 
}

Result IRSerialWriter::write(IRModule* module, IRSerialData* serialInfo)
{
    // Lets find all of the instructions
    // Each instruction can only have one parent
    // That being the case if we traverse the module in breadth first order (following children specifically)
    // Then all 'child' nodes will be in order, and thus we only have to store the start instruction
    // and the amount of instructions 

    m_serialData = serialInfo;

    serialInfo->clear();

    // We reserve 0 for null
    m_insts.Clear();
    m_insts.Add(nullptr);

    // Reset
    m_instMap.Clear();
    m_decorations.Clear();
    
    // Stack for parentInst
    List<IRParentInst*> parentInstStack;
  
    IRModuleInst* moduleInst = module->getModuleInst();
    parentInstStack.Add(moduleInst);

    // Add to the map
    _addInstruction(moduleInst);

    // Traverse all of the instructions
    while (parentInstStack.Count())
    {
        // If it's in the stack it is assumed it is already in the inst map
        IRParentInst* parentInst = parentInstStack.Last();
        parentInstStack.RemoveLast();
        SLANG_ASSERT(m_instMap.ContainsKey(parentInst));

        
        // Okay we go through each of the children in order. If they are IRInstParent derived, we add to stack to process later 
        // cos we want breadth first...

        const Ser::InstIndex startChildInstIndex = Ser::InstIndex(m_insts.Count());
        
        IRInstListBase childrenList = parentInst->getChildren();
        for (IRInst* child : childrenList)
        {
            // This instruction can't be in the map...
            SLANG_ASSERT(!m_instMap.ContainsKey(child));

            _addInstruction(child);
            
            IRParentInst* childAsParent = as<IRParentInst>(child);
            if (childAsParent)
            {
                parentInstStack.Add(childAsParent);
            }
        }

        // If it had any children, then store the information about it
        if (Ser::InstIndex(m_insts.Count()) != startChildInstIndex)
        {
            Ser::InstRun run;
            run.m_parentIndex = m_instMap[parentInst];
            run.m_startInstIndex = startChildInstIndex;
            run.m_numChildren = Ser::SizeType(m_insts.Count() - int(startChildInstIndex));

            m_serialData->m_childRuns.Add(run);
        }
    }

    // Now fix the decorations 
    {
        const int decorationBaseIndex = int(m_insts.Count());
        m_serialData->m_decorationBaseIndex = decorationBaseIndex;
        const int numDecorRuns = int(m_serialData->m_decorationRuns.Count());

        // Work out the total num of decoration
        int totalNumDecorations = 0;
        if (numDecorRuns)
        {
            const auto& lastDecorInfo = m_serialData->m_decorationRuns.Last();
            totalNumDecorations = int(lastDecorInfo.m_startInstIndex) + lastDecorInfo.m_numChildren;
        }

        // Fix the indices
        for (int i = 0; i < numDecorRuns; ++i)
        {
            Ser::InstRun& info = m_serialData->m_decorationRuns[i];
            info.m_startInstIndex = Ser::InstIndex(decorationBaseIndex + int(info.m_startInstIndex));
        }

        // Set to the right size
        m_serialData->m_insts.SetSize(decorationBaseIndex + totalNumDecorations);
        // Clear all instructions
        memset(m_serialData->m_insts.begin(), 0, sizeof(Ser::Inst) * m_serialData->m_insts.Count());
    }

    // Need to set up the actual instructions
    {
        const int numInsts = int(m_insts.Count());

        for (int i = 1; i < numInsts; ++i)
        {
            IRInst* srcInst = m_insts[i];
            Ser::Inst& dstInst = m_serialData->m_insts[i];
            
            dstInst.m_op = uint8_t(srcInst->op);
            dstInst.m_payloadType = Ser::Inst::PayloadType::Empty;
            
            dstInst.m_resultTypeIndex = getInstIndex(srcInst->getFullType());

            // Need to special case 
            if (srcInst->op != (srcInst->op & kIROpMeta_OpMask))
            {
                // For now ignore
                continue;
            }
            
            IRConstant* irConst = as<IRConstant>(srcInst);    
            if (irConst)
            {
                switch (srcInst->op)
                {
                    case kIROp_StringLit:
                    {
                        auto stringLit = static_cast<IRStringLit*>(srcInst);
                        dstInst.m_payloadType = Ser::Inst::PayloadType::String_1;
                        dstInst.m_payload.m_stringIndices[0] = getStringIndex(stringLit->getStringSlice());
                        break;
                    }
                    case kIROp_IntLit:
                    {
                        dstInst.m_payloadType = Ser::Inst::PayloadType::Int64;
                        dstInst.m_payload.m_int64 = irConst->value.intVal;
                        break;
                    }
                    case kIROp_FloatLit:
                    {
                        dstInst.m_payloadType = Ser::Inst::PayloadType::Float64;
                        dstInst.m_payload.m_float64 = irConst->value.floatVal; 
                        break;
                    }
                    case kIROp_boolConst:
                    {
                        dstInst.m_payloadType = Ser::Inst::PayloadType::String_1;
                        dstInst.m_payload.m_uint32 = irConst->value.intVal ? 1 : 0;
                        break;
                    }
                }
                
                continue;
            }

            const int numOperands = int(srcInst->operandCount);
            Ser::InstIndex* dstOperands = nullptr;

            if (numOperands <= Ser::kNumOperands)
            {
                dstOperands = dstInst.m_payload.m_operands;
                dstInst.m_payloadType = Ser::Inst::PayloadType(numOperands);
            }
            else
            {
                dstInst.m_payloadType = Ser::Inst::PayloadType::ExternalOperand;

                int operandArrayBaseIndex = int(m_serialData->m_externalOperands.Count());
                m_serialData->m_externalOperands.SetSize(operandArrayBaseIndex + numOperands);

                dstOperands = m_serialData->m_externalOperands.begin() + operandArrayBaseIndex; 

                auto& externalOperands = dstInst.m_payload.m_externalOperand;
                externalOperands.m_arrayIndex = Ser::ArrayIndex(operandArrayBaseIndex);
                externalOperands.m_size = Ser::SizeType(numOperands);
            }

            for (int j = 0; j < numOperands; ++j)
            {
                const Ser::InstIndex dstInstIndex = getInstIndex(srcInst->getOperand(j));
                dstOperands[j] = dstInstIndex;
            }
        }
    }

    // Now need to do the decorations

    {
        const int decorationBaseIndex = m_serialData->m_decorationBaseIndex;
        const int numDecor = int(m_decorations.Count());
        SLANG_ASSERT(decorationBaseIndex + numDecor == m_serialData->m_insts.Count());

        // Have to be able to store in a byte!
        SLANG_COMPILE_TIME_ASSERT(kIROpCount + kIRDecorationOp_CountOf < 0x100);

        for (int i = 0; i < numDecor; ++i)
        {
            IRDecoration* srcDecor = m_decorations[i];
            Ser::Inst& dstInst = m_serialData->m_insts[decorationBaseIndex + i];

            dstInst.m_op = uint8_t(kIROpCount + srcDecor->op);

            switch (srcDecor->op)
            {
                case kIRDecorationOp_HighLevelDecl:
                {
                    // TODO!
                    // Decl* decl;
                    break;
                }
                case kIRDecorationOp_Layout:
                {
                    // TODO!
                    // Layout* layout;
                    break;
                }
                case kIRDecorationOp_LoopControl:
                {
                    auto loopDecor = static_cast<IRLoopControlDecoration*>(srcDecor);

                    dstInst.m_payloadType = Ser::Inst::PayloadType::UInt32;
                    dstInst.m_payload.m_uint32 = uint32_t(loopDecor->mode);
                    break;
                }
                case kIRDecorationOp_Target:
                {
                    auto targetDecor = static_cast<IRTargetDecoration*>(srcDecor);

                    dstInst.m_payloadType = Ser::Inst::PayloadType::String_1;
                    dstInst.m_payload.m_stringIndices[0] = getStringIndex(targetDecor->targetName);
                    break;
                }
                case kIRDecorationOp_TargetIntrinsic:
                {
                    auto targetDecor = static_cast<IRTargetIntrinsicDecoration*>(srcDecor);
                    dstInst.m_payloadType = Ser::Inst::PayloadType::String_2;

                    dstInst.m_payload.m_stringIndices[0] = getStringIndex(targetDecor->targetName);
                    dstInst.m_payload.m_stringIndices[1] = getStringIndex(targetDecor->definition);
                    break;
                }
                case kIRDecorationOp_GLSLOuterArray:
                {
                    auto arrayDecor = static_cast<IRGLSLOuterArrayDecoration*>(srcDecor);
                    dstInst.m_payloadType = Ser::Inst::PayloadType::String_1;

                    dstInst.m_payload.m_stringIndices[0] = getStringIndex(arrayDecor->outerArrayName);
                    break;
                }
                case kIRDecorationOp_Semantic:
                {
                    auto semanticDecor = static_cast<IRSemanticDecoration*>(srcDecor);

                    dstInst.m_payloadType = Ser::Inst::PayloadType::String_1;
                    dstInst.m_payload.m_stringIndices[0] = getStringIndex(semanticDecor->semanticName);
                    break;
                }
                case kIRDecorationOp_NameHint:
                {
                    auto nameDecor = static_cast<IRNameHintDecoration*>(srcDecor);

                    dstInst.m_payloadType = Ser::Inst::PayloadType::String_1;
                    dstInst.m_payload.m_stringIndices[0] = getStringIndex(nameDecor->name);
                    break;
                }
                default:
                {
                    SLANG_ASSERT(!"Unhandled decoration type");
                    return SLANG_FAIL;
                }
            }
        }
    }

    m_serialData = nullptr;

    return SLANG_OK;
}

IRSerialData::StringIndex IRSerialWriter::getStringIndex(const UnownedStringSlice& slice)
{
    const int len = int(slice.size());
    if (len <= 0)
    {
        return Ser::kEmptyStringIndex;
    }

    Ser::StringIndex index;
    if (m_stringMap.TryGetValue(slice, index))
    {
        return index;
    }

    // We need to write into the the string array
    char prefixBytes[6];
    const int numPrefixBytes = EncodeUnicodePointToUTF8(prefixBytes, len);
    const int baseIndex = int(m_serialData->m_strings.Count());

    m_serialData->m_strings.SetSize(baseIndex + numPrefixBytes + len);

    char* dst = m_serialData->m_strings.begin() + baseIndex;

    memcpy(dst, prefixBytes, numPrefixBytes);
    memcpy(dst + numPrefixBytes, slice.begin(), len);

    // Add to the map. Unfortunately we can't use the string storage in the array, because the address changes with size
    // so we have to store backing string in an arena
    const char* arenaChars = m_arena.allocateString(slice.begin(), len);
    m_stringMap.Add(UnownedStringSlice(arenaChars, len), Ser::StringIndex(baseIndex));

    return Ser::StringIndex(baseIndex);
}

template <typename T>
static size_t _calcChunkSize(const List<T>& array)
{
    typedef IRSerialBinary Bin;

    if (array.Count())
    {
        const size_t size = sizeof(Bin::ArrayHeader) + sizeof(T) * array.Count();
        return (size + 3) & ~size_t(3);
    }
    else
    {
        return 0;
    }
}

template <typename T>
Result _writeArrayChunk(uint32_t chunkId, const List<T>& array, Stream* stream)
{
    typedef IRSerialBinary Bin;

    if (array.Count() == 0)
    {
        return SLANG_OK;
    }

    size_t payloadSize = sizeof(Bin::ArrayHeader) - sizeof(Bin::Chunk) + sizeof(T) * array.Count();

    Bin::ArrayHeader header;
    header.m_chunk.m_type = chunkId;
    header.m_chunk.m_size = uint32_t(payloadSize);
    header.m_numEntries = uint32_t(array.Count());

    stream->Write(&header, sizeof(header));

    stream->Write(array.begin(), sizeof(T) * array.Count());

    // All chunks have sizes rounded to dword size
    if (payloadSize & 3)
    {
        const uint8_t pad[4] = { 0, 0, 0, 0 };
        // Pad outs
        int padSize = 4 - (payloadSize & 3);
        stream->Write(pad, padSize);
    }

    return SLANG_OK;
}

/* static */Result IRSerialWriter::writeStream(const IRSerialData& data, Stream* stream)
{
    typedef IRSerialBinary Bin;

    size_t totalSize = 0;
    
    totalSize += sizeof(Bin::SlangHeader) + 
        _calcChunkSize(data.m_insts) + 
        _calcChunkSize(data.m_childRuns) +
        _calcChunkSize(data.m_decorationRuns) +
        _calcChunkSize(data.m_externalOperands) +
        _calcChunkSize(data.m_strings);

    {
        Bin::Chunk riffHeader;
        riffHeader.m_type = Bin::kRiffFourCc;
        riffHeader.m_size = uint32_t(totalSize);

        stream->Write(&riffHeader, sizeof(riffHeader));
    }
    {
        Bin::SlangHeader slangHeader;
        slangHeader.m_chunk.m_type = Bin::kSlangFourCc;
        slangHeader.m_chunk.m_size = uint32_t(sizeof(slangHeader) - sizeof(Bin::Chunk));
        slangHeader.m_decorationBase = uint32_t(data.m_decorationBaseIndex);

        stream->Write(&slangHeader, sizeof(slangHeader));
    }

    _writeArrayChunk(Bin::kInstFourCc, data.m_insts, stream);
    _writeArrayChunk(Bin::kChildRunFourCc, data.m_childRuns, stream);
    _writeArrayChunk(Bin::kDecoratorRunFourCc, data.m_decorationRuns, stream);
    _writeArrayChunk(Bin::kExternalOperandsFourCc, data.m_externalOperands, stream);
    _writeArrayChunk(Bin::kStringFourCc, data.m_strings, stream);
    
    return SLANG_OK;
}

Result serializeModule(IRModule* module, Stream* stream)
{
   IRSerialWriter serializer;
   IRSerialData serialData;

   SLANG_RETURN_ON_FAIL(serializer.write(module, &serialData));

   if (stream)
   {
        SLANG_RETURN_ON_FAIL(IRSerialWriter::writeStream(serialData, stream));
    }

    return SLANG_OK;
}

Result readModule(Stream* stream)
{
    IRSerialData serialData;
    IRSerialReader::readStream(stream, &serialData);

    return SLANG_OK;
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! IRSerialReader !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

template <typename T>
Result _readArrayChunk(const IRSerialBinary::Chunk& chunk, Stream* stream, List<T>& arrayOut)
{
    typedef IRSerialBinary Bin;

    Bin::ArrayHeader header;
    header.m_chunk = chunk;

    stream->Read(&header.m_chunk + 1, sizeof(header) - sizeof(Bin::Chunk));

    size_t payloadSize = sizeof(Bin::ArrayHeader) - sizeof(Bin::Chunk) + sizeof(T) * header.m_numEntries;
    if (payloadSize != header.m_chunk.m_size)
    {
        return SLANG_FAIL;
    }

    arrayOut.SetSize(header.m_numEntries);

    stream->Read(arrayOut.begin(), sizeof(T) * header.m_numEntries);

    // All chunks have sizes rounded to dword size
    if (payloadSize & 3)
    {
        const uint8_t pad[4] = { 0, 0, 0, 0 };
        // Pad outs
        int padSize = 4 - (payloadSize & 3);
        stream->Seek(SeekOrigin::Current, padSize);
    }

    return SLANG_OK;
}

int64_t _calcChunkTotalSize(const IRSerialBinary::Chunk& chunk)
{
    int64_t size = chunk.m_size + sizeof(IRSerialBinary::Chunk);
    return (size + 3) & ~int64_t(3);
}

/* static */Result IRSerialReader::readStream(Stream* stream, IRSerialData* dataOut)
{
    typedef IRSerialBinary Bin;

    dataOut->clear();

    int64_t remainingBytes = 0;
    {
        Bin::Chunk header;
        stream->Read(&header, sizeof(header));
        if (header.m_type != Bin::kRiffFourCc)
        {
            return SLANG_FAIL;
        }

        remainingBytes = header.m_size;
    }
    
    while (remainingBytes > 0)
    {
        Bin::Chunk chunk;

        stream->Read(&chunk, sizeof(chunk));

        switch (chunk.m_type)
        {
            case Bin::kSlangFourCc:
            {
                // Slang header
                Bin::SlangHeader header;
                header.m_chunk = chunk;

                // NOTE! Really we should only read what we know the size to be...
                // and skip if it's larger

                stream->Read(&header.m_chunk + 1, sizeof(header) - sizeof(chunk));

                dataOut->m_decorationBaseIndex = header.m_decorationBase;

                remainingBytes -= _calcChunkTotalSize(chunk);
                break;
            }
            case Bin::kInstFourCc:
            {
                SLANG_RETURN_ON_FAIL(_readArrayChunk(chunk, stream, dataOut->m_insts));
                remainingBytes -= _calcChunkTotalSize(chunk);
                break;    
            }
            case Bin::kDecoratorRunFourCc:
            {
                SLANG_RETURN_ON_FAIL(_readArrayChunk(chunk, stream, dataOut->m_decorationRuns));
                remainingBytes -= _calcChunkTotalSize(chunk);
                break;
            }
            case Bin::kChildRunFourCc:
            {
                SLANG_RETURN_ON_FAIL(_readArrayChunk(chunk, stream, dataOut->m_childRuns));
                remainingBytes -= _calcChunkTotalSize(chunk);
                break;
            }
            case Bin::kExternalOperandsFourCc:
            {
                SLANG_RETURN_ON_FAIL(_readArrayChunk(chunk, stream, dataOut->m_externalOperands));
                remainingBytes -= _calcChunkTotalSize(chunk);
                break;
            }
            case Bin::kStringFourCc:
            {
                SLANG_RETURN_ON_FAIL(_readArrayChunk(chunk, stream, dataOut->m_strings));
                remainingBytes -= _calcChunkTotalSize(chunk);
                break;
            }
            default:
            {
                remainingBytes -= _calcChunkTotalSize(chunk);

                // Unhandled chunk... skip it
                int skipSize = (chunk.m_size + 3) & ~3;   
                stream->Seek(SeekOrigin::Current, skipSize);
                break;
            }
        }
    }

    return SLANG_OK;
}

} // namespace Slang
