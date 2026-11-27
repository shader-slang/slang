// Tests that untagged unions are lowered in dependency order, so the type that a union lowers to
// does not depend on where it and the unions it holds are placed in the module.

#include "compiler-core/slang-diagnostic-sink.h"
#include "slang/ir-passes/slang-ir-lower-dynamic-dispatch-insts.h"
#include "slang/slang-module.h"
#include "slang/slang-session.h"
#include "static-unit-test-env.h"
#include "unit-test/slang-unit-test.h"

#include <initializer_list>

using namespace Slang;

namespace
{

// A module of hand-built untagged unions, laid out in a chosen order and then lowered.
struct UntaggedUnionFixture
{
    explicit UntaggedUnionFixture(UnitTestContext* context)
        : env(context), module(IRModule::create(env.getSessionImpl())), builder(module.get())
    {
        builder.setInsertInto(module.get());

        auto targetModule = env.checkModuleFromSource("untaggedUnionTarget", "");
        SLANG_CHECK_ABORT(targetModule);
        auto linkage = targetModule->getLinkage();
        SLANG_CHECK_ABORT(linkage->targets.getCount() == 1);
        targetProgram = targetModule->getTargetProgram(linkage->targets[0]);
    }

    IRUntaggedUnionType* makeUnion(IRInst* a, IRInst* b)
    {
        HashSet<IRInst*> elements;
        elements.add(a);
        elements.add(b);
        return builder.getUntaggedUnionType(builder.getSet(kIROp_TypeSet, elements));
    }

    // Move `insts` to the end of the module, in the given order.
    void placeAtEndInOrder(std::initializer_list<IRInst*> insts)
    {
        for (auto inst : insts)
        {
            inst->removeFromParent();
            inst->insertAtEnd(module->getModuleInst());
        }
    }

    // Lower the untagged unions and return the number of errors reported.
    Index lowerAndGetErrorCount()
    {
        DiagnosticSink sink;
        lowerUntaggedUnionTypes(module.get(), targetProgram, &sink);
        return sink.getErrorCount();
    }

    // Lower the untagged unions, check that no errors are reported, and return the size of the
    // `AnyValue` that `var` then stores.
    IRIntegerValue lowerAndGetStoredAnyValueSize(IRGlobalVar* var)
    {
        SLANG_CHECK_ABORT(lowerAndGetErrorCount() == 0);

        auto storedType = cast<IRPtrTypeBase>(var->getDataType())->getValueType();
        auto anyValueType = as<IRAnyValueType>(storedType);
        SLANG_CHECK_ABORT(anyValueType);
        return getIntVal(anyValueType->getSize());
    }

    // Declaration order matters: `module` is created from `env`, and `builder` from `module`.
    StaticUnitTestEnv env;
    RefPtr<IRModule> module;
    IRBuilder builder;
    TargetProgram* targetProgram = nullptr;
};

} // namespace

// The outer union comes before the inner union it holds, as happens when lowering a tagged union
// creates the inner union after the unions that hold it.
SLANG_UNIT_TEST(irUntaggedUnionLoweringSizesUnloweredNestedUnion, NoRenderApi)
{
    UntaggedUnionFixture fixture(unitTestContext);
    auto& builder = fixture.builder;
    auto floatType = builder.getFloatType();

    auto inner = fixture.makeUnion(floatType, builder.getVectorType(floatType, 4));
    auto outer = fixture.makeUnion(builder.getTupleType(inner, builder.getIntType()), floatType);
    auto outerVar = builder.createGlobalVar(outer);
    fixture.placeAtEndInOrder({outer, inner});

    // `inner` takes 16 bytes, so the tuple holding it takes 20.
    SLANG_CHECK(fixture.lowerAndGetStoredAnyValueSize(outerVar) == 20);
}

// Lowering an inner union can leave the outer union with only one payload type, which the outer
// union is then replaced by, whichever of the two comes first in the module.
SLANG_UNIT_TEST(irUntaggedUnionLoweringMergesPayloadsOfNestedUnion, NoRenderApi)
{
    for (bool outerFirst : {true, false})
    {
        UntaggedUnionFixture fixture(unitTestContext);
        auto& builder = fixture.builder;
        auto doubleType = builder.getDoubleType();

        auto inner = fixture.makeUnion(doubleType, builder.getNoneTypeElement());
        auto outer = fixture.makeUnion(inner, doubleType);
        auto outerVar = builder.createGlobalVar(outer);
        if (outerFirst)
            fixture.placeAtEndInOrder({outer, inner});
        else
            fixture.placeAtEndInOrder({inner, outer});

        SLANG_CHECK_ABORT(fixture.lowerAndGetErrorCount() == 0);
        SLANG_CHECK(cast<IRPtrTypeBase>(outerVar->getDataType())->getValueType() == doubleType);
    }
}

// A union whose only payload is a non-copyable struct stores that struct directly, so a union
// holding it cannot be packed into an `AnyValue`, whichever of the two comes first in the module.
SLANG_UNIT_TEST(irUntaggedUnionLoweringDiagnosesNestedUnstorablePayload, NoRenderApi)
{
    for (bool outerFirst : {true, false})
    {
        UntaggedUnionFixture fixture(unitTestContext);
        auto& builder = fixture.builder;
        auto floatType = builder.getFloatType();

        auto nonCopyable = builder.createStructType();
        builder.createStructField(nonCopyable, builder.createStructKey(), floatType);
        builder.addDecoration(nonCopyable, kIROp_NonCopyableTypeDecoration);

        auto inner = fixture.makeUnion(nonCopyable, builder.getNoneTypeElement());
        auto outer =
            fixture.makeUnion(builder.getTupleType(inner, builder.getIntType()), floatType);
        builder.createGlobalVar(outer);
        if (outerFirst)
            fixture.placeAtEndInOrder({outer, inner});
        else
            fixture.placeAtEndInOrder({inner, outer});

        SLANG_CHECK(fixture.lowerAndGetErrorCount() == 1);
    }
}
