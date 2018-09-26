// ir-serialize.cpp
#include "ir-serialize.h"

#include "../core/text-io.h"

#include "ir-insts.h"

namespace Slang {

/* Note that an IRInst can be derived from, but when it derived from it's new members are IRUse variables, and they in 
effect alias over the operands - and reflected in the operand count. There _could_ be other members after these IRUse 
variables, but in practice there do not appear to be.

The only difference to this is IRParentInst derived types, as it contains IRInstListBase children. Thus IRParentInst derived classes can 
have no operands - because it would write over the top of IRInstListBase.  BUT they can contain members after the list 
types which do this are

* IRModuleInst      - Presumably we can just set to the module pointer on reconstruction
* IRGlobalValue     - There are types derived from this type, but they don't add a parameter

Note! That on an IRInst there is an IRType* variable (accessed as getFullType()). As it stands it may NOT actually point 
to an IRType derived type. Its 'ok' as long as it's an instruction that can be used in the place of the type. So this code does not 
bother to check if it's correct, and just casts it.
*/

static bool isParentDerived(IROp opIn)
{
    const int op = (kIROpMeta_PseudoOpMask & opIn);
    return op >= kIROp_FirstParentInst && op <= kIROp_LastParentInst;
}

static bool isGlobalValueDerived(IROp opIn)
{
    const int op = (kIROpMeta_PseudoOpMask & opIn);
    return op >= kIROp_FirstGlobalValue && op <= kIROp_LastGlobalValue;
}

static bool isTextureTypeBase(IROp opIn)
{
    const int op = (kIROpMeta_PseudoOpMask & opIn);
    return op >= kIROp_FirstTextureTypeBase && op <= kIROp_LastTextureTypeBase;
}

static bool isConstant(IROp opIn)
{
    const int op = (kIROpMeta_PseudoOpMask & opIn);
    return op >= kIROp_FirstConstant && op <= kIROp_LastConstant;
}

struct PrefixString;

namespace { // anonymous

struct CharReader
{
    char operator()(int pos) const { SLANG_UNUSED(pos); return *m_pos++; }
    CharReader(const char* pos) :m_pos(pos) {}
    mutable const char* m_pos;
};

} // anonymous

static UnownedStringSlice asStringSlice(const PrefixString* prefixString)
{
    const char* prefix = (char*)prefixString;

    CharReader reader(prefix);
    const int len = GetUnicodePointFromUTF8(reader);
    return UnownedStringSlice(reader.m_pos, len);
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! IRSerialInfo !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


UnownedStringSlice IRSerialData::getStringSlice(StringIndex index) const
{
    if (index == StringIndex(0))
    {
        return UnownedStringSlice(m_strings.begin() + 1, UInt(0));
    }
    return asStringSlice((const PrefixString*)(m_strings.begin() + int(index)));
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
        // cos we want breadth first so the order of children is the same as their index order, meaning we don't need to store explicit indices
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
            
            // Can't be any psuedo ops
            SLANG_ASSERT(!isPseudoOp(srcInst->op)); 

            dstInst.m_op = uint8_t(srcInst->op & kIROpMeta_OpMask);
            dstInst.m_payloadType = Ser::Inst::PayloadType::Empty;
            
            dstInst.m_resultTypeIndex = getInstIndex(srcInst->getFullType());

            IRConstant* irConst = as<IRConstant>(srcInst);
            if (irConst)
            {
                switch (srcInst->op)
                {
                    // Special handling for the ir const derived types
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
                        dstInst.m_payloadType = Ser::Inst::PayloadType::UInt32;
                        dstInst.m_payload.m_uint32 = irConst->value.intVal ? 1 : 0;
                        break;
                    }
                    default:
                    {
                        SLANG_RELEASE_ASSERT(!"Unhandled constant type");
                        return SLANG_FAIL;
                    }
                }
                continue;
            }
            IRGlobalValue* globValue = as<IRGlobalValue>(srcInst);
            if (globValue)
            {
                dstInst.m_payloadType = Ser::Inst::PayloadType::String_1;
                dstInst.m_payload.m_stringIndices[0] = getStringIndex(globValue->mangledName);
                continue;
            }

            IRTextureTypeBase* textureBase = as<IRTextureTypeBase>(srcInst);
            if (textureBase)
            {
                dstInst.m_payloadType = Ser::Inst::PayloadType::UInt32;
                dstInst.m_payload.m_uint32 = uint32_t(srcInst->op) >> kIROpMeta_OtherShift;
                continue;
            }

            // ModuleInst is different, in so far as it holds a pointer to IRModule, but we don't need 
            // to save that off in a special way, so can just use regular path
             
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

Name* IRSerialReader::getName(Ser::StringIndex index)
{
    if (index == Ser::kNullStringIndex)
    {
        return nullptr;
    }

    StringRepresentation* rep = getStringRepresentation(index);
    String string(rep);
    Session* session = m_module->getSession();

    return session->getNameObj(string);
}

String IRSerialReader::getString(Ser::StringIndex index)
{
    return String(getStringRepresentation(index));
}

UnownedStringSlice IRSerialReader::getStringSlice(Ser::StringIndex index)
{
    return asStringSlice((PrefixString*)(m_serialData->m_strings.begin() + int(index)));
}

StringRepresentation* IRSerialReader::getStringRepresentation(Ser::StringIndex index)
{
    if (index == Ser::kNullStringIndex)
    {
        return nullptr;
    }

    const int linearIndex = findStringLinearIndex(index);
    StringRepresentation* rep = m_stringRepresentationCache[linearIndex];
    if (rep)
    {
        return rep;
    }

    const UnownedStringSlice slice = getStringSlice(index);
    String string(slice);

    StringRepresentation* stringRep = string.getStringRepresentation();   
    m_module->addRefObjectToFree(stringRep);

    m_stringRepresentationCache[linearIndex] = stringRep;

    return stringRep;
}

char* IRSerialReader::getCStr(Ser::StringIndex index)
{
    // It turns out StringRepresentation is always 0 terminated, so can just use that
    StringRepresentation* rep = getStringRepresentation(index);
    return rep->getData();
}

int IRSerialReader::findStringLinearIndex(Ser::StringIndex index)
{
    // Binary chop to find the linear index
    int start = 0;
    int end = int(m_stringStarts.Count());

    while (start < end)
    {
        int center = (start + end) >> 1;
        const Ser::StringIndex cur = m_stringStarts[center];

        if (cur == index)
        {
            return center;
        }

        if (cur < index)
        {
            start = center + 1;
        }
        else 
        {
            end = center;
        }
    }

    return -1;
}

void IRSerialReader::_calcStringStarts()
{
    m_stringStarts.Clear();

    const char* start = m_serialData->m_strings.begin();
    const char* cur = start;
    const char* end = m_serialData->m_strings.end();

    while (cur < end)
    {
        m_stringStarts.Add(Ser::StringIndex(cur - start));

        CharReader reader(cur);
        const int len = GetUnicodePointFromUTF8(reader);
        cur = reader.m_pos + len;
    }

    m_stringRepresentationCache.Clear();
    // Resize cache
    m_stringRepresentationCache.SetSize(m_stringStarts.Count());
    // Make sure all values are null initially
    memset(m_stringRepresentationCache.begin(), 0, sizeof(StringRepresentation*) * m_stringStarts.Count());
}

/* static */Result IRSerialReader::read(const IRSerialData& data, TranslationUnitRequest* translationUnit, IRModule** moduleOut)
{
    typedef Ser::Inst::PayloadType PayloadType;

    *moduleOut = nullptr;

    m_serialData = &data;
    _calcStringStarts();

    auto compileRequest = translationUnit->compileRequest;

    //SharedIRGenContext sharedContextStorage;
    //SharedIRGenContext* sharedContext = &sharedContextStorage;

    //sharedContext->compileRequest = compileRequest;
    //sharedContext->mainModuleDecl = translationUnit->SyntaxNode;

    //IRGenContext contextStorage(sharedContext);
    //IRGenContext* context = &contextStorage;

    SharedIRBuilder sharedBuilderStorage;
    SharedIRBuilder* sharedBuilder = &sharedBuilderStorage;
    sharedBuilder->module = nullptr;
    sharedBuilder->session = compileRequest->mSession;

    IRBuilder builderStorage;
    IRBuilder* builder = &builderStorage;
    builder->sharedBuilder = sharedBuilder;

    RefPtr<IRModule> module = builder->createModule();
    sharedBuilder->module = module;

    m_module = module;
    
    //context->irBuilder = builder;

    // Add all the instructions

    List<IRInst*> insts;
    List<IRDecoration*> decorations;

    const int numInsts = data.m_decorationBaseIndex;
    const int numDecorations = int(data.m_insts.Count() - numInsts);

    SLANG_ASSERT(numInsts > 0);

    insts.SetSize(numInsts);
    insts[0] = nullptr;

    decorations.SetSize(numDecorations);

    // 0 holds null
    // The first instruction must be the module
    {
        const Ser::Inst& srcInst = data.m_insts[1];
        SLANG_ASSERT(srcInst.m_op == kIROp_Module);
        SLANG_ASSERT(srcInst.getNumOperands() == 0);
        SLANG_ASSERT(srcInst.m_payloadType == PayloadType::Empty);

        // We need to create directly, because it has extra data
        IRModuleInst* moduleInst = static_cast<IRModuleInst*>(createEmptyInstWithSize(module, kIROp_Module, sizeof(IRModuleInst)));
        
        moduleInst->module = module;
        module->moduleInst = moduleInst;

        insts[1] = moduleInst;
    }

    for (int i = 2; i < numInsts; ++i)
    {
        const Ser::Inst& srcInst = data.m_insts[i];

        const IROp op((IROp)srcInst.m_op);

        if (isParentDerived(op))
        {
            // Cannot have operands
            SLANG_ASSERT(srcInst.getNumOperands() == 0);

            if (isGlobalValueDerived(op))
            {
                IRGlobalValue* globalValueInst = static_cast<IRGlobalValue*>(createEmptyInstWithSize(module, op, sizeof(IRGlobalValue)));
                insts[i] = globalValueInst;
                // Set the global value
                SLANG_ASSERT(srcInst.m_payloadType == PayloadType::String_1);
                globalValueInst->mangledName = getName(srcInst.m_payload.m_stringIndices[0]);
            }
            else
            {
                // Just needs to big enough to hold IRParentInst
                IRParentInst* parentInst = static_cast<IRParentInst*>(createEmptyInstWithSize(module, op, sizeof(IRParentInst)));
                insts[i] = parentInst;
            }
        }
        else
        {
            if (isConstant(op))
            {
                // Handling of constants

                // Calculate the minimum object size (ie not including the payload of value)    
                const size_t prefixSize = offsetof(IRConstant, value);

                IRConstant* irConst = nullptr;
                switch (op)
                {                    
                    case kIROp_boolConst:
                    {
                        SLANG_ASSERT(srcInst.m_payloadType == PayloadType::UInt32);
                        irConst = static_cast<IRConstant*>(createEmptyInstWithSize(module, op, prefixSize + sizeof(IRIntegerValue)));
                        irConst->value.intVal = srcInst.m_payload.m_uint32 != 0;
                        break;
                    }
                    case kIROp_IntLit:
                    {
                        SLANG_ASSERT(srcInst.m_payloadType == PayloadType::Int64);
                        irConst = static_cast<IRConstant*>(createEmptyInstWithSize(module, op, prefixSize + sizeof(IRIntegerValue)));
                        irConst->value.intVal = srcInst.m_payload.m_int64; 
                        break;
                    }
                    case kIROp_FloatLit:
                    {
                        SLANG_ASSERT(srcInst.m_payloadType == PayloadType::Float64);
                        irConst = static_cast<IRConstant*>(createEmptyInstWithSize(module, op,  prefixSize + sizeof(IRFloatingPointValue)));
                        irConst->value.floatVal = srcInst.m_payload.m_float64;
                        break;
                    }
                    case kIROp_StringLit:
                    {
                        SLANG_ASSERT(srcInst.m_payloadType == PayloadType::String_1);

                        const UnownedStringSlice slice = getStringSlice(srcInst.m_payload.m_stringIndices[0]);
                        
                        const size_t sliceSize = slice.size();
                        const size_t instSize = prefixSize + offsetof(IRConstant::StringValue, chars) + sliceSize;

                        irConst = static_cast<IRConstant*>(createEmptyInstWithSize(module, op, instSize));

                        IRConstant::StringValue& dstString = irConst->value.stringVal;

                        dstString.numChars = uint32_t(sliceSize);
                        // Turn into pointer to avoid warning of array overrun
                        char* dstChars = dstString.chars;
                        // Copy the chars
                        memcpy(dstChars, slice.begin(), sliceSize);
                        break;
                    }
                    default:
                    {
                        SLANG_ASSERT(!"Unknown constant type");
                        return SLANG_FAIL;
                    }
                }

                insts[i] = irConst;
            }
            else if (isTextureTypeBase(op))
            {
                IRTextureTypeBase* inst = static_cast<IRTextureTypeBase*>(createEmptyInstWithSize(module, op, sizeof(IRTextureTypeBase)));
                SLANG_ASSERT(srcInst.m_payloadType == PayloadType::UInt32);

                // Reintroduce the texture type bits into the the
                const uint32_t other = srcInst.m_payload.m_uint32;
                inst->op = IROp(uint32_t(inst->op) | (other << kIROpMeta_OtherShift));

                insts[i] = inst;
            }
            else
            {
                int numOperands = srcInst.getNumOperands();
                insts[i] = createEmptyInst(module, op, numOperands);
            }
        }                    
    } 

    // Patch up the operands
    
    for (int i = 1; i < numInsts; ++i)
    {
        const Ser::Inst& srcInst = data.m_insts[i];
        const IROp op((IROp)srcInst.m_op);

        IRInst* dstInst = insts[i];

        // Set the result type
        if (srcInst.m_resultTypeIndex != Ser::InstIndex(0))
        {
            IRInst* resultInst = insts[int(srcInst.m_resultTypeIndex)];
            // NOTE! Counter intuitively the IRType* paramter may not be IRType* derived for example 
            // IRGlobalGenericParam is valid, but isn't IRType* derived

            //SLANG_RELEASE_ASSERT(as<IRType>(resultInst));
            dstInst->setFullType(static_cast<IRType*>(resultInst));
        }
       
        //if (!isParentDerived(op))
        {
            const Ser::InstIndex* srcOperandIndices;
            const int numOperands = data.getOperands(srcInst, &srcOperandIndices);
                         
            for (int j = 0; j < numOperands; j++)
            {
                dstInst->setOperand(j, insts[int(srcOperandIndices[j])]);
            }
        }
    }
    
    // Patch up the children

    {
        const int numChildRuns = int(data.m_childRuns.Count());
        for (int i = 0; i < numChildRuns; i++)
        {
            const auto& run = data.m_childRuns[i];

            IRInst* inst = insts[int(run.m_parentIndex)];
            IRParentInst* parentInst = as<IRParentInst>(inst);
            SLANG_ASSERT(parentInst);

            for (int j = 0; j < int(run.m_numChildren); ++j)
            {
                IRInst* child = insts[j + int(run.m_startInstIndex)];
                SLANG_ASSERT(child->parent == nullptr);
                //child->parent = parentInst;
                child->insertAtEnd(parentInst);
            }
        }
    }

    // Add the decorations
    for (int i = 0; i < numDecorations; ++i)
    {
        const Ser::Inst& srcInst = data.m_insts[i + numInsts];
        IRDecorationOp decorOp = IRDecorationOp(srcInst.m_op - kIROpCount);
        SLANG_ASSERT(decorOp < kIRDecorationOp_CountOf);

        switch (decorOp)
        {
            case kIRDecorationOp_HighLevelDecl:
            {
                auto decor = createEmptyDecoration<IRHighLevelDeclDecoration>(m_module);
                decorations[i] = decor;

                // TODO!
                // Decl* decl;
                break;
            }
            case kIRDecorationOp_Layout:
            {
                auto decor = createEmptyDecoration<IRLayoutDecoration>(m_module);
                decorations[i] = decor;

                // TODO!
                // Layout* layout;
                break;
            }
            case kIRDecorationOp_LoopControl:
            {
                auto decor = createEmptyDecoration<IRLoopControlDecoration>(m_module);
                decorations[i] = decor;

                SLANG_ASSERT(srcInst.m_payloadType == PayloadType::UInt32);
                decor->mode = IRLoopControl(srcInst.m_payload.m_uint32);
                
                break;
            }
            case kIRDecorationOp_Target:
            {
                auto decor = createEmptyDecoration<IRTargetDecoration>(m_module);
                decorations[i] = decor;

                SLANG_ASSERT(srcInst.m_payloadType == PayloadType::String_1);
                decor->targetName = getStringRepresentation(srcInst.m_payload.m_stringIndices[0]);
                break;
            }
            case kIRDecorationOp_TargetIntrinsic:
            {
                auto decor = createEmptyDecoration<IRTargetIntrinsicDecoration>(m_module);
                decorations[i] = decor;

                SLANG_ASSERT(srcInst.m_payloadType == PayloadType::String_2);
                decor->targetName = getStringRepresentation(srcInst.m_payload.m_stringIndices[0]);
                decor->definition = getStringRepresentation(srcInst.m_payload.m_stringIndices[1]);
                break;
            }
            case kIRDecorationOp_GLSLOuterArray:
            {
                auto decor = createEmptyDecoration<IRGLSLOuterArrayDecoration>(m_module);
                decorations[i] = decor;

                SLANG_ASSERT(srcInst.m_payloadType == PayloadType::String_1);
                decor->outerArrayName = getCStr(srcInst.m_payload.m_stringIndices[0]);
                break;
            }
            case kIRDecorationOp_Semantic:
            {
                auto decor = createEmptyDecoration<IRSemanticDecoration>(m_module);
                decorations[i] = decor;

                SLANG_ASSERT(srcInst.m_payloadType == PayloadType::String_1);
                decor->semanticName = getName(srcInst.m_payload.m_stringIndices[0]);
                break;
            }
            case kIRDecorationOp_NameHint:
            {
                auto decor = createEmptyDecoration<IRNameHintDecoration>(m_module);
                decorations[i] = decor;

                SLANG_ASSERT(srcInst.m_payloadType == PayloadType::String_1);
                decor->name = getName(srcInst.m_payload.m_stringIndices[0]);
                break;
            }
            default:
            {
                SLANG_ASSERT(!"Unhandled decoration type");
                return SLANG_FAIL;
            }
        }
        
        // Make sure something is set
        SLANG_ASSERT(decorations[i]);
    }

    // Associate the decorations with the instructions

    {
        const int decorationBaseIndex = m_serialData->m_decorationBaseIndex;

        const int numRuns = int(m_serialData->m_decorationRuns.Count());
        for (int i = 0; i < numRuns; ++i)
        {
            const Ser::InstRun& run = m_serialData->m_decorationRuns[i];

            // Decorations must be associated with instructions
            SLANG_ASSERT(int(run.m_parentIndex) < decorationBaseIndex);

            IRInst* inst = insts[int(run.m_parentIndex)];
            SLANG_ASSERT(int(run.m_startInstIndex) >= decorationBaseIndex && int(run.m_startInstIndex) + run.m_numChildren <= m_serialData->m_insts.Count());

            // Go in reverse order so that linked list is in same order as original
            for (int j = int(run.m_numChildren) - 1; j >= 0; --j)
            {
                IRDecoration* decor = decorations[int(run.m_startInstIndex) + j - decorationBaseIndex];

                // And to the linked list on the 
                decor->next = inst->firstDecoration;
                inst->firstDecoration = decor;
            }
        }
    }

    *moduleOut = module.detach();
    return SLANG_OK;
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!! Free functions !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


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

Result readModule(TranslationUnitRequest* translationUnit, Stream* stream, IRModule** moduleOut)
{
    *moduleOut = nullptr;

    IRSerialData serialData;
    IRSerialReader::readStream(stream, &serialData);

    RefPtr<IRModule> module;
    IRSerialReader reader;

    SLANG_RETURN_ON_FAIL(reader.read(serialData, translationUnit, module.writeRef()));

    *moduleOut = module.detach();

    return SLANG_OK;
}


} // namespace Slang
