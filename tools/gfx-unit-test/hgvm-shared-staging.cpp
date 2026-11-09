#include "hgvm-test-helpers.h"
#include "slang-hgvm/slang-hgvm-debug-map.h"
#include "slang-hgvm/slang-hgvm-interpreter.h"
#include "slang-hgvm/slang-hgvm-staged-records.h"

using namespace Slang;
using namespace Slang::HGVM;

namespace
{
Address findAllocatedClass(InterpreterCore& interpreter, uint64_t programId)
{
    auto map = interpreter.getDebugMap(programId);
    for (const auto& function : map->getFunctions())
    {
        for (uint32_t pc = function.entry; pc < function.end;)
        {
            const auto* record = interpreter.getRecord({programId, pc});
            if ((record->opcodeAndFlags & StagedRecord::kOpcodeMask) ==
                uint32_t(HGBCOpcode::AllocateObject))
                return reinterpret_cast<const StagedRecord::AllocateObject*>(record)
                    ->classTypeAddress;
            pc += record->recordWordCount;
        }
    }
    return {};
}

uint32_t findFunctionEntry(InterpreterCore& interpreter, uint64_t programId, HGBCId functionId)
{
    for (const auto& function : interpreter.getDebugMap(programId)->getFunctions())
        if (function.id == functionId)
            return function.entry;
    return UINT32_MAX;
}

RefPtr<Program> makeRecursiveAliasProgram()
{
    enum : HGBCId
    {
        UInt = 1,
        StringType,
        FirstNode,
        SecondNode,
        FunctionType,
        MainName,
        NextName,
        ValueName,
        TypeName,
        FirstIdentity,
        SecondIdentity,
        Export,
        Main,
        Entry,
        Object,
        Result,
    };
    HGBC::Program code;
    auto append = [&](HGBCSectionTag section,
                      HGBCOpcode opcode,
                      HGBC::Vector<uint32_t> operands) -> HGBC::Record&
    {
        if (code.sections.empty() || code.sections.back().tag != section)
        {
            HGBC::Section newSection;
            newSection.tag = section;
            code.sections.add(newSection);
        }
        code.sections.back().recordIndices.add(code.records.size());
        HGBC::Record record;
        record.opcode = opcode;
        record.operands = std::move(operands);
        code.records.add(std::move(record));
        return code.records.back();
    };
    auto string = [&](HGBCId id, const char* text)
    {
        uint32_t size = uint32_t(std::strlen(text));
        append(HGBCSectionTag::LITR, HGBCOpcode::StringLit, {StringType, id, size})
            .payload.assign(text, text + size);
    };

    append(HGBCSectionTag::TYPE, HGBCOpcode::UIntType, {UInt});
    append(HGBCSectionTag::TYPE, HGBCOpcode::StringType, {StringType});
    append(
        HGBCSectionTag::TYPE,
        HGBCOpcode::ClassType,
        {FirstNode, NextName, SecondNode, ValueName, UInt});
    append(
        HGBCSectionTag::TYPE,
        HGBCOpcode::ClassType,
        {SecondNode, NextName, FirstNode, ValueName, UInt});
    append(HGBCSectionTag::TYPE, HGBCOpcode::FuncType, {FunctionType, UInt});
    string(MainName, "main");
    string(NextName, "next");
    string(ValueName, "value");
    string(TypeName, "shared_recursive_alias.Node");
    append(
        HGBCSectionTag::META,
        HGBCOpcode::Decorate,
        {FirstIdentity, uint32_t(HGBCDecorationKind::TypeMangledName), FirstNode, TypeName});
    append(
        HGBCSectionTag::META,
        HGBCOpcode::Decorate,
        {SecondIdentity, uint32_t(HGBCDecorationKind::TypeMangledName), SecondNode, TypeName});
    append(
        HGBCSectionTag::EXPT,
        HGBCOpcode::Export,
        {Export, uint32_t(HGBCExportKind::Function), MainName, Main});
    append(HGBCSectionTag::FUNC, HGBCOpcode::Func, {FunctionType, Main});
    append(HGBCSectionTag::FUNC, HGBCOpcode::Block, {Entry});
    append(HGBCSectionTag::FUNC, HGBCOpcode::AllocateObject, {SecondNode, Object});
    append(HGBCSectionTag::FUNC, HGBCOpcode::FieldExtract, {UInt, Result, Object, ValueName});
    append(HGBCSectionTag::FUNC, HGBCOpcode::Return, {Result});
    append(HGBCSectionTag::FUNC, HGBCOpcode::BlockEnd, {});
    append(HGBCSectionTag::FUNC, HGBCOpcode::FunctionEnd, {});
    HGBC::Vector<uint8_t> bytes;
    SLANG_CHECK_ABORT(HGBC::writeProgram(code, bytes));
    RefPtr<Program> program;
    String error;
    auto result = Program::load(bytes.data(), bytes.size(), "recursive-alias", program, error);
    if (SLANG_FAILED(result))
        fprintf(stderr, "%s\n", error.getBuffer());
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
    return program;
}
} // namespace

SLANG_UNIT_TEST(hgvmSharedStagingRecursiveAlias)
{
    // Distinct HGBC IDs may carry one lowered nominal identity. A recursive alias must receive
    // the completed allocation offsets before its own AllocateObject and FieldExtract stage.
    auto program = makeRecursiveAliasProgram();
    InterpreterCore interpreter;
    RefPtr<BoundFunction> function;
    String error;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        interpreter
            .bindFunction(program, program->findFunctionExport("main")->id, function, error)));
    Value result;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(interpreter.invoke(function, {}, result, error)));
    uint32_t number = 1;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(result.getScalarData(ScalarType::UInt32, &number, sizeof(number), error)));
    SLANG_CHECK(number == 0);
}

