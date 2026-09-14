// unit-test-ir-transform-params-to-constref.cpp
//
// Tests the ownership boundary between native value ABIs and the generic aggregate-parameter
// optimization. A target-intrinsic spelling alone is not such a boundary because it can apply to
// a different target; only the compiler producer that requires a native value ABI may opt out.

#include "compiler-core/slang-diagnostic-sink.h"
#include "slang/slang-ir-transform-params-to-constref.h"
#include "static-unit-test-env.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

static IRStructType* _createOneFieldStruct(IRBuilder& builder)
{
    auto type = builder.createStructType();
    auto key = builder.createStructKey();
    builder.createStructField(type, key, builder.getFloatType());
    return type;
}

static IRParam* _createIdentityParameterFunction(IRBuilder& builder, IRType* parameterType)
{
    auto function = builder.createFunc();
    IRType* parameterTypes[] = {parameterType};
    function->setFullType(builder.getFuncType(
        SLANG_COUNT_OF(parameterTypes),
        parameterTypes,
        builder.getVoidType()));

    builder.setInsertInto(function);
    builder.emitBlock();
    auto parameter = builder.emitParam(parameterType);
    builder.emitReturn();
    return parameter;
}

SLANG_UNIT_TEST(transformParamsToConstRefUsesExplicitValueABIMarker)
{
    StaticUnitTestEnv env(unitTestContext);
    RefPtr<IRModule> module = IRModule::create(env.getSessionImpl());
    IRBuilder builder(module);
    builder.setInsertInto(module->getModuleInst());

    // Consider a source struct with an HLSL-only intrinsic spelling. On another target it remains
    // an ordinary aggregate, so the presence of that unrelated decoration must not disable the
    // normal `borrow in` optimization.
    auto ordinaryType = _createOneFieldStruct(builder);
    builder.addTargetIntrinsicDecoration(
        ordinaryType,
        CapabilitySet(Slang::CapabilityName::hlsl),
        UnownedTerminatedStringSlice("HlslOnlyType"));
    auto ordinaryParameter = _createIdentityParameterFunction(builder, ordinaryType);

    // A compiler producer that really owns a native value ABI states that invariant explicitly.
    // This mirrors the synthesized Metal `array_ref` type without coupling the pass test to ray
    // tracing lowering.
    builder.setInsertInto(module->getModuleInst());
    auto nativeValueType = _createOneFieldStruct(builder);
    builder.addDecoration(nativeValueType, kIROp_PreserveValueParameterABIDecoration);
    auto nativeValueParameter = _createIdentityParameterFunction(builder, nativeValueType);

    DiagnosticSink sink;
    SLANG_CHECK(SLANG_SUCCEEDED(transformParamsToConstRef(module, &sink)));

    auto transformedType = as<IRBorrowInParamType>(ordinaryParameter->getDataType());
    SLANG_CHECK_ABORT(transformedType);
    SLANG_CHECK(transformedType->getValueType() == ordinaryType);
    SLANG_CHECK(nativeValueParameter->getDataType() == nativeValueType);
}
