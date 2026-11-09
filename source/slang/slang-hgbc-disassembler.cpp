// slang-hgbc-disassembler.cpp

#include "slang-hgbc-disassembler.h"

#include "../compiler-core/slang-diagnostic-sink.h"
#include "../core/slang-blob.h"
#include "../core/slang-dictionary.h"
#include "../core/slang-writer.h"
#include "slang-hgbc.h"

namespace Slang
{
namespace
{

void diagnoseRaw(DiagnosticSink* sink, const char* message)
{
    if (sink)
        sink->diagnoseRaw(Severity::Error, message);
}

void diagnoseRaw(DiagnosticSink* sink, const String& message)
{
    if (sink)
        sink->diagnoseRaw(Severity::Error, message.getUnownedSlice());
}

bool isNameChar(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9');
}

char toLowerAscii(char c)
{
    if (c >= 'A' && c <= 'Z')
        return char(c - 'A' + 'a');
    return c;
}

char toUpperAscii(char c)
{
    if (c >= 'a' && c <= 'z')
        return char(c - 'a' + 'A');
    return c;
}

String makeValueBaseName(const String& text)
{
    StringBuilder sb;
    bool haveAny = false;
    bool lastWasSeparator = false;

    auto slice = text.getUnownedSlice();
    for (auto cursor = slice.begin(); cursor != slice.end(); ++cursor)
    {
        char c = *cursor;
        if (isNameChar(c))
        {
            if (!haveAny && c >= '0' && c <= '9')
                sb.appendChar('_');

            if (lastWasSeparator && haveAny)
                c = toUpperAscii(c);

            sb.appendChar(c);
            haveAny = true;
            lastWasSeparator = false;
        }
        else
        {
            lastWasSeparator = true;
        }
    }

    if (!haveAny)
        return "value";

    return sb.produceString();
}

String makePascalBaseName(const String& text)
{
    StringBuilder sb;
    bool nextUpper = true;

    auto slice = text.getUnownedSlice();
    for (auto cursor = slice.begin(); cursor != slice.end(); ++cursor)
    {
        char c = *cursor;
        if (!isNameChar(c))
        {
            nextUpper = true;
            continue;
        }

        if (c >= '0' && c <= '9' && sb.getLength() == 0)
            sb.appendChar('_');

        if (nextUpper)
            c = toUpperAscii(c);
        else
            c = toLowerAscii(c);

        sb.appendChar(c);
        nextUpper = false;
    }

    if (sb.getLength() == 0)
        return "Value";

    return sb.produceString();
}

bool isConciseStringLiteral(const String& text)
{
    if (text.getLength() > 80)
        return false;

    for (char c : text.getUnownedSlice())
    {
        if (c == '\r' || c == '\n')
            return false;
    }
    return true;
}

void appendEscapedString(StringBuilder& sb, const uint8_t* data, Index size)
{
    static const char hexChars[] = "0123456789abcdef";

    sb.appendChar('"');
    for (Index i = 0; i < size; i++)
    {
        uint8_t c = data[i];
        switch (c)
        {
        case '\\':
            sb << "\\\\";
            break;
        case '"':
            sb << "\\\"";
            break;
        case '\n':
            sb << "\\n";
            break;
        case '\r':
            sb << "\\r";
            break;
        case '\t':
            sb << "\\t";
            break;
        default:
            if (c >= 0x20 && c <= 0x7e)
            {
                sb.appendChar(char(c));
            }
            else
            {
                sb << "\\x";
                sb.appendChar(hexChars[c >> 4]);
                sb.appendChar(hexChars[c & 0xf]);
            }
            break;
        }
    }
    sb.appendChar('"');
}

using HGBCRecord = HGBC::Record;
using HGBCParseResult = HGBC::Program;

int getResultOperandIndex(HGBCOpcode opcode)
{
    return HGBC::getResultOperandIndex(opcode);
}

struct FriendlyIdBuilder
{
    HGBCDisassemblyOptions const& options;
    Dictionary<HGBCId, String> idNames;
    Dictionary<String, uint32_t> nameUseCounts;
    Dictionary<HGBCId, String> stringLiterals;
    Dictionary<HGBCId, HGBCId> funcResultToType;
    Dictionary<HGBCId, String> nameHints;

