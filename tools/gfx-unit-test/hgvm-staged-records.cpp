#include "core/slang-exception.h"
#include "slang-hgvm/slang-hgvm-staged-record-builder.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;
using namespace Slang::HGVM;

SLANG_UNIT_TEST(hgvmStagedRecordRelocation)
{
    StagedRecordStorage storage;
    StagedRecordBuilder builder(storage);
    Address arguments[] = {Address::makeRelativeFrame(8), Address::makeRelativeFrame(24)};
    Address call = builder.emitCall(Address::makeRelativeFrame(40), makeArrayView(arguments));
    Address branch = builder.emitBranch();
    Address conditional = builder.emitBranchConditional(arguments[0]);

    // Force repeated growth between an unresolved call and its callee. Only word addresses
    // survive construction; pointers are borrowed from the completed program below.
    for (uint32_t i = 0; i < 4096; ++i)
        builder.emitPrint(ValueKind::Scalar, ScalarType::UInt32, arguments[0]);

    Address function = builder.emitFunction(64, 8);
    uint32_t firstParameter = builder.getWordOffset();
    uint32_t managedLeaves[] = {8};
    builder.emitParameter(arguments[0], 16, makeArrayView(managedLeaves));
    builder.emitParameter(arguments[1], 4, {});
    uint32_t entry = builder.getWordOffset();
    builder.emitReturn();
    uint32_t exit = builder.getWordOffset();
    builder.emitFinishFrameExit();
    builder.patchCallTarget(call, function);
    builder.patchBranchTarget(branch, entry);
    builder.patchConditionalTargets(conditional, entry, exit);
    builder.patchFunction(function, entry, exit, firstParameter, 2);
    builder.finalize();

    const auto* callRecord =
        storage.getRecord<StagedRecord::Call>(uint32_t(call.getUnsignedPayload()));
    SLANG_CHECK(callRecord->functionAddress.value == function.value);
    SLANG_CHECK(callRecord->getArguments().getCount() == 2);
    SLANG_CHECK(callRecord->getArgument(1)->value == arguments[1].value);
    const auto* callee =
        storage.getRecord<StagedRecord::Function>(uint32_t(function.getUnsignedPayload()));
    SLANG_CHECK(callee->entryProgramCounter == entry && callee->exitProgramCounter == exit);
    const auto* parameter = callee->getFirstParameter();
    SLANG_CHECK_ABORT(parameter);
    SLANG_CHECK(parameter->getManagedLeafOffsets().getCount() == 1);
    SLANG_CHECK(*parameter->getManagedLeafOffset(0) == 8);
    SLANG_CHECK(parameter->header.resultAddress.value == arguments[0].value);
    // An odd number of 32-bit leaf offsets leaves padding before the next aligned record.
    parameter = parameter->getNext();
    SLANG_CHECK(uintptr_t(parameter) % StagedRecord::kRecordAlignment == 0);
    SLANG_CHECK(parameter->getManagedLeafOffsets().getCount() == 0);
    SLANG_CHECK(parameter->header.resultAddress.value == arguments[1].value);
    SLANG_CHECK(
        storage.getRecord<StagedRecord::Branch>(uint32_t(branch.getUnsignedPayload()))
            ->targetProgramCounter == entry);
    const auto* conditionalRecord = storage.getRecord<StagedRecord::BranchConditional>(
        uint32_t(conditional.getUnsignedPayload()));
    SLANG_CHECK(conditionalRecord->trueProgramCounter == entry);
    SLANG_CHECK(conditionalRecord->falseProgramCounter == exit);
}

