// slang-serialize-ir-types.cpp
#include "slang-serialize-ir-types.h"

#include "../core/slang-text-io.h"
#include "../core/slang-byte-encode-util.h"

#include "slang-ir-insts.h"

#include "../core/slang-math.h"

namespace Slang {

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
