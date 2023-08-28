#include "slang-spirv-core-grammar.h"

#include "../core/slang-rtti-util.h"
#include "../core/slang-string-util.h"
#include "slang-json-native.h"
#include "slang-core-diagnostics.h"

namespace Slang
{
using SpvWord = uint32_t;

//
// Structs which mirror the structure of spirv.core.grammar.json
//
// Commented members are those which currently don't use
struct InstructionPrintingClass
{
    UnownedStringSlice tag;
    UnownedStringSlice heading;
};
SLANG_MAKE_STRUCT_RTTI_INFO(
    InstructionPrintingClass,
    SLANG_RTTI_FIELD(tag),
    SLANG_OPTIONAL_RTTI_FIELD(heading)
);

struct Operand
{
    UnownedStringSlice kind;
    UnownedStringSlice quantifier;
    // UnownedStringSlice name;
};
SLANG_MAKE_STRUCT_RTTI_INFO(
    Operand,
    SLANG_RTTI_FIELD(kind),
    SLANG_OPTIONAL_RTTI_FIELD(quantifier)
    //SLANG_RTTI_FIELD(name),
);

struct Instruction
{
    UnownedStringSlice opname;
    UnownedStringSlice class_;
    SpvWord opcode;
    List<UnownedStringSlice> capabilities;
    List<Operand> operands;
};
SLANG_MAKE_STRUCT_RTTI_INFO(
    Instruction,
    SLANG_RTTI_FIELD(opname),
    SLANG_RTTI_FIELD_IMPL(class_, "class", 0),
    SLANG_RTTI_FIELD(opcode),
    SLANG_OPTIONAL_RTTI_FIELD(capabilities),
    SLANG_OPTIONAL_RTTI_FIELD(operands)
);

struct Enumerant
{
    UnownedStringSlice enumerant;
    JSONValue value;
    List<UnownedStringSlice> capabilities;
    // List<Operand> parameters;
    // UnownedStringSlice version;
    // UnownedStringSlice lastVersion;
    // List<UnownedStringSlice> extensions;
};
SLANG_MAKE_STRUCT_RTTI_INFO(
    Enumerant,
    SLANG_RTTI_FIELD(enumerant),
    SLANG_RTTI_FIELD(value),
    SLANG_OPTIONAL_RTTI_FIELD(capabilities),
    // SLANG_OPTIONAL_RTTI_FIELD(parameters),
    // SLANG_OPTIONAL_RTTI_FIELD(version),
    // SLANG_OPTIONAL_RTTI_FIELD(lastVersion),
    // SLANG_OPTIONAL_RTTI_FIELD(extensions)
);

struct OperandKind
{
    UnownedStringSlice category;
    UnownedStringSlice kind;
    List<Enumerant> enumerants;
};
SLANG_MAKE_STRUCT_RTTI_INFO(
    OperandKind,
    SLANG_RTTI_FIELD(category),
    SLANG_RTTI_FIELD(kind),
    SLANG_OPTIONAL_RTTI_FIELD(enumerants)
);

struct SPIRVSpec
{
    // List<UnownedStringSlice> copyright;
    // UnownedStringSlice magic_number;
    // UInt32 major_version;
    // UInt32 minor_version;
    // UInt32 revision;
    List<InstructionPrintingClass> instruction_printing_class;
    List<Instruction> instructions;
    List<OperandKind> operand_kinds;
};
SLANG_MAKE_STRUCT_RTTI_INFO(
    SPIRVSpec,
    // SLANG_RTTI_FIELD(copyright),
    // SLANG_RTTI_FIELD(magic_number),
    // SLANG_RTTI_FIELD(major_version)
    // SLANG_RTTI_FIELD(minor_version)
    // SLANG_RTTI_FIELD(revision)
    SLANG_RTTI_FIELD(instruction_printing_class),
    SLANG_RTTI_FIELD(instructions),
    SLANG_RTTI_FIELD(operand_kinds)
);

static Dictionary<UnownedStringSlice, SpvWord> operandKindToDict(
        JSONContainer& container,
        DiagnosticSink& sink,
        const OperandKind& k)
{
    Dictionary<UnownedStringSlice, SpvWord> dict;
    dict.reserve(k.enumerants.getCount());
    for(const auto& e : k.enumerants)
    {
        SpvWord valueInt = 0;
        switch(e.value.getKind())
        {
            case JSONValue::Kind::Integer:
            {
                // TODO: Range check here?
                valueInt = SpvWord(container.asInteger(e.value));
                break;
            }
            case JSONValue::Kind::String:
            {
                Int i = 0;
                const auto str = container.getString(e.value);
                if(SLANG_FAILED(StringUtil::parseInt(str, i)))
                    sink.diagnose(
                        e.value.loc,
                        MiscDiagnostics::spirvCoreGrammarJSONParseFailure,
                        "Expected an integer value"
                    );
                // TODO: Range check here?
                valueInt = SpvWord(i);
                break;
             }
             default:
                sink.diagnose(
                    e.value.loc,
                    MiscDiagnostics::spirvCoreGrammarJSONParseFailure,
                    "Expected an integer value (or a string with an integer inside)"
                );
        }
        dict.add(e.enumerant, valueInt);
    }
    return dict;
}

//
//
//
RefPtr<SPIRVCoreGrammarInfo> SPIRVCoreGrammarInfo::loadFromJSON(SourceView& source, DiagnosticSink& sink)
{
    //
    // Load the JSON
    //
    SLANG_ASSERT(source.getSourceManager() == sink.getSourceManager());
    JSONLexer lexer;
    lexer.init(&source, &sink);
    JSONParser parser;
    JSONContainer container(sink.getSourceManager());
    JSONBuilder builder(&container);
    RttiTypeFuncsMap typeMap;
    typeMap = JSONNativeUtil::getTypeFuncsMap();
    SLANG_RETURN_NULL_ON_FAIL(parser.parse(&lexer, &source, &builder, &sink));
    JSONToNativeConverter converter(&container, &typeMap, &sink);
    SPIRVSpec spec;
    if(SLANG_FAILED(converter.convert(builder.getRootValue(), &spec)))
    {
        // TODO: not having a source loc here is not great...
        sink.diagnoseWithoutSourceView(
            SourceLoc{},
            MiscDiagnostics::spirvCoreGrammarJSONParseFailure,
            "Failed to match SPIR-V grammar JSON to the expected schema"
        );
        return nullptr;
    }

    //
    // Convert to the internal representation
    //
    RefPtr<SPIRVCoreGrammarInfo> res{new SPIRVCoreGrammarInfo};

    res->enumCategories.dict.reserve(spec.operand_kinds.getCount());
    int32_t i = 0;
    for(const auto& c : spec.operand_kinds)
    {
        res->enumCategories.dict.add(c.kind, EnumCategory{i});
        i++;
    }

    res->opcodes.dict.reserve(spec.instructions.getCount());
    for(const auto& i : spec.instructions)
    {
        res->opcodes.dict.add(i.opname, SpvOp(i.opcode));

        const auto class_ =
              i.class_ == "Type-Declaration" ? OpInfo::TypeDeclaration
            : i.class_ == "Constant-Creation" ? OpInfo::ConstantCreation
            : OpInfo::Other;

        const auto resultTypeIndex
            = i.operands.findFirstIndex([](const auto& o){return o.kind == "IdResultType";});
        const auto resultIdIndex
            = i.operands.findFirstIndex([](const auto& o){return o.kind == "IdResult";});
        SLANG_ASSERT(resultTypeIndex >= -1 || resultTypeIndex <= 0);
        SLANG_ASSERT(resultIdIndex >= -1 || resultTypeIndex <= 1);

        // Start with 1 because of the opcode itself
        uint16_t minWordCount = 1;
        uint16_t maxWordCount = 1;
        for(const auto& o : i.operands)
        {
            if(maxWordCount == 0xffff)
            {
                // We are about to overflow maxWordCount, either someone has
                // put 2^16-1 operands in the json, or we have a "*" quantified
                // operand not in the last position and should implement
                // support for that
                sink.diagnoseWithoutSourceView(
                    SourceLoc{},
                    MiscDiagnostics::spirvCoreGrammarJSONParseFailure,
                    "\"*\"-qualified operand wasn't the last operand"
                );
            }
            if(o.quantifier == "")
            {
                // This catches the case where an "?" or "*" qualified operand
                // appears before any unqualified operands
                if(minWordCount != maxWordCount)
                    sink.diagnoseWithoutSourceView(
                        SourceLoc{},
                        MiscDiagnostics::spirvCoreGrammarJSONParseFailure,
                        "\"*\" or \"?\" operand appeared before an unqualified operand"
                    );
                minWordCount++;
                maxWordCount++;
            }
            else if(o.quantifier == "?")
            {
                maxWordCount++;
            }
            else if(o.quantifier == "*")
            {
                maxWordCount = 0xffff;
            }
        }

        // There are duplicate opcodes in the json (for renamed instructions,
        // or the same instruction with different capabilities), for now just
        // keep the first one.
        res->opInfos.dict.addIfNotExists(SpvOp(i.opcode), {
            class_,
            static_cast<int8_t>(resultTypeIndex),
            static_cast<int8_t>(resultIdIndex),
            minWordCount,
            maxWordCount,
        });
        res->opNames.dict.addIfNotExists(SpvOp(i.opcode), i.opname);
    }
    for(const auto& k : spec.operand_kinds)
    {
        const auto d = operandKindToDict(container, sink, k);
        for(const auto& [n, v] : d)
        {
            // Add the string to this slice pool as we'll be taking ownership
            // of it shortly but don't want to invalidate it in the meantime.
            const auto s = container.getStringSlicePool().addAndGetSlice(String(k.kind) + n);
            res->allEnums.dict.add(s, v);
        }

        if(k.kind == "Capability")
            for(const auto& [n, v] : d)
                res->capabilities.dict.add(n, SpvCapability(v));
    }
    // Steal the strings from the JSON container before it dies
    res->strings.swapWith(container.getStringSlicePool());
    return res;
}
}
