// Distinguish a possible memory effect from proof of a known shader exit.
#include "slang/slang-ir-util.h"
#include "static-unit-test-env.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

SLANG_UNIT_TEST(irShaderTerminationSeparatesUnknownEffectsFromKnownExits)
{
    StaticUnitTestEnv env(unitTestContext);
    IRFixtureBuilder fixture(env.getSessionImpl());
    IRBuilder builder(fixture.getModule());
    const auto known = ShaderTerminationQueryMode::KnownExitsOnly;
    const auto conservative = ShaderTerminationQueryMode::IncludeUnresolvedCalls;

    // A function-valued formal is canonical before specialization resolves its call.
    // It must protect local stores, but cannot justify forced hoisting or E55214.
    builder.setInsertInto(fixture.getModule());
    auto wrapper = builder.createFunc();
    IRType* callableType = builder.getFuncType(0, nullptr, builder.getVoidType());
    wrapper->setFullType(builder.getFuncType(1, &callableType, builder.getVoidType()));
    builder.setInsertInto(wrapper);
    builder.emitBlock();
    auto unknown = builder.emitParam(callableType);
    auto local = builder.emitVar(builder.getUIntType());
    auto call = builder.emitCallInst(builder.getVoidType(), unknown, 0, nullptr);
    builder.emitReturn();
    SLANG_CHECK(!mayInvokeShaderTerminatingIntrinsic(wrapper, known));
    SLANG_CHECK(mayInvokeShaderTerminatingIntrinsic(wrapper, conservative));
    SLANG_CHECK(canInstHaveSideEffectAtAddress(wrapper, call, local, nullptr));

    auto exit = fixture.addFunctionEndingInGenericAsm("exit");
    builder.addKnownBuiltinDecoration(exit, KnownBuiltinDeclName::AcceptHitAndEndSearch);
    auto knownWrapper = fixture.addVoidFunctionCalling("knownWrapper", false, exit);
    SLANG_CHECK(mayInvokeShaderTerminatingIntrinsic(knownWrapper, known));
    SLANG_CHECK(mayInvokeShaderTerminatingIntrinsic(knownWrapper, conservative));

    // Visit both roots of a cycle, then add and remove a marked call. Queries must
    // terminate and reflect the current graph rather than a cached partial DFS result.
    auto a = fixture.addVoidFunction("a", false);
    auto b = fixture.addVoidFunctionCalling("b", false, a);
    builder.setInsertBefore(a->getFirstBlock()->getTerminator());
    auto cycleCall = builder.emitCallInst(builder.getVoidType(), b, 0, nullptr);
    for (auto mode : {known, conservative})
    {
        SLANG_CHECK(!mayInvokeShaderTerminatingIntrinsic(a, mode));
        SLANG_CHECK(!mayInvokeShaderTerminatingIntrinsic(b, mode));
    }
    builder.setInsertBefore(cycleCall);
    auto addedCall = builder.emitCallInst(builder.getVoidType(), exit, 0, nullptr);
    for (auto mode : {known, conservative})
    {
        SLANG_CHECK(mayInvokeShaderTerminatingIntrinsic(a, mode));
        SLANG_CHECK(mayInvokeShaderTerminatingIntrinsic(b, mode));
    }
    addedCall->removeAndDeallocate();
    SLANG_CHECK(!mayInvokeShaderTerminatingIntrinsic(a, known));
    SLANG_CHECK(!mayInvokeShaderTerminatingIntrinsic(b, known));
}