    FriendlyIdBuilder(HGBCDisassemblyOptions const& options)
        : options(options)
    {
    }

    String uniqueName(const String& requestedName)
    {
        uint32_t* useCount = nameUseCounts.tryGetValue(requestedName);
        if (!useCount)
        {
            nameUseCounts.add(requestedName, 1);
            return requestedName;
        }

        (*useCount)++;
        StringBuilder sb;
        sb << requestedName;
        sb << "_";
        sb << *useCount;
        return sb.produceString();
    }

    void assignName(HGBCId id, const String& baseName)
    {
        if (id == 0 || idNames.containsKey(id))
            return;

        StringBuilder prefixed;
        prefixed << "%";
        prefixed << baseName;
        idNames.add(id, uniqueName(prefixed.produceString()));
    }

    HGBCId getResultId(const HGBCRecord& record)
    {
        int resultIndex = getResultOperandIndex(record.opcode);
        if (resultIndex < 0 || record.operands.getCount() <= resultIndex)
            return 0;
        return record.operands[resultIndex];
    }

    void collect(const HGBCParseResult& program)
    {
        for (auto& record : program.records)
        {
            if (record.opcode == HGBCOpcode::StringLit && record.operands.getCount() >= 2)
            {
                const char* payloadBegin =
                    record.payload.getCount()
                        ? reinterpret_cast<const char*>(record.payload.getBuffer())
                        : "";
                String text(
                    UnownedStringSlice(payloadBegin, payloadBegin + record.payload.getCount()));
                stringLiterals.set(record.operands[1], text);
            }
        }

        for (auto& record : program.records)
        {
            if (record.opcode == HGBCOpcode::Decorate && record.operands.getCount() >= 4 &&
                record.operands[1] == uint32_t(HGBCDecorationKind::NameHint))
            {
                HGBCId targetId = record.operands[2];
                HGBCId nameId = record.operands[3];
                if (auto name = stringLiterals.tryGetValue(nameId))
                    nameHints.set(targetId, *name);
            }

            if (record.opcode == HGBCOpcode::Func && record.operands.getCount() >= 2)
                funcResultToType.set(record.operands[1], record.operands[0]);
        }
    }

    void assignFriendlyNames(const HGBCParseResult& program)
    {
        if (!options.useFriendlyIds)
            return;

        for (auto& hint : nameHints)
            assignName(hint.first, makeValueBaseName(hint.second));

        for (auto& literal : stringLiterals)
        {
            StringBuilder base;
            base << "str";
            base
                << (isConciseStringLiteral(literal.second) ? makePascalBaseName(literal.second)
                                                           : String("Text"));
            assignName(literal.first, base.produceString());
        }

        for (auto& pair : funcResultToType)
        {
            HGBCId funcId = pair.first;
            HGBCId typeId = pair.second;
            if (idNames.containsKey(typeId))
                continue;

            String funcName = getIdName(funcId);
            String base = funcName.getUnownedSlice().begin()[0] == '%'
                              ? String(funcName.getUnownedSlice().begin() + 1, funcName.end())
                              : funcName;
            StringBuilder typeName;
            typeName << base;
            typeName << "FuncType";
            assignName(typeId, typeName.produceString());
        }

        for (auto& function : program.functions)
        {
            String funcName = getIdName(function.id);
            String currentFuncBase =
                funcName.getUnownedSlice().begin()[0] == '%'
                    ? String(funcName.getUnownedSlice().begin() + 1, funcName.end())
                    : funcName;

            uint32_t blockCounter = 0;
            for (auto& block : function.blocks)
            {
                StringBuilder blockName;
                if (currentFuncBase.getLength())
                    blockName << currentFuncBase;
                else
                    blockName << "block";
                blockName << "Block";
                if (blockCounter)
                    blockName << blockCounter;
                assignName(block.id, blockName.produceString());
                blockCounter++;
            }
        }
    }

    String getIdName(HGBCId id)
    {
        if (options.useFriendlyIds)
        {
            if (auto name = idNames.tryGetValue(id))
                return *name;
        }

        StringBuilder sb;
        sb << "%id";
        sb << id;
        return sb.produceString();
    }

