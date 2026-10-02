// Distinguish a possible memory effect from proof of a known shader exit.
#include "slang/slang-ir-defer-buffer-load.h"
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

SLANG_UNIT_TEST(irImageAccessPreservesLocalStorage)
{
    StaticUnitTestEnv env(unitTestContext);
    IRFixtureBuilder fixture(env.getSessionImpl());
    IRBuilder builder(fixture.getModule());
    builder.setInsertInto(fixture.getModule());
    auto zero = builder.getIntValue(builder.getIntType(), 0);
    auto one = builder.getIntValue(builder.getIntType(), 1);
    auto texture = builder.getTextureType(
        builder.getUIntType(),
        builder.getType(kIROp_TextureShape1DType),
        zero,
        zero,
        zero,
        one,
        zero,
        zero,
        zero);
    auto array =
        builder.getArrayType(builder.getUIntType(), builder.getIntValue(builder.getIntType(), 2));
    auto record = builder.createStructType();
    auto field = builder.createStructField(record, builder.createStructKey(), array);
    auto global = builder.createGlobalVar(builder.getUIntType());
    auto function = builder.createFunc();
    IRType* params[] = {texture, builder.getPtrType(builder.getUIntType())};
    function->setFullType(builder.getFuncType(2, params, builder.getVoidType()));
    builder.setInsertInto(function);
    builder.emitBlock();
    auto image = builder.emitParam(texture);
    auto pointer = builder.emitParam(params[1]);
    auto local = builder.emitVar(record);
    auto fieldAddress = builder.emitFieldAddress(local, field->getKey());
    auto elementAddress = builder.emitElementAddress(fieldAddress, zero);
    ShortList<IRInst*> args;
    args.add(image);
    args.add(zero);
    auto read = builder.emitImageLoad(builder.getUIntType(), args);
    args.add(builder.getIntValue(builder.getUIntType(), 7));
    auto write = builder.emitImageStore(builder.getVoidType(), args);
    auto snapshot = builder.emitLoad(local);
    auto beforeWrite = builder.emitFieldExtract(array, snapshot, field->getKey());
    auto store = builder.emitStore(elementAddress, args[2]);
    auto afterWrite = builder.emitFieldExtract(array, snapshot, field->getKey());
    builder.emitReturn();

    // A required aggregate-leaf rewrite must still preserve a value loaded before mutation.
    SLANG_CHECK(isMemoryLocationUnmodifiedBetweenLoadAndUser(nullptr, snapshot, beforeWrite));
    SLANG_CHECK(!isMemoryLocationUnmodifiedBetweenLoadAndUser(nullptr, snapshot, afterWrite));

    for (auto access : {read, write})
    {
        // Texel memory and local aggregate storage are distinct. This does not assert that
        // different texture handles, globals or arbitrary caller addresses cannot alias.
        for (IRInst* address : {static_cast<IRInst*>(local), fieldAddress, elementAddress})
            SLANG_CHECK(!canInstHaveSideEffectAtAddress(function, access, address, nullptr));
        for (IRInst* address : {static_cast<IRInst*>(global), static_cast<IRInst*>(pointer)})
            SLANG_CHECK(canInstHaveSideEffectAtAddress(function, access, address, nullptr));
    }
    SLANG_CHECK(canInstHaveSideEffectAtAddress(function, store, elementAddress, nullptr));
    SLANG_CHECK(canInstHaveSideEffectAtAddress(function, store, local, nullptr));
}
