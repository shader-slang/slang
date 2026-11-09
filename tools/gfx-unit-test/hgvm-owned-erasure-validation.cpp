#include "slang-hgvm/slang-hgvm.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

namespace
{

enum ErasureValidationId : HGBCId
{
    UIntType = 1,
    FloatType,
    VoidType,
    StringType,
    BoxType,
    UnionType,
    DestinationUnionType,
    EmptyUnionType,
    StructType,
    OtherStructType,
    UIntPointerType,
    BoxPointerType,
    UnionPointerType,
    BoxHolderType,
    BoxHolderPointerType,
    BoxArrayType,
    BoxArrayPointerType,
    OutBoxType,
    InOutBoxType,
    BorrowedStructType,
    BorrowedArrayType,
    FunctionType,
    ValueFunctionType,
    RefFunctionType,
    UIntValue,
    FloatValue,
    FieldName,
    NullBox,
    NullUnion,
    NullEmptyUnion,
    Function,
    Block,
    UIntAddress,
    BoxAddress,
    UnionAddress,
    BoxHolderAddress,
    BoxArrayAddress,
    StructValue,
    BoxValue,
    UnboxedValue,
    UnionValue,
    CaseValue,
    PayloadValue,
    ExtractedValue,
    ConvertedValue,
    VoidExtractedValue,
    ValueCall,
    RefCall,
    ValueFunction,
    ValueBlock,
    ValueParameter,
    RefFunction,
    RefBlock,
    OutParameter,
    InOutParameter,
};

HGBC::Record& appendErasureRecord(
    HGBC::Program& program,
    HGBCSectionTag sectionTag,
    HGBCOpcode opcode,
    HGBC::Vector<uint32_t> operands)
{
    if (program.sections.empty() || program.sections.back().tag != sectionTag)
    {
        HGBC::Section section;
        section.tag = sectionTag;
        program.sections.add(section);
    }
    program.sections.back().recordIndices.add(program.records.size());
    HGBC::Record record;
    record.opcode = opcode;
    record.operands = std::move(operands);
    program.records.add(std::move(record));
    return program.records.back();
}

/// Constructs wire records directly so invalid programs cannot be rejected by the compiler first.
/// The fixture intentionally includes an empty type, a payload-free case, and a reordered case map.
HGBC::Program makeErasureValidationProgram()
{
    HGBC::Program program;
    const auto type = HGBCSectionTag::TYPE;
    appendErasureRecord(program, type, HGBCOpcode::UIntType, {UIntType});
    appendErasureRecord(program, type, HGBCOpcode::Float32Type, {FloatType});
    appendErasureRecord(program, type, HGBCOpcode::VoidType, {VoidType});
    appendErasureRecord(program, type, HGBCOpcode::StringType, {StringType});
    appendErasureRecord(program, type, HGBCOpcode::BoxType, {BoxType});
    appendErasureRecord(
        program,
        type,
        HGBCOpcode::TaggedUnionType,
        {UnionType, 7, UIntType, 9, FloatType, 11, VoidType});
    appendErasureRecord(
        program,
        type,
        HGBCOpcode::TaggedUnionType,
        {DestinationUnionType, 20, FloatType, 21, UIntType, 22, VoidType});
    appendErasureRecord(program, type, HGBCOpcode::TaggedUnionType, {EmptyUnionType});
    appendErasureRecord(program, type, HGBCOpcode::StructType, {StructType, FieldName, UIntType});
    appendErasureRecord(
        program,
        type,
        HGBCOpcode::StructType,
        {OtherStructType, FieldName, UIntType});
    appendErasureRecord(program, type, HGBCOpcode::PtrType, {UIntPointerType, UIntType});
    appendErasureRecord(program, type, HGBCOpcode::PtrType, {BoxPointerType, BoxType});
    appendErasureRecord(program, type, HGBCOpcode::PtrType, {UnionPointerType, UnionType});
    appendErasureRecord(program, type, HGBCOpcode::StructType, {BoxHolderType, FieldName, BoxType});
    appendErasureRecord(program, type, HGBCOpcode::PtrType, {BoxHolderPointerType, BoxHolderType});
    appendErasureRecord(program, type, HGBCOpcode::ArrayType, {BoxArrayType, 2, BoxType});
    appendErasureRecord(program, type, HGBCOpcode::PtrType, {BoxArrayPointerType, BoxArrayType});
    appendErasureRecord(program, type, HGBCOpcode::OutType, {OutBoxType, BoxType});
    appendErasureRecord(program, type, HGBCOpcode::InOutType, {InOutBoxType, BoxType});
    appendErasureRecord(
        program,
        type,
        HGBCOpcode::StructType,
        {BorrowedStructType, FieldName, UIntPointerType});
    appendErasureRecord(
        program,
        type,
        HGBCOpcode::ArrayType,
        {BorrowedArrayType, 2, BorrowedStructType});
    appendErasureRecord(program, type, HGBCOpcode::FuncType, {FunctionType, VoidType});
    appendErasureRecord(
        program,
        type,
        HGBCOpcode::FuncType,
        {ValueFunctionType, VoidType, BoxType});
    appendErasureRecord(
        program,
        type,
        HGBCOpcode::FuncType,
        {RefFunctionType, VoidType, OutBoxType, InOutBoxType});

    const auto literal = HGBCSectionTag::LITR;
    appendErasureRecord(program, literal, HGBCOpcode::IntLit, {UIntType, UIntValue, 7});
    appendErasureRecord(program, literal, HGBCOpcode::FloatLit, {FloatType, FloatValue, 0});
    appendErasureRecord(program, literal, HGBCOpcode::StringLit, {StringType, FieldName, 1})
        .payload = {'x'};
    appendErasureRecord(program, literal, HGBCOpcode::NullLit, {BoxType, NullBox});
    appendErasureRecord(program, literal, HGBCOpcode::NullLit, {UnionType, NullUnion});
    appendErasureRecord(program, literal, HGBCOpcode::NullLit, {EmptyUnionType, NullEmptyUnion});

    const auto function = HGBCSectionTag::FUNC;
    appendErasureRecord(program, function, HGBCOpcode::Func, {FunctionType, Function});
    appendErasureRecord(program, function, HGBCOpcode::Block, {Block});
    appendErasureRecord(program, function, HGBCOpcode::Var, {UIntPointerType, UIntAddress});
    appendErasureRecord(program, function, HGBCOpcode::Var, {BoxPointerType, BoxAddress});
    appendErasureRecord(program, function, HGBCOpcode::Var, {UnionPointerType, UnionAddress});
    appendErasureRecord(
        program,
        function,
        HGBCOpcode::Var,
        {BoxHolderPointerType, BoxHolderAddress});
    appendErasureRecord(program, function, HGBCOpcode::Var, {BoxArrayPointerType, BoxArrayAddress});
    appendErasureRecord(
        program,
        function,
        HGBCOpcode::MakeStruct,
        {StructType, StructValue, UIntValue});
    appendErasureRecord(
        program,
        function,
        HGBCOpcode::Box,
        {BoxType, BoxValue, UIntType, UIntValue});
    appendErasureRecord(program, function, HGBCOpcode::Unbox, {UIntType, UnboxedValue, BoxValue});
    appendErasureRecord(
        program,
        function,
        HGBCOpcode::MakeTaggedUnion,
        {UnionType, UnionValue, UIntValue, BoxValue});
    appendErasureRecord(
        program,
        function,
        HGBCOpcode::GetTaggedUnionCase,
        {UIntType, CaseValue, UnionValue});
    appendErasureRecord(
        program,
        function,
        HGBCOpcode::GetTaggedUnionPayload,
        {BoxType, PayloadValue, UnionValue});
    appendErasureRecord(
        program,
        function,
        HGBCOpcode::ExtractTaggedUnion,
        {UIntType, ExtractedValue, UnionValue, 7});
    appendErasureRecord(
        program,
        function,
        HGBCOpcode::ConvertTaggedUnion,
        {DestinationUnionType, ConvertedValue, UnionValue, 7, 21, 9, 20, 11, 22});
    appendErasureRecord(
        program,
        function,
        HGBCOpcode::ExtractTaggedUnion,
        {VoidType, VoidExtractedValue, NullUnion, 11});
    appendErasureRecord(
        program,
        function,
        HGBCOpcode::Call,
        {VoidType, ValueCall, ValueFunction, BoxValue});
    appendErasureRecord(
        program,
        function,
        HGBCOpcode::Call,
        {VoidType, RefCall, RefFunction, BoxAddress, BoxAddress});
    appendErasureRecord(program, function, HGBCOpcode::Return, {});
    appendErasureRecord(program, function, HGBCOpcode::BlockEnd, {});
    appendErasureRecord(program, function, HGBCOpcode::FunctionEnd, {});
    appendErasureRecord(program, function, HGBCOpcode::Func, {ValueFunctionType, ValueFunction});
    appendErasureRecord(program, function, HGBCOpcode::Block, {ValueBlock});
    appendErasureRecord(program, function, HGBCOpcode::Param, {BoxType, ValueParameter});
    appendErasureRecord(program, function, HGBCOpcode::Return, {});
    appendErasureRecord(program, function, HGBCOpcode::BlockEnd, {});
    appendErasureRecord(program, function, HGBCOpcode::FunctionEnd, {});
    appendErasureRecord(program, function, HGBCOpcode::Func, {RefFunctionType, RefFunction});
    appendErasureRecord(program, function, HGBCOpcode::Block, {RefBlock});
    appendErasureRecord(program, function, HGBCOpcode::Param, {OutBoxType, OutParameter});
    appendErasureRecord(program, function, HGBCOpcode::Param, {InOutBoxType, InOutParameter});
    appendErasureRecord(program, function, HGBCOpcode::Return, {});
    appendErasureRecord(program, function, HGBCOpcode::BlockEnd, {});
    appendErasureRecord(program, function, HGBCOpcode::FunctionEnd, {});
    return program;
}

HGBC::Record& findErasureRecord(HGBC::Program& program, HGBCOpcode opcode)
{
    for (auto& record : program.records)
        if (record.opcode == opcode)
            return record;
    SLANG_UNEXPECTED("owned-erasure fixture is missing a record");
}

void checkErasureValidation(const HGBC::Program& source, const char* expectedError = nullptr)
{
    HGBC::Vector<uint8_t> bytes;
    SLANG_CHECK_ABORT(HGBC::writeProgram(source, bytes));
    RefPtr<HGVM::Program> program;
    String error;
    auto result = HGVM::Program::load(bytes.data(), bytes.size(), "owned-erasure", program, error);
    if (expectedError)
    {
        SLANG_CHECK(SLANG_FAILED(result));
        if (!error.contains(expectedError))
            fprintf(stderr, "Expected '%s', got '%s'\n", expectedError, error.getBuffer());
        SLANG_CHECK(error.contains(expectedError));
    }
    else
    {
        if (SLANG_FAILED(result))
            fprintf(stderr, "%s\n", error.getBuffer());
        SLANG_CHECK(SLANG_SUCCEEDED(result));
    }
}

} // namespace