    void assignFallbacks(const HGBC::Vector<HGBCRecord>& records)
    {
        for (auto& record : records)
        {
            HGBCId resultId = getResultId(record);
            if (resultId)
                assignName(
                    resultId,
                    getIdName(resultId).getUnownedSlice().begin()[0] == '%'
                        ? String(
                              getIdName(resultId).getUnownedSlice().begin() + 1,
                              getIdName(resultId).end())
                        : getIdName(resultId));
        }
    }
};

struct HGBCPrinter
{
    HGBCParseResult const& parseResult;
    HGBCDisassemblyOptions const& options;
    FriendlyIdBuilder& ids;
    StringBuilder out = StringBuilder();

    void appendId(HGBCId id) { out << ids.getIdName(id); }

    void appendOperand(const HGBCOperandInfo& operandInfo, uint32_t value)
    {
        switch (operandInfo.kind)
        {
        case HGBCOperandKind::Id:
        case HGBCOperandKind::TypeId:
        case HGBCOperandKind::ResultId:
            appendId(value);
            break;
        case HGBCOperandKind::Bool:
            out << (value ? "true" : "false");
            break;
        case HGBCOperandKind::Enum:
            if (const char* enumName = getHGBCEnumValueName(operandInfo.enumKind, value))
                out << enumName;
            else
                out << value;
            break;
        case HGBCOperandKind::UInt:
        case HGBCOperandKind::FloatBits:
        case HGBCOperandKind::ByteCount:
            out << value;
            break;
        }
    }
    void appendPrefix(const HGBCRecord& record, uint32_t indentation = 1)
    {
        for (uint32_t i = 0; i < indentation; ++i)
            out << "    ";
        if (options.showOffsets)
        {
            out << "/* +";
            out << record.debugInfo.sectionOffset;
            out << " */ ";
        }
    }

    void appendBinaryBytes(const HGBCRecord& record)
    {
        static const char hexChars[] = "0123456789abcdef";
        out << " {";
        if (record.payload.getCount())
            out << "\n        ";

        for (size_t i = 0; i < record.payload.getCount(); i++)
        {
            uint8_t value = record.payload[i];
            out << "0x";
            out.appendChar(hexChars[value >> 4]);
            out.appendChar(hexChars[value & 0xf]);
            if (i + 1 != record.payload.getCount())
                out << " ";
        }

        if (record.payload.getCount())
            out << "\n    ";
        out << "}";
    }

    void appendGenericRecord(const HGBCRecord& record)
    {
        const HGBCOpcodeInfo* opcodeInfo = getHGBCOpcodeInfo(record.opcode);
        SLANG_RELEASE_ASSERT(opcodeInfo);

        const int resultIndex = opcodeInfo->resultOperandIndex;
        const int typeIndex = opcodeInfo->typeOperandIndex;
        if (resultIndex >= 0)
        {
            appendId(record.operands[resultIndex]);
            if (typeIndex >= 0)
            {
                out << " : ";
                appendId(record.operands[typeIndex]);
            }
            out << " = ";
        }

        out << opcodeInfo->name;
        for (size_t i = 0; i < record.operands.getCount(); ++i)
        {
            if (i == resultIndex || i == typeIndex)
                continue;

            const HGBCOperandInfo* operandInfo = getHGBCOperandInfo(*opcodeInfo, size_t(i));
            SLANG_RELEASE_ASSERT(operandInfo);
            out << " ";
            appendOperand(*operandInfo, record.operands[i]);
        }
    }

    void appendInstruction(const HGBCRecord& record)
    {
        switch (record.opcode)
        {
        case HGBCOpcode::FloatLit:
            {
                uint64_t bits = record.operands[2];
                if (record.operands.getCount() == 4)
                    bits |= uint64_t(record.operands[3]) << 32;
                double value = record.operands.getCount() == 4
                                   ? bitCast<double>(bits)
                                   : double(bitCast<float>(uint32_t(bits)));
                appendId(record.operands[1]);
                out << " : ";
                appendId(record.operands[0]);
                out << " = OpFloatLit " << value << " /* bits " << bits << " */";
                break;
            }
        case HGBCOpcode::StringLit:
            appendId(record.operands[1]);
            out << " : ";
            appendId(record.operands[0]);
            out << " = OpStringLit " << record.operands[2] << " ";
            appendEscapedString(out, record.payload.getBuffer(), record.payload.getCount());
            break;
        case HGBCOpcode::BinaryLit:
            appendId(record.operands[0]);
            out << " = OpBinaryLit " << record.operands[1];
            if (options.showBinaryLiteralBytes)
                appendBinaryBytes(record);
            else
                out << " { ... }";
            break;
        default:
            appendGenericRecord(record);
            break;
        }
    }