SLANG_UNIT_TEST(hgvmStagedRecordOperands)
{
    StagedRecordStorage storage;
    StagedRecordBuilder builder(storage);
    // A literal with a non-word-sized payload must not misalign the next record or its tail.
    const char literal[] = {'a', 'b', 'c'};
    builder.appendAlignedBytes(literal, sizeof(literal));
    Address values[] = {Address::makeRelativeFrame(8), Address::makeRelativeFrame(16)};
    StagedRecord::FormatArgument formatArguments[] = {
        {values[0], ValueKind::Scalar, ScalarType::UInt32},
        {values[1], ValueKind::String, ScalarType::UInt32}};
    Address format = builder.emitFormatString(values[0], values[1], makeArrayView(formatArguments));
    Address emptyFormat = builder.emitFormatString(values[0], values[1], {});
    StagedRecord::DispatchArgument bindings[] = {{values[0], values[1]}};
    Address dispatch = builder.emitComputeCall(values[0], values[1], makeArrayView(bindings));
    Address graphics =
        builder.emitGraphicsCall(values[0], values[1], {}, {}, {}, makeArrayView(bindings));
    Address module = builder.emitSlangModule(values[0], 3, values[1], 4, makeArrayView(values));
    StagedRecord::SpecializationArgument specialization[] = {
        {HGBCSpecializationArgKind::Type, values[0], 3}};
    Address specialized =
        builder.emitSpecializeSlangEntryPoint(values[1], makeArrayView(specialization));
    StagedRecord::AggregateElementCopy elements[] = {{values[1], 4, 4}};
    Address aggregate =
        builder.emitMakeAggregate(HGBCOpcode::MakeStruct, values[0], 8, makeArrayView(elements));
    Address numeric = builder.emitMakeNumeric(
        HGBCOpcode::MakeVector,
        values[0],
        ScalarType::UInt32,
        makeArrayView(values));
    StagedRecord::TypeInfo type;
    type.kind = TypeKind::Struct;
    type.byteSize = 8;
    Address fieldNameAddress = Address::makeAbsoluteInstruction(
        builder.appendAlignedBytes(literal, sizeof(literal)));
    StagedRecord::StructFieldInfo fields[] = {{values[0], fieldNameAddress, sizeof(literal), 4}};
    Address typeAddress = builder.emitTypeInfo(type, makeArrayView(fields), {});
    builder.finalize();

    const auto* formatRecord =
        storage.getRecord<StagedRecord::FormatString>(uint32_t(format.getUnsignedPayload()));
    SLANG_CHECK(formatRecord->getArgument(1)->kind == ValueKind::String);
    SLANG_CHECK(formatRecord->getArgument(0)->valueAddress.value == values[0].value);
    SLANG_CHECK(
        format.getUnsignedPayload() + formatRecord->header.recordWordCount ==
        emptyFormat.getUnsignedPayload());
    const auto* emptyRecord =
        storage.getRecord<StagedRecord::FormatString>(uint32_t(emptyFormat.getUnsignedPayload()));
    SLANG_CHECK(emptyRecord->getArguments().getCount() == 0);
    SLANG_CHECK(
        emptyFormat.getUnsignedPayload() + emptyRecord->header.recordWordCount ==
        dispatch.getUnsignedPayload());
    SLANG_CHECK(
        storage.getRecord<StagedRecord::ComputeCall>(uint32_t(dispatch.getUnsignedPayload()))
            ->getArgument(0)
            ->converterAddress.value == values[1].value);
    SLANG_CHECK(
        storage.getRecord<StagedRecord::GraphicsCall>(uint32_t(graphics.getUnsignedPayload()))
            ->getArguments()
            .getCount() == 1);
    SLANG_CHECK(
        storage.getRecord<StagedRecord::LazyObject>(uint32_t(module.getUnsignedPayload()))
            ->getDependencies()[1]
            .value == values[1].value);
    SLANG_CHECK(
        storage.getRecord<StagedRecord::LazyObject>(uint32_t(specialized.getUnsignedPayload()))
            ->getSpecializationArguments()[0]
            .valueByteSize == 3);
    SLANG_CHECK(
        storage.getRecord<StagedRecord::MakeAggregate>(uint32_t(aggregate.getUnsignedPayload()))
            ->getElementCopy(0)
            ->destinationByteOffset == 4);
    SLANG_CHECK(
        storage.getRecord<StagedRecord::MakeNumeric>(uint32_t(numeric.getUnsignedPayload()))
            ->getLane(1)
            ->value == values[1].value);
    const auto* typeRecord =
        storage.getRecord<StagedRecord::TypeInfo>(uint32_t(typeAddress.getUnsignedPayload()));
    SLANG_CHECK(typeRecord->getFields().getCount() == 1);
    SLANG_CHECK(typeRecord->getFields()[0].fieldNameAddress.value == fieldNameAddress.value);
    SLANG_CHECK(typeRecord->getFields()[0].fieldNameByteCount == sizeof(literal));
}

