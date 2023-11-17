// slang-serialize-ir.cpp
#include "slang-serialize-ir.h"

#include "../core/slang-text-io.h"
#include "../core/slang-byte-encode-util.h"

#include "slang-ir-insts.h"

#include "../core/slang-math.h"

namespace Slang {

static bool _isConstant(IROp opIn)
{
    const int op = (kIROpMask_OpMask & opIn);
    return op >= kIROp_FirstConstant && op <= kIROp_LastConstant;
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! IRSerialWriter !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

void IRSerialWriter::_addInstruction(IRInst* inst)
{
    // It cannot already be in the map
    SLANG_ASSERT(!m_instMap.containsKey(inst));

    // Add to the map
    m_instMap.add(inst, Ser::InstIndex(m_insts.getCount()));
    m_insts.add(inst);
}

Result IRSerialWriter::_calcDebugInfo(SerialSourceLocWriter* sourceLocWriter)
{
    // We need to find the unique source Locs
    // We are not going to store SourceLocs directly, because there may be multiple views mapping down to
    // the same underlying source file

    // First find all the unique locs
    struct InstLoc
    {
        typedef InstLoc ThisType;

        SLANG_FORCE_INLINE bool operator<(const ThisType& rhs) const { return sourceLoc < rhs.sourceLoc || (sourceLoc == rhs.sourceLoc && instIndex < rhs.instIndex); }

        uint32_t instIndex;
        uint32_t sourceLoc;
    };

    // Find all of the source locations and their associated instructions
    List<InstLoc> instLocs;
    const Index numInsts = m_insts.getCount();
    for (Index i = 1; i < numInsts; i++)
    {
        IRInst* srcInst = m_insts[i];
        if (!srcInst->sourceLoc.isValid())
        {
            continue;
        }
        InstLoc instLoc;
        instLoc.instIndex = uint32_t(i);
        instLoc.sourceLoc = uint32_t(srcInst->sourceLoc.getRaw());
        instLocs.add(instLoc);
    }

    // Sort them
    instLocs.sort();
    
    // Look for runs
    const InstLoc* startInstLoc = instLocs.begin();
    const InstLoc* endInstLoc = instLocs.end();

    while (startInstLoc < endInstLoc)
    {
        const uint32_t startSourceLoc = startInstLoc->sourceLoc;
        
        // Find the run with the same source loc

        const InstLoc* curInstLoc = startInstLoc + 1;
        uint32_t curInstIndex = startInstLoc->instIndex + 1;

        // Find the run size with same source loc and run of instruction indices
        for (; curInstLoc < endInstLoc && curInstLoc->sourceLoc == startSourceLoc && curInstLoc->instIndex == curInstIndex; ++curInstLoc, ++curInstIndex)
        {
        }

        // Add the run

        IRSerialData::SourceLocRun sourceLocRun;
        sourceLocRun.m_numInst = curInstIndex - startInstLoc->instIndex;;
        sourceLocRun.m_startInstIndex = IRSerialData::InstIndex(startInstLoc->instIndex);
        sourceLocRun.m_sourceLoc = sourceLocWriter->addSourceLoc(SourceLoc::fromRaw(startSourceLoc));

        m_serialData->m_debugSourceLocRuns.add(sourceLocRun);

        // Next
        startInstLoc = curInstLoc;
    }

    return SLANG_OK;
}

Result IRSerialWriter::write(IRModule* module, SerialSourceLocWriter* sourceLocWriter, SerialOptionFlags options, IRSerialData* serialData)
{
    typedef Ser::Inst::PayloadType PayloadType;

    m_serialData = serialData;

    serialData->clear();

    // We reserve 0 for null
    m_insts.clear();
    m_insts.add(nullptr);

    // Reset
    m_instMap.clear();
    m_decorations.clear();
    
    // Stack for parentInst
    List<IRInst*> parentInstStack;
  
    IRModuleInst* moduleInst = module->getModuleInst();
    parentInstStack.add(moduleInst);

    // Add to the map
    _addInstruction(moduleInst);

    // Traverse all of the instructions
    while (parentInstStack.getCount())
    {
        // If it's in the stack it is assumed it is already in the inst map
        IRInst* parentInst = parentInstStack.getLast();
        parentInstStack.removeLast();
        SLANG_ASSERT(m_instMap.containsKey(parentInst));

        // Okay we go through each of the children in order. If they are IRInstParent derived, we add to stack to process later 
        // cos we want breadth first so the order of children is the same as their index order, meaning we don't need to store explicit indices
        const Ser::InstIndex startChildInstIndex = Ser::InstIndex(m_insts.getCount());
        
        IRInstListBase childrenList = parentInst->getDecorationsAndChildren();
        for (IRInst* child : childrenList)
        {
            // This instruction can't be in the map...
            SLANG_ASSERT(!m_instMap.containsKey(child));

            _addInstruction(child);
            
            parentInstStack.add(child);
        }

        // If it had any children, then store the information about it
        if (Ser::InstIndex(m_insts.getCount()) != startChildInstIndex)
        {
            Ser::InstRun run;
            run.m_parentIndex = m_instMap[parentInst];
            run.m_startInstIndex = startChildInstIndex;
            run.m_numChildren = Ser::SizeType(m_insts.getCount() - int(startChildInstIndex));

            m_serialData->m_childRuns.add(run);
        }
    }

#if 0
    {
        List<IRInst*> workInsts;
        calcInstructionList(module, workInsts);
        SLANG_ASSERT(workInsts.getCount() == m_insts.getCount());
        for (UInt i = 0; i < workInsts.getCount(); ++i)
        {
            SLANG_ASSERT(workInsts[i] == m_insts[i]);
        }
    }
#endif

    // Set to the right size
    m_serialData->m_insts.setCount(m_insts.getCount());
    // Clear all instructions
    memset(m_serialData->m_insts.begin(), 0, sizeof(Ser::Inst) * m_serialData->m_insts.getCount());

    // Need to set up the actual instructions
    {
        const Index numInsts = m_insts.getCount();

        for (Index i = 1; i < numInsts; ++i)
        {
            IRInst* srcInst = m_insts[i];
            Ser::Inst& dstInst = m_serialData->m_insts[i];

            dstInst.m_op = uint16_t(srcInst->getOp() & kIROpMask_OpMask);
            dstInst.m_payloadType = PayloadType::Empty;
            
            dstInst.m_resultTypeIndex = getInstIndex(srcInst->getFullType());

            IRConstant* irConst = as<IRConstant>(srcInst);
            if (irConst)
            {
                switch (srcInst->getOp())
                {
                    // Special handling for the ir const derived types
                    case kIROp_StringLit:
                    {
                        auto stringLit = static_cast<IRStringLit*>(srcInst);
                        dstInst.m_payloadType = PayloadType::String_1;
                        dstInst.m_payload.m_stringIndices[0] = getStringIndex(stringLit->getStringSlice());
                        break;
                    }
                    case kIROp_IntLit:
                    {
                        dstInst.m_payloadType = PayloadType::Int64;
                        dstInst.m_payload.m_int64 = irConst->value.intVal;
                        break;
                    }
                    case kIROp_PtrLit:
                    {
                        dstInst.m_payloadType = PayloadType::Int64;
                        dstInst.m_payload.m_int64 = (intptr_t) irConst->value.ptrVal;
                        break;
                    }
                    case kIROp_FloatLit:
                    {
                        dstInst.m_payloadType = PayloadType::Float64;
                        dstInst.m_payload.m_float64 = irConst->value.floatVal; 
                        break;
                    }
                    case kIROp_BoolLit:
                    {
                        dstInst.m_payloadType = PayloadType::UInt32;
                        dstInst.m_payload.m_uint32 = irConst->value.intVal ? 1 : 0;
                        break;
                    }
                    case kIROp_VoidLit:
                    {
                        dstInst.m_payloadType = PayloadType::Empty;
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

            // ModuleInst is different, in so far as it holds a pointer to IRModule, but we don't need 
            // to save that off in a special way, so can just use regular path
             
            const int numOperands = int(srcInst->operandCount);
            Ser::InstIndex* dstOperands = nullptr;

            if (numOperands <= Ser::Inst::kMaxOperands)
            {
                // Checks the compile below is valid
                SLANG_COMPILE_TIME_ASSERT(PayloadType(0) == PayloadType::Empty && PayloadType(1) == PayloadType::Operand_1 && PayloadType(2) == PayloadType::Operand_2);
                
                dstInst.m_payloadType = PayloadType(numOperands);
                dstOperands = dstInst.m_payload.m_operands;
            }
            else
            {
                dstInst.m_payloadType = PayloadType::OperandExternal;

                int operandArrayBaseIndex = int(m_serialData->m_externalOperands.getCount());
                m_serialData->m_externalOperands.setCount(operandArrayBaseIndex + numOperands);

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

    // Convert strings into a string table
    {
        SerialStringTableUtil::encodeStringTable(m_stringSlicePool, serialData->m_stringTable);
    }

    // If the option to use RawSourceLocations is enabled, serialize out as is
    if (options & SerialOptionFlag::RawSourceLocation)
    {
        const Index numInsts = m_insts.getCount();
        serialData->m_rawSourceLocs.setCount(numInsts);

        Ser::RawSourceLoc* dstLocs =  serialData->m_rawSourceLocs.begin();
        // 0 is null, just mark as no location
        dstLocs[0] = Ser::RawSourceLoc(0);
        for (Index i = 1; i < numInsts; ++i)
        {
            IRInst* srcInst = m_insts[i];
            dstLocs[i] = Ser::RawSourceLoc(srcInst->sourceLoc.getRaw());
        }
    }

    if ((options & SerialOptionFlag::SourceLocation) && sourceLocWriter)
    {
        _calcDebugInfo(sourceLocWriter);
    }

    m_serialData = nullptr;
    return SLANG_OK;
}

Result _encodeInsts(SerialCompressionType compressionType, const List<IRSerialData::Inst>& instsIn, List<uint8_t>& encodeArrayOut)
{
    typedef IRSerialData::Inst::PayloadType PayloadType;

    if (compressionType != SerialCompressionType::VariableByteLite)
    {
        return SLANG_FAIL;
    }

    encodeArrayOut.clear();
    
    const size_t numInsts = size_t(instsIn.getCount());
    const IRSerialData::Inst* insts = instsIn.begin();
        
    uint8_t* encodeOut = encodeArrayOut.begin();
    uint8_t* encodeEnd = encodeArrayOut.end();

    // Calculate the maximum instruction size with worst case possible encoding
    // 2 bytes hold the payload size, and the result type
    // Note that if there were some free bits, we could encode some of this stuff into bits, but if we remove payloadType, then there are no free bits
    const size_t maxInstSize = 1 + ByteEncodeUtil::kMaxLiteEncodeUInt16 + Math::Max(sizeof(insts->m_payload.m_float64), size_t(2 * ByteEncodeUtil::kMaxLiteEncodeUInt32));

    for (size_t i = 0; i < numInsts; ++i)
    {
        const auto& inst = insts[i];

        // Make sure there is space for the largest possible instruction
        if (encodeOut + maxInstSize >= encodeEnd)
        {
            const size_t offset = size_t(encodeOut - encodeArrayOut.begin());
            
            const UInt oldCapacity = encodeArrayOut.getCapacity();

            encodeArrayOut.reserve(oldCapacity + (oldCapacity >> 1) + maxInstSize);
            const UInt capacity = encodeArrayOut.getCapacity();
            encodeArrayOut.setCount(capacity);

            encodeOut = encodeArrayOut.begin() + offset;
            encodeEnd = encodeArrayOut.end();
        }
        encodeOut += ByteEncodeUtil::encodeLiteUInt32(inst.m_op, encodeOut);

        *encodeOut++ = uint8_t(inst.m_payloadType);

        encodeOut += ByteEncodeUtil::encodeLiteUInt32((uint32_t)inst.m_resultTypeIndex, encodeOut);

        switch (inst.m_payloadType)
        {
            case PayloadType::Empty:
            {
                break;
            }
            case PayloadType::Operand_1:
            case PayloadType::String_1:
            case PayloadType::UInt32:
            {
                // 1 UInt32
                encodeOut += ByteEncodeUtil::encodeLiteUInt32((uint32_t)inst.m_payload.m_operands[0], encodeOut);
                break;
            }
            case PayloadType::Operand_2:
            case PayloadType::OperandAndUInt32:
            case PayloadType::OperandExternal:
            case PayloadType::String_2:
            {
                // 2 UInt32
                encodeOut += ByteEncodeUtil::encodeLiteUInt32((uint32_t)inst.m_payload.m_operands[0], encodeOut);
                encodeOut += ByteEncodeUtil::encodeLiteUInt32((uint32_t)inst.m_payload.m_operands[1], encodeOut);
                break;
            }
            case PayloadType::Float64:
            {
                memcpy(encodeOut, &inst.m_payload.m_float64, sizeof(inst.m_payload.m_float64));
                encodeOut += sizeof(inst.m_payload.m_float64);
                break;
            }
            case PayloadType::Int64:
            {
                memcpy(encodeOut, &inst.m_payload.m_int64, sizeof(inst.m_payload.m_int64));
                encodeOut += sizeof(inst.m_payload.m_int64);
                break;
            }
        }
    }

    // Fix the size 
    encodeArrayOut.setCount(UInt(encodeOut - encodeArrayOut.begin()));
    return SLANG_OK;
}

Result _writeInstArrayChunk(SerialCompressionType compressionType, FourCC chunkId, const List<IRSerialData::Inst>& array, RiffContainer* container)
{
    typedef RiffContainer::Chunk Chunk;
    typedef RiffContainer::ScopeChunk ScopeChunk;

    if (array.getCount() == 0)
    {
        return SLANG_OK;
    }

    switch (compressionType)
    {
        case SerialCompressionType::None:
        {
            return SerialRiffUtil::writeArrayChunk(compressionType, chunkId, array, container);
        }
        case SerialCompressionType::VariableByteLite:
        {
            List<uint8_t> compressedPayload;
            SLANG_RETURN_ON_FAIL(_encodeInsts(compressionType, array, compressedPayload));

            ScopeChunk scope(container, Chunk::Kind::Data, SLANG_MAKE_COMPRESSED_FOUR_CC(chunkId));

            SerialBinary::CompressedArrayHeader header;
            header.numEntries = uint32_t(array.getCount());
            header.numCompressedEntries = 0;          

            container->write(&header, sizeof(header));
            container->write(compressedPayload.getBuffer(), compressedPayload.getCount());

            return SLANG_OK;
        }
        default: break;
    }
    return SLANG_FAIL;
}

/* static */Result IRSerialWriter::writeContainer(const IRSerialData& data, SerialCompressionType compressionType, RiffContainer* container)
{
    typedef RiffContainer::Chunk Chunk;
    typedef RiffContainer::ScopeChunk ScopeChunk;

    ScopeChunk scopeModule(container, Chunk::Kind::List, Bin::kIRModuleFourCc);

    SLANG_RETURN_ON_FAIL(_writeInstArrayChunk(compressionType, Bin::kInstFourCc, data.m_insts, container));
    SLANG_RETURN_ON_FAIL(SerialRiffUtil::writeArrayChunk(compressionType, Bin::kChildRunFourCc, data.m_childRuns, container));
    SLANG_RETURN_ON_FAIL(SerialRiffUtil::writeArrayChunk(compressionType, Bin::kExternalOperandsFourCc, data.m_externalOperands, container));
    SLANG_RETURN_ON_FAIL(SerialRiffUtil::writeArrayChunk(SerialCompressionType::None, SerialBinary::kStringTableFourCc, data.m_stringTable, container));

    SLANG_RETURN_ON_FAIL(SerialRiffUtil::writeArrayChunk(SerialCompressionType::None, Bin::kUInt32RawSourceLocFourCc, data.m_rawSourceLocs, container));

    if (data.m_debugSourceLocRuns.getCount())
    {
        SerialRiffUtil::writeArrayChunk(compressionType, Bin::kDebugSourceLocRunFourCc, data.m_debugSourceLocRuns, container);
    }

    return SLANG_OK;
}

/* static */void IRSerialWriter::calcInstructionList(IRModule* module, List<IRInst*>& instsOut)
{
    // We reserve 0 for null
    instsOut.setCount(1);
    instsOut[0] = nullptr;

    // Stack for parentInst
    List<IRInst*> parentInstStack;

    IRModuleInst* moduleInst = module->getModuleInst();
    parentInstStack.add(moduleInst);

    // Add to list
    instsOut.add(moduleInst);

    // Traverse all of the instructions
    while (parentInstStack.getCount())
    {
        // If it's in the stack it is assumed it is already in the inst map
        IRInst* parentInst = parentInstStack.getLast();
        parentInstStack.removeLast();

        IRInstListBase childrenList = parentInst->getDecorationsAndChildren();
        for (IRInst* child : childrenList)
        {
            instsOut.add(child);
            parentInstStack.add(child);
        }
    }
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! IRSerialReader !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

static Result _decodeInsts(SerialCompressionType compressionType, const uint8_t* encodeCur, size_t encodeInSize, List<IRSerialData::Inst>& instsOut)
{
    const uint8_t* encodeEnd = encodeCur + encodeInSize;

    typedef IRSerialData::Inst::PayloadType PayloadType;

    if (compressionType != SerialCompressionType::VariableByteLite)
    {
        return SLANG_FAIL;
    }

    const size_t numInsts = size_t(instsOut.getCount());
    IRSerialData::Inst* insts = instsOut.begin();

    for (size_t i = 0; i < numInsts; ++i)
    {
        if (encodeCur >= encodeEnd)
        {
            SLANG_ASSERT(!"Invalid decode");
            return SLANG_FAIL;
        }

        auto& inst = insts[i];
        uint32_t instOp = 0;
        encodeCur += ByteEncodeUtil::decodeLiteUInt32(encodeCur, &instOp);
        inst.m_op = (uint16_t)instOp;

        const PayloadType payloadType = PayloadType(*encodeCur++);
        inst.m_payloadType = payloadType;
        
        // Read the result value
        encodeCur += ByteEncodeUtil::decodeLiteUInt32(encodeCur, (uint32_t*)&inst.m_resultTypeIndex);

        switch (inst.m_payloadType)
        {
            case PayloadType::Empty:
            {
                break;
            }
            case PayloadType::Operand_1:
            case PayloadType::String_1:
            case PayloadType::UInt32:
            {
                // 1 UInt32
                encodeCur += ByteEncodeUtil::decodeLiteUInt32(encodeCur, (uint32_t*)&inst.m_payload.m_operands[0]);
                break;
            }
            case PayloadType::Operand_2:
            case PayloadType::OperandAndUInt32:
            case PayloadType::OperandExternal:
            case PayloadType::String_2:
            {
                // 2 UInt32
                encodeCur += ByteEncodeUtil::decodeLiteUInt32(encodeCur, 2, (uint32_t*)&inst.m_payload.m_operands[0]);
                break;
            }
            case PayloadType::Float64:
            {
                memcpy(&inst.m_payload.m_float64, encodeCur, sizeof(inst.m_payload.m_float64));
                encodeCur += sizeof(inst.m_payload.m_float64);
                break;
            }
            case PayloadType::Int64:
            {
                memcpy(&inst.m_payload.m_int64, encodeCur, sizeof(inst.m_payload.m_int64));
                encodeCur += sizeof(inst.m_payload.m_int64);
                break;
            }
        }
    }

    return SLANG_OK;
}

static Result _readInstArrayChunk(SerialCompressionType containerCompressionType, RiffContainer::DataChunk* chunk, List<IRSerialData::Inst>& arrayOut)
{
    SerialCompressionType compressionType = SerialCompressionType::None;
    if (chunk->m_fourCC == SLANG_MAKE_COMPRESSED_FOUR_CC(chunk->m_fourCC))
    {
        compressionType = SerialCompressionType(containerCompressionType);
    }

    switch (compressionType)
    {
        case SerialCompressionType::None:
        {
            SerialRiffUtil::ListResizerForType<IRSerialData::Inst> resizer(arrayOut);
            return SerialRiffUtil::readArrayChunk(compressionType, chunk, resizer);
        }
        case SerialCompressionType::VariableByteLite:
        {
            RiffReadHelper read = chunk->asReadHelper();

            SerialBinary::CompressedArrayHeader header;
            SLANG_RETURN_ON_FAIL(read.read(header));

            arrayOut.setCount(header.numEntries);

            SLANG_RETURN_ON_FAIL(_decodeInsts(compressionType, read.getData(), read.getRemainingSize(), arrayOut));
            break;
        }
        default:
        {
            return SLANG_FAIL;
        }
    }

    return SLANG_OK;
}

/* static */Result IRSerialReader::readContainer(RiffContainer::ListChunk* module, SerialCompressionType containerCompressionType, IRSerialData* outData)
{
    typedef IRSerialBinary Bin;

    outData->clear();

    for (RiffContainer::Chunk* chunk = module->m_containedChunks; chunk; chunk = chunk->m_next)
    {
        RiffContainer::DataChunk* dataChunk = as<RiffContainer::DataChunk>(chunk);
        if (!dataChunk)
        {
            continue;
        }
        
        switch (dataChunk->m_fourCC)
        {
            case SLANG_MAKE_COMPRESSED_FOUR_CC(Bin::kInstFourCc):
            case Bin::kInstFourCc:
            {
                SLANG_RETURN_ON_FAIL(_readInstArrayChunk(containerCompressionType, dataChunk, outData->m_insts));
                break;
            }
            case SLANG_MAKE_COMPRESSED_FOUR_CC(Bin::kChildRunFourCc):
            case Bin::kChildRunFourCc:
            {
                SLANG_RETURN_ON_FAIL(SerialRiffUtil::readArrayChunk(containerCompressionType, dataChunk, outData->m_childRuns));
                break;
            }
            case SLANG_MAKE_COMPRESSED_FOUR_CC(Bin::kExternalOperandsFourCc):
            case Bin::kExternalOperandsFourCc:
            {
                SLANG_RETURN_ON_FAIL(SerialRiffUtil::readArrayChunk(containerCompressionType, dataChunk, outData->m_externalOperands));
                break;
            }
            case SerialBinary::kStringTableFourCc:
            {
                SLANG_RETURN_ON_FAIL(SerialRiffUtil::readArrayUncompressedChunk(dataChunk, outData->m_stringTable));
                break;
            }
            case Bin::kUInt32RawSourceLocFourCc:
            {
                SLANG_RETURN_ON_FAIL(SerialRiffUtil::readArrayUncompressedChunk(dataChunk, outData->m_rawSourceLocs));
                break;
            }
            case SLANG_MAKE_COMPRESSED_FOUR_CC(Bin::kDebugSourceLocRunFourCc):
            case Bin::kDebugSourceLocRunFourCc:
            {
                SLANG_RETURN_ON_FAIL(SerialRiffUtil::readArrayChunk(containerCompressionType, dataChunk, outData->m_debugSourceLocRuns));
                break;
            }
            default:
            {
                break;
            }
        }
    }

    return SLANG_OK;
}

Result IRSerialReader::read(const IRSerialData& data, Session* session, SerialSourceLocReader* sourceLocReader, RefPtr<IRModule>& outModule)
{
    // Only used in debug builds
    [[maybe_unused]] typedef Ser::Inst::PayloadType PayloadType;

    m_serialData = &data;

    auto module = IRModule::create(session);
    outModule = module;
    m_module = module;

    // Convert m_stringTable into StringSlicePool.
    SerialStringTableUtil::decodeStringTable(data.m_stringTable.getBuffer(), data.m_stringTable.getCount(), m_stringTable);

    // Each IR instruction has:
    //
    // * An opcode
    // * Zero or more operands
    // * Zero or more children
    //
    // Most instructions are entirely defined by those properties.
    // 
    // The instructions that represent simple constants (integers, strings, etc.) are
    // unique in that they have "payload" data that holds their value, instead of having
    // any operands.
    //
    // The deserialization logic here is set up to handle an arbitrary configuration
    // of IR instructions, which means it can handle cases where:
    //
    // * An instruction earlier in the serialized stream might refer to an instruction
    //   later in the stream, as one of its operands or (transitive) children.
    //
    // * An instruction in the stream transitively depends on itself via operand
    //   and/or child relationships.
    //
    // In order to handle these cases, deserialization proceeds in multiple passes.
    // In the first pass, `IRInst`s are allocated for each instruction in the stream,
    // based on their memory requirements (number of operands in the ordinary case
    // and payload size in the case of simple constants). Subsequent passes then
    // fill in the operands and/or children.
    //
    // Note that as a result of the strategy used here, it is not possible for the
    // deserialization logic to interact with any systems for deduplication or
    // simplification of instructions. An alternative version of the deserializer that
    // uses the `IRBuilder` interface instead might be possible, but would need a
    // plan for how to handle forward and/or circular references in the IR module.

    // Add all the instructions
    List<IRInst*> insts;

    const Index numInsts = data.m_insts.getCount();

    SLANG_ASSERT(numInsts > 0);

    insts.setCount(numInsts);
    insts[0] = nullptr;

    // 0 holds null
    // 1 holds the IRModuleInst
    {
        // Check that insts[1] is the module inst
        const Ser::Inst& srcInst = data.m_insts[1];
        SLANG_RELEASE_ASSERT(srcInst.m_op == kIROp_Module);
        SLANG_ASSERT(srcInst.m_payloadType == PayloadType::Empty);

        // The root IR instruction for the module will already have
        // been created as part of creating `module` above.
        //
        auto moduleInst = module->getModuleInst();

        // Set the IRModuleInst
        insts[1] = moduleInst; 
    }

    for (Index i = 2; i < numInsts; ++i)
    {
        const Ser::Inst& srcInst = data.m_insts[i];

        const IROp op((IROp)srcInst.m_op);

        if (_isConstant(op))
        {
            // Handling of constants

            // Calculate the minimum object size (ie not including the payload of value)    
            const size_t prefixSize = SLANG_OFFSET_OF(IRConstant, value);

            // All IR constants have zero operands.
            Int operandCount = 0;

            IRConstant* irConst = nullptr;
            switch (op)
            {                    
                case kIROp_BoolLit:
                {
                    // TODO: Most of these cases could use the templated `_allocateInst<T>`
                    // *if* we had distinct `IRConstant` subtypes to represent these
                    // cases and their subtype-specific payloads.

                    SLANG_ASSERT(srcInst.m_payloadType == PayloadType::UInt32);
                    irConst = static_cast<IRConstant*>(module->_allocateInst(op, operandCount, prefixSize + sizeof(IRIntegerValue)));
                    irConst->value.intVal = srcInst.m_payload.m_uint32 != 0;
                    break;
                }
                case kIROp_IntLit:
                {
                    SLANG_ASSERT(srcInst.m_payloadType == PayloadType::Int64);
                    irConst = static_cast<IRConstant*>(module->_allocateInst(op, operandCount, prefixSize + sizeof(IRIntegerValue)));
                    irConst->value.intVal = srcInst.m_payload.m_int64; 
                    break;
                }
                case kIROp_PtrLit:
                {
                    SLANG_ASSERT(srcInst.m_payloadType == PayloadType::Int64);
                    irConst = static_cast<IRConstant*>(module->_allocateInst(op, operandCount, prefixSize + sizeof(void*)));
                    irConst->value.ptrVal = (void*) (intptr_t) srcInst.m_payload.m_int64; 
                    break;
                }
                case kIROp_FloatLit:
                {
                    SLANG_ASSERT(srcInst.m_payloadType == PayloadType::Float64);
                    irConst = static_cast<IRConstant*>(module->_allocateInst(op, operandCount,  prefixSize + sizeof(IRFloatingPointValue)));
                    irConst->value.floatVal = srcInst.m_payload.m_float64;
                    break;
                }
                case kIROp_VoidLit:
                {
                    SLANG_ASSERT(srcInst.m_payloadType == PayloadType::Empty);
                    irConst = static_cast<IRConstant*>(module->_allocateInst(
                        op, operandCount, prefixSize));
                    break;
                }
                case kIROp_StringLit:
                {
                    SLANG_ASSERT(srcInst.m_payloadType == PayloadType::String_1);

                    const UnownedStringSlice slice = m_stringTable.getSlice(StringSlicePool::Handle(srcInst.m_payload.m_stringIndices[0]));
                        
                    const size_t sliceSize = slice.getLength();
                    const size_t instSize = prefixSize + SLANG_OFFSET_OF(IRConstant::StringValue, chars) + sliceSize;

                    irConst = static_cast<IRConstant*>(module->_allocateInst(op, operandCount, instSize));

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
        else
        {
            int numOperands = srcInst.getNumOperands();
            insts[i] = module->_allocateInst(op, numOperands);
        }
    }

    // Patch up the operands
    for (Index i = 1; i < numInsts; ++i)
    {
        const Ser::Inst& srcInst = data.m_insts[i];
        const IROp op((IROp)srcInst.m_op);
        SLANG_UNUSED(op);

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

            auto dstOperands = dstInst->getOperands();

            for (int j = 0; j < numOperands; j++)
            {
                dstOperands[j].init(dstInst, insts[int(srcOperandIndices[j])]);
            }
        }
    }
    
    // Patch up the children
    {
        const Index numChildRuns = data.m_childRuns.getCount();
        for (Index i = 0; i < numChildRuns; i++)
        {
            const auto& run = data.m_childRuns[i];

            IRInst* inst = insts[int(run.m_parentIndex)];

            for (int j = 0; j < int(run.m_numChildren); ++j)
            {
                IRInst* child = insts[j + int(run.m_startInstIndex)];
                SLANG_ASSERT(child->parent == nullptr);
                child->insertAtEnd(inst);
            }
        }
    }

    // Re-add source locations, if they are defined
    if (m_serialData->m_rawSourceLocs.getCount() == numInsts)
    {
        const Ser::RawSourceLoc* srcLocs = m_serialData->m_rawSourceLocs.begin();
        for (Index i = 1; i < numInsts; ++i)
        {
            IRInst* dstInst = insts[i];
            
            dstInst->sourceLoc.setRaw(Slang::SourceLoc::RawValue(srcLocs[i]));
        }
    }

    // We now need to apply the runs
    if (sourceLocReader && m_serialData->m_debugSourceLocRuns.getCount())
    {
        List<IRSerialData::SourceLocRun> sourceRuns(m_serialData->m_debugSourceLocRuns);
        // They are now in source location order
        sourceRuns.sort();

        // Just guess initially 0 for the source file that contains the initial run
        SerialSourceLocData::SourceRange range = SerialSourceLocData::SourceRange::getInvalid();
        int fix = 0;
        
        const Index numRuns = sourceRuns.getCount();
        for (Index i = 0; i < numRuns; ++i)
        {
            const auto& run = sourceRuns[i];

            // Work out the fixed source location
            SourceLoc sourceLoc;
            if (run.m_sourceLoc)
            {
                if (!range.contains(run.m_sourceLoc))
                {
                    fix = sourceLocReader->calcFixSourceLoc(run.m_sourceLoc, range);
                }
                sourceLoc = sourceLocReader->calcFixedLoc(run.m_sourceLoc, fix, range);
            }

            // Write to all the instructions
            SLANG_ASSERT(Index(uint32_t(run.m_startInstIndex) + run.m_numInst) <= insts.getCount());
            IRInst** dstInsts = insts.getBuffer() + int(run.m_startInstIndex);

            const int runSize = int(run.m_numInst);
            for (int j = 0; j < runSize; ++j)
            {
                dstInsts[j]->sourceLoc = sourceLoc;
            }
        }
    }

    return SLANG_OK;
}

} // namespace Slang