    void appendRecord(const HGBCRecord& record, uint32_t indentation = 1)
    {
        appendPrefix(record, indentation);
        appendInstruction(record);
        out << "\n";
    }
    bool hasRecordsForSection(HGBCSectionTag sectionTag)
    {
        for (auto& record : parseResult.records)
        {
            if (record.debugInfo.section == sectionTag)
                return true;
        }
        return false;
    }

    void appendSection(HGBCSectionTag sectionTag)
    {
        if (!hasRecordsForSection(sectionTag))
            return;

        out << "Section(\"";
        out << getHGBCSectionName(sectionTag);
        out << "\")\n";

        for (auto& record : parseResult.records)
        {
            if (record.debugInfo.section == sectionTag)
                appendRecord(record);
        }

        out << "\n";
    }

    void appendStructuredFunctionSection()
    {
        if (!hasRecordsForSection(HGBCSectionTag::FUNC))
            return;

        out << "Section(\"";
        out << getHGBCSectionName(HGBCSectionTag::FUNC);
        out << "\")\n";

        bool hasFunction = false;
        bool hasBlockInFunction = false;
        for (auto& record : parseResult.records)
        {
            if (record.debugInfo.section != HGBCSectionTag::FUNC)
                continue;

            switch (record.opcode)
            {
            case HGBCOpcode::Func:
                if (hasFunction)
                    out << "\n";
                appendPrefix(record, 1);
                appendInstruction(record);
                out << "\n    {\n";
                hasFunction = true;
                hasBlockInFunction = false;
                break;

            case HGBCOpcode::Block:
                if (hasBlockInFunction)
                    out << "\n";
                appendPrefix(record, 2);
                appendInstruction(record);
                out << "\n        {\n";
                hasBlockInFunction = true;
                break;

            case HGBCOpcode::BlockEnd:
                out << "        } // OpBlockEnd\n";
                break;

            case HGBCOpcode::FunctionEnd:
                out << "    } // OpFunctionEnd\n";
                break;

            default:
                appendRecord(record, 3);
                break;
            }
        }

        out << "\n";
    }

    void appendOffsetTable()
    {
        if (!options.showOffsets)
            return;

        out << "Section(\"OFST\")\n";
        for (auto& section : parseResult.sections)
        {
            out << "    ";
            out << getHGBCSectionName(section.tag);
            out << " offset=";
            out << section.offset;
            out << " size=";
            out << section.size;
            out << "\n";
        }
        out << "\n";
    }