SLANG_UNIT_TEST(hgvmOwnedErasureValidation)
{
    const auto source = makeErasureValidationProgram();
    checkErasureValidation(source);

    struct Corruption
    {
        HGBCOpcode opcode;
        size_t operandIndex;
        uint32_t value;
        const char* error;
    };
    const Corruption corruptions[] = {
        {HGBCOpcode::TaggedUnionType, 3, 7, "defines a case tag more than once"},
        {HGBCOpcode::TaggedUnionType, 2, UIntPointerType, "case payload must be an owning value"},
        {HGBCOpcode::TaggedUnionType, 2, BorrowedArrayType, "case payload must be an owning value"},
        {HGBCOpcode::Box, 0, UIntType, "result type must be a box"},
        {HGBCOpcode::Box, 2, FloatType, "payload does not match its declared concrete type"},
        {HGBCOpcode::Box, 3, UIntAddress, "payload does not match its declared concrete type"},
        {HGBCOpcode::Box, 2, UIntPointerType, "without borrowed addresses"},
        {HGBCOpcode::Box, 2, BorrowedStructType, "without borrowed addresses"},
        {HGBCOpcode::Box, 2, BorrowedArrayType, "without borrowed addresses"},
        {HGBCOpcode::Box, 2, VoidType, "without borrowed addresses"},
        {HGBCOpcode::Unbox, 0, UIntPointerType, "without borrowed addresses"},
        {HGBCOpcode::Unbox, 0, VoidType, "without borrowed addresses"},
        {HGBCOpcode::Unbox, 2, UIntValue, "operand must be a box"},
        {HGBCOpcode::Unbox, 2, BoxAddress, "operand must be a box"},
        {HGBCOpcode::MakeTaggedUnion, 0, BoxType, "result must be a tagged union type"},
        {HGBCOpcode::MakeTaggedUnion, 2, FloatValue, "case tag must be uint"},
        {HGBCOpcode::MakeTaggedUnion, 2, UIntAddress, "case tag must be uint"},
        {HGBCOpcode::MakeTaggedUnion, 3, UIntValue, "payload must be a box"},
        {HGBCOpcode::MakeTaggedUnion, 3, BoxAddress, "payload must be a box"},
        {HGBCOpcode::GetTaggedUnionCase, 0, FloatType, "case result must be uint"},
        {HGBCOpcode::GetTaggedUnionCase, 2, BoxValue, "operand must be a tagged union value"},
        {HGBCOpcode::GetTaggedUnionCase, 2, UnionAddress, "operand must be a tagged union value"},
        {HGBCOpcode::GetTaggedUnionPayload, 0, UIntType, "payload result must be a box"},
        {HGBCOpcode::GetTaggedUnionPayload,
         2,
         UnionAddress,
         "operand must be a tagged union value"},
        {HGBCOpcode::ExtractTaggedUnion, 2, UnionAddress, "operand must be a tagged union value"},
        {HGBCOpcode::ExtractTaggedUnion, 3, 123, "case tag is not declared"},
        {HGBCOpcode::ExtractTaggedUnion, 0, FloatType, "result does not match the case"},
        {HGBCOpcode::ConvertTaggedUnion, 3, 123, "tag not declared"},
        {HGBCOpcode::ConvertTaggedUnion, 4, 123, "tag not declared"},
        {HGBCOpcode::ConvertTaggedUnion, 4, 20, "preserve each concrete payload type"},
        {HGBCOpcode::ConvertTaggedUnion, 2, UnionAddress, "source and destination must be tagged"},
    };
    for (const auto& corruption : corruptions)
    {
        auto invalid = source;
        findErasureRecord(invalid, corruption.opcode).operands[corruption.operandIndex] =
            corruption.value;
        checkErasureValidation(invalid, corruption.error);
    }

    // A partial map is checked narrowing. Cases are keyed by tag, independently of declaration
    // order.
    auto missingCase = source;
    findErasureRecord(missingCase, HGBCOpcode::ConvertTaggedUnion)
        .operands = {DestinationUnionType, ConvertedValue, UnionValue, 7, 21, 9, 20};
    checkErasureValidation(missingCase);
    auto duplicateCase = source;
    findErasureRecord(duplicateCase, HGBCOpcode::ConvertTaggedUnion)
        .operands = {DestinationUnionType, ConvertedValue, UnionValue, 7, 21, 7, 21, 11, 22};
    checkErasureValidation(duplicateCase, "source tag more than once");
    auto reordered = source;
    findErasureRecord(reordered, HGBCOpcode::ConvertTaggedUnion)
        .operands = {DestinationUnionType, ConvertedValue, UnionValue, 11, 22, 7, 21, 9, 20};
    checkErasureValidation(reordered);

    // Equal layouts do not make two named concrete structs interchangeable inside a box.
    auto nominalMismatch = source;
    findErasureRecord(nominalMismatch, HGBCOpcode::Box)
        .operands = {BoxType, BoxValue, OtherStructType, StructValue};
    checkErasureValidation(nominalMismatch, "payload does not match its declared concrete type");

    // Opaque union storage cannot be forged by treating its representation as ordinary fields.
    auto forgedAggregate = source;
    findErasureRecord(forgedAggregate, HGBCOpcode::MakeStruct).operands = {
        UnionType,
        StructValue,
        UIntValue};
    checkErasureValidation(forgedAggregate, "malformed struct construction operands");
    auto forgedField = source;
    auto& field = findErasureRecord(forgedField, HGBCOpcode::MakeStruct);
    field.opcode = HGBCOpcode::FieldExtract;
    field.operands = {UIntType, StructValue, UnionValue, FieldName};
    checkErasureValidation(forgedField, "base is not an aggregate value");

    // Generic value copies must not provide a second way to smuggle pointer bytes into a box.
    // Each instruction below copies an ordinary value slot at runtime; an OpLoad is required
    // before passing a local's address to one of these boundaries.
    auto forgedStore = source;
    auto& store = findErasureRecord(forgedStore, HGBCOpcode::MakeStruct);
    store.opcode = HGBCOpcode::Store;
    store.operands = {BoxAddress, BoxAddress};
    checkErasureValidation(forgedStore, "value does not match the pointer's addressed type");
    auto forgedStructField = source;
    findErasureRecord(forgedStructField, HGBCOpcode::MakeStruct).operands = {
        BoxHolderType,
        StructValue,
        BoxAddress};
    checkErasureValidation(forgedStructField, "does not match the struct field type");
    auto forgedArrayElement = source;
    auto& array = findErasureRecord(forgedArrayElement, HGBCOpcode::MakeStruct);
    array.opcode = HGBCOpcode::MakeArray;
    array.operands = {BoxArrayType, StructValue, BoxAddress, BoxAddress};
    checkErasureValidation(forgedArrayElement, "element does not match the array element type");
    auto forgedFieldSource = source;
    auto& fieldSource = findErasureRecord(forgedFieldSource, HGBCOpcode::MakeStruct);
    fieldSource.opcode = HGBCOpcode::FieldExtract;
    fieldSource.operands = {BoxType, StructValue, BoxHolderAddress, FieldName};
    checkErasureValidation(forgedFieldSource, "base is not an aggregate value");
    auto forgedArraySource = source;
    auto& arraySource = findErasureRecord(forgedArraySource, HGBCOpcode::MakeStruct);
    arraySource.opcode = HGBCOpcode::ExtractElement;
    arraySource.operands = {BoxType, StructValue, BoxArrayAddress, UIntValue};
    checkErasureValidation(forgedArraySource, "base does not have an indexable type");
    auto forgedValueCall = source;
    findErasureRecord(forgedValueCall, HGBCOpcode::Call).operands[3] = BoxAddress;
    checkErasureValidation(forgedValueCall, "callee parameter expects type");
    auto forgedReturn = source;
    findErasureRecord(forgedReturn, HGBCOpcode::FuncType).operands = {FunctionType, BoxType};
    findErasureRecord(forgedReturn, HGBCOpcode::Return).operands = {BoxAddress};
    checkErasureValidation(forgedReturn, "value type does not match the function result type");

    // The valid fixture above also passes Ptr<Box> to both Out<Box> and InOut<Box> parameters.
    // Only an ordinary by-value argument or return requires a loaded Box value.
    auto ownedReturn = source;
    findErasureRecord(ownedReturn, HGBCOpcode::FuncType).operands = {FunctionType, BoxType};
    findErasureRecord(ownedReturn, HGBCOpcode::Return).operands = {BoxValue};
    checkErasureValidation(ownedReturn);

    // These are intentionally locally valid: execution checks dynamic payload and case identity.
    auto wrongRuntimeType = source;
    findErasureRecord(wrongRuntimeType, HGBCOpcode::Unbox).operands[0] = FloatType;
    checkErasureValidation(wrongRuntimeType);
    auto wrongRuntimeCase = source;
    findErasureRecord(wrongRuntimeCase, HGBCOpcode::IntLit).operands[2] = 123;
    checkErasureValidation(wrongRuntimeCase);
}