SLANG_UNIT_TEST(hgvmStagedRecordSizeLimit)
{
    StagedRecordStorage storage;
    StagedRecordBuilder builder(storage);
    List<Address> arguments;
    arguments.setCount(32768);
    bool rejected = false;
    try
    {
        builder.emitCall({}, arguments.getArrayView());
    }
    catch (const Exception&)
    {
        rejected = true;
    }
    SLANG_CHECK(rejected);
    SLANG_CHECK(storage.getWordCount() == 0);
    // Failed emission must not leave behind an unresolved fixup or a partially written record.
    builder.emitReturn();
    builder.finalize();
}

SLANG_UNIT_TEST(hgvmStagedRecordAppend)
{
    StagedRecordStorage storage;
    Address function;
    uint32_t entry = 0;
    {
        StagedRecordBuilder builder(storage);
        function = builder.emitFunction(64, 8);
        entry = builder.getWordOffset();
        builder.emitReturn();
        uint32_t exit = builder.getWordOffset();
        builder.emitFinishFrameExit();
        builder.patchFunction(function, entry, exit, 0, 0);
        builder.finalize();
    }

    const Index firstWordCount = storage.getWordCount();
    Address call;
    {
        StagedRecordBuilder builder(storage);
        SLANG_CHECK(builder.getWordOffset() == firstWordCount);
        call = builder.emitCall({}, {});
        // A new call may refer to an already committed function without patching that function.
        builder.patchCallTarget(call, function);
        for (uint32_t i = 0; i < 4096; ++i)
            builder.emitReturn();

        SLANG_CHECK(storage.getWordCount() == firstWordCount);
        SLANG_CHECK(
            storage.getRecord<StagedRecord::Function>(uint32_t(function.getUnsignedPayload()))
                ->entryProgramCounter == entry);
        SLANG_CHECK(
            builder.getRecord<StagedRecord::Call>(uint32_t(call.getUnsignedPayload()))
                ->functionAddress.value == function.value);
        SLANG_CHECK(builder.getWordOffset() > firstWordCount);
        builder.finalize();
    }

    SLANG_CHECK(storage.getWordCount() > firstWordCount);
    SLANG_CHECK(call.getUnsignedPayload() == uint64_t(firstWordCount));
    SLANG_CHECK(
        storage.getRecord<StagedRecord::Call>(uint32_t(call.getUnsignedPayload()))
            ->functionAddress.value == function.value);
}

SLANG_UNIT_TEST(hgvmStagedRecordRollback)
{
    StagedRecordStorage storage;
    Address original;
    {
        StagedRecordBuilder builder(storage);
        original = builder.emitReturn();
        builder.finalize();
    }
    const size_t committedSize = storage.getByteSize();
    const uint32_t nextWord = uint32_t(storage.getWordCount());
    {
        StagedRecordBuilder builder(storage);
        // Unresolved fixups belong to this transaction and must not survive its destruction.
        builder.emitBranch();
        for (uint32_t i = 0; i < 4096; ++i)
            builder.emitReturn();
        SLANG_CHECK(builder.getWordOffset() > nextWord);
    }
    SLANG_CHECK(storage.getByteSize() == committedSize);
    SLANG_CHECK(
        storage.getRecord<StagedRecord::Header>(uint32_t(original.getUnsignedPayload()))
            ->opcodeAndFlags == uint32_t(HGBCOpcode::Return));

    {
        StagedRecordBuilder builder(storage);
        SLANG_CHECK(builder.getWordOffset() == nextWord);
        builder.emitFunction(64, 8);
        builder.rollback();
    }
    SLANG_CHECK(storage.getByteSize() == committedSize);
    {
        StagedRecordBuilder builder(storage);
        SLANG_CHECK(builder.emitReturn().getUnsignedPayload() == nextWord);
        builder.finalize();
    }
    SLANG_CHECK(storage.getByteSize() > committedSize);
}

