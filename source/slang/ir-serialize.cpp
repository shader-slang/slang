// ir-serialize.cpp
#include "ir-serialize.h"

#include "../core/text-io.h"
#include "../core/slang-byte-encode-util.h"

#include "ir-insts.h"

#include "../core/slang-math.h"

namespace Slang {

// Needed for linkage with some compilers
/* static */ const IRSerialData::StringIndex IRSerialData::kNullStringIndex;
/* static */ const IRSerialData::StringIndex IRSerialData::kEmptyStringIndex;

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

/* static */const IRSerialData::PayloadInfo IRSerialData::s_payloadInfos[int(Inst::PayloadType::CountOf)] = 
{
    { 0, 0 },   // Empty
    { 1, 0 },   // Operand_1
    { 2, 0 },   // Operand_2
    { 1, 0 },   // OperandAndUInt32,
    { 0, 0 },   // OperandExternal - This isn't correct, Operand has to be specially handled
    { 0, 1 },   // String_1,              
    { 0, 2 },   // String_2,              
    { 0, 0 },   // UInt32,               
    { 0, 0 },   // Float64,
    { 0, 0 }    // Int64,
};

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

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! StringRepresentationCache !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

StringRepresentationCache::StringRepresentationCache():
    m_stringTable(nullptr),
    m_namePool(nullptr),
    m_scopeManager(nullptr)
{
}

void StringRepresentationCache::init(const List<char>* stringTable, NamePool* namePool, ObjectScopeManager* scopeManager)
{
    m_stringTable = stringTable;
    m_namePool = namePool;
    m_scopeManager = scopeManager;

    // Decode the table
    m_entries.SetSize(StringSlicePool::kNumDefaultHandles);
    SLANG_COMPILE_TIME_ASSERT(StringSlicePool::kNumDefaultHandles == 2);

    {
        Entry& entry = m_entries[0];
        entry.m_numChars = 0;
        entry.m_startIndex = 0;
        entry.m_object = nullptr;
    }
    {
        Entry& entry = m_entries[1];
        entry.m_numChars = 0;
        entry.m_startIndex = 0;
        entry.m_object = nullptr;
    }

    {
        const char* start = stringTable->begin();
        const char* cur = start;
        const char* end = stringTable->end();

        while (cur < end)
        {
            CharReader reader(cur);
            const int len = GetUnicodePointFromUTF8(reader);

            Entry entry;
            entry.m_startIndex = uint32_t(reader.m_pos - start);
            entry.m_numChars = len;
            entry.m_object = nullptr;

            m_entries.Add(entry);

            cur = reader.m_pos + len;
        }
    }

    m_entries.Compress();
}

Name* StringRepresentationCache::getName(Handle handle)
{
    if (handle == StringSlicePool::kNullHandle)
    {
        return nullptr;
    }

    Entry& entry = m_entries[int(handle)];
    if (entry.m_object)
    {
        Name* name = dynamic_cast<Name*>(entry.m_object);
        if (name)
        {
            return name;
        }
        StringRepresentation* stringRep = static_cast<StringRepresentation*>(entry.m_object);
        // Promote it to a name
        name = m_namePool->getName(String(stringRep));
        entry.m_object = name;
        return name;
    }

    Name* name = m_namePool->getName(String(getStringSlice(handle)));
    entry.m_object = name;
    return name;
}

String StringRepresentationCache::getString(Handle handle)
{
    return String(getStringRepresentation(handle));
}

UnownedStringSlice StringRepresentationCache::getStringSlice(Handle handle) const
{
    const Entry& entry = m_entries[int(handle)];
    const char* start = m_stringTable->begin();

    return UnownedStringSlice(start + entry.m_startIndex, int(entry.m_numChars));
}

StringRepresentation* StringRepresentationCache::getStringRepresentation(Handle handle)
{
    if (handle == StringSlicePool::kNullHandle || handle == StringSlicePool::kEmptyHandle)
    {
        return nullptr;
    }

    Entry& entry = m_entries[int(handle)];
    if (entry.m_object)
    {
        Name* name = dynamic_cast<Name*>(entry.m_object);
        if (name)
        {
            return name->text.getStringRepresentation();
        }
        return static_cast<StringRepresentation*>(entry.m_object);
    }

    const UnownedStringSlice slice = getStringSlice(handle);
    const UInt size = slice.size();

    StringRepresentation* stringRep = StringRepresentation::createWithCapacityAndLength(size, size);
    memcpy(stringRep->getData(), slice.begin(), size);
    entry.m_object = stringRep;

    // Keep the StringRepresentation in scope
    m_scopeManager->add(stringRep);
    
    return stringRep;
}

char* StringRepresentationCache::getCStr(Handle handle)
{
    // It turns out StringRepresentation is always 0 terminated, so can just use that
    StringRepresentation* rep = getStringRepresentation(handle);
    return rep->getData();
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! SerialStringTableUtil !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

/* static */void SerialStringTableUtil::encodeStringTable(const StringSlicePool& pool, List<char>& stringTable)
{
    // Skip the default handles -> nothing is encoded via them
    return encodeStringTable(pool.getSlices().begin() + StringSlicePool::kNumDefaultHandles, pool.getNumSlices() - StringSlicePool::kNumDefaultHandles, stringTable);
}
    
/* static */void SerialStringTableUtil::encodeStringTable(const UnownedStringSlice* slices, size_t numSlices, List<char>& stringTable)
{
    stringTable.Clear();
    for (size_t i = 0; i < numSlices; ++i)
    {
        const UnownedStringSlice slice = slices[i];
        const int len = int(slice.size());
        
        // We need to write into the the string array
        char prefixBytes[6];
        const int numPrefixBytes = EncodeUnicodePointToUTF8(prefixBytes, len);
        const int baseIndex = int(stringTable.Count());

        stringTable.SetSize(baseIndex + numPrefixBytes + len);

        char* dst = stringTable.begin() + baseIndex;

        memcpy(dst, prefixBytes, numPrefixBytes);
        memcpy(dst + numPrefixBytes, slice.begin(), len);   
    }
}

/* static */void SerialStringTableUtil::decodeStringTable(const List<char>& stringTable, List<UnownedStringSlice>& slices)
{
    const char* start = stringTable.begin();
    const char* cur = start;
    const char* end = stringTable.end();

    while (cur < end)
    {
        CharReader reader(cur);
        const int len = GetUnicodePointFromUTF8(reader);
        slices.Add(UnownedStringSlice(reader.m_pos, len));
        cur = reader.m_pos + len;
    }
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! IRSerialData !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

template<typename T>
static size_t _calcArraySize(const List<T>& list)
{
    return list.Count() * sizeof(T);
}

size_t IRSerialData::calcSizeInBytes() const
{
    return 
        _calcArraySize(m_insts) + 
        _calcArraySize(m_childRuns) + 
        _calcArraySize(m_decorationRuns) + 
        _calcArraySize(m_externalOperands) + 
        _calcArraySize(m_stringTable) + 
        /* Raw source locs */
        _calcArraySize(m_rawSourceLocs) +
        /* Debug */
        _calcArraySize(m_debugSourceFiles) + 
        _calcArraySize(m_debugLineOffsets) + 
        _calcArraySize(m_debugViewEntries) + 
        _calcArraySize(m_debugLocRuns) + 
        _calcArraySize(m_debugStrings);
}

void IRSerialData::clear()
{
    // First Instruction is null
    m_insts.SetSize(1);
    memset(&m_insts[0], 0, sizeof(Inst));

    m_childRuns.Clear();
    m_decorationRuns.Clear();
    m_externalOperands.Clear();
    m_rawSourceLocs.Clear();

    // Debug data
    m_debugSourceFiles.Clear(); 
    m_debugLineOffsets.Clear(); 
    m_debugViewEntries.Clear(); 
    m_debugLocRuns.Clear();     
    m_debugStrings.Clear();

    m_stringTable.SetSize(2);
    m_stringTable[int(kNullStringIndex)] = 0;
    m_stringTable[int(kEmptyStringIndex)] = 0;

    m_decorationBaseIndex = 0;
}

template <typename T>
static bool _isEqual(const List<T>& aIn, const List<T>& bIn)
{
    if (aIn.Count() != bIn.Count())
    {
        return false;
    }

    size_t size = size_t(aIn.Count());

    const T* a = aIn.begin();
    const T* b = bIn.begin();

    if (a == b)
    {
        return true;
    }

    for (size_t i = 0; i < size; ++i)
    {
        if (a[i] != b[i])
        {
            return false;
        }
    }

    return true;
}


bool IRSerialData::operator==(const ThisType& rhs) const
{
    return (this == &rhs) || 
        (m_decorationBaseIndex == rhs.m_decorationBaseIndex &&
        _isEqual(m_insts, rhs.m_insts) &&
        _isEqual(m_childRuns, rhs.m_childRuns) &&
        _isEqual(m_decorationRuns, rhs.m_decorationRuns) &&
        _isEqual(m_externalOperands, rhs.m_externalOperands) &&
        _isEqual(m_rawSourceLocs, rhs.m_rawSourceLocs) &&
        _isEqual(m_stringTable, rhs.m_stringTable));
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

// Find a view index that matches the view by file (and perhaps other characteristics in the future)
int _findSourceViewIndex(const List<SourceView*>& viewsIn, SourceView* view)
{
    const int numViews = int(viewsIn.Count());
    SourceView*const* views = viewsIn.begin();
    
    SourceFile* sourceFile = view->getSourceFile();

    for (int i = 0; i < numViews; ++i)
    {
        SourceView* curView = views[i];
        // For now we just match on source file
        if (curView->getSourceFile() == sourceFile)
        {
            // It's a hit
            return i;
        }
    }
    return -1;
}


Result IRSerialWriter::write(IRModule* module, SourceManager* sourceManager, OptionFlags options, IRSerialData* serialData)
{
    typedef Ser::Inst::PayloadType PayloadType;

    SLANG_UNUSED(sourceManager);

    m_serialData = serialData;

    serialData->clear();

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
            
            // Can't be any pseudo ops
            SLANG_ASSERT(!isPseudoOp(srcInst->op)); 

            dstInst.m_op = uint8_t(srcInst->op & kIROpMeta_OpMask);
            dstInst.m_payloadType = PayloadType::Empty;
            
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
                dstInst.m_payloadType = PayloadType::String_1;
                dstInst.m_payload.m_stringIndices[0] = getStringIndex(globValue->mangledName);
                continue;
            }

            IRTextureTypeBase* textureBase = as<IRTextureTypeBase>(srcInst);
            if (textureBase)
            {
                dstInst.m_payloadType = PayloadType::OperandAndUInt32;
                dstInst.m_payload.m_operandAndUInt32.m_uint32 = uint32_t(srcInst->op) >> kIROpMeta_OtherShift;
                dstInst.m_payload.m_operandAndUInt32.m_operand = getInstIndex(textureBase->getElementType());
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
        SLANG_ASSERT(decorationBaseIndex + numDecor == int(m_serialData->m_insts.Count()));

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

                    dstInst.m_payloadType = PayloadType::UInt32;
                    dstInst.m_payload.m_uint32 = uint32_t(loopDecor->mode);
                    break;
                }
                case kIRDecorationOp_Target:
                {
                    auto targetDecor = static_cast<IRTargetDecoration*>(srcDecor);

                    dstInst.m_payloadType = PayloadType::String_1;
                    dstInst.m_payload.m_stringIndices[0] = getStringIndex(targetDecor->targetName);
                    break;
                }
                case kIRDecorationOp_TargetIntrinsic:
                {
                    auto targetDecor = static_cast<IRTargetIntrinsicDecoration*>(srcDecor);
                    dstInst.m_payloadType = PayloadType::String_2;

                    dstInst.m_payload.m_stringIndices[0] = getStringIndex(targetDecor->targetName);
                    dstInst.m_payload.m_stringIndices[1] = getStringIndex(targetDecor->definition);
                    break;
                }
                case kIRDecorationOp_GLSLOuterArray:
                {
                    auto arrayDecor = static_cast<IRGLSLOuterArrayDecoration*>(srcDecor);
                    dstInst.m_payloadType = PayloadType::String_1;

                    dstInst.m_payload.m_stringIndices[0] = getStringIndex(arrayDecor->outerArrayName);
                    break;
                }
                case kIRDecorationOp_Semantic:
                {
                    auto semanticDecor = static_cast<IRSemanticDecoration*>(srcDecor);

                    dstInst.m_payloadType = PayloadType::String_1;
                    dstInst.m_payload.m_stringIndices[0] = getStringIndex(semanticDecor->semanticName);
                    break;
                }
                case kIRDecorationOp_InterpolationMode:
                {
                    auto semanticDecor = static_cast<IRInterpolationModeDecoration*>(srcDecor);
                    dstInst.m_payloadType = PayloadType::UInt32;
                    dstInst.m_payload.m_uint32 = uint32_t(semanticDecor->mode);
                    break;
                }
                case kIRDecorationOp_NameHint:
                {
                    auto nameDecor = static_cast<IRNameHintDecoration*>(srcDecor);

                    dstInst.m_payloadType = PayloadType::String_1;
                    dstInst.m_payload.m_stringIndices[0] = getStringIndex(nameDecor->name);
                    break;
                }
                case kIRDecorationOp_VulkanRayPayload:
                case kIRDecorationOp_VulkanHitAttributes:
                {
                    dstInst.m_payloadType = PayloadType::Empty;
                    break;
                }
                case kIRDecorationOp_RequireGLSLExtension:
                {
                    auto extDecor = static_cast<IRRequireGLSLExtensionDecoration*>(srcDecor);

                    dstInst.m_payloadType = PayloadType::String_1;
                    dstInst.m_payload.m_stringIndices[0] = getStringIndex(extDecor->extensionName);
                    break;
                }
                case kIRDecorationOp_RequireGLSLVersion:
                {
                    auto verDecor = static_cast<IRRequireGLSLVersionDecoration*>(srcDecor);

                    dstInst.m_payloadType = PayloadType::UInt32;
                    dstInst.m_payload.m_uint32 = uint32_t(verDecor->languageVersion);
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

    // Convert strings into a string table
    {
        SerialStringTableUtil::encodeStringTable(m_stringSlicePool, serialData->m_stringTable);
    }


    // If the option to use RawSourceLocations is enabled, serialize out as is
    if (options & OptionFlag::RawSourceLocation)
    {
        const int numInsts = int(m_insts.Count());
        serialData->m_rawSourceLocs.SetSize(numInsts);

        Ser::RawSourceLoc* dstLocs =  serialData->m_rawSourceLocs.begin();
        // 0 is null, just mark as no location
        dstLocs[0] = Ser::RawSourceLoc(0);
        for (int i = 1; i < numInsts; ++i)
        {
            IRInst* srcInst = m_insts[i];
            dstLocs[i] = Ser::RawSourceLoc(srcInst->sourceLoc.getRaw());
        }
    }

    if (true)
    {
        StringSlicePool debugStringPool;

        struct SourceLocInstIndex  
        {
            typedef SourceLocInstIndex ThisType;
            bool operator<(const ThisType& rhs) const
            {
                return sourceLoc < rhs.sourceLoc || (sourceLoc == rhs.sourceLoc && instIndex < rhs.instIndex);
            }
            bool operator==(const ThisType& rhs) const
            {
                return sourceLoc == rhs.sourceLoc && instIndex == rhs.instIndex;
            }

            SourceLoc::RawValue sourceLoc;
            uint32_t instIndex;
        };

        // Find all of the instructions 

        List<SourceLocInstIndex> locs;
        const int numInsts = int(m_insts.Count());
        for (int i = 1; i < numInsts; ++i)
        {
            auto srcInst = m_insts[i];
            if (srcInst->sourceLoc.isValid())
            {
                locs.Add(SourceLocInstIndex{srcInst->sourceLoc.getRaw(), uint32_t(i) });
            }    
        }

        // Okay lets order all SourceLocs for easier lookup, and better compressibility 
        locs.Sort();

        // Lets filter this down so we only have unique source locs
        struct SourceLocRun
        {
            SourceLoc::RawValue sourceLoc;
            uint32_t startInstIndex;
            uint32_t numInst;
            //uint32_t sourceFileIndex;
            uint32_t lineNo;
            uint32_t columnNo;
            uint32_t pathIndex;
        };

        // We could just fill up this table with everything...

        /* Really the challenge here is not about 'finding out the paths/line numbers etc used'
        a big part of the problem is how to reconstruct the data into a SourceManager such that it can be 
        used correctly?

        We *could* I suppose work backwards and work out via line and column numbers something that would 
        work, but it would require finding all the instructions on a line and taking the maximum column as the line 
        length and then so forth. 

        An alternative would be to save enough data to be able to construct something that is compatible. 
        For line numbers we only have to save the data about lines that are referenced. We also have to be able to reconstruct 
        the gaps.

        Then we would still have to save off the data for the entries that are used. 
        
        Such an approach also implies we can have multiple 'views' if we really want to. 

        I could test all of this, by storing all the line number information, then storing and recreating and making sure 
        it matches. 

        */


        // List of unique source views
        List<SourceView*> sourceViews;
        // List of runs
        List<SourceLocRun> locRuns;
        {
            SourceView* sourceView = sourceManager->findSourceView(SourceLoc::fromRaw(locs[0].sourceLoc));
            SLANG_ASSERT(sourceView);
            SourceRange viewRange = sourceView->getRange();

            uint32_t sourceLocFix = 0;
            //uint32_t sourceViewIndex = 0;
            sourceViews.Add(sourceView);

            const int numLocs = int(locs.Count());
            int i = 0;
            while (i < numLocs)
            {   
                const int startI = i;
                const auto& loc = locs[startI];

                const uint32_t startInstIndex = loc.instIndex;
                const uint32_t fixInstIndex = loc.instIndex - uint32_t(startI);
                
                const auto rawSourceLoc = loc.sourceLoc;

                // Look for a run
                for (++i; i < numLocs; ++i)
                {
                    const auto& runLoc = locs[i];
                    if (runLoc.sourceLoc != rawSourceLoc || runLoc.instIndex != fixInstIndex + i) 
                    {
                        break;
                    }
                }

                {
                    const SourceLoc sourceLoc(SourceLoc::fromRaw(rawSourceLoc));
                    // Check if this is still in the current source view
                    if (!viewRange.contains(sourceLoc))
                    {
                        // Look up the SourceView it is
                        SourceView* nextSourceView = sourceManager->findSourceView(sourceLoc);
                        SLANG_ASSERT(nextSourceView);
                        // Update the range being used
                        viewRange = nextSourceView->getRange();
                    
                        // See if there a pre-existing view that has the same source backing it
                        int foundViewIndex = _findSourceViewIndex(sourceViews, nextSourceView);
                        if (foundViewIndex >= 0)
                        {
                            sourceView = sourceViews[foundViewIndex];
                            //sourceViewIndex = uint32_t(foundViewIndex);
                            // With the fix we need to map from the current range to the found range
                            sourceLocFix =  sourceView->getRange().begin.getRaw() - viewRange.begin.getRaw();
                        }
                        else
                        {
                            // We have a new source view
                            //sourceViewIndex = uint32_t(sourceViews.Count());
                            sourceViews.Add(nextSourceView);
                            sourceLocFix = 0;
                            sourceView = nextSourceView;
                        }

                        // After all this the source must be in range
                        SLANG_ASSERT(viewRange.contains(sourceLoc));

                        // Moreover the corrected loc must be in the range of the matched sourceView
                        SLANG_ASSERT(sourceView->getRange().contains(SourceLoc::fromRaw(rawSourceLoc + sourceLocFix)));
                    }
                }

                // Add it 
                SourceLocRun run;

                run.sourceLoc = rawSourceLoc + sourceLocFix;
                run.startInstIndex = startInstIndex;
                run.numInst = uint32_t(i - startI);
                //run.sourceFileIndex = sourceViewIndex;

                SourceLoc fixSourceLoc(SourceLoc::fromRaw(rawSourceLoc + sourceLocFix));
                
                
                // Work out the line/column no from the file if there are no modifications
                {
                    SourceFile* sourceFile = sourceView->getSourceFile();
                    const int offset = sourceView->getRange().getOffset(fixSourceLoc);
                    
                    int lineIndex = sourceFile->calcLineIndexFromOffset(offset);
                    int colIndex = sourceFile->calcColumnIndex(lineIndex, offset);

                    run.lineNo = uint32_t(lineIndex + 1);
                    run.columnNo = uint32_t(colIndex + 1);
                }

                // Mark the index as used
                //run.pathIndex = uint32_t(remapPathIndex);

                locRuns.Add(run);
            }
        }
    }
        

    m_serialData = nullptr;
    return SLANG_OK;
}

template <typename T>
static size_t _calcChunkSize(IRSerialBinary::CompressionType compressionType, const List<T>& array)
{
    typedef IRSerialBinary Bin;

    if (array.Count())
    {
        switch (compressionType)
        {
            case Bin::CompressionType::None:
            {
                const size_t size = sizeof(Bin::ArrayHeader) + sizeof(T) * array.Count();
                return (size + 3) & ~size_t(3);
            }
            case Bin::CompressionType::VariableByteLite:
            {
                const size_t payloadSize = ByteEncodeUtil::calcEncodeLiteSizeUInt32((const uint32_t*)array.begin(), (array.Count() * sizeof(T)) / sizeof(uint32_t));
                const size_t size = sizeof(Bin::CompressedArrayHeader) + payloadSize;
                return (size + 3) & ~size_t(3);
            }
            default:
            {
                SLANG_ASSERT(!"Unhandled compression type");
                return 0;
            }
        }
    }
    else
    {
        return 0;
    }
}

static Result _writeArrayChunk(IRSerialBinary::CompressionType compressionType, uint32_t chunkId, const void* data, size_t numEntries, size_t typeSize, Stream* stream)
{
    typedef IRSerialBinary Bin;

    if (numEntries == 0)
    {
        return SLANG_OK;
    }

    size_t payloadSize;

    switch (compressionType)
    {
        case Bin::CompressionType::None:
        {
            payloadSize = sizeof(Bin::ArrayHeader) - sizeof(Bin::Chunk) + typeSize * numEntries;

            Bin::ArrayHeader header;
            header.m_chunk.m_type = chunkId;
            header.m_chunk.m_size = uint32_t(payloadSize);
            header.m_numEntries = uint32_t(numEntries);

            stream->Write(&header, sizeof(header));

            stream->Write(data, typeSize * numEntries);
            break;
        }
        case Bin::CompressionType::VariableByteLite:
        {
            List<uint8_t> compressedPayload;

            size_t numCompressedEntries = (numEntries * typeSize) / sizeof(uint32_t);

            ByteEncodeUtil::encodeLiteUInt32((const uint32_t*)data, numCompressedEntries, compressedPayload);

            payloadSize = sizeof(Bin::CompressedArrayHeader) - sizeof(Bin::Chunk) + compressedPayload.Count();

            Bin::CompressedArrayHeader header;
            header.m_chunk.m_type = SLANG_MAKE_COMPRESSED_FOUR_CC(chunkId);
            header.m_chunk.m_size = uint32_t(payloadSize);
            header.m_numEntries = uint32_t(numEntries);
            header.m_numCompressedEntries = uint32_t(numCompressedEntries);

            stream->Write(&header, sizeof(header));

            stream->Write(compressedPayload.begin(), compressedPayload.Count());
            break;
        }
        default:
        {
            return SLANG_FAIL;
        }
    }
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

template <typename T>
Result _writeArrayChunk(IRSerialBinary::CompressionType compressionType, uint32_t chunkId, const List<T>& array, Stream* stream)
{
    return _writeArrayChunk(compressionType, chunkId, array.begin(), size_t(array.Count()), sizeof(T), stream);
}

Result _encodeInsts(IRSerialBinary::CompressionType compressionType, const List<IRSerialData::Inst>& instsIn, List<uint8_t>& encodeArrayOut)
{
    typedef IRSerialBinary Bin;
    typedef IRSerialData::Inst::PayloadType PayloadType;

    if (compressionType != Bin::CompressionType::VariableByteLite)
    {
        return SLANG_FAIL;
    }
    encodeArrayOut.Clear();
    
    const size_t numInsts = size_t(instsIn.Count());
    const IRSerialData::Inst* insts = instsIn.begin();
        
    uint8_t* encodeOut = encodeArrayOut.begin();
    uint8_t* encodeEnd = encodeArrayOut.end();

    // Calculate the maximum instruction size with worst case possible encoding
    // 2 bytes hold the payload size, and the result type
    // Note that if there were some free bits, we could encode some of this stuff into bits, but if we remove payloadType, then there are no free bits
    const size_t maxInstSize = 2 + ByteEncodeUtil::kMaxLiteEncodeUInt32 + Math::Max(sizeof(insts->m_payload.m_float64), size_t(2 * ByteEncodeUtil::kMaxLiteEncodeUInt32));

    for (size_t i = 0; i < numInsts; ++i)
    {
        const auto& inst = insts[i];

        // Make sure there is space for the largest possible instruction
        if (encodeOut + maxInstSize >= encodeEnd)
        {
            const size_t offset = size_t(encodeOut - encodeArrayOut.begin());
            
            const UInt oldCapacity = encodeArrayOut.Capacity();

            encodeArrayOut.Reserve(oldCapacity + (oldCapacity >> 1) + maxInstSize);
            const UInt capacity = encodeArrayOut.Capacity();
            encodeArrayOut.SetSize(capacity);

            encodeOut = encodeArrayOut.begin() + offset;
            encodeEnd = encodeArrayOut.end();
        }

        *encodeOut++ = uint8_t(inst.m_op);
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
    encodeArrayOut.SetSize(UInt(encodeOut - encodeArrayOut.begin()));
    return SLANG_OK;
}

Result _writeInstArrayChunk(IRSerialBinary::CompressionType compressionType, uint32_t chunkId, const List<IRSerialData::Inst>& array, Stream* stream)
{
    typedef IRSerialBinary Bin;
    if (array.Count() == 0)
    {
        return SLANG_OK;
    }

    switch (compressionType)
    {
        case Bin::CompressionType::None:
        {
            return _writeArrayChunk(compressionType, chunkId, array, stream);
        }
        case Bin::CompressionType::VariableByteLite:
        {
            List<uint8_t> compressedPayload;
            SLANG_RETURN_ON_FAIL(_encodeInsts(compressionType, array, compressedPayload));
            
            size_t payloadSize = sizeof(Bin::CompressedArrayHeader) - sizeof(Bin::Chunk) + compressedPayload.Count();

            Bin::CompressedArrayHeader header;
            header.m_chunk.m_type = SLANG_MAKE_COMPRESSED_FOUR_CC(chunkId);
            header.m_chunk.m_size = uint32_t(payloadSize);
            header.m_numEntries = uint32_t(array.Count());
            header.m_numCompressedEntries = 0;          

            stream->Write(&header, sizeof(header));
            stream->Write(compressedPayload.begin(), compressedPayload.Count());
    
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
        default: break;
    }
    return SLANG_FAIL;
}

static size_t _calcInstChunkSize(IRSerialBinary::CompressionType compressionType, const List<IRSerialData::Inst>& instsIn)
{
    typedef IRSerialBinary Bin;
    typedef IRSerialData::Inst::PayloadType PayloadType;

    switch (compressionType)
    {
        case Bin::CompressionType::None:
        {
            return _calcChunkSize(compressionType, instsIn);
        }
        case Bin::CompressionType::VariableByteLite:
        {
            size_t size = sizeof(Bin::CompressedArrayHeader);

            size_t numInsts = size_t(instsIn.Count());
            size += numInsts * 2;           // op and payload

            
            IRSerialData::Inst* insts = instsIn.begin();

            for (size_t i = 0; i < numInsts; ++i)
            {
                const auto& inst = insts[i];

                size += ByteEncodeUtil::calcEncodeLiteSizeUInt32((uint32_t)inst.m_resultTypeIndex);

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
                        size += ByteEncodeUtil::calcEncodeLiteSizeUInt32((uint32_t)inst.m_payload.m_operands[0]);
                        break;
                    }
                    case PayloadType::Operand_2:
                    case PayloadType::OperandAndUInt32:
                    case PayloadType::OperandExternal:
                    case PayloadType::String_2:
                    {
                        // 2 UInt32
                        size += ByteEncodeUtil::calcEncodeLiteSizeUInt32((uint32_t)inst.m_payload.m_operands[0]);
                        size += ByteEncodeUtil::calcEncodeLiteSizeUInt32((uint32_t)inst.m_payload.m_operands[1]);
                        break;
                    }
                    case PayloadType::Float64:
                    {
                        size += sizeof(inst.m_payload.m_float64);
                        break;
                    }
                    case PayloadType::Int64:
                    {
                        size += sizeof(inst.m_payload.m_int64);
                        break;
                    }
                }
            }

            return (size + 3) & ~size_t(3);
        }
        default: break;
    }

    SLANG_ASSERT(!"Unhandled compression type");
    return 0;
}

/* static */Result IRSerialWriter::writeStream(const IRSerialData& data, Bin::CompressionType compressionType, Stream* stream)
{
    size_t totalSize = 0;
    
    totalSize += sizeof(Bin::SlangHeader) + 
        _calcInstChunkSize(compressionType, data.m_insts) +
        _calcChunkSize(compressionType, data.m_childRuns) +
        _calcChunkSize(compressionType, data.m_decorationRuns) +
        _calcChunkSize(compressionType, data.m_externalOperands) +
        _calcChunkSize(Bin::CompressionType::None, data.m_stringTable) + 
        _calcChunkSize(Bin::CompressionType::None, data.m_rawSourceLocs);

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
        slangHeader.m_compressionType = uint32_t(Bin::CompressionType::VariableByteLite);

        stream->Write(&slangHeader, sizeof(slangHeader));
    }

    SLANG_RETURN_ON_FAIL(_writeInstArrayChunk(compressionType, Bin::kInstFourCc, data.m_insts, stream));
    SLANG_RETURN_ON_FAIL(_writeArrayChunk(compressionType, Bin::kChildRunFourCc, data.m_childRuns, stream));
    SLANG_RETURN_ON_FAIL(_writeArrayChunk(compressionType, Bin::kDecoratorRunFourCc, data.m_decorationRuns, stream));
    SLANG_RETURN_ON_FAIL(_writeArrayChunk(compressionType, Bin::kExternalOperandsFourCc, data.m_externalOperands, stream));
    SLANG_RETURN_ON_FAIL(_writeArrayChunk(Bin::CompressionType::None, Bin::kStringFourCc, data.m_stringTable, stream));

    SLANG_RETURN_ON_FAIL(_writeArrayChunk(Bin::CompressionType::None, Bin::kUInt32SourceLocFourCc, data.m_rawSourceLocs, stream));
    
    return SLANG_OK;
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! IRSerialReader !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

class ListResizer
{
    public:
    virtual void* setSize(size_t newSize) = 0;
    SLANG_FORCE_INLINE size_t getTypeSize() const { return m_typeSize; }
    ListResizer(size_t typeSize):m_typeSize(typeSize) {}

    protected:
    size_t m_typeSize;
};

template <typename T>
class ListResizerForType: public ListResizer
{
    public:
    typedef ListResizer Parent;

    SLANG_FORCE_INLINE ListResizerForType(List<T>& list):
        Parent(sizeof(T)), 
        m_list(list) 
    {}
    
    virtual void* setSize(size_t newSize) SLANG_OVERRIDE
    {
        m_list.SetSize(UInt(newSize));
        return (void*)m_list.begin();
    }

    protected:
    List<T>& m_list;
};

static Result _readArrayChunk(IRSerialBinary::CompressionType compressionType, const IRSerialBinary::Chunk& chunk, Stream* stream, size_t* numReadInOut, ListResizer& listOut)
{
    typedef IRSerialBinary Bin;

    const size_t typeSize = listOut.getTypeSize();

    switch (compressionType)
    {
        case Bin::CompressionType::VariableByteLite:
        {
            // We have a compressed header
            Bin::CompressedArrayHeader header;
            header.m_chunk = chunk;

            stream->Read(&header.m_chunk + 1, sizeof(header) - sizeof(Bin::Chunk));
            *numReadInOut += sizeof(header) - sizeof(Bin::Chunk);

            void* data = listOut.setSize(header.m_numEntries);
            
            // Need to read all the compressed data... 
            size_t payloadSize = header.m_chunk.m_size - (sizeof(header) - sizeof(Bin::Chunk));

            List<uint8_t> compressedPayload;
            compressedPayload.SetSize(payloadSize);

            stream->Read(compressedPayload.begin(), payloadSize);
            *numReadInOut += payloadSize;
        
            SLANG_ASSERT(header.m_numCompressedEntries == uint32_t((header.m_numEntries * typeSize) / sizeof(uint32_t)));

            // Decode..
            ByteEncodeUtil::decodeLiteUInt32(compressedPayload.begin(), header.m_numCompressedEntries, (uint32_t*)data);
            break;
        }
        case Bin::CompressionType::None:
        {
            // Read uncompressed
            Bin::ArrayHeader header;
            header.m_chunk = chunk;

            stream->Read(&header.m_chunk + 1, sizeof(header) - sizeof(Bin::Chunk));
            *numReadInOut += sizeof(header) - sizeof(Bin::Chunk);

            const size_t payloadSize = header.m_numEntries * typeSize;

            void* data = listOut.setSize(header.m_numEntries);

            stream->Read(data, payloadSize);
            *numReadInOut += payloadSize;
            break;
        }
    }

    // All chunks have sizes rounded to dword size
    if (*numReadInOut & 3)
    {
        const uint8_t pad[4] = { 0, 0, 0, 0 };
        // Pad outs
        int padSize = 4 - int(*numReadInOut & 3);
        stream->Seek(SeekOrigin::Current, padSize);

        *numReadInOut += padSize;
    }

    return SLANG_OK;
}

template <typename T>
Result _readArrayChunk(const IRSerialBinary::SlangHeader& header, const IRSerialBinary::Chunk& chunk, Stream* stream, size_t* numReadInOut, List<T>& arrayOut)
{
    typedef IRSerialBinary Bin;

    Bin::CompressionType compressionType = Bin::CompressionType::None;

    if (chunk.m_type == SLANG_MAKE_COMPRESSED_FOUR_CC(chunk.m_type))
    {
        // If it has compression, use the compression type set in the header
        compressionType = Bin::CompressionType(header.m_compressionType);
    }
    ListResizerForType<T> resizer(arrayOut);
    return _readArrayChunk(compressionType, chunk, stream, numReadInOut, resizer);
}  

template <typename T>
Result _readArrayUncompressedChunk(const IRSerialBinary::SlangHeader& header, const IRSerialBinary::Chunk& chunk, Stream* stream, size_t* numReadInOut, List<T>& arrayOut)
{
    typedef IRSerialBinary Bin;
    SLANG_UNUSED(header);
    ListResizerForType<T> resizer(arrayOut);
    return _readArrayChunk(Bin::CompressionType::None, chunk, stream, numReadInOut, resizer);
}

static Result _decodeInsts(IRSerialBinary::CompressionType compressionType, const List<uint8_t>& encodeIn, List<IRSerialData::Inst>& instsOut)
{
    typedef IRSerialBinary Bin;
    typedef IRSerialData::Inst::PayloadType PayloadType;

    if (compressionType != Bin::CompressionType::VariableByteLite)
    {
        return SLANG_FAIL;
    }

    const size_t numInsts = size_t(instsOut.Count());
    IRSerialData::Inst* insts = instsOut.begin();

    const uint8_t* encodeCur = encodeIn.begin();

    for (size_t i = 0; i < numInsts; ++i)
    {
        auto& inst = insts[i];

        inst.m_op = *encodeCur++;
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

Result _readInstArrayChunk(const IRSerialBinary::SlangHeader& slangHeader, const IRSerialBinary::Chunk& chunk, Stream* stream, size_t* numReadInOut, List<IRSerialData::Inst>& arrayOut)
{
    typedef IRSerialBinary Bin;

    Bin::CompressionType compressionType = Bin::CompressionType::None;
    if (chunk.m_type == SLANG_MAKE_COMPRESSED_FOUR_CC(chunk.m_type))
    {
        compressionType = Bin::CompressionType(slangHeader.m_compressionType);
    }

    switch (compressionType)
    {
        case Bin::CompressionType::None:
        {
            ListResizerForType<IRSerialData::Inst> resizer(arrayOut);
            return _readArrayChunk(compressionType, chunk, stream, numReadInOut, resizer);
        }
        case Bin::CompressionType::VariableByteLite:
        {
            // We have a compressed header
            Bin::CompressedArrayHeader header;
            header.m_chunk = chunk;

            stream->Read(&header.m_chunk + 1, sizeof(header) - sizeof(Bin::Chunk));
            *numReadInOut += sizeof(header) - sizeof(Bin::Chunk);
            
            // Need to read all the compressed data... 
            size_t payloadSize = header.m_chunk.m_size - (sizeof(header) - sizeof(Bin::Chunk));

            List<uint8_t> compressedPayload;
            compressedPayload.SetSize(payloadSize);

            stream->Read(compressedPayload.begin(), payloadSize);
            *numReadInOut += payloadSize;

            arrayOut.SetSize(header.m_numEntries);

            SLANG_RETURN_ON_FAIL(_decodeInsts(compressionType, compressedPayload, arrayOut));
            break;
        }
        default:
        {
            return SLANG_FAIL;
        }
    }

    // All chunks have sizes rounded to dword size
    if (*numReadInOut & 3)
    {
        // Pad outs
        int padSize = 4 - int(*numReadInOut & 3);
        stream->Seek(SeekOrigin::Current, padSize);
        *numReadInOut += padSize;
    }

    return SLANG_OK;
}

int64_t _calcChunkTotalSize(const IRSerialBinary::Chunk& chunk)
{
    int64_t size = chunk.m_size + sizeof(IRSerialBinary::Chunk);
    return (size + 3) & ~int64_t(3);
}

/* static */Result IRSerialReader::_skip(const IRSerialBinary::Chunk& chunk, Stream* stream, int64_t* remainingBytesInOut)
{
    typedef IRSerialBinary Bin;
    int64_t chunkSize = _calcChunkTotalSize(chunk);
    if (remainingBytesInOut)
    {
        *remainingBytesInOut -= chunkSize;
    }

    // Skip the payload (we don't need to skip the Chunk because that was already read
    stream->Seek(SeekOrigin::Current, chunkSize - sizeof(IRSerialBinary::Chunk));
    return SLANG_OK;
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
    
    // Header
    // Chunk will not be kSlangFourCC if not read yet
    Bin::SlangHeader slangHeader;
    memset(&slangHeader, 0, sizeof(slangHeader));

    while (remainingBytes > 0)
    {
        Bin::Chunk chunk;

        stream->Read(&chunk, sizeof(chunk));

        size_t bytesRead = sizeof(chunk);

        switch (chunk.m_type)
        {
            case Bin::kSlangFourCc:
            {
                // Slang header
                slangHeader.m_chunk = chunk;

                // NOTE! Really we should only read what we know the size to be...
                // and skip if it's larger

                stream->Read(&slangHeader.m_chunk + 1, sizeof(slangHeader) - sizeof(chunk));

                dataOut->m_decorationBaseIndex = slangHeader.m_decorationBase;
                remainingBytes -= _calcChunkTotalSize(chunk);
                break;
            }
            case SLANG_MAKE_COMPRESSED_FOUR_CC(Bin::kInstFourCc):
            case Bin::kInstFourCc:
            {
                SLANG_RETURN_ON_FAIL(_readInstArrayChunk(slangHeader, chunk, stream, &bytesRead, dataOut->m_insts));
                remainingBytes -= _calcChunkTotalSize(chunk);
                break;    
            }
            case SLANG_MAKE_COMPRESSED_FOUR_CC(Bin::kDecoratorRunFourCc):
            case Bin::kDecoratorRunFourCc:
            {
                SLANG_RETURN_ON_FAIL(_readArrayChunk(slangHeader, chunk, stream, &bytesRead, dataOut->m_decorationRuns));
                remainingBytes -= _calcChunkTotalSize(chunk);
                break;
            }
            case SLANG_MAKE_COMPRESSED_FOUR_CC(Bin::kChildRunFourCc):
            case Bin::kChildRunFourCc:
            {
                SLANG_RETURN_ON_FAIL(_readArrayChunk(slangHeader, chunk, stream, &bytesRead, dataOut->m_childRuns));
                remainingBytes -= _calcChunkTotalSize(chunk);
                break;
            }
            case SLANG_MAKE_COMPRESSED_FOUR_CC(Bin::kExternalOperandsFourCc):
            case Bin::kExternalOperandsFourCc:
            {
                SLANG_RETURN_ON_FAIL(_readArrayChunk(slangHeader, chunk, stream, &bytesRead, dataOut->m_externalOperands));
                remainingBytes -= _calcChunkTotalSize(chunk);
                break;
            }
            case Bin::kStringFourCc:
            {
                SLANG_RETURN_ON_FAIL(_readArrayUncompressedChunk(slangHeader, chunk, stream, &bytesRead, dataOut->m_stringTable));
                remainingBytes -= _calcChunkTotalSize(chunk);
                break;
            }
            case Bin::kUInt32SourceLocFourCc:
            {
                
                SLANG_RETURN_ON_FAIL(_readArrayUncompressedChunk(slangHeader, chunk, stream, &bytesRead, dataOut->m_rawSourceLocs));
                remainingBytes -= _calcChunkTotalSize(chunk);
                break;
            }
            default:
            {
                SLANG_RETURN_ON_FAIL(_skip(chunk, stream, &remainingBytes));
                break;
            }
        }
    }

    return SLANG_OK;
}

IRDecoration* IRSerialReader::_createDecoration(const Ser::Inst& srcInst)
{
    typedef Ser::Inst::PayloadType PayloadType;

    IRDecorationOp decorOp = IRDecorationOp(srcInst.m_op - kIROpCount);
    SLANG_ASSERT(decorOp < kIRDecorationOp_CountOf);

    switch (decorOp)
    {
        case kIRDecorationOp_HighLevelDecl:
        {
            // TODO!
            // Decl* decl;
            return createEmptyDecoration<IRHighLevelDeclDecoration>(m_module);
        }
        case kIRDecorationOp_Layout:
        {
            // TODO!
            // Layout* layout;
            return createEmptyDecoration<IRLayoutDecoration>(m_module);
        }
        case kIRDecorationOp_LoopControl:
        {
            auto decor = createEmptyDecoration<IRLoopControlDecoration>(m_module);
            SLANG_ASSERT(srcInst.m_payloadType == PayloadType::UInt32);
            decor->mode = IRLoopControl(srcInst.m_payload.m_uint32);
            return decor;
        }
        case kIRDecorationOp_Target:
        {
            auto decor = createEmptyDecoration<IRTargetDecoration>(m_module);
            SLANG_ASSERT(srcInst.m_payloadType == PayloadType::String_1);
            decor->targetName = m_stringRepresentationCache.getStringRepresentation(StringHandle(srcInst.m_payload.m_stringIndices[0]));
            return decor;
        }
        case kIRDecorationOp_TargetIntrinsic:
        {
            auto decor = createEmptyDecoration<IRTargetIntrinsicDecoration>(m_module);
            SLANG_ASSERT(srcInst.m_payloadType == PayloadType::String_2);
            decor->targetName = m_stringRepresentationCache.getStringRepresentation(StringHandle(srcInst.m_payload.m_stringIndices[0]));
            decor->definition = m_stringRepresentationCache.getStringRepresentation(StringHandle(srcInst.m_payload.m_stringIndices[1]));
            return decor;
        }
        case kIRDecorationOp_GLSLOuterArray:
        {
            auto decor = createEmptyDecoration<IRGLSLOuterArrayDecoration>(m_module);
            SLANG_ASSERT(srcInst.m_payloadType == PayloadType::String_1);
            decor->outerArrayName = m_stringRepresentationCache.getCStr(StringHandle(srcInst.m_payload.m_stringIndices[0]));
            return decor;
        }
        case kIRDecorationOp_Semantic:
        {
            auto decor = createEmptyDecoration<IRSemanticDecoration>(m_module);
            SLANG_ASSERT(srcInst.m_payloadType == PayloadType::String_1);
            decor->semanticName = m_stringRepresentationCache.getName(StringHandle(srcInst.m_payload.m_stringIndices[0]));
            return decor;
        }
        case kIRDecorationOp_InterpolationMode:
        {
            auto decor = createEmptyDecoration<IRInterpolationModeDecoration>(m_module);
            SLANG_ASSERT(srcInst.m_payloadType == Ser::Inst::PayloadType::UInt32);
            decor->mode = IRInterpolationMode(srcInst.m_payload.m_uint32);
            return decor;
        }
        case kIRDecorationOp_NameHint:
        {
            auto decor = createEmptyDecoration<IRNameHintDecoration>(m_module);
            SLANG_ASSERT(srcInst.m_payloadType == PayloadType::String_1);
            decor->name = m_stringRepresentationCache.getName(StringHandle(srcInst.m_payload.m_stringIndices[0]));
            return decor;
        }
        case kIRDecorationOp_VulkanRayPayload:
        {
            auto decor = createEmptyDecoration<IRVulkanRayPayloadDecoration>(m_module);
            SLANG_ASSERT(srcInst.m_payloadType == PayloadType::Empty);
            return decor;
        }
        case kIRDecorationOp_VulkanHitAttributes:
        {
            auto decor = createEmptyDecoration<IRVulkanHitAttributesDecoration>(m_module);
            SLANG_ASSERT(srcInst.m_payloadType == PayloadType::Empty);
            return decor;
        }
        case kIRDecorationOp_RequireGLSLExtension:
        {
            auto decor = createEmptyDecoration<IRRequireGLSLExtensionDecoration>(m_module);
            SLANG_ASSERT(srcInst.m_payloadType == PayloadType::String_1);
            decor->extensionName = getStringRepresentation(srcInst.m_payload.m_stringIndices[0]);
            return decor;
        }
        case kIRDecorationOp_RequireGLSLVersion:
        {
            auto decor = createEmptyDecoration<IRRequireGLSLVersionDecoration>(m_module);
            SLANG_ASSERT(srcInst.m_payloadType == Ser::Inst::PayloadType::UInt32);
            decor->languageVersion = Int(srcInst.m_payload.m_uint32);
            return decor;
        }
        default:
        {
            SLANG_ASSERT(!"Unhandled decoration type");
            return nullptr;
        }
    }
}

/* static */Result IRSerialReader::read(const IRSerialData& data, Session* session, RefPtr<IRModule>& moduleOut)
{
    typedef Ser::Inst::PayloadType PayloadType;

    m_serialData = &data;
 
    auto module = new IRModule();
    moduleOut = module;
    m_module = module;

    module->session = session;

    // Set up the string rep cache
    m_stringRepresentationCache.init(&data.m_stringTable, session->getNamePool(), module->getObjectScopeManager());
    
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
    // 1 holds the IRModuleInst
    {
        // Check that insts[1] is the module inst
        const Ser::Inst& srcInst = data.m_insts[1];
        SLANG_RELEASE_ASSERT(srcInst.m_op == kIROp_Module);
        SLANG_ASSERT(srcInst.m_payloadType == PayloadType::Empty);

        // Create the module inst
        auto moduleInst = static_cast<IRModuleInst*>(createEmptyInstWithSize(module, kIROp_Module, sizeof(IRModuleInst)));
        module->moduleInst = moduleInst;
        moduleInst->module = module;

        // Set the IRModuleInst
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
                globalValueInst->mangledName = m_stringRepresentationCache.getName(StringHandle(srcInst.m_payload.m_stringIndices[0]));
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
                const size_t prefixSize = SLANG_OFFSET_OF(IRConstant, value);

                IRConstant* irConst = nullptr;
                switch (op)
                {                    
                    case kIROp_BoolLit:
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

                        const UnownedStringSlice slice = m_stringRepresentationCache.getStringSlice(StringHandle(srcInst.m_payload.m_stringIndices[0]));
                        
                        const size_t sliceSize = slice.size();
                        const size_t instSize = prefixSize + SLANG_OFFSET_OF(IRConstant::StringValue, chars) + sliceSize;

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
                IRTextureTypeBase* inst = static_cast<IRTextureTypeBase*>(createEmptyInst(module, op, 1));
                SLANG_ASSERT(srcInst.m_payloadType == PayloadType::OperandAndUInt32);

                // Reintroduce the texture type bits into the the
                const uint32_t other = srcInst.m_payload.m_operandAndUInt32.m_uint32;
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

    // Create the decorations
    for (int i = 0; i < numDecorations; ++i)
    {
        IRDecoration* decor = _createDecoration(data.m_insts[i + numInsts]);
        if (!decor)
        {
            return SLANG_FAIL;
        }
        decorations[i] = decor;
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

            // Calculate the offset in the decoration list, which index 0, is decorationBaseIndex in instruction indices
            const int decorStartIndex = int(run.m_startInstIndex) - decorationBaseIndex;

            // Go in reverse order so that linked list is in same order as original
            for (int j = int(run.m_numChildren) - 1; j >= 0; --j)
            {
                IRDecoration* decor = decorations[decorStartIndex + j]; 
                // And to the linked list on the 
                decor->next = inst->firstDecoration;
                inst->firstDecoration = decor;
            }
        }
    }

    // Re-add source locations, if they are defined
    if (int(m_serialData->m_rawSourceLocs.Count()) == numInsts)
    {
        const Ser::RawSourceLoc* srcLocs = m_serialData->m_rawSourceLocs.begin();
        for (int i = 1; i < numInsts; ++i)
        {
            IRInst* dstInst = insts[i];
            
            dstInst->sourceLoc.setRaw(Slang::SourceLoc::RawValue(srcLocs[i]));
        }
    }

    return SLANG_OK;
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!! Free functions !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

#if 0

Result serializeModule(IRModule* module, SourceManager* sourceManager, Stream* stream)
{
    IRSerialWriter serializer;
    IRSerialData serialData;

    SLANG_RETURN_ON_FAIL(serializer.write(module, sourceManager, IRSerialWriter::OptionFlag::RawSourceLocation, &serialData));

    if (stream)
    {
        SLANG_RETURN_ON_FAIL(IRSerialWriter::writeStream(serialData, IRSerialBinary::CompressionType::VariableByteLite, stream));
    }

    return SLANG_OK;
}

Result readModule(Session* session, Stream* stream, RefPtr<IRModule>& moduleOut)
{
    IRSerialData serialData;
    IRSerialReader::readStream(stream, &serialData);

    IRSerialReader reader;
    return reader.read(serialData, session, moduleOut);
}

#endif

} // namespace Slang
