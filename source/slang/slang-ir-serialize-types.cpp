// slang-ir-serialize-types.cpp
#include "slang-ir-serialize-types.h"

#include "../core/slang-text-io.h"
#include "../core/slang-byte-encode-util.h"

#include "slang-ir-insts.h"

#include "../core/slang-math.h"

namespace Slang {

// Needed for linkage with some compilers
/* static */ const IRSerialData::StringIndex IRSerialData::kNullStringIndex;
/* static */ const IRSerialData::StringIndex IRSerialData::kEmptyStringIndex;

/* Note that an IRInst can be derived from, but when it derived from it's new members are IRUse variables, and they in 
effect alias over the operands - and reflected in the operand count. There _could_ be other members after these IRUse 
variables, but only a few types include extra data, and these do not have any operands:

* IRConstant        - Needs special-case handling
* IRModuleInst      - Presumably we can just set to the module pointer on reconstruction

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

// Check all compressible chunk ids, start with upper case 'S'
SLANG_COMPILE_TIME_ASSERT(SLANG_FOUR_CC_GET_FIRST_CHAR(IRSerialBinary::kInstFourCc) == 'S');
SLANG_COMPILE_TIME_ASSERT(SLANG_FOUR_CC_GET_FIRST_CHAR(IRSerialBinary::kChildRunFourCc) == 'S');
SLANG_COMPILE_TIME_ASSERT(SLANG_FOUR_CC_GET_FIRST_CHAR(IRSerialBinary::kExternalOperandsFourCc) == 'S');

// Compressed version starts with 's'
SLANG_COMPILE_TIME_ASSERT(SLANG_FOUR_CC_GET_FIRST_CHAR(SLANG_MAKE_COMPRESSED_FOUR_CC(IRSerialBinary::kInstFourCc)) == 's');

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
    m_entries.setCount(StringSlicePool::kDefaultHandlesCount);
    SLANG_COMPILE_TIME_ASSERT(StringSlicePool::kDefaultHandlesCount == 2);

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

            m_entries.add(entry);

            cur = reader.m_pos + len;
        }
    }

    m_entries.compress();
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
        Name* name = dynamicCast<Name>(entry.m_object);
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
        Name* name = dynamicCast<Name>(entry.m_object);
        if (name)
        {
            return name->text.getStringRepresentation();
        }
        return static_cast<StringRepresentation*>(entry.m_object);
    }

    const UnownedStringSlice slice = getStringSlice(handle);
    const UInt size = slice.getLength();

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
    return encodeStringTable(pool.getAdded(), stringTable);
}
    
/* static */void SerialStringTableUtil::encodeStringTable(const ConstArrayView<UnownedStringSlice>& slices, List<char>& stringTable)
{
    stringTable.clear();
    for (const auto& slice : slices)
    {
        const int len = int(slice.getLength());
        
        // We need to write into the the string array
        char prefixBytes[6];
        const int numPrefixBytes = EncodeUnicodePointToUTF8(prefixBytes, len);
        const Index baseIndex = stringTable.getCount();

        stringTable.setCount(baseIndex + numPrefixBytes + len);

        char* dst = stringTable.begin() + baseIndex;

        memcpy(dst, prefixBytes, numPrefixBytes);
        memcpy(dst + numPrefixBytes, slice.begin(), len);   
    }
}

/* static */void SerialStringTableUtil::appendDecodedStringTable(const List<char>& stringTable, List<UnownedStringSlice>& slicesOut)
{
    const char* start = stringTable.begin();
    const char* cur = start;
    const char* end = stringTable.end();

    while (cur < end)
    {
        CharReader reader(cur);
        const int len = GetUnicodePointFromUTF8(reader);
        slicesOut.add(UnownedStringSlice(reader.m_pos, len));
        cur = reader.m_pos + len;
    }
}

/* static */void SerialStringTableUtil::decodeStringTable(const List<char>& stringTable, List<UnownedStringSlice>& slicesOut)
{
    slicesOut.setCount(2);
    slicesOut[0] = UnownedStringSlice(nullptr, size_t(0));
    slicesOut[1] = UnownedStringSlice("", size_t(0));

    appendDecodedStringTable(stringTable, slicesOut);
}

/* static */void SerialStringTableUtil::calcStringSlicePoolMap(const List<UnownedStringSlice>& slices, StringSlicePool& pool, List<StringSlicePool::Handle>& indexMapOut)
{
    SLANG_ASSERT(slices.getCount() >= StringSlicePool::kDefaultHandlesCount);
    SLANG_ASSERT(slices[int(StringSlicePool::kNullHandle)] == "" && slices[int(StringSlicePool::kNullHandle)].begin() == nullptr);
    SLANG_ASSERT(slices[int(StringSlicePool::kEmptyHandle)] == "");

    indexMapOut.setCount(slices.getCount());
    // Set up all of the defaults
    for (int i = 0; i < StringSlicePool::kDefaultHandlesCount; ++i)
    {
        indexMapOut[i] = StringSlicePool::Handle(i);
    }

    const Index numSlices = slices.getCount();
    for (Index i = StringSlicePool::kDefaultHandlesCount; i < numSlices ; ++i)
    {
        indexMapOut[i] = pool.add(slices[i]);
    }
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! IRSerialData !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

template<typename T>
static size_t _calcArraySize(const List<T>& list)
{
    return list.getCount() * sizeof(T);
}

size_t IRSerialData::calcSizeInBytes() const
{
    return
        _calcArraySize(m_insts) +
        _calcArraySize(m_childRuns) +
        _calcArraySize(m_externalOperands) +
        _calcArraySize(m_stringTable) +
        /* Raw source locs */
        _calcArraySize(m_rawSourceLocs) +
        /* Debug */
        _calcArraySize(m_debugStringTable) +
        _calcArraySize(m_debugLineInfos) +
        _calcArraySize(m_debugSourceInfos) +
        _calcArraySize(m_debugAdjustedLineInfos) +
        _calcArraySize(m_debugSourceLocRuns);
}

IRSerialData::IRSerialData()
{
    clear();
}

void IRSerialData::clear()
{
    // First Instruction is null
    m_insts.setCount(1);
    memset(&m_insts[0], 0, sizeof(Inst));

    m_childRuns.clear();
    m_externalOperands.clear();
    m_rawSourceLocs.clear();

    m_stringTable.clear();
    
    // Debug data
    m_debugLineInfos.clear();
    m_debugAdjustedLineInfos.clear();
    m_debugSourceInfos.clear();
    m_debugSourceLocRuns.clear();
    m_debugStringTable.clear();
}

template <typename T>
static bool _isEqual(const List<T>& aIn, const List<T>& bIn)
{
    if (aIn.getCount() != bIn.getCount())
    {
        return false;
    }

    size_t size = size_t(aIn.getCount());

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
        (_isEqual(m_insts, rhs.m_insts) &&
        _isEqual(m_childRuns, rhs.m_childRuns) &&
        _isEqual(m_externalOperands, rhs.m_externalOperands) &&
        _isEqual(m_rawSourceLocs, rhs.m_rawSourceLocs) &&
        _isEqual(m_stringTable, rhs.m_stringTable) &&
        /* Debug */
        _isEqual(m_debugStringTable, rhs.m_debugStringTable) &&
        _isEqual(m_debugLineInfos, rhs.m_debugLineInfos) &&
        _isEqual(m_debugAdjustedLineInfos, rhs.m_debugAdjustedLineInfos) &&
        _isEqual(m_debugSourceInfos, rhs.m_debugSourceInfos) &&
        _isEqual(m_debugSourceLocRuns, rhs.m_debugSourceLocRuns));
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!! IRSerialTypeUtil !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

#define SLANG_SERIAL_BINARY_COMPRESSION_TYPE(x) \
    x(None, none) \
    x(VariableByteLite, lite)

/* static */SlangResult IRSerialTypeUtil::parseCompressionType(const UnownedStringSlice& text, IRSerialCompressionType& outType)
{
    struct Pair
    {
        UnownedStringSlice name;
        IRSerialCompressionType type;
    };

#define SLANG_SERIAL_BINARY_PAIR(type, name) { UnownedStringSlice::fromLiteral(#name), IRSerialCompressionType::type},

    static const Pair s_pairs[] = {
        SLANG_SERIAL_BINARY_COMPRESSION_TYPE(SLANG_SERIAL_BINARY_PAIR)
    };

    for (const auto& pair : s_pairs)
    {
        if (pair.name == text)
        {
            outType = pair.type;
            return SLANG_OK;
        }
    }
    return SLANG_FAIL;
}

/* static */UnownedStringSlice IRSerialTypeUtil::getText(IRSerialCompressionType type)
{
#define SLANG_SERIAL_BINARY_CASE(type, name) case IRSerialCompressionType::type: return UnownedStringSlice::fromLiteral(#name);
    switch (type)
    {
        SLANG_SERIAL_BINARY_COMPRESSION_TYPE(SLANG_SERIAL_BINARY_CASE)
        default: break;
    }

    SLANG_ASSERT(!"Unknown compression type");
    return UnownedStringSlice::fromLiteral("unknown");
}

} // namespace Slang