    void print(bool structureFunctions = false)
    {
        out << "% HGBC Disassembly\n\n";
        appendOffsetTable();
        for (auto section : kHGBCRecordSections)
        {
            if (structureFunctions && section == HGBCSectionTag::FUNC)
                appendStructuredFunctionSection();
            else
                appendSection(section);
        }
    }
};

bool readDisassemblyProgram(ISlangBlob* blob, HGBCParseResult& outProgram, DiagnosticSink* sink)
{
    if (!blob)
    {
        diagnoseRaw(sink, "HGBC disassembly requires a blob\n");
        return false;
    }

    std::string parseError;
    if (!HGBC::readProgram(
            blob->getBufferPointer(),
            blob->getBufferSize(),
            outProgram,
            &parseError))
    {
        diagnoseRaw(sink, parseError.c_str());
        return false;
    }
    return true;
}

String getDisassemblyTypeName(const HGBCParseResult& program, FriendlyIdBuilder& ids, HGBCId typeId)
{
    StringBuilder result;
    result << ids.getIdName(typeId);

    if (const std::string* slangName =
            HGBC::findStringDecoration(program, typeId, HGBCDecorationKind::SlangTypeName))
    {
        result << " (" << slangName->c_str() << ")";
        return result.produceString();
    }

    auto recordIt = program.basicIndex.resultRecords.find(typeId);
    if (recordIt == program.basicIndex.resultRecords.end())
        return result.produceString();

    const HGBCRecord& typeRecord = program.records[recordIt->second];
    if (const HGBCOpcodeInfo* opcodeInfo = getHGBCOpcodeInfo(typeRecord.opcode))
        result << " (" << opcodeInfo->name << ")";
    return result.produceString();
}

void enumerateDisassemblySymbols(
    const HGBCParseResult& program,
    FriendlyIdBuilder& ids,
    HGBCDisassemblyOptions const& options,
    HGBCDisassemblySymbolCallback callback,
    void* userData)
{
    if (!callback)
        return;

    for (auto& record : program.records)
    {
        HGBCId resultId = HGBC::getResultId(record);
        if (!resultId)
            continue;

        String name = ids.getIdName(resultId);
        const char* sectionName = getHGBCSectionName(record.debugInfo.section);
        String typeName;

        const HGBCOpcodeInfo* opcodeInfo = getHGBCOpcodeInfo(record.opcode);
        if (opcodeInfo && opcodeInfo->typeOperandIndex >= 0 &&
            size_t(opcodeInfo->typeOperandIndex) < record.operands.getCount())
        {
            typeName =
                getDisassemblyTypeName(program, ids, record.operands[opcodeInfo->typeOperandIndex]);
        }

        HGBCPrinter definitionPrinter{program, options, ids};
        definitionPrinter.appendInstruction(record);
        callback(
            userData,
            name.getBuffer(),
            size_t(name.getLength()),
            sectionName,
            strlen(sectionName),
            typeName.getBuffer(),
            size_t(typeName.getLength()),
            definitionPrinter.out.getBuffer(),
            size_t(definitionPrinter.out.getLength()));
    }
}

} // namespace

SlangResult disassembleHGBC(
    ISlangBlob* blob,
    ISlangWriter* writer,
    DiagnosticSink* sink,
    HGBCDisassemblyOptions const& options)
{
    if (!blob || !writer)
    {
        diagnoseRaw(sink, "HGBC disassembly requires a blob and writer\n");
        return SLANG_FAIL;
    }

    HGBCParseResult parseResult;
    if (!readDisassemblyProgram(blob, parseResult, sink))
        return SLANG_FAIL;

    FriendlyIdBuilder idBuilder(options);
    idBuilder.collect(parseResult);
    idBuilder.assignFriendlyNames(parseResult);

    HGBCPrinter printer{parseResult, options, idBuilder};
    printer.print();

    return writer->write(printer.out.getBuffer(), printer.out.getLength());
}

SlangResult writeHGBCDisassemblyDocument(
    ISlangBlob* blob,
    ISlangWriter* writer,
    HGBCDisassemblySymbolCallback symbolCallback,
    void* userData,
    DiagnosticSink* sink,
    HGBCDisassemblyOptions const& options)
{
    if (!blob || !writer)
    {
        diagnoseRaw(sink, "HGBC document disassembly requires a blob and writer\n");
        return SLANG_FAIL;
    }

    HGBCParseResult parseResult;
    if (!readDisassemblyProgram(blob, parseResult, sink))
        return SLANG_FAIL;

    FriendlyIdBuilder idBuilder(options);
    idBuilder.collect(parseResult);
    idBuilder.assignFriendlyNames(parseResult);

    HGBCPrinter printer{parseResult, options, idBuilder};
    printer.print(true);
    SLANG_RETURN_ON_FAIL(writer->write(printer.out.getBuffer(), printer.out.getLength()));
    enumerateDisassemblySymbols(parseResult, idBuilder, options, symbolCallback, userData);
    return SLANG_OK;
}

void enumerateHGBCRecordDisassembly(
    const HGBC::Program& program,
    std::span<const size_t> recordIndices,
    HGBCDisassemblyInstructionCallback instructionCallback,
    void* userData,
    HGBCDisassemblyOptions const& options)
{
    if (!instructionCallback)
        return;

    FriendlyIdBuilder idBuilder(options);
    idBuilder.collect(program);
    idBuilder.assignFriendlyNames(program);

    for (size_t recordIndex : recordIndices)
    {
        SLANG_RELEASE_ASSERT(recordIndex < program.records.size());
        HGBCPrinter printer{program, options, idBuilder};
        printer.appendInstruction(program.records[recordIndex]);
        instructionCallback(userData, printer.out.getBuffer(), size_t(printer.out.getLength()));
    }
}

} // namespace Slang
