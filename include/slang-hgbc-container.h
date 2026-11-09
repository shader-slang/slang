// slang-hgbc-container.h
#pragma once

#include "slang-hgbc-format.h"

#include <algorithm>
#include <assert.h>
#include <stdint.h>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace Slang
{
namespace HGBC
{

template<typename T>
struct Vector : public std::vector<T>
{
    using Base = std::vector<T>;
    using Base::Base;

    size_t getCount() const { return this->size(); }
    T* getBuffer() { return this->empty() ? nullptr : this->data(); }
    const T* getBuffer() const { return this->empty() ? nullptr : this->data(); }

    void add(const T& value) { this->push_back(value); }
    void add(T&& value) { this->push_back(std::move(value)); }

    void addRange(const T* data, size_t count)
    {
        if (count)
            this->insert(this->end(), data, data + count);
    }
};

struct DebugInfo
{
    HGBCSectionTag section = HGBCSectionTag::FUNC;
    size_t offset = 0;
    uint32_t sectionOffset = 0;
};

struct Record
{
    DebugInfo debugInfo;
    HGBCOpcode opcode = HGBCOpcode::Return;
    uint8_t extension = 0;
    Vector<uint32_t> operands;
    Vector<uint8_t> payload;
};

struct Section
{
    HGBCSectionTag tag = HGBCSectionTag::FUNC;
    uint32_t offset = 0;
    uint32_t size = 0;
    Vector<size_t> recordIndices;
};

struct Block
{
    HGBCId id = 0;
    size_t recordIndex = 0;
    size_t endRecordIndex = 0;
    Vector<size_t> recordIndices;
};

struct Function
{
    HGBCId id = 0;
    HGBCId typeId = 0;
    size_t recordIndex = 0;
    size_t endRecordIndex = 0;
    Vector<Block> blocks;
    std::unordered_map<HGBCId, size_t> blockIndices;
};

struct BasicIndex
{
    std::unordered_map<HGBCId, size_t> resultRecords;
    std::unordered_map<HGBCId, std::string> strings;
    std::unordered_set<HGBCId> uintTypeIds;
    std::unordered_set<HGBCId> intTypeIds;
    // Floating-point categories use `floatTypeIds`; subsets retain width-specific identity where
    // validation or execution needs the physical representation.
    std::unordered_set<HGBCId> float16TypeIds;
    std::unordered_set<HGBCId> float64TypeIds;
    std::unordered_set<HGBCId> floatTypeIds;
    std::unordered_set<HGBCId> boolTypeIds;
    std::unordered_set<HGBCId> stringTypeIds;
    std::unordered_set<HGBCId> timerTypeIds;
    std::unordered_map<HGBCId, size_t> structTypeRecordIndices;
    std::unordered_map<HGBCId, size_t> classTypeRecordIndices;
    std::unordered_map<HGBCId, size_t> functionRecordIndices;
    std::unordered_map<HGBCId, size_t> functionIndices;
    std::unordered_map<std::string, HGBCId> exports;
    std::unordered_map<std::string, HGBCId> typeExports;
    std::unordered_map<HGBCId, Vector<HGBCId>> constructorExports;
    std::unordered_map<HGBCId, std::unordered_map<std::string, Vector<HGBCId>>> methodExports;
    std::unordered_map<HGBCId, std::unordered_map<std::string, Vector<HGBCId>>> staticMethodExports;

    void clear()
    {
        resultRecords.clear();
        strings.clear();
        uintTypeIds.clear();
        intTypeIds.clear();
        float16TypeIds.clear();
        float64TypeIds.clear();
        floatTypeIds.clear();
        boolTypeIds.clear();
        stringTypeIds.clear();
        timerTypeIds.clear();
        structTypeRecordIndices.clear();
        classTypeRecordIndices.clear();
        functionRecordIndices.clear();
        functionIndices.clear();
        exports.clear();
        typeExports.clear();
        constructorExports.clear();
        methodExports.clear();
        staticMethodExports.clear();
    }
};

struct StructFieldDescriptor
{
    std::string name;
    HGBCId typeId = 0;
};

struct StructTypeDescriptor
{
    HGBCId typeId = 0;
    std::string name;
    Vector<StructFieldDescriptor> fields;
    std::unordered_map<std::string, size_t> fieldIndices;
};

using ClassTypeDescriptor = StructTypeDescriptor;

/// Identifies one possible payload of an opaque tagged value. Void denotes a payload-free case.
struct TaggedUnionCaseDescriptor
{
    uint32_t caseTag = 0;
    HGBCId payloadTypeId = 0;
};

/// Preserves case identity independently of payload layout and of the order of case declarations.
struct TaggedUnionTypeDescriptor
{
    HGBCId typeId = 0;
    Vector<TaggedUnionCaseDescriptor> cases;
    std::unordered_map<uint32_t, size_t> caseIndices;
};

/// Records the storage size attached to an HGBC type declaration.
///
/// This is distinct from `OpTypeLayout`: `OpTypeSize` says that an HGBC array occupies a fixed
/// number of bytes, while `OpTypeLayout` describes that logical array in a particular external
/// domain such as structured-buffer memory.
struct TypeSizeDescriptor
{
    HGBCId typeId = 0;
    uint32_t byteSize = 0;
    uint32_t byteAlignment = 1;
    uint32_t byteStride = 0;
};

struct SourceSpan
{
    HGBCId sourceId = 0;
    uint32_t lineStart = 0;
    uint32_t lineEnd = 0;
    uint32_t columnStart = 0;
    uint32_t columnEnd = 0;

    bool isValid() const { return sourceId != 0; }
};

struct Program
{
    Vector<Section> sections;
    Vector<Record> records;
    Vector<Function> functions;
    BasicIndex basicIndex;
    std::unordered_map<HGBCId, std::unordered_map<uint32_t, Vector<size_t>>>
        decorationRecordIndices;
    Vector<StructTypeDescriptor> structTypes;
    std::unordered_map<HGBCId, size_t> structTypeIndices;
    Vector<ClassTypeDescriptor> classTypes;
    std::unordered_map<HGBCId, size_t> classTypeIndices;
    Vector<TaggedUnionTypeDescriptor> taggedUnionTypes;
    std::unordered_map<HGBCId, size_t> taggedUnionTypeIndices;
    Vector<TypeSizeDescriptor> typeSizes;
    std::unordered_map<HGBCId, size_t> typeSizeIndices;

    void clear()
    {
        sections.clear();
        records.clear();
        functions.clear();
        basicIndex.clear();
        decorationRecordIndices.clear();
        structTypes.clear();
        structTypeIndices.clear();
        classTypes.clear();
        classTypeIndices.clear();
        taggedUnionTypes.clear();
        taggedUnionTypeIndices.clear();
        typeSizes.clear();
        typeSizeIndices.clear();
    }
};

inline uint32_t alignToFour(uint32_t value)
{
    return (value + 3) & ~uint32_t(3);
}

inline bool fail(std::string* outError, const char* message)
{
    if (outError)
        *outError = message;
    return false;
}

inline bool fail(std::string* outError, const std::string& message)
{
    if (outError)
        *outError = message;
    return false;
}

inline bool isKnownOpcode(HGBCOpcode opcode)
{
    return getHGBCOpcodeInfo(opcode) != nullptr;
}

inline int getResultOperandIndex(HGBCOpcode opcode)
{
    return getHGBCResultOperandIndex(opcode);
}

inline int getTypeOperandIndex(HGBCOpcode opcode)
{
    return getHGBCTypeOperandIndex(opcode);
}

inline int getPayloadSizeOperandIndex(HGBCOpcode opcode)
{
    return getHGBCPayloadSizeOperandIndex(opcode);
}

inline bool hasTypeOperand(HGBCOpcode opcode)
{
    return getTypeOperandIndex(opcode) >= 0;
}
inline HGBCId getResultId(const Record& record)
{
    int resultIndex = getResultOperandIndex(record.opcode);
    if (resultIndex < 0 || record.operands.size() <= size_t(resultIndex))
        return 0;
    return record.operands[size_t(resultIndex)];
}

inline bool getPayloadSize(const Record& record, uint32_t& outPayloadSize, std::string* outError)
{
    outPayloadSize = 0;

    int payloadSizeIndex = getPayloadSizeOperandIndex(record.opcode);
    if (payloadSizeIndex < 0)
        return true;

    if (record.operands.size() <= size_t(payloadSizeIndex))
        return fail(outError, "HGBC payload record is missing its byte-count operand");

    outPayloadSize = record.operands[size_t(payloadSizeIndex)];
    return true;
}

inline bool canRead(size_t dataSize, size_t offset, size_t byteCount)
{
    return offset <= dataSize && byteCount <= dataSize - offset;
}

inline bool readU16(const uint8_t* data, size_t dataSize, size_t& offset, uint16_t& outValue)
{
    if (!canRead(dataSize, offset, sizeof(uint16_t)))
        return false;

    outValue = uint16_t(uint32_t(data[offset]) | (uint32_t(data[offset + 1]) << 8));
    offset += sizeof(uint16_t);
    return true;
}

inline bool readU32(const uint8_t* data, size_t dataSize, size_t& offset, uint32_t& outValue)
{
    if (!canRead(dataSize, offset, sizeof(uint32_t)))
        return false;

    outValue = uint32_t(data[offset]) | (uint32_t(data[offset + 1]) << 8) |
               (uint32_t(data[offset + 2]) << 16) | (uint32_t(data[offset + 3]) << 24);
    offset += sizeof(uint32_t);
    return true;
}

inline uint32_t readU32At(const uint8_t* data, size_t offset)
{
    return uint32_t(data[offset]) | (uint32_t(data[offset + 1]) << 8) |
           (uint32_t(data[offset + 2]) << 16) | (uint32_t(data[offset + 3]) << 24);
}

inline void appendU32(Vector<uint8_t>& out, uint32_t value)
{
    out.add(uint8_t(value & 0xff));
    out.add(uint8_t((value >> 8) & 0xff));
    out.add(uint8_t((value >> 16) & 0xff));
    out.add(uint8_t((value >> 24) & 0xff));
}

inline void alignToFourBytes(Vector<uint8_t>& out)
{
    while ((out.size() & 3) != 0)
        out.add(0);
}

inline Section* findSection(Program& program, HGBCSectionTag tag)
{
    for (auto& section : program.sections)
    {
        if (section.tag == tag)
            return &section;
    }
    return nullptr;
}

inline Record& addRecord(
    Program& program,
    HGBCOpcode opcode,
    const uint32_t* operands,
    size_t operandCount,
    const void* payload = nullptr,
    size_t payloadSize = 0)
{
    const HGBCOpcodeInfo* opcodeInfo = getHGBCOpcodeInfo(opcode);
    assert(opcodeInfo);
    assert(isValidHGBCOperandCount(*opcodeInfo, operandCount));
    assert(opcodeInfo->payloadSizeOperandIndex >= 0 || (payload == nullptr && payloadSize == 0));
    if (opcodeInfo->payloadSizeOperandIndex >= 0)
    {
        assert(size_t(opcodeInfo->payloadSizeOperandIndex) < operandCount);
        assert(operands[opcodeInfo->payloadSizeOperandIndex] == payloadSize);
    }
    HGBCSectionTag sectionTag = opcodeInfo->section;

    Section* section = findSection(program, sectionTag);
    if (!section)
    {
        Section newSection;
        newSection.tag = sectionTag;
        program.sections.add(std::move(newSection));
        section = &program.sections.back();
    }

    Record record;
    record.debugInfo.section = sectionTag;
    record.opcode = opcode;
    if (operandCount)
        record.operands.addRange(operands, operandCount);
    if (payloadSize)
    {
        const auto* payloadBytes = static_cast<const uint8_t*>(payload);
        record.payload.addRange(payloadBytes, payloadSize);
    }

    size_t recordIndex = program.records.size();
    program.records.add(std::move(record));
    section->recordIndices.add(recordIndex);
    return program.records.back();
}

inline bool buildFunctionBlocks(Program& program, std::string* outError = nullptr)
{
    program.functions.clear();

    Function* currentFunction = nullptr;
    Block* currentBlock = nullptr;

    for (size_t recordIndex = 0; recordIndex < program.records.size(); recordIndex++)
    {
        const Record& record = program.records[recordIndex];
        if (record.debugInfo.section != HGBCSectionTag::FUNC)
            continue;

        switch (record.opcode)
        {
        case HGBCOpcode::Func:
            {
                if (currentBlock)
                    return fail(outError, "HGBC OpFunc appears before the current block is closed");
                if (currentFunction)
                    return fail(
                        outError,
                        "HGBC OpFunc appears before the current function is closed");
                if (record.operands.size() < 2)
                    return fail(outError, "HGBC OpFunc is missing operands");

                Function function;
                function.typeId = record.operands[0];
                function.id = record.operands[1];
                function.recordIndex = recordIndex;
                program.functions.add(std::move(function));
                currentFunction = &program.functions.back();
                break;
            }

        case HGBCOpcode::Block:
            {
                if (!currentFunction)
                    return fail(outError, "HGBC OpBlock appears outside a function");
                if (currentBlock)
                    return fail(
                        outError,
                        "HGBC OpBlock appears before the current block is closed");
                if (record.operands.size() < 1)
                    return fail(outError, "HGBC OpBlock is missing its block id");

                HGBCId blockId = record.operands[0];
                if (currentFunction->blockIndices.find(blockId) !=
                    currentFunction->blockIndices.end())
                    return fail(outError, "HGBC function defines a block id more than once");

                Block block;
                block.id = blockId;
                block.recordIndex = recordIndex;
                size_t blockIndex = currentFunction->blocks.size();
                currentFunction->blocks.add(std::move(block));
                currentFunction->blockIndices.emplace(blockId, blockIndex);
                currentBlock = &currentFunction->blocks.back();
                break;
            }

        case HGBCOpcode::BlockEnd:
            if (!currentBlock)
                return fail(outError, "HGBC OpBlockEnd appears outside a block");
            currentBlock->endRecordIndex = recordIndex;
            currentBlock = nullptr;
            break;

        case HGBCOpcode::FunctionEnd:
            if (!currentFunction)
                return fail(outError, "HGBC OpFunctionEnd appears outside a function");
            if (currentBlock)
                return fail(
                    outError,
                    "HGBC OpFunctionEnd appears before the current block is closed");
            currentFunction->endRecordIndex = recordIndex;
            currentFunction = nullptr;
            break;

        default:
            if (!currentBlock)
                return fail(outError, "HGBC function instruction appears outside a block");
            currentBlock->recordIndices.add(recordIndex);
            break;
        }
    }

    if (currentBlock)
        return fail(outError, "HGBC block is missing OpBlockEnd");
    if (currentFunction)
        return fail(outError, "HGBC function is missing OpFunctionEnd");

    return true;
}

inline bool buildBasicIndex(
    const Program& program,
    BasicIndex& outIndex,
    std::string* outError = nullptr);
inline bool buildDecorationIndex(Program& program, std::string* outError = nullptr);
inline bool buildTypeSizeDescriptors(Program& program, std::string* outError = nullptr);
inline bool buildStructTypeDescriptors(Program& program, std::string* outError = nullptr);
inline bool buildTaggedUnionTypeDescriptors(Program& program, std::string* outError = nullptr);
inline bool buildClassTypeDescriptors(Program& program, std::string* outError = nullptr);

inline bool readProgram(
    const void* inputData,
    size_t inputSize,
    Program& outProgram,
    std::string* outError = nullptr)
{
    outProgram.clear();

    const auto* data = static_cast<const uint8_t*>(inputData);
    if (!canRead(inputSize, 0, 8))
        return fail(outError, "HGBC file is truncated before the header");

    size_t cursor = 0;
    uint32_t magic = 0;
    uint32_t sectionCount = 0;
    if (!readU32(data, inputSize, cursor, magic) || !readU32(data, inputSize, cursor, sectionCount))
    {
        return fail(outError, "HGBC file is truncated before the header");
    }

    if (magic != kHGBCMagic)
        return fail(outError, "input is not an HGBC binary");

    if (!canRead(inputSize, cursor, 8))
        return fail(outError, "HGBC binary is missing its offset table");

    uint32_t ofstTag = 0;
    uint32_t ofstSize = 0;
    if (!readU32(data, inputSize, cursor, ofstTag) || !readU32(data, inputSize, cursor, ofstSize))
        return fail(outError, "HGBC binary has a truncated offset table");

    if (ofstTag != uint32_t(HGBCSectionTag::OFST))
        return fail(outError, "HGBC binary is missing its offset table");

    if (ofstSize != sectionCount * 12)
        return fail(outError, "HGBC offset table has an unexpected size");

    if (!canRead(inputSize, cursor, ofstSize))
        return fail(outError, "HGBC offset table is truncated");

    for (uint32_t i = 0; i < sectionCount; i++)
    {
        uint32_t rawTag = 0;
        uint32_t sectionOffset = 0;
        uint32_t sectionSize = 0;
        if (!readU32(data, inputSize, cursor, rawTag) ||
            !readU32(data, inputSize, cursor, sectionOffset) ||
            !readU32(data, inputSize, cursor, sectionSize))
        {
            return fail(outError, "HGBC offset table is truncated");
        }

        if (!canRead(inputSize, sectionOffset, 8) ||
            !canRead(inputSize, size_t(sectionOffset) + 8, sectionSize))
        {
            return fail(outError, "HGBC section is outside the file");
        }

        uint32_t actualTag = readU32At(data, sectionOffset);
        uint32_t actualSize = readU32At(data, size_t(sectionOffset) + 4);
        if (actualTag != rawTag || actualSize != sectionSize)
            return fail(outError, "HGBC section offset table does not match section header");

        Section section;
        section.tag = HGBCSectionTag(rawTag);
        section.offset = sectionOffset;
        section.size = sectionSize;
        outProgram.sections.add(std::move(section));
    }

    for (auto& section : outProgram.sections)
    {
        size_t sectionStart = size_t(section.offset) + 8;
        size_t sectionEnd = sectionStart + section.size;
        cursor = sectionStart;

        while (cursor < sectionEnd)
        {
            size_t recordStart = cursor;
            if (!canRead(inputSize, cursor, sizeof(HGBCInstHeader)) ||
                cursor + sizeof(HGBCInstHeader) > sectionEnd)
            {
                return fail(outError, "truncated HGBC instruction header");
            }

            uint16_t rawOpcode = 0;
            if (!readU16(data, inputSize, cursor, rawOpcode))
                return fail(outError, "truncated HGBC instruction header");
            if (!canRead(inputSize, cursor, 2))
                return fail(outError, "truncated HGBC instruction header");

            HGBCOpcode opcode = HGBCOpcode(rawOpcode);
            uint8_t extension = data[cursor++];
            uint8_t operandCount = data[cursor++];

            if (!isKnownOpcode(opcode))
                return fail(outError, "unknown HGBC opcode");

            const HGBCOpcodeInfo* opcodeInfo = getHGBCOpcodeInfo(opcode);
            if (opcodeInfo->section != section.tag)
                return fail(outError, "HGBC opcode appears in the wrong section");
            if (!isValidHGBCOperandCount(*opcodeInfo, operandCount))
                return fail(outError, "HGBC instruction has an invalid operand count");

            if (!canRead(inputSize, cursor, size_t(operandCount) * sizeof(uint32_t)) ||
                cursor + size_t(operandCount) * sizeof(uint32_t) > sectionEnd)
            {
                return fail(outError, "truncated HGBC instruction operands");
            }

            Record record;
            record.debugInfo.section = section.tag;
            record.debugInfo.offset = recordStart;
            record.debugInfo.sectionOffset = uint32_t(recordStart - sectionStart);
            record.opcode = opcode;
            record.extension = extension;
            record.operands.reserve(operandCount);

            for (uint8_t i = 0; i < operandCount; i++)
            {
                uint32_t operand = 0;
                if (!readU32(data, inputSize, cursor, operand))
                    return fail(outError, "truncated HGBC instruction operands");
                record.operands.add(operand);
            }

            uint32_t payloadSize = 0;
            if (!getPayloadSize(record, payloadSize, outError))
                return false;

            if (!canRead(inputSize, cursor, payloadSize) || cursor + payloadSize > sectionEnd)
                return fail(outError, "truncated HGBC instruction payload");

            if (payloadSize)
                record.payload.addRange(data + cursor, payloadSize);
            cursor += payloadSize;

            size_t alignedCursor = sectionStart + alignToFour(uint32_t(cursor - sectionStart));
            if (alignedCursor > sectionEnd)
                return fail(outError, "HGBC instruction padding exceeds section size");
            cursor = alignedCursor;

            size_t recordIndex = outProgram.records.size();
            outProgram.records.add(std::move(record));
            section.recordIndices.add(recordIndex);
        }
    }

    if (!buildFunctionBlocks(outProgram, outError))
        return false;
    if (!buildBasicIndex(outProgram, outProgram.basicIndex, outError))
        return false;
    if (!buildDecorationIndex(outProgram, outError))
        return false;
    if (!buildTypeSizeDescriptors(outProgram, outError))
        return false;
    if (!buildStructTypeDescriptors(outProgram, outError))
        return false;
    if (!buildClassTypeDescriptors(outProgram, outError))
        return false;
    if (!buildTaggedUnionTypeDescriptors(outProgram, outError))
        return false;
    return true;
}

inline bool writeRecordPayload(const Record& record, Vector<uint8_t>& out, std::string* outError)
{
    const HGBCOpcodeInfo* opcodeInfo = getHGBCOpcodeInfo(record.opcode);
    if (!opcodeInfo)
        return fail(outError, "unknown HGBC opcode");
    if (!isValidHGBCOperandCount(*opcodeInfo, record.operands.size()))
        return fail(outError, "HGBC record has an invalid operand count");

    int payloadSizeIndex = opcodeInfo->payloadSizeOperandIndex;
    if (payloadSizeIndex < 0 && !record.payload.empty())
        return fail(outError, "HGBC record has a payload but its opcode does not allow one");

    if (payloadSizeIndex >= 0)
    {
        if (record.operands.size() <= size_t(payloadSizeIndex))
            return fail(outError, "HGBC payload record is missing its byte-count operand");
        if (record.operands[size_t(payloadSizeIndex)] != record.payload.size())
            return fail(outError, "HGBC payload byte count does not match its payload size");
    }

    appendU32(
        out,
        uint32_t(uint16_t(record.opcode)) | (uint32_t(record.extension) << 16) |
            (uint32_t(record.operands.size()) << 24));
    for (auto operand : record.operands)
        appendU32(out, operand);
    if (!record.payload.empty())
        out.addRange(record.payload.data(), record.payload.size());
    alignToFourBytes(out);
    return true;
}

inline bool writeProgram(
    const Program& program,
    Vector<uint8_t>& outBytes,
    std::string* outError = nullptr)
{
    outBytes.clear();

    struct EncodedSection
    {
        HGBCSectionTag tag = HGBCSectionTag::FUNC;
        Vector<uint8_t> payload;
    };

    Vector<EncodedSection> encodedSections;
    for (const auto& section : program.sections)
    {
        EncodedSection encodedSection;
        encodedSection.tag = section.tag;

        for (auto recordIndex : section.recordIndices)
        {
            if (recordIndex >= program.records.size())
                return fail(outError, "HGBC section references a missing record");

            const Record& record = program.records[recordIndex];
            const HGBCOpcodeInfo* opcodeInfo = getHGBCOpcodeInfo(record.opcode);
            if (!opcodeInfo)
                return fail(outError, "unknown HGBC opcode");
            if (opcodeInfo->section != section.tag)
                return fail(outError, "HGBC record is stored in the wrong section");

            if (!writeRecordPayload(record, encodedSection.payload, outError))
                return false;
        }

        if (!encodedSection.payload.empty())
            encodedSections.add(std::move(encodedSection));
    }

    const uint32_t headerSize = 8;
    const uint32_t sectionHeaderSize = 8;
    const uint32_t ofstEntrySize = 12;
    const uint32_t ofstPayloadSize = uint32_t(encodedSections.size() * ofstEntrySize);
    const uint32_t ofstSectionSize = sectionHeaderSize + ofstPayloadSize;

    Vector<uint32_t> sectionOffsets;
    uint32_t nextOffset = headerSize + ofstSectionSize;
    for (auto& section : encodedSections)
    {
        sectionOffsets.add(nextOffset);
        nextOffset += sectionHeaderSize + uint32_t(section.payload.size());
    }

    appendU32(outBytes, kHGBCMagic);
    appendU32(outBytes, uint32_t(encodedSections.size()));

    appendU32(outBytes, uint32_t(HGBCSectionTag::OFST));
    appendU32(outBytes, ofstPayloadSize);
    for (size_t i = 0; i < encodedSections.size(); i++)
    {
        appendU32(outBytes, uint32_t(encodedSections[i].tag));
        appendU32(outBytes, sectionOffsets[i]);
        appendU32(outBytes, uint32_t(encodedSections[i].payload.size()));
    }

    for (auto& section : encodedSections)
    {
        appendU32(outBytes, uint32_t(section.tag));
        appendU32(outBytes, uint32_t(section.payload.size()));
        outBytes.addRange(section.payload.data(), section.payload.size());
    }

    return true;
}

inline bool buildBasicIndex(const Program& program, BasicIndex& outIndex, std::string* outError)
{
    outIndex.clear();

    for (size_t i = 0; i < program.records.size(); i++)
    {
        const Record& record = program.records[i];
        int resultIndex = getResultOperandIndex(record.opcode);
        if (resultIndex >= 0 && record.operands.size() > size_t(resultIndex))
        {
            HGBCId resultId = record.operands[size_t(resultIndex)];
            if (!outIndex.resultRecords.emplace(resultId, i).second)
                return fail(outError, "HGBC defines a result id more than once");
        }

        switch (record.opcode)
        {
        case HGBCOpcode::StringLit:
            if (record.operands.size() < 3)
                return fail(outError, "HGBC OpStringLit is missing operands");
            outIndex.strings.emplace(
                record.operands[1],
                std::string(record.payload.begin(), record.payload.end()));
            break;

        case HGBCOpcode::UIntType:
            if (record.operands.size() < 1)
                return fail(outError, "HGBC OpUIntType is missing its result id");
            outIndex.uintTypeIds.emplace(record.operands[0]);
            break;

        case HGBCOpcode::IntType:
            if (record.operands.size() < 1)
                return fail(outError, "HGBC OpIntType is missing its result id");
            outIndex.intTypeIds.emplace(record.operands[0]);
            break;

        case HGBCOpcode::Float32Type:
            if (record.operands.size() < 1)
                return fail(outError, "HGBC OpFloat32Type is missing its result id");
            outIndex.floatTypeIds.emplace(record.operands[0]);
            break;

        case HGBCOpcode::Float64Type:
            if (record.operands.size() < 1)
                return fail(outError, "HGBC OpFloat64Type is missing its result id");
            outIndex.float64TypeIds.emplace(record.operands[0]);
            outIndex.floatTypeIds.emplace(record.operands[0]);
            break;

        case HGBCOpcode::Float16Type:
            if (record.operands.size() < 1)
                return fail(outError, "HGBC OpFloat16Type is missing its result id");
            outIndex.float16TypeIds.emplace(record.operands[0]);
            outIndex.floatTypeIds.emplace(record.operands[0]);
            break;

        case HGBCOpcode::BoolType:
            if (record.operands.size() < 1)
                return fail(outError, "HGBC OpBoolType is missing its result id");
            outIndex.boolTypeIds.emplace(record.operands[0]);
            break;

        case HGBCOpcode::StringType:
            if (record.operands.size() < 1)
                return fail(outError, "HGBC OpStringType is missing its result id");
            outIndex.stringTypeIds.emplace(record.operands[0]);
            break;

        case HGBCOpcode::TimerType:
            if (record.operands.size() < 1)
                return fail(outError, "HGBC OpTimerType is missing its result id");
            outIndex.timerTypeIds.emplace(record.operands[0]);
            break;

        case HGBCOpcode::StructType:
            if (record.operands.size() < 1)
                return fail(outError, "HGBC OpStructType is missing its result id");
            outIndex.structTypeRecordIndices.emplace(record.operands[0], i);
            break;

        case HGBCOpcode::ClassType:
            if (record.operands.size() < 1)
                return fail(outError, "HGBC OpClassType is missing its result id");
            outIndex.classTypeRecordIndices.emplace(record.operands[0], i);
            break;

        case HGBCOpcode::Func:
            if (record.operands.size() < 2)
                return fail(outError, "HGBC OpFunc is missing operands");
            outIndex.functionRecordIndices.emplace(record.operands[1], i);
            break;

        default:
            break;
        }
    }

    for (size_t i = 0; i < program.functions.size(); i++)
    {
        const Function& function = program.functions[i];
        if (!outIndex.functionIndices.emplace(function.id, i).second)
            return fail(outError, "HGBC defines a function id more than once");
    }

    for (const Record& record : program.records)
    {
        if (record.opcode != HGBCOpcode::Export)
            continue;

        if (record.operands.size() < 4)
            return fail(outError, "HGBC OpExport is missing operands");

        HGBCExportKind exportKind = HGBCExportKind(record.operands[1]);
        HGBCId nameId = record.operands[2];
        HGBCId targetId = record.operands[3];

        auto stringIt = outIndex.strings.find(nameId);
        if (stringIt == outIndex.strings.end())
            return fail(outError, "HGBC OpExport references a missing string literal");

        switch (exportKind)
        {
        case HGBCExportKind::Function:
            if (record.operands.size() != 4)
                return fail(outError, "HGBC function OpExport has an unexpected owner type");
            if (outIndex.functionRecordIndices.find(targetId) ==
                outIndex.functionRecordIndices.end())
                return fail(outError, "HGBC OpExport references a missing function");

            if (!outIndex.exports.emplace(stringIt->second, targetId).second)
                return fail(outError, "HGBC function export is defined more than once");
            break;

        case HGBCExportKind::Type:
            if (record.operands.size() != 4)
                return fail(outError, "HGBC type OpExport has an unexpected owner type");
            if (outIndex.structTypeRecordIndices.find(targetId) ==
                    outIndex.structTypeRecordIndices.end() &&
                outIndex.classTypeRecordIndices.find(targetId) ==
                    outIndex.classTypeRecordIndices.end())
                return fail(outError, "HGBC OpExport references a missing aggregate type");

            if (!outIndex.typeExports.emplace(stringIt->second, targetId).second)
                return fail(outError, "HGBC type export is defined more than once");
            break;

        case HGBCExportKind::Constructor:
        case HGBCExportKind::Method:
        case HGBCExportKind::StaticMethod:
            {
                if (record.operands.size() != 5)
                    return fail(outError, "HGBC member OpExport is missing its owner type");
                if (outIndex.functionRecordIndices.find(targetId) ==
                    outIndex.functionRecordIndices.end())
                    return fail(outError, "HGBC member OpExport references a missing function");

                HGBCId ownerTypeId = record.operands[4];
                if (outIndex.structTypeRecordIndices.find(ownerTypeId) ==
                        outIndex.structTypeRecordIndices.end() &&
                    outIndex.classTypeRecordIndices.find(ownerTypeId) ==
                        outIndex.classTypeRecordIndices.end())
                    return fail(
                        outError,
                        "HGBC member OpExport references a missing owner aggregate type");

                const Record& functionRecord =
                    program.records[outIndex.functionRecordIndices.find(targetId)->second];
                if (functionRecord.operands.size() != 2)
                    return fail(outError, "HGBC member OpExport references a malformed function");
                auto functionTypeIt = outIndex.resultRecords.find(functionRecord.operands[0]);
                if (functionTypeIt == outIndex.resultRecords.end())
                    return fail(outError, "HGBC member OpExport function type is missing");
                const Record& functionTypeRecord = program.records[functionTypeIt->second];
                if (functionTypeRecord.opcode != HGBCOpcode::FuncType ||
                    functionTypeRecord.operands.size() < 2)
                {
                    return fail(outError, "HGBC member OpExport function type is malformed");
                }

                if (exportKind == HGBCExportKind::Constructor)
                {
                    if (functionTypeRecord.operands[1] != ownerTypeId)
                        return fail(
                            outError,
                            "HGBC constructor OpExport must return its owner type");
                    outIndex.constructorExports[ownerTypeId].add(targetId);
                }
                else if (exportKind == HGBCExportKind::Method)
                {
                    if (functionTypeRecord.operands.size() < 3)
                        return fail(
                            outError,
                            "HGBC method OpExport function is missing its receiver");
                    HGBCId receiverTypeId = functionTypeRecord.operands[2];
                    auto receiverTypeIt = outIndex.resultRecords.find(receiverTypeId);
                    if (receiverTypeIt != outIndex.resultRecords.end())
                    {
                        const Record& receiverTypeRecord = program.records[receiverTypeIt->second];
                        if (hasHGBCOpcodeFlag(
                                receiverTypeRecord.opcode,
                                HGBCOpcodeFlags::AddressType))
                        {
                            if (receiverTypeRecord.operands.size() != 2)
                                return fail(
                                    outError,
                                    "HGBC method OpExport receiver type is malformed");
                            receiverTypeId = receiverTypeRecord.operands[1];
                        }
                    }
                    if (receiverTypeId != ownerTypeId)
                        return fail(
                            outError,
                            "HGBC method OpExport receiver does not match its owner type");
                    outIndex.methodExports[ownerTypeId][stringIt->second].add(targetId);
                }
                else
                {
                    outIndex.staticMethodExports[ownerTypeId][stringIt->second].add(targetId);
                }
                break;
            }

        default:
            return fail(outError, "HGBC OpExport has an unknown export kind");
        }
    }

    return true;
}

inline bool buildDecorationIndex(Program& program, std::string* /*outError*/)
{
    program.decorationRecordIndices.clear();

    for (size_t recordIndex = 0; recordIndex < program.records.size(); recordIndex++)
    {
        const Record& record = program.records[recordIndex];
        if (record.opcode != HGBCOpcode::Decorate || record.operands.size() < 3)
            continue;

        uint32_t kind = record.operands[1];
        HGBCId targetId = record.operands[2];
        program.decorationRecordIndices[targetId][kind].add(recordIndex);
    }

    return true;
}

inline bool buildTypeSizeDescriptors(Program& program, std::string* outError)
{
    program.typeSizes.clear();
    program.typeSizeIndices.clear();

    for (const Record& record : program.records)
    {
        if (record.opcode != HGBCOpcode::TypeSize)
            continue;

        if (record.operands.size() < 5)
            return fail(outError, "HGBC OpTypeSize is missing operands");

        HGBCId typeId = record.operands[1];
        if (program.basicIndex.resultRecords.find(typeId) == program.basicIndex.resultRecords.end())
            return fail(outError, "HGBC OpTypeSize references a missing type id");

        if (record.operands[3] == 0)
            return fail(outError, "HGBC OpTypeSize byte alignment must be non-zero");

        TypeSizeDescriptor descriptor;
        descriptor.typeId = typeId;
        descriptor.byteSize = record.operands[2];
        descriptor.byteAlignment = record.operands[3];
        descriptor.byteStride = record.operands[4];

        size_t descriptorIndex = program.typeSizes.size();
        if (!program.typeSizeIndices.emplace(typeId, descriptorIndex).second)
            return fail(outError, "HGBC defines a type size more than once");
        program.typeSizes.add(std::move(descriptor));
    }

    return true;
}

inline const std::string* findStringLiteral(const Program& program, HGBCId id)
{
    auto stringIt = program.basicIndex.strings.find(id);
    if (stringIt == program.basicIndex.strings.end())
        return nullptr;
    return &stringIt->second;
}


inline const std::string* findStringDecoration(
    const Program& program,
    HGBCId targetId,
    HGBCDecorationKind kind)
{
    auto targetIt = program.decorationRecordIndices.find(targetId);
    if (targetIt == program.decorationRecordIndices.end())
        return nullptr;

    auto kindIt = targetIt->second.find(uint32_t(kind));
    if (kindIt == targetIt->second.end())
        return nullptr;

    for (size_t recordIndex : kindIt->second)
    {
        if (recordIndex >= program.records.size())
            continue;

        const Record& record = program.records[recordIndex];
        if (record.operands.size() < 4)
            continue;

        if (auto string = findStringLiteral(program, record.operands[3]))
            return string;
    }

    return nullptr;
}

inline std::string getNameHint(const Program& program, HGBCId targetId, const std::string& fallback)
{
    if (auto name = findStringDecoration(program, targetId, HGBCDecorationKind::NameHint))
        return *name;
    return fallback;
}

inline bool buildAggregateTypeDescriptors(
    Program& program,
    const std::unordered_map<HGBCId, size_t>& recordIndices,
    HGBCOpcode opcode,
    const char* opcodeName,
    const char* typeKind,
    Vector<StructTypeDescriptor>& descriptors,
    std::unordered_map<HGBCId, size_t>& descriptorIndices,
    std::string* outError)
{
    descriptors.clear();
    descriptorIndices.clear();

    for (const auto& pair : recordIndices)
    {
        HGBCId typeId = pair.first;
        size_t recordIndex = pair.second;
        if (recordIndex >= program.records.size())
            return fail(
                outError,
                std::string("HGBC ") + typeKind + " type index references a missing record");

        const Record& record = program.records[recordIndex];
        if (record.opcode != opcode)
            return fail(outError, std::string("HGBC ") + typeKind + " type index is malformed");
        if (record.operands.size() < 1)
            return fail(outError, std::string("HGBC ") + opcodeName + " is missing its result id");
        if (((record.operands.size() - 1) % 2) != 0)
            return fail(
                outError,
                std::string("HGBC ") + opcodeName + " expects field name/type operand pairs");

        StructTypeDescriptor descriptor;
        descriptor.typeId = typeId;
        descriptor.name =
            getNameHint(program, typeId, std::string("type ") + std::to_string(typeId));

        for (size_t i = 1; i + 1 < record.operands.size(); i += 2)
        {
            auto fieldName = findStringLiteral(program, record.operands[i]);
            if (!fieldName)
                return fail(
                    outError,
                    std::string("HGBC ") + opcodeName + " references a missing field name string");

            StructFieldDescriptor field;
            field.name = *fieldName;
            field.typeId = record.operands[i + 1];
            size_t fieldIndex = descriptor.fields.size();
            if (!descriptor.fieldIndices.emplace(field.name, fieldIndex).second)
                return fail(
                    outError,
                    std::string("HGBC ") + opcodeName + " defines a field more than once");
            descriptor.fields.add(std::move(field));
        }

        size_t descriptorIndex = descriptors.size();
        if (!descriptorIndices.emplace(typeId, descriptorIndex).second)
            return fail(
                outError,
                std::string("HGBC defines a ") + typeKind + " type id more than once");
        descriptors.add(std::move(descriptor));
    }

    return true;
}

inline bool buildStructTypeDescriptors(Program& program, std::string* outError)
{
    return buildAggregateTypeDescriptors(
        program,
        program.basicIndex.structTypeRecordIndices,
        HGBCOpcode::StructType,
        "OpStructType",
        "struct",
        program.structTypes,
        program.structTypeIndices,
        outError);
}

inline bool buildClassTypeDescriptors(Program& program, std::string* outError)
{
    if (!buildAggregateTypeDescriptors(
            program,
            program.basicIndex.classTypeRecordIndices,
            HGBCOpcode::ClassType,
            "OpClassType",
            "class",
            program.classTypes,
            program.classTypeIndices,
            outError))
    {
        return false;
    }

    for (const auto& pair : program.basicIndex.typeExports)
    {
        if (program.structTypeIndices.find(pair.second) == program.structTypeIndices.end() &&
            program.classTypeIndices.find(pair.second) == program.classTypeIndices.end())
        {
            return fail(
                outError,
                "HGBC type export references a missing aggregate type descriptor");
        }
    }

    return true;
}

/// Builds the authoritative case table once, rejecting ambiguous tags before any consumer uses it.
inline bool buildTaggedUnionTypeDescriptors(Program& program, std::string* outError)
{
    program.taggedUnionTypes.clear();
    program.taggedUnionTypeIndices.clear();
    for (const Record& record : program.records)
    {
        if (record.opcode != HGBCOpcode::TaggedUnionType)
            continue;
        if (record.operands.empty() || (record.operands.size() - 1) % 2 != 0)
            return fail(outError, "HGBC OpTaggedUnionType expects case tag/type operand pairs");

        TaggedUnionTypeDescriptor descriptor;
        descriptor.typeId = record.operands[0];
        for (size_t i = 1; i < record.operands.size(); i += 2)
        {
            TaggedUnionCaseDescriptor caseDescriptor;
            caseDescriptor.caseTag = record.operands[i];
            caseDescriptor.payloadTypeId = record.operands[i + 1];
            if (!descriptor.caseIndices.emplace(caseDescriptor.caseTag, descriptor.cases.size())
                     .second)
            {
                return fail(outError, "HGBC OpTaggedUnionType defines a case tag more than once");
            }
            descriptor.cases.add(caseDescriptor);
        }
        if (!program.taggedUnionTypeIndices
                 .emplace(descriptor.typeId, program.taggedUnionTypes.size())
                 .second)
        {
            return fail(outError, "HGBC defines a tagged union type id more than once");
        }
        program.taggedUnionTypes.add(std::move(descriptor));
    }
    return true;
}

inline const TaggedUnionTypeDescriptor* findTaggedUnionType(const Program& program, HGBCId typeId)
{
    auto typeIt = program.taggedUnionTypeIndices.find(typeId);
    if (typeIt == program.taggedUnionTypeIndices.end())
        return nullptr;
    return &program.taggedUnionTypes[typeIt->second];
}

inline const StructTypeDescriptor* findStructType(const Program& program, HGBCId typeId)
{
    auto typeIt = program.structTypeIndices.find(typeId);
    if (typeIt == program.structTypeIndices.end())
        return nullptr;
    if (typeIt->second >= program.structTypes.size())
        return nullptr;
    return &program.structTypes[typeIt->second];
}

inline const ClassTypeDescriptor* findClassType(const Program& program, HGBCId typeId)
{
    auto typeIt = program.classTypeIndices.find(typeId);
    if (typeIt == program.classTypeIndices.end())
        return nullptr;
    if (typeIt->second >= program.classTypes.size())
        return nullptr;
    return &program.classTypes[typeIt->second];
}

inline const StructTypeDescriptor* findStructTypeExport(
    const Program& program,
    const std::string& name)
{
    auto exportIt = program.basicIndex.typeExports.find(name);
    if (exportIt == program.basicIndex.typeExports.end())
        return nullptr;
    return findStructType(program, exportIt->second);
}

inline const TypeSizeDescriptor* findTypeSize(const Program& program, HGBCId typeId)
{
    auto typeIt = program.typeSizeIndices.find(typeId);
    if (typeIt == program.typeSizeIndices.end())
        return nullptr;
    if (typeIt->second >= program.typeSizes.size())
        return nullptr;
    return &program.typeSizes[typeIt->second];
}

} // namespace HGBC
} // namespace Slang