SLANG_UNIT_TEST(hgvmStagedRecordCommittedPatch)
{
    StagedRecordStorage storage;
    Address branch;
    uint32_t originalTarget = 0;
    {
        StagedRecordBuilder builder(storage);
        branch = builder.emitBranch();
        originalTarget = builder.getWordOffset();
        builder.emitReturn();
        builder.patchBranchTarget(branch, originalTarget);
        builder.finalize();
    }
    {
        StagedRecordBuilder builder(storage);
        uint32_t newTarget = builder.getWordOffset();
        builder.emitReturn();
        bool rejected = false;
        try
        {
            builder.patchBranchTarget(branch, newTarget);
        }
        catch (const Exception&)
        {
            rejected = true;
        }
        SLANG_CHECK(rejected);
        builder.finalize();
    }
    SLANG_CHECK(
        storage.getRecord<StagedRecord::Branch>(uint32_t(branch.getUnsignedPayload()))
            ->targetProgramCounter == originalTarget);
}

SLANG_UNIT_TEST(hgvmStagedRecordCanonicalTypes)
{
    StagedRecordStorage storage;
    StagedRecordBuilder builder(storage);
    Address type = Address::makeAbsoluteInstruction(256);
    Address arrayType = Address::makeAbsoluteInstruction(512);
    Address slot = Address::makeRelativeFrame(8);
    uint32_t managedOffsets[] = {0};
    Address box = builder.emitBox(slot, slot, type, 8, 8, makeArrayView(managedOffsets));
    Address field = builder.emitFieldExtract(slot, slot, 16, 8, type);
    Address allocation = builder.emitAllocateRuntimeArray(
        slot,
        slot,
        arrayType,
        8,
        8,
        ScalarType::UInt32,
        makeArrayView(managedOffsets));
    StagedRecord::TaggedUnionCase cases[] = {{0, {}}, {1, type}};
    Address tagged = builder.emitMakeTaggedUnion(slot, slot, slot, makeArrayView(cases));
    builder.finalize();

    const auto* boxRecord =
        storage.getRecord<StagedRecord::Box>(uint32_t(box.getUnsignedPayload()));
    SLANG_CHECK(boxRecord->payloadTypeAddress.value == type.value);
    SLANG_CHECK(boxRecord->getManagedLeafOffsets().getCount() == 1);
    SLANG_CHECK(boxRecord->getManagedLeafOffsets()[0] == 0);
    SLANG_CHECK(
        storage.getRecord<StagedRecord::FieldExtract>(uint32_t(field.getUnsignedPayload()))
            ->classTypeAddress.value == type.value);
    const auto* arrayRecord = storage.getRecord<StagedRecord::AllocateRuntimeArray>(
        uint32_t(allocation.getUnsignedPayload()));
    SLANG_CHECK(arrayRecord->runtimeArrayTypeAddress.value == arrayType.value);
    SLANG_CHECK(arrayRecord->getManagedLeafOffsets().getCount() == 1);
    const auto* taggedRecord =
        storage.getRecord<StagedRecord::MakeTaggedUnion>(uint32_t(tagged.getUnsignedPayload()));
    SLANG_CHECK(!taggedRecord->getCases()[0].payloadTypeAddress);
    SLANG_CHECK(taggedRecord->getCases()[1].payloadTypeAddress.value == type.value);
}
