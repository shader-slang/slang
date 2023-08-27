#include "slang-spirv-core-grammar.h"

#include "../core/slang-rtti-util.h"
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

struct SPIRVSpec
{
    // We don't currently use these
    // List<UnownedStringSlice> copyright;
    // UnownedStringSlice magic_number;
    // UInt32 major_version;
    // UInt32 minor_version;
    // UInt32 revision;
    List<InstructionPrintingClass> instruction_printing_class;
    List<Instruction> instructions;
};
SLANG_MAKE_STRUCT_RTTI_INFO(
    SPIRVSpec,
    // SLANG_RTTI_FIELD(copyright),
    // SLANG_RTTI_FIELD(magic_number),
    // SLANG_RTTI_FIELD(major_version)
    // SLANG_RTTI_FIELD(minor_version)
    // SLANG_RTTI_FIELD(revision)
    SLANG_RTTI_FIELD(instruction_printing_class),
    SLANG_RTTI_FIELD(instructions)
);

//
//
//
RefPtr<SPIRVCoreGrammarInfo> loadSPIRVCoreGrammarInfo(SourceView& source, DiagnosticSink& sink)
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
        sink.diagnoseWithoutSourceView(SourceLoc{}, MiscDiagnostics::spirvCoreGrammarJSONParseFailure);
        return nullptr;
    }

    //
    // Convert to the internal representation
    //
    RefPtr<SPIRVCoreGrammarInfo> res{new SPIRVCoreGrammarInfo};
    res->spvOps.dict.reserve(spec.instructions.getCount());
    for(const auto& i : spec.instructions)
        res->spvOps.dict.add(i.opname, SpvOp(i.opcode));
    return res;
}
}