SLANG_UNIT_TEST(hgvmSharedStagingRecursiveClass)
{
    const char* source = R"(
        class Node
        {
            Node next;
            RuntimeArray<Node> children;
            int value;
            __init(int v) { value = v; }
        }
        [host] int main()
        {
            var root = new Node(19);
            root.next = new Node(23);
            root.children = RuntimeArray<Node>::alloc(1);
            root.children[0] = root.next;
            return root.value + root.children[0].value;
        }
    )";
    auto first = compileHGVMTestProgram(
        unitTestContext->slangGlobalSession,
        source,
        SLANG_DEBUG_INFO_LEVEL_NONE,
        "hgvm_shared_recursive");
    auto second = compileHGVMTestProgram(
        unitTestContext->slangGlobalSession,
        source,
        SLANG_DEBUG_INFO_LEVEL_NONE,
        "hgvm_shared_recursive");
    InterpreterCore interpreter;
    String error;
    uint64_t firstId = 0, secondId = 0;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(interpreter.prepareProgram(first, firstId, error)));
    Address firstType = findAllocatedClass(interpreter, firstId);
    SLANG_CHECK_ABORT(bool(firstType));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(interpreter.prepareProgram(second, secondId, error)));
    Address secondType = findAllocatedClass(interpreter, secondId);
    SLANG_CHECK(firstId != secondId);
    SLANG_CHECK(firstType.value == secondType.value);
    // Re-read the original program after append: only word addresses survive arena growth.
    SLANG_CHECK(findAllocatedClass(interpreter, firstId).value == firstType.value);
    SLANG_CHECK(
        findFunctionEntry(interpreter, secondId, second->findFunctionExport("main")->id) >
        findFunctionEntry(interpreter, firstId, first->findFunctionExport("main")->id));

    for (Program* program : {first.Ptr(), second.Ptr()})
    {
        RefPtr<BoundFunction> function;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            interpreter
                .bindFunction(program, program->findFunctionExport("main")->id, function, error)));
        Value result;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(interpreter.invoke(function, {}, result, error)));
        int32_t number = 0;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            result.getScalarData(ScalarType::Int32, &number, sizeof(number), error)));
        SLANG_CHECK(number == 42);
    }
}

SLANG_UNIT_TEST(hgvmSharedStagingRejectsConflictingDefinition)
{
    auto producer = compileHGVMTestProgram(
        unitTestContext->slangGlobalSession,
        R"(
            struct Point { float x; float y; }
            [host] Point main() { return Point(19.0f, 23.0f); }
        )",
        SLANG_DEBUG_INFO_LEVEL_NONE,
        "hgvm_shared_rollback");
    auto conflicting = compileHGVMTestProgram(
        unitTestContext->slangGlobalSession,
        R"(
            struct TransactionOnly { double value; }
            struct Point { int x; float y; }
            [host] TransactionOnly main(Point p) { return TransactionOnly(p.x + p.y); }
        )",
        SLANG_DEBUG_INFO_LEVEL_NONE,
        "hgvm_shared_rollback");
    auto consumer = compileHGVMTestProgram(
        unitTestContext->slangGlobalSession,
        R"(
            struct TransactionOnly { int value; }
            struct Point { float x; float y; }
            [host] TransactionOnly main(Point p)
            {
                return TransactionOnly(int(p.x + p.y));
            }
        )",
        SLANG_DEBUG_INFO_LEVEL_NONE,
        "hgvm_shared_rollback");
    auto reader = compileHGVMTestProgram(
        unitTestContext->slangGlobalSession,
        R"(
            struct TransactionOnly { int value; }
            [host] int main(TransactionOnly result) { return result.value; }
        )",
        SLANG_DEBUG_INFO_LEVEL_NONE,
        "hgvm_shared_rollback");

    InterpreterCore interpreter;
    String error;
    RefPtr<BoundFunction> makePoint;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        interpreter
            .bindFunction(producer, producer->findFunctionExport("main")->id, makePoint, error)));
    Value point;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(interpreter.invoke(makePoint, {}, point, error)));
    for (int attempt = 0; attempt < 2; ++attempt)
    {
        uint64_t rejectedId = 0;
        SLANG_CHECK(SLANG_FAILED(interpreter.prepareProgram(conflicting, rejectedId, error)));
        SLANG_CHECK(error.contains("disagree on the definition"));
    }
    uint64_t consumerId = 0;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(interpreter.prepareProgram(consumer, consumerId, error)));

    // A rejected load must not publish type identities or consume any shared arena addresses.
    InterpreterCore baseline;
    uint64_t baselineProducerId = 0, baselineConsumerId = 0;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(baseline.prepareProgram(producer, baselineProducerId, error)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(baseline.prepareProgram(consumer, baselineConsumerId, error)));
    auto consumerFunctionId = consumer->findFunctionExport("main")->id;
    SLANG_CHECK(
        findFunctionEntry(interpreter, consumerId, consumerFunctionId) ==
        findFunctionEntry(baseline, baselineConsumerId, consumerFunctionId));

    RefPtr<BoundFunction> sum;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(interpreter.bindFunction(consumer, consumerFunctionId, sum, error)));
    Argument arguments[] = {Argument::fromValue(point)};
    Value result;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(interpreter.invoke(sum, makeArrayView(arguments), result, error)));
    RefPtr<BoundFunction> read;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        interpreter.bindFunction(reader, reader->findFunctionExport("main")->id, read, error)));
    Argument readArguments[] = {Argument::fromValue(result)};
    Value scalar;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(interpreter.invoke(read, makeArrayView(readArguments), scalar, error)));
    int32_t number = 0;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(scalar.getScalarData(ScalarType::Int32, &number, sizeof(number), error)));
    SLANG_CHECK(number == 42);
}
