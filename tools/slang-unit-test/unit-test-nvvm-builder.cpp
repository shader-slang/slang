// unit-test-nvvm-builder.cpp

#include "unit-test-nvvm-support.h"

SLANG_UNIT_TEST(nvvmIRBuilderRejectsInvalidABI)
{
    NVVMIRBuilder builder;
    SlangNVVMModuleHandle_1 module = nullptr;
    SLANG_CHECK(builder.createModule(toSlice("uninitialized"), module) == SLANG_E_UNINITIALIZED);
    SLANG_CHECK(module == nullptr);

    gFakeNVVMBuilder.reset();
    SlangNVVMBuilderAPI_V1 invalidAPI = _makeFakeNVVMBuilderAPI();
    invalidAPI.structureSize -= 1;
    SLANG_CHECK(NVVMIRBuilder::initialize(invalidAPI, nullptr, builder) == SLANG_E_NO_INTERFACE);
    invalidAPI = _makeFakeNVVMBuilderAPI();
    invalidAPI.abiVersion += 1;
    SLANG_CHECK(NVVMIRBuilder::initialize(invalidAPI, nullptr, builder) == SLANG_E_NO_INTERFACE);
    invalidAPI = _makeFakeNVVMBuilderAPI();
    invalidAPI.llvmVersionMajor = 15;
    SLANG_CHECK(NVVMIRBuilder::initialize(invalidAPI, nullptr, builder) == SLANG_E_NO_INTERFACE);
    invalidAPI = _makeFakeNVVMBuilderAPI();
    invalidAPI.pointerModel = 0;
    SLANG_CHECK(NVVMIRBuilder::initialize(invalidAPI, nullptr, builder) == SLANG_E_NO_INTERFACE);
    invalidAPI = _makeFakeNVVMBuilderAPI();
    invalidAPI.serializeModule = nullptr;
    SLANG_CHECK(NVVMIRBuilder::initialize(invalidAPI, nullptr, builder) == SLANG_E_NO_INTERFACE);
    SLANG_CHECK(
        NVVMIRBuilder::initialize(_makeFakeNVVMBuilderAPI(), nullptr, builder) ==
        SLANG_E_INVALID_ARG);

    gFakeNVVMBuilder.api = _makeFakeNVVMBuilderAPI();
    gFakeNVVMBuilder.omitAPISymbol = true;
    ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
    SLANG_CHECK(NVVMIRBuilder::load(String(), loader, builder) == SLANG_E_NO_INTERFACE);
    SLANG_CHECK(gFakeNVVMBuilder.loadedPath == "slang-llvm-nvvm");
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVMBuilder.destroyedLibraryCount == 1);

    // Once a provider advertises V2, a malformed V2 table is an incompatibility rather than a
    // reason to silently downgrade to its otherwise valid V1 export.
    gFakeNVVMBuilder.reset();
    gFakeNVVMBuilder.api = _makeFakeNVVMBuilderAPI();
    gFakeNVVMBuilder.apiV2 = _makeFakeNVVMBuilderAPIV2();
    gFakeNVVMBuilder.apiV2.serializeModuleWithDiagnostics = nullptr;
    gFakeNVVMBuilder.omitAPIV2Symbol = false;
    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder rejectedBuilder;
        SLANG_CHECK(NVVMIRBuilder::load(String(), loader, rejectedBuilder) == SLANG_E_NO_INTERFACE);
        SLANG_CHECK(!rejectedBuilder.isInitialized());
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVMBuilder.destroyedLibraryCount == 1);

    gFakeNVVMBuilder.reset();
    gFakeNVVMBuilder.api = _makeFakeNVVMBuilderAPI();
    {
        ComPtr<ISlangSharedLibraryLoader> nullHandleLoader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder nullHandleBuilder;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(NVVMIRBuilder::load(String(), nullHandleLoader, nullHandleBuilder)));
        gFakeNVVMBuilder.returnNullModule = true;
        module = nullptr;
        SLANG_CHECK(
            nullHandleBuilder.createModule(toSlice("missing-output"), module) == SLANG_FAIL);
        SLANG_CHECK(module == nullptr);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVMBuilder.destroyedLibraryCount == 1);
}

SLANG_UNIT_TEST(nvvmIRBuilderNegotiatesV3Features)
{
    const uint32_t featureOffset = sizeof(void*) == 8 ? 392u : 224u;
    const uint32_t unaryOffset = sizeof(void*) == 8 ? 424u : 256u;
    const uint32_t binaryOffset = sizeof(void*) == 8 ? 432u : 260u;
    const uint32_t compareOffset = sizeof(void*) == 8 ? 440u : 264u;
    const uint32_t minimumSize = sizeof(void*) == 8 ? 448u : 268u;
    const uint32_t floatingTypeOffset = sizeof(void*) == 8 ? 448u : 268u;
    const uint32_t floatingBinaryOffset = sizeof(void*) == 8 ? 456u : 272u;
    const uint32_t floatingMinimumSize = sizeof(void*) == 8 ? 464u : 276u;
    const uint32_t completeSize = sizeof(void*) == 8 ? 464u : 280u;
    SLANG_CHECK(offsetof(SlangNVVMBuilderAPI_V3, features) == featureOffset);
    SLANG_CHECK(offsetof(SlangNVVMBuilderAPI_V3, emitIntegerUnary) == unaryOffset);
    SLANG_CHECK(offsetof(SlangNVVMBuilderAPI_V3, emitIntegerBinary) == binaryOffset);
    SLANG_CHECK(offsetof(SlangNVVMBuilderAPI_V3, emitIntegerCompare) == compareOffset);
    SLANG_CHECK(offsetof(SlangNVVMBuilderAPI_V3, getFloatingPointType) == floatingTypeOffset);
    SLANG_CHECK(offsetof(SlangNVVMBuilderAPI_V3, emitFloatingBinary) == floatingBinaryOffset);
    SLANG_CHECK(SLANG_NVVM_BUILDER_API_V3_MIN_SIZE == minimumSize);
    SLANG_CHECK(SLANG_NVVM_BUILDER_API_V3_SCALAR_FLOAT32_ADD_MIN_SIZE == floatingMinimumSize);
    SLANG_CHECK(sizeof(SlangNVVMBuilderAPI_V3) == completeSize);

    gFakeNVVMBuilder.reset();
    {
        ComPtr<ISlangSharedLibrary> library(new FakeNVVMBuilderLibrary);
        SlangNVVMBuilderAPI_V3 api = _makeFakeNVVMBuilderAPIV3();
        api.features.words[SLANG_NVVM_BUILDER_FEATURE_SCALAR_INTEGER_MULTIPLY / 64u] &=
            ~(uint64_t(1) << (SLANG_NVVM_BUILDER_FEATURE_SCALAR_INTEGER_MULTIPLY % 64u));

        NVVMIRBuilder builder;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(NVVMIRBuilder::initialize(api, library, builder)));
        SLANG_CHECK_ABORT(builder.getAPIV3() != nullptr);
        SLANG_CHECK_ABORT(builder.getAPIV2() != nullptr);
        SLANG_CHECK(builder.getAPIV3()->structureSize == sizeof(SlangNVVMBuilderAPI_V3));
        SLANG_CHECK(builder.getVersionString().indexOf("builder-abi=3") >= 0);
        SLANG_CHECK(builder.getVersionString().indexOf("feature-words=") >= 0);
        SLANG_CHECK(builder.supportsScalarOperations());
        SLANG_CHECK(!builder.supportsScalarIntegerMultiply());
        SLANG_CHECK(builder.supportsRawRWStructuredBufferI32());

        SlangNVVMBuilderFeatureSet_3 requiredFeatures = {};
        requiredFeatures.words[SLANG_NVVM_BUILDER_FEATURE_SCALAR_MEMORY / 64u] |=
            uint64_t(1) << (SLANG_NVVM_BUILDER_FEATURE_SCALAR_MEMORY % 64u);
        SLANG_CHECK(builder.supportsFeatures(requiredFeatures));
        requiredFeatures.words[SLANG_NVVM_BUILDER_FEATURE_SCALAR_INTEGER_MULTIPLY / 64u] |=
            uint64_t(1) << (SLANG_NVVM_BUILDER_FEATURE_SCALAR_INTEGER_MULTIPLY % 64u);
        SLANG_CHECK(!builder.supportsFeatures(requiredFeatures));
        requiredFeatures = {};
        requiredFeatures.words[0] = uint64_t(1) << SLANG_NVVM_BUILDER_FEATURE_COUNT_3;
        SLANG_CHECK(!builder.supportsFeatures(requiredFeatures));

        SlangNVVMValueHandle_1 value = reinterpret_cast<SlangNVVMValueHandle_1>(uintptr_t(1));
        SLANG_CHECK(
            builder.emitIntegerUnary(nullptr, SlangNVVMIntegerUnaryOp_3(99), nullptr, value) ==
            SLANG_E_INVALID_ARG);
        SLANG_CHECK(value == nullptr);
        SLANG_CHECK(
            gFakeNVVMBuilder.scalarV3FamilyCallCounts[Index(FakeNVVMBuilderScalarFamily::Unary)] ==
            0);
        value = reinterpret_cast<SlangNVVMValueHandle_1>(uintptr_t(1));
        SLANG_CHECK(
            builder.emitIntegerBinaryOperation(
                nullptr,
                SlangNVVMIntegerBinaryOp_3(99),
                nullptr,
                nullptr,
                value) == SLANG_E_INVALID_ARG);
        SLANG_CHECK(value == nullptr);
        SLANG_CHECK(
            gFakeNVVMBuilder.scalarV3FamilyCallCounts[Index(FakeNVVMBuilderScalarFamily::Binary)] ==
            0);
        value = reinterpret_cast<SlangNVVMValueHandle_1>(uintptr_t(1));
        SLANG_CHECK(
            builder.emitIntegerCompare(
                nullptr,
                SlangNVVMIntegerCompareOp_3(99),
                nullptr,
                nullptr,
                value) == SLANG_E_INVALID_ARG);
        SLANG_CHECK(value == nullptr);
        SLANG_CHECK(
            gFakeNVVMBuilder
                .scalarV3FamilyCallCounts[Index(FakeNVVMBuilderScalarFamily::Compare)] == 0);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmIRBuilderNegotiatesFloat32AddAPI)
{
    // The exact Slice 28 V3 core remains valid when it does not advertise the appended semantic.
    gFakeNVVMBuilder.reset();
    {
        ComPtr<ISlangSharedLibrary> library(new FakeNVVMBuilderLibrary);
        SlangNVVMBuilderAPI_V3 api = _makeFakeNVVMBuilderAPIV3();
        api.features.words[SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_ADD / 64u] &=
            ~(uint64_t(1) << (SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_ADD % 64u));
        api.features.words[SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_SUBTRACT / 64u] &=
            ~(uint64_t(1) << (SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_SUBTRACT % 64u));
        api.features.words[SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_MULTIPLY / 64u] &=
            ~(uint64_t(1) << (SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_MULTIPLY % 64u));
        api.structureSize = uint32_t(SLANG_NVVM_BUILDER_API_V3_MIN_SIZE);
        NVVMIRBuilder builder;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(NVVMIRBuilder::initialize(api, library, builder)));
        SLANG_CHECK(!builder.supportsFeature(SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_ADD));
        SlangNVVMTypeHandle_1 type = _getFakeNVVMBuilderIntegerType();
        SLANG_CHECK(
            builder.getFloatingPointType(_getFakeNVVMBuilderModule(), 32, type) ==
            SLANG_E_NOT_AVAILABLE);
        SLANG_CHECK(type == nullptr);
        SLANG_CHECK(gFakeNVVMBuilder.getFloatingPointTypeCallCount == 0);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);

    // Advertising float32 addition makes the complete two-callback extension mandatory.
    const uint32_t partialSizes[] = {
        uint32_t(SLANG_NVVM_BUILDER_API_V3_MIN_SIZE),
        uint32_t(offsetof(SlangNVVMBuilderAPI_V3, getFloatingPointType) + 1),
        uint32_t(offsetof(SlangNVVMBuilderAPI_V3, emitFloatingBinary) + 1),
    };
    for (uint32_t partialSize : partialSizes)
    {
        gFakeNVVMBuilder.reset();
        ComPtr<ISlangSharedLibrary> library(new FakeNVVMBuilderLibrary);
        SlangNVVMBuilderAPI_V3 api = _makeFakeNVVMBuilderAPIV3();
        api.structureSize = partialSize;
        NVVMIRBuilder builder;
        SLANG_CHECK(NVVMIRBuilder::initialize(api, library, builder) == SLANG_E_NO_INTERFACE);
        SLANG_CHECK(!builder.isInitialized());
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);

    for (int callbackIndex = 0; callbackIndex < 2; ++callbackIndex)
    {
        gFakeNVVMBuilder.reset();
        ComPtr<ISlangSharedLibrary> library(new FakeNVVMBuilderLibrary);
        SlangNVVMBuilderAPI_V3 api = _makeFakeNVVMBuilderAPIV3();
        if (callbackIndex == 0)
            api.getFloatingPointType = nullptr;
        else
            api.emitFloatingBinary = nullptr;
        NVVMIRBuilder builder;
        SLANG_CHECK(NVVMIRBuilder::initialize(api, library, builder) == SLANG_E_NO_INTERFACE);
        SLANG_CHECK(!builder.isInitialized());
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);

    // The complete prefix forwards exact float types and ordered operands, while wrappers clear
    // stale or provider-written outputs on every failure.
    gFakeNVVMBuilder.reset();
    {
        ComPtr<ISlangSharedLibrary> library(new FakeNVVMBuilderLibrary);
        NVVMIRBuilder builder;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            NVVMIRBuilder::initialize(_makeFakeNVVMBuilderAPIV3(), library, builder)));
        SlangNVVMTypeHandle_1 floatType = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            builder.getFloatingPointType(_getFakeNVVMBuilderModule(), 32, floatType)));
        SLANG_CHECK(floatType == _getFakeNVVMBuilderFloatType());

        SlangNVVMTypeHandle_1 functionType = nullptr;
        const SlangNVVMTypeHandle_1 parameterTypes[] = {floatType, floatType};
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionType(
            _getFakeNVVMBuilderModule(),
            _getFakeNVVMBuilderVoidType(),
            parameterTypes,
            SLANG_COUNT_OF(parameterTypes),
            functionType)));
        SlangNVVMValueHandle_1 function = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
            _getFakeNVVMBuilderModule(),
            functionType,
            toSlice("fakeFloatAdd"),
            function)));
        SlangNVVMValueHandle_1 left = nullptr;
        SlangNVVMValueHandle_1 right = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            builder.getFunctionParameter(_getFakeNVVMBuilderModule(), function, 0, left)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            builder.getFunctionParameter(_getFakeNVVMBuilderModule(), function, 1, right)));
        SlangNVVMBlockHandle_1 block = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            builder.createBlock(_getFakeNVVMBuilderModule(), function, toSlice("entry"), block)));
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.setInsertBlock(_getFakeNVVMBuilderModule(), block)));

        SlangNVVMValueHandle_1 result = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitFloatingBinary(
            _getFakeNVVMBuilderModule(),
            SLANG_NVVM_FLOATING_BINARY_OP_ADD,
            left,
            right,
            result)));
        SLANG_CHECK(result == _getFakeNVVMBuilderScalarOperation(0));
        SLANG_CHECK(gFakeNVVMBuilder.scalarOperations[0].operands[0].index == 0);
        SLANG_CHECK(gFakeNVVMBuilder.scalarOperations[0].operands[1].index == 1);

        result = _getFakeNVVMBuilderFunction();
        SLANG_CHECK(
            builder.emitFloatingBinary(
                _getFakeNVVMBuilderModule(),
                SlangNVVMFloatingBinaryOp_3(99),
                left,
                right,
                result) == SLANG_E_INVALID_ARG);
        SLANG_CHECK(result == nullptr);
        SLANG_CHECK(
            gFakeNVVMBuilder
                .scalarV3FamilyCallCounts[Index(FakeNVVMBuilderScalarFamily::FloatingBinary)] == 1);

        gFakeNVVMBuilder.failFloatingPointTypeAfterWrite = true;
        floatType = _getFakeNVVMBuilderIntegerType();
        SLANG_CHECK(
            builder.getFloatingPointType(_getFakeNVVMBuilderModule(), 32, floatType) == SLANG_FAIL);
        SLANG_CHECK(floatType == nullptr);
        gFakeNVVMBuilder.failFloatingPointTypeAfterWrite = false;

        _setFakeNVVMBuilderScalarOperationFailure(
            gFakeNVVMBuilder.failScalarOperationAfterWrite,
            FakeNVVMBuilderScalarFamily::FloatingBinary,
            SLANG_NVVM_FLOATING_BINARY_OP_ADD,
            true);
        result = _getFakeNVVMBuilderFunction();
        SLANG_CHECK(
            builder.emitFloatingBinary(
                _getFakeNVVMBuilderModule(),
                SLANG_NVVM_FLOATING_BINARY_OP_ADD,
                left,
                right,
                result) == SLANG_FAIL);
        SLANG_CHECK(result == nullptr);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
}

static void _runNVVMIRBuilderNegotiatesFloat32BinaryAPI(
    NVVMFloat32BinaryTestOperation testOperation)
{
    const NVVMFloat32BinaryTestCase& testCase = _getNVVMFloat32BinaryTestCase(testOperation);

    gFakeNVVMBuilder.reset();
    {
        ComPtr<ISlangSharedLibrary> library(new FakeNVVMBuilderLibrary);
        SlangNVVMBuilderAPI_V3 api = _makeFakeNVVMBuilderAPIV3();
        api.features.words[testCase.feature / 64u] &= ~(uint64_t(1) << (testCase.feature % 64u));
        NVVMIRBuilder builder;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(NVVMIRBuilder::initialize(api, library, builder)));
        SlangNVVMValueHandle_1 result = _getFakeNVVMBuilderFunction();
        SLANG_CHECK(
            builder.emitFloatingBinary(
                _getFakeNVVMBuilderModule(),
                testCase.operation,
                nullptr,
                nullptr,
                result) == SLANG_E_NOT_AVAILABLE);
        SLANG_CHECK(result == nullptr);
        SLANG_CHECK(
            gFakeNVVMBuilder
                .scalarV3FamilyCallCounts[Index(FakeNVVMBuilderScalarFamily::FloatingBinary)] == 0);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);

    gFakeNVVMBuilder.reset();
    {
        ComPtr<ISlangSharedLibrary> library(new FakeNVVMBuilderLibrary);
        SlangNVVMBuilderAPI_V3 api = _makeFakeNVVMBuilderAPIV3();
        const SlangNVVMBuilderFeature_3 floatFeatures[] = {
            SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_ADD,
            SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_SUBTRACT,
            SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_MULTIPLY,
        };
        for (auto feature : floatFeatures)
            api.features.words[feature / 64u] &= ~(uint64_t(1) << (feature % 64u));
        api.features.words[testCase.feature / 64u] |= uint64_t(1) << (testCase.feature % 64u);
        api.structureSize = uint32_t(SLANG_NVVM_BUILDER_API_V3_MIN_SIZE);
        NVVMIRBuilder builder;
        SLANG_CHECK(NVVMIRBuilder::initialize(api, library, builder) == SLANG_E_NO_INTERFACE);
        SLANG_CHECK(!builder.isInitialized());
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);

    gFakeNVVMBuilder.reset();
    {
        ComPtr<ISlangSharedLibrary> library(new FakeNVVMBuilderLibrary);
        NVVMIRBuilder builder;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            NVVMIRBuilder::initialize(_makeFakeNVVMBuilderAPIV3(), library, builder)));
        SlangNVVMTypeHandle_1 floatType = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            builder.getFloatingPointType(_getFakeNVVMBuilderModule(), 32, floatType)));
        const SlangNVVMTypeHandle_1 parameterTypes[] = {floatType, floatType};
        SlangNVVMTypeHandle_1 functionType = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionType(
            _getFakeNVVMBuilderModule(),
            _getFakeNVVMBuilderVoidType(),
            parameterTypes,
            SLANG_COUNT_OF(parameterTypes),
            functionType)));
        SlangNVVMValueHandle_1 function = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
            _getFakeNVVMBuilderModule(),
            functionType,
            UnownedStringSlice(testCase.kernelName),
            function)));
        SlangNVVMValueHandle_1 left = nullptr;
        SlangNVVMValueHandle_1 right = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            builder.getFunctionParameter(_getFakeNVVMBuilderModule(), function, 0, left)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            builder.getFunctionParameter(_getFakeNVVMBuilderModule(), function, 1, right)));
        SlangNVVMBlockHandle_1 block = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            builder.createBlock(_getFakeNVVMBuilderModule(), function, toSlice("entry"), block)));
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.setInsertBlock(_getFakeNVVMBuilderModule(), block)));
        SlangNVVMValueHandle_1 result = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitFloatingBinary(
            _getFakeNVVMBuilderModule(),
            testCase.operation,
            left,
            right,
            result)));
        SLANG_CHECK(result == _getFakeNVVMBuilderScalarOperation(0));
        SLANG_CHECK(gFakeNVVMBuilder.scalarOperations[0].key.operation == testCase.operation);
        SLANG_CHECK(gFakeNVVMBuilder.scalarOperations[0].operands[0].index == 0);
        SLANG_CHECK(gFakeNVVMBuilder.scalarOperations[0].operands[1].index == 1);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmIRBuilderNegotiatesFloat32SubtractAPI)
{
    _runNVVMIRBuilderNegotiatesFloat32BinaryAPI(NVVMFloat32BinaryTestOperation::Subtract);
}

SLANG_UNIT_TEST(nvvmIRBuilderNegotiatesFloat32MultiplyAPI)
{
    _runNVVMIRBuilderNegotiatesFloat32BinaryAPI(NVVMFloat32BinaryTestOperation::Multiply);
}

SLANG_UNIT_TEST(nvvmIRBuilderPrefersV3AndRejectsMalformedPresentV3)
{
    gFakeNVVMBuilder.reset();
    gFakeNVVMBuilder.api = _makeFakeNVVMBuilderAPI();
    gFakeNVVMBuilder.apiV2 = _makeFakeNVVMBuilderAPIV2();
    gFakeNVVMBuilder.apiV3 = _makeFakeNVVMBuilderAPIV3();
    gFakeNVVMBuilder.apiV3.structureSize = uint32_t(sizeof(SlangNVVMBuilderAPI_V3) + 16);
    gFakeNVVMBuilder.omitAPIV2Symbol = false;
    gFakeNVVMBuilder.omitAPIV3Symbol = false;
    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(NVVMIRBuilder::load(String(), loader, builder)));
        SLANG_CHECK_ABORT(builder.getAPIV3() != nullptr);
        SLANG_CHECK(builder.getAPIV3()->structureSize == sizeof(SlangNVVMBuilderAPI_V3));
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);

    // A present V3 symbol is authoritative. A broken V3 deployment must not hide behind V2.
    gFakeNVVMBuilder.reset();
    gFakeNVVMBuilder.api = _makeFakeNVVMBuilderAPI();
    gFakeNVVMBuilder.apiV2 = _makeFakeNVVMBuilderAPIV2();
    gFakeNVVMBuilder.apiV3 = _makeFakeNVVMBuilderAPIV3();
    gFakeNVVMBuilder.apiV3.emitIntegerCompare = nullptr;
    gFakeNVVMBuilder.omitAPIV2Symbol = false;
    gFakeNVVMBuilder.omitAPIV3Symbol = false;
    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK(NVVMIRBuilder::load(String(), loader, builder) == SLANG_E_NO_INTERFACE);
        SLANG_CHECK(!builder.isInitialized());
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmIRBuilderForwardsVersionedABI)
{
    gFakeNVVMBuilder.reset();
    gFakeNVVMBuilder.api = _makeFakeNVVMBuilderAPI();
    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(NVVMIRBuilder::load(String(), loader, builder)));
        loader.setNull();
        SLANG_CHECK(builder.isInitialized());
        SLANG_CHECK(!builder.supportsSerializationDiagnostics());
        SLANG_CHECK(builder.getAPIV2() == nullptr);
        SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 1);
        SLANG_CHECK(builder.getAPI().llvmVersionMajor == 14);
        SLANG_CHECK(builder.getAPI().llvmVersionMinor == 0);
        SLANG_CHECK(builder.getAPI().llvmVersionPatch == 6);
        SLANG_CHECK(builder.getAPI().nvvmIRVersionMajor == 2);
        SLANG_CHECK(builder.getAPI().nvvmIRVersionMinor == 0);
        SLANG_CHECK(builder.getAPI().pointerModel == SLANG_NVVM_POINTER_MODEL_TYPED);

        static const char kModuleName[] = {'f', 'a', 'k', 'e', 0, 'm', 'o', 'd'};
        ScopedNVVMBuilderModule scope;
        scope.builder = &builder;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.createModule(
            UnownedStringSlice(kModuleName, SLANG_COUNT_OF(kModuleName)),
            scope.module)));

        SlangNVVMTypeHandle_1 voidType = nullptr;
        SlangNVVMTypeHandle_1 functionType = nullptr;
        SlangNVVMValueHandle_1 function = nullptr;
        SlangNVVMBlockHandle_1 block = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(scope.module, voidType)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            builder.getFunctionType(scope.module, voidType, nullptr, 0, functionType)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
            scope.module,
            functionType,
            toSlice("callerNamedEmpty"),
            function)));
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.createBlock(scope.module, function, toSlice("entry"), block)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(scope.module, block)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitReturnVoid(scope.module)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.markFunctionAsKernel(scope.module, function)));

        size_t requiredSize = 0;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getAPI().serializeModule(
            scope.module,
            SLANG_NVVM_SERIALIZATION_FORMAT_BITCODE,
            nullptr,
            0,
            &requiredSize)));
        uint8_t insufficientStorage[1] = {0xa5};
        size_t reportedSize = 0;
        SLANG_CHECK(
            builder.getAPI().serializeModule(
                scope.module,
                SLANG_NVVM_SERIALIZATION_FORMAT_BITCODE,
                insufficientStorage,
                sizeof(insufficientStorage),
                &reportedSize) == SLANG_E_BUFFER_TOO_SMALL);
        SLANG_CHECK(reportedSize == requiredSize);
        SLANG_CHECK(insufficientStorage[0] == 0xa5);

        ComPtr<ISlangBlob> assembly;
        ComPtr<ISlangBlob> bitcode;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(
            scope.module,
            SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
            assembly)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(
            scope.module,
            SLANG_NVVM_SERIALIZATION_FORMAT_BITCODE,
            bitcode)));
        SLANG_CHECK(assembly->getBufferSize() == ::strlen("fake LLVM assembly"));
        SLANG_CHECK(
            ::memcmp(
                assembly->getBufferPointer(),
                "fake LLVM assembly",
                assembly->getBufferSize()) == 0);
        static const uint8_t kExpectedBitcode[] = {0x42, 0x43, 0xc0, 0xde, 0x00, 0x11};
        SLANG_CHECK(bitcode->getBufferSize() == SLANG_COUNT_OF(kExpectedBitcode));
        SLANG_CHECK(
            ::memcmp(
                bitcode->getBufferPointer(),
                kExpectedBitcode,
                SLANG_COUNT_OF(kExpectedBitcode)) == 0);

        ComPtr<ISlangBlob> unavailableDiagnosticSerialization;
        String unavailableDiagnostics = "stale diagnostics";
        SLANG_CHECK(
            builder.serializeModule(
                scope.module,
                SLANG_NVVM_SERIALIZATION_FORMAT_BITCODE,
                unavailableDiagnosticSerialization,
                unavailableDiagnostics) == SLANG_E_NOT_AVAILABLE);
        SLANG_CHECK(unavailableDiagnosticSerialization == nullptr);
        SLANG_CHECK(unavailableDiagnostics.getLength() == 0);

        gFakeNVVMBuilder.reportMismatchedWriteSize = true;
        ComPtr<ISlangBlob> malformedOutput;
        SLANG_CHECK(
            builder.serializeModule(
                scope.module,
                SLANG_NVVM_SERIALIZATION_FORMAT_BITCODE,
                malformedOutput) == SLANG_FAIL);
        SLANG_CHECK(malformedOutput == nullptr);
        gFakeNVVMBuilder.reportMismatchedWriteSize = false;

        SLANG_CHECK(gFakeNVVMBuilder.moduleName.getLength() == SLANG_COUNT_OF(kModuleName));
        SLANG_CHECK(
            ::memcmp(
                gFakeNVVMBuilder.moduleName.getBuffer(),
                kModuleName,
                SLANG_COUNT_OF(kModuleName)) == 0);
        SLANG_CHECK(gFakeNVVMBuilder.functionName == "callerNamedEmpty");
        SLANG_CHECK(gFakeNVVMBuilder.blockName == "entry");
        SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.getVoidTypeCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.getFunctionTypeCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.declareFunctionCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.createBlockCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.setInsertBlockCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.emitReturnVoidCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.markFunctionAsKernelCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.serializeQueryCallCount == 4);
        SLANG_CHECK(gFakeNVVMBuilder.serializeWriteCallCount == 4);

        builder.destroyModule(scope.module);
        scope.module = nullptr;
        SLANG_CHECK(gFakeNVVMBuilder.destroyModuleCallCount == 1);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVMBuilder.destroyedLibraryCount == 1);
}

SLANG_UNIT_TEST(nvvmIRBuilderNegotiatesScalarAPI)
{
    // A V1-only provider cannot expose the appended operations. The host rejects every scalar
    // wrapper locally and clears every handle output before returning.
    gFakeNVVMBuilder.reset();
    gFakeNVVMBuilder.api = _makeFakeNVVMBuilderAPI();
    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(NVVMIRBuilder::load(String(), loader, builder)));
        loader.setNull();
        SLANG_CHECK(!builder.supportsScalarOperations());
        SLANG_CHECK(builder.getAPIV2() == nullptr);

        ScopedNVVMBuilderModule scope;
        scope.builder = &builder;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.createModule(toSlice("fake-v1-scalar-module"), scope.module)));

        SlangNVVMTypeHandle_1 integerType = _getFakeNVVMBuilderVoidType();
        SlangNVVMTypeHandle_1 pointerType = _getFakeNVVMBuilderVoidType();
        SlangNVVMValueHandle_1 parameter = _getFakeNVVMBuilderFunction();
        SlangNVVMValueHandle_1 loadedValue = _getFakeNVVMBuilderFunction();
        SLANG_CHECK(builder.getIntegerType(scope.module, 32, integerType) == SLANG_E_NOT_AVAILABLE);
        SLANG_CHECK(integerType == nullptr);
        SLANG_CHECK(
            builder.getPointerType(
                scope.module,
                _getFakeNVVMBuilderIntegerType(),
                SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
                pointerType) == SLANG_E_NOT_AVAILABLE);
        SLANG_CHECK(pointerType == nullptr);
        SLANG_CHECK(
            builder
                .getFunctionParameter(scope.module, _getFakeNVVMBuilderFunction(), 0, parameter) ==
            SLANG_E_NOT_AVAILABLE);
        SLANG_CHECK(parameter == nullptr);
        SLANG_CHECK(
            builder.emitLoad(scope.module, _getFakeNVVMBuilderParameter(), 4, loadedValue) ==
            SLANG_E_NOT_AVAILABLE);
        SLANG_CHECK(loadedValue == nullptr);
        SLANG_CHECK(
            builder.emitStore(
                scope.module,
                _getFakeNVVMBuilderLoad(),
                _getFakeNVVMBuilderParameter(),
                4) == SLANG_E_NOT_AVAILABLE);
        SLANG_CHECK(gFakeNVVMBuilder.getIntegerTypeCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.getPointerTypeCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.getFunctionParameterCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitLoadCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitStoreCallCount == 0);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVMBuilder.destroyedLibraryCount == 1);

    // A new provider must honor a Slice 3b caller's reported capacity even when the caller has
    // additional sentinel storage after that prefix.
    gFakeNVVMBuilder.reset();
    gFakeNVVMBuilder.api = _makeFakeNVVMBuilderAPI();
    gFakeNVVMBuilder.apiV2 = _makeFakeNVVMBuilderAPIV2();
    struct BoundedV2Query
    {
        SlangNVVMBuilderAPI_V2 api;
        uint8_t trailingSentinels[16];
    } boundedQuery;
    ::memset(&boundedQuery, 0xa5, sizeof(boundedQuery));
    boundedQuery.api.structureSize = uint32_t(SLANG_NVVM_BUILDER_API_V2_MIN_SIZE);
    boundedQuery.api.abiVersion = SLANG_NVVM_BUILDER_ABI_VERSION_2;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_fakeGetNVVMBuilderAPIV2(&boundedQuery.api)));
    SLANG_CHECK(boundedQuery.api.structureSize == sizeof(SlangNVVMBuilderAPI_V2));
    const uint8_t* boundedBytes = reinterpret_cast<const uint8_t*>(&boundedQuery);
    for (Index i = SLANG_NVVM_BUILDER_API_V2_MIN_SIZE; i < Index(sizeof(boundedQuery)); ++i)
        SLANG_CHECK(boundedBytes[i] == 0xa5);

    // A Slice 3b-sized V2 provider remains usable for diagnostics, but none of the appended
    // scalar-memory wrappers may call beyond the prefix the provider reported.
    gFakeNVVMBuilder.reset();
    gFakeNVVMBuilder.api = _makeFakeNVVMBuilderAPI();
    gFakeNVVMBuilder.apiV2 = _makeFakeNVVMBuilderAPIV2();
    gFakeNVVMBuilder.apiV2.structureSize = uint32_t(SLANG_NVVM_BUILDER_API_V2_MIN_SIZE);
    gFakeNVVMBuilder.omitAPIV2Symbol = false;
    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(NVVMIRBuilder::load(String(), loader, builder)));
        loader.setNull();
        SLANG_CHECK(builder.supportsSerializationDiagnostics());
        SLANG_CHECK(!builder.supportsScalarOperations());
        SLANG_CHECK_ABORT(builder.getAPIV2() != nullptr);
        SLANG_CHECK(builder.getAPIV2()->structureSize == SLANG_NVVM_BUILDER_API_V2_MIN_SIZE);

        ScopedNVVMBuilderModule scope;
        scope.builder = &builder;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.createModule(toSlice("fake-old-v2-module"), scope.module)));

        ComPtr<ISlangBlob> bitcode;
        String diagnostics = "stale diagnostics";
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(
            scope.module,
            SLANG_NVVM_SERIALIZATION_FORMAT_BITCODE,
            bitcode,
            diagnostics)));
        SLANG_CHECK_ABORT(bitcode != nullptr);
        SLANG_CHECK(diagnostics.getLength() == 0);

        SlangNVVMTypeHandle_1 integerType = _getFakeNVVMBuilderVoidType();
        SLANG_CHECK(builder.getIntegerType(scope.module, 32, integerType) == SLANG_E_NOT_AVAILABLE);
        SLANG_CHECK(integerType == nullptr);

        SlangNVVMTypeHandle_1 pointerType = _getFakeNVVMBuilderVoidType();
        SLANG_CHECK(
            builder.getPointerType(
                scope.module,
                _getFakeNVVMBuilderIntegerType(),
                SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
                pointerType) == SLANG_E_NOT_AVAILABLE);
        SLANG_CHECK(pointerType == nullptr);

        SlangNVVMValueHandle_1 parameter = _getFakeNVVMBuilderFunction();
        SLANG_CHECK(
            builder
                .getFunctionParameter(scope.module, _getFakeNVVMBuilderFunction(), 7, parameter) ==
            SLANG_E_NOT_AVAILABLE);
        SLANG_CHECK(parameter == nullptr);

        SlangNVVMValueHandle_1 loadedValue = _getFakeNVVMBuilderFunction();
        SLANG_CHECK(
            builder.emitLoad(scope.module, _getFakeNVVMBuilderParameter(), 4, loadedValue) ==
            SLANG_E_NOT_AVAILABLE);
        SLANG_CHECK(loadedValue == nullptr);
        SLANG_CHECK(
            builder.emitStore(
                scope.module,
                _getFakeNVVMBuilderLoad(),
                _getFakeNVVMBuilderParameter(),
                4) == SLANG_E_NOT_AVAILABLE);

        SLANG_CHECK(gFakeNVVMBuilder.getIntegerTypeCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.getPointerTypeCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.getFunctionParameterCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitLoadCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitStoreCallCount == 0);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVMBuilder.destroyedLibraryCount == 1);

    // A provider may expose exactly the old prefix or the complete scalar prefix, but no
    // intermediate byte count can describe a coherent callable capability.
    gFakeNVVMBuilder.reset();
    gFakeNVVMBuilder.api = _makeFakeNVVMBuilderAPI();
    gFakeNVVMBuilder.apiV2 = _makeFakeNVVMBuilderAPIV2();
    gFakeNVVMBuilder.apiV2.structureSize = uint32_t(SLANG_NVVM_BUILDER_API_V2_MIN_SIZE + 1);
    gFakeNVVMBuilder.omitAPIV2Symbol = false;
    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder rejectedBuilder;
        SLANG_CHECK(NVVMIRBuilder::load(String(), loader, rejectedBuilder) == SLANG_E_NO_INTERFACE);
        SLANG_CHECK(!rejectedBuilder.isInitialized());
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVMBuilder.destroyedLibraryCount == 1);

    // Claiming the complete scalar prefix makes every function in that prefix mandatory.
    gFakeNVVMBuilder.reset();
    gFakeNVVMBuilder.api = _makeFakeNVVMBuilderAPI();
    gFakeNVVMBuilder.apiV2 = _makeFakeNVVMBuilderAPIV2();
    gFakeNVVMBuilder.apiV2.emitStore = nullptr;
    gFakeNVVMBuilder.omitAPIV2Symbol = false;
    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder rejectedBuilder;
        SLANG_CHECK(NVVMIRBuilder::load(String(), loader, rejectedBuilder) == SLANG_E_NO_INTERFACE);
        SLANG_CHECK(!rejectedBuilder.isInitialized());
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVMBuilder.destroyedLibraryCount == 1);

    // A complete scalar prefix is forwarded exactly through the host wrappers.
    gFakeNVVMBuilder.reset();
    gFakeNVVMBuilder.api = _makeFakeNVVMBuilderAPI();
    gFakeNVVMBuilder.apiV2 = _makeFakeNVVMBuilderAPIV2();
    gFakeNVVMBuilder.omitAPIV2Symbol = false;
    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(NVVMIRBuilder::load(String(), loader, builder)));
        loader.setNull();
        SLANG_CHECK(builder.supportsScalarOperations());
        SLANG_CHECK(builder.supportsScalarPointerArithmetic());
        SLANG_CHECK(builder.supportsScalarArrayAddressing());
        SLANG_CHECK(builder.getVersionString().indexOf("scalar-array-addressing=1") >= 0);
        SLANG_CHECK(builder.getVersionString().indexOf("scalar-pointer-arithmetic=1") >= 0);
        SLANG_CHECK_ABORT(builder.getAPIV2() != nullptr);
        SLANG_CHECK(builder.getAPIV2()->structureSize == sizeof(SlangNVVMBuilderAPI_V2));

        ScopedNVVMBuilderModule scope;
        scope.builder = &builder;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.createModule(toSlice("fake-scalar-v2-module"), scope.module)));

        SlangNVVMTypeHandle_1 integerType = nullptr;
        SlangNVVMTypeHandle_1 pointerType = nullptr;
        SlangNVVMValueHandle_1 parameter = nullptr;
        SlangNVVMValueHandle_1 loadedValue = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getIntegerType(scope.module, 32, integerType)));
        SLANG_CHECK(integerType == _getFakeNVVMBuilderIntegerType());
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getPointerType(
            scope.module,
            integerType,
            SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
            pointerType)));
        SLANG_CHECK(pointerType == _getFakeNVVMBuilderPointerType());
        SlangNVVMTypeHandle_1 voidType = nullptr;
        SlangNVVMTypeHandle_1 scalarFunctionType = nullptr;
        SlangNVVMValueHandle_1 scalarFunction = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(scope.module, voidType)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            builder.getFunctionType(scope.module, voidType, &pointerType, 1, scalarFunctionType)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
            scope.module,
            scalarFunctionType,
            toSlice("fakeScalarMemory"),
            scalarFunction)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            builder.getFunctionParameter(scope.module, scalarFunction, 0, parameter)));
        SLANG_CHECK(parameter == _getFakeNVVMBuilderParameter());
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.emitLoad(scope.module, parameter, 4, loadedValue)));
        SLANG_CHECK(loadedValue == _getFakeNVVMBuilderLoad());
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.emitStore(scope.module, loadedValue, parameter, 4)));

        gFakeNVVMBuilder.returnNullIntegerType = true;
        integerType = _getFakeNVVMBuilderVoidType();
        SLANG_CHECK(builder.getIntegerType(scope.module, 32, integerType) == SLANG_FAIL);
        SLANG_CHECK(integerType == nullptr);
        gFakeNVVMBuilder.returnNullIntegerType = false;

        gFakeNVVMBuilder.failIntegerTypeAfterWrite = true;
        integerType = _getFakeNVVMBuilderVoidType();
        SLANG_CHECK(builder.getIntegerType(scope.module, 32, integerType) == SLANG_FAIL);
        SLANG_CHECK(integerType == nullptr);
        gFakeNVVMBuilder.failIntegerTypeAfterWrite = false;

        SLANG_CHECK(gFakeNVVMBuilder.getIntegerTypeCallCount == 3);
        SLANG_CHECK(gFakeNVVMBuilder.getPointerTypeCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.getFunctionParameterCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.emitLoadCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.emitStoreCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.integerBitWidth == 32);
        SLANG_CHECK(gFakeNVVMBuilder.pointerAddressSpace == SLANG_NVVM_ADDRESS_SPACE_GLOBAL);
        SLANG_CHECK(gFakeNVVMBuilder.functionParameterIndex == 0);
        SLANG_CHECK(gFakeNVVMBuilder.loadAlignment == 4);
        SLANG_CHECK(gFakeNVVMBuilder.storeAlignment == 4);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVMBuilder.destroyedLibraryCount == 1);

    // A future provider reports its complete size, but the host retains only its local prefix.
    gFakeNVVMBuilder.reset();
    gFakeNVVMBuilder.api = _makeFakeNVVMBuilderAPI();
    gFakeNVVMBuilder.apiV2 = _makeFakeNVVMBuilderAPIV2();
    gFakeNVVMBuilder.apiV2.structureSize = uint32_t(sizeof(SlangNVVMBuilderAPI_V2) + 16);
    gFakeNVVMBuilder.omitAPIV2Symbol = false;
    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(NVVMIRBuilder::load(String(), loader, builder)));
        loader.setNull();
        SLANG_CHECK(builder.supportsScalarOperations());
        SLANG_CHECK(builder.supportsScalarPointerArithmetic());
        SLANG_CHECK(builder.supportsScalarArrayAddressing());
        SLANG_CHECK(builder.getVersionString().indexOf("scalar-array-addressing=1") >= 0);
        SLANG_CHECK(builder.getVersionString().indexOf("scalar-pointer-arithmetic=1") >= 0);
        SLANG_CHECK_ABORT(builder.getAPIV2() != nullptr);
        SLANG_CHECK(builder.getAPIV2()->structureSize == sizeof(SlangNVVMBuilderAPI_V2));
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVMBuilder.destroyedLibraryCount == 1);
}

SLANG_UNIT_TEST(nvvmIRBuilderNegotiatesScalarControlFlowAPI)
{
    // An exact Slice 4 provider remains valid, but none of the appended control-flow wrappers may
    // read or call beyond that frozen prefix.
    gFakeNVVMBuilder.reset();
    gFakeNVVMBuilder.api = _makeFakeNVVMBuilderAPI();
    gFakeNVVMBuilder.apiV2 = _makeFakeNVVMBuilderAPIV2();
    gFakeNVVMBuilder.apiV2.structureSize = uint32_t(SLANG_NVVM_BUILDER_API_V2_SCALAR_MIN_SIZE);
    gFakeNVVMBuilder.omitAPIV2Symbol = false;
    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(NVVMIRBuilder::load(String(), loader, builder)));
        loader.setNull();
        SLANG_CHECK(builder.supportsScalarOperations());
        SLANG_CHECK(!builder.supportsScalarControlFlow());

        SlangNVVMValueHandle_1 value = _getFakeNVVMBuilderFunction();
        SLANG_CHECK(
            builder.emitIntegerBinary(
                _getFakeNVVMBuilderModule(),
                SLANG_NVVM_INTEGER_BINARY_OP_ADD,
                _getFakeNVVMBuilderParameter(),
                _getFakeNVVMBuilderParameter(),
                value) == SLANG_E_NOT_AVAILABLE);
        SLANG_CHECK(value == nullptr);
        value = _getFakeNVVMBuilderFunction();
        SLANG_CHECK(
            builder.emitIntegerSignedLessThan(
                _getFakeNVVMBuilderModule(),
                _getFakeNVVMBuilderParameter(),
                _getFakeNVVMBuilderParameter(),
                value) == SLANG_E_NOT_AVAILABLE);
        SLANG_CHECK(value == nullptr);
        SLANG_CHECK(
            builder.emitBranch(_getFakeNVVMBuilderModule(), _getFakeNVVMBuilderBlock()) ==
            SLANG_E_NOT_AVAILABLE);
        SLANG_CHECK(
            builder.emitConditionalBranch(
                _getFakeNVVMBuilderModule(),
                _getFakeNVVMBuilderParameter(),
                _getFakeNVVMBuilderBlock(),
                _getFakeNVVMBuilderBlock()) == SLANG_E_NOT_AVAILABLE);
        SLANG_CHECK(gFakeNVVMBuilder.scalarOperations.getCount() == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitBranchCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitConditionalBranchCallCount == 0);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);

    // A byte count inside the new all-or-none block is malformed.
    gFakeNVVMBuilder.reset();
    gFakeNVVMBuilder.api = _makeFakeNVVMBuilderAPI();
    gFakeNVVMBuilder.apiV2 = _makeFakeNVVMBuilderAPIV2();
    gFakeNVVMBuilder.apiV2.structureSize = uint32_t(SLANG_NVVM_BUILDER_API_V2_SCALAR_MIN_SIZE + 1);
    gFakeNVVMBuilder.omitAPIV2Symbol = false;
    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK(NVVMIRBuilder::load(String(), loader, builder) == SLANG_E_NO_INTERFACE);
        SLANG_CHECK(!builder.isInitialized());
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);

    // Claiming the complete prefix makes all four operations mandatory.
    gFakeNVVMBuilder.reset();
    gFakeNVVMBuilder.api = _makeFakeNVVMBuilderAPI();
    gFakeNVVMBuilder.apiV2 = _makeFakeNVVMBuilderAPIV2();
    gFakeNVVMBuilder.apiV2.emitConditionalBranch = nullptr;
    gFakeNVVMBuilder.omitAPIV2Symbol = false;
    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK(NVVMIRBuilder::load(String(), loader, builder) == SLANG_E_NO_INTERFACE);
        SLANG_CHECK(!builder.isInitialized());
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);

    // A complete prefix is forwarded exactly, and a provider failure cannot leak a written
    // output handle through the host wrapper.
    gFakeNVVMBuilder.reset();
    gFakeNVVMBuilder.api = _makeFakeNVVMBuilderAPI();
    gFakeNVVMBuilder.apiV2 = _makeFakeNVVMBuilderAPIV2();
    gFakeNVVMBuilder.omitAPIV2Symbol = false;
    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(NVVMIRBuilder::load(String(), loader, builder)));
        loader.setNull();
        SLANG_CHECK(builder.supportsScalarControlFlow());
        SLANG_CHECK(builder.getAPIV2()->structureSize == sizeof(SlangNVVMBuilderAPI_V2));
        SLANG_CHECK(builder.getVersionString().indexOf("scalar-control-flow=1") >= 0);

        SlangNVVMTypeHandle_1 integerFunctionType = nullptr;
        const SlangNVVMTypeHandle_1 integerType = _getFakeNVVMBuilderIntegerType();
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionType(
            _getFakeNVVMBuilderModule(),
            integerType,
            &integerType,
            1,
            integerFunctionType)));
        SlangNVVMValueHandle_1 integerFunction = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
            _getFakeNVVMBuilderModule(),
            integerFunctionType,
            toSlice("fakeControlFunction"),
            integerFunction)));
        SlangNVVMValueHandle_1 integerParameter = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionParameter(
            _getFakeNVVMBuilderModule(),
            integerFunction,
            0,
            integerParameter)));
        SlangNVVMBlockHandle_1 insertBlock = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.createBlock(
            _getFakeNVVMBuilderModule(),
            integerFunction,
            toSlice("entry"),
            insertBlock)));
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.setInsertBlock(_getFakeNVVMBuilderModule(), insertBlock)));

        SlangNVVMValueHandle_1 binary = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitIntegerBinary(
            _getFakeNVVMBuilderModule(),
            SLANG_NVVM_INTEGER_BINARY_OP_ADD,
            integerParameter,
            integerParameter,
            binary)));
        SLANG_CHECK(binary == _getFakeNVVMBuilderScalarOperation(0));

        SlangNVVMValueHandle_1 condition = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitIntegerSignedLessThan(
            _getFakeNVVMBuilderModule(),
            integerParameter,
            binary,
            condition)));
        SLANG_CHECK(condition == _getFakeNVVMBuilderScalarOperation(1));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            builder.emitBranch(_getFakeNVVMBuilderModule(), _getFakeNVVMBuilderBlock())));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitConditionalBranch(
            _getFakeNVVMBuilderModule(),
            condition,
            _getFakeNVVMBuilderBlock(),
            _getFakeNVVMBuilderBlock())));

        _setFakeNVVMBuilderScalarOperationFailure(
            gFakeNVVMBuilder.failScalarOperationAfterWrite,
            FakeNVVMBuilderScalarFamily::Binary,
            SLANG_NVVM_INTEGER_BINARY_OP_3_SUB,
            true);
        binary = _getFakeNVVMBuilderFunction();
        SLANG_CHECK(
            builder.emitIntegerBinary(
                _getFakeNVVMBuilderModule(),
                SLANG_NVVM_INTEGER_BINARY_OP_SUB,
                integerParameter,
                integerParameter,
                binary) == SLANG_FAIL);
        SLANG_CHECK(binary == nullptr);
        SLANG_CHECK(
            gFakeNVVMBuilder.scalarFamilyCallCounts[Index(FakeNVVMBuilderScalarFamily::Binary)] ==
            2);
        SLANG_CHECK(
            gFakeNVVMBuilder.scalarFamilyCallCounts[Index(FakeNVVMBuilderScalarFamily::Compare)] ==
            1);
        SLANG_CHECK(gFakeNVVMBuilder.emitBranchCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.emitConditionalBranchCallCount == 1);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmIRBuilderNegotiatesScalarSSAAPI)
{
    // The exact Slice 7 provider remains valid and cannot be called through the appended wrappers.
    gFakeNVVMBuilder.reset();
    gFakeNVVMBuilder.api = _makeFakeNVVMBuilderAPI();
    gFakeNVVMBuilder.apiV2 = _makeFakeNVVMBuilderAPIV2();
    gFakeNVVMBuilder.apiV2.structureSize =
        uint32_t(SLANG_NVVM_BUILDER_API_V2_SCALAR_CONTROL_FLOW_MIN_SIZE);
    gFakeNVVMBuilder.omitAPIV2Symbol = false;
    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(NVVMIRBuilder::load(String(), loader, builder)));
        loader.setNull();
        SLANG_CHECK(builder.supportsScalarControlFlow());
        SLANG_CHECK(!builder.supportsScalarSSA());

        SlangNVVMValueHandle_1 value = _getFakeNVVMBuilderFunction();
        SLANG_CHECK(
            builder.getIntegerConstant(
                _getFakeNVVMBuilderModule(),
                _getFakeNVVMBuilderIntegerType(),
                1,
                value) == SLANG_E_NOT_AVAILABLE);
        SLANG_CHECK(value == nullptr);
        value = _getFakeNVVMBuilderFunction();
        SLANG_CHECK(
            builder.emitIntegerPhi(
                _getFakeNVVMBuilderModule(),
                _getFakeNVVMBuilderBlock(),
                _getFakeNVVMBuilderIntegerType(),
                value) == SLANG_E_NOT_AVAILABLE);
        SLANG_CHECK(value == nullptr);
        SLANG_CHECK(
            builder.addIntegerPhiIncoming(
                _getFakeNVVMBuilderModule(),
                _getFakeNVVMBuilderIntegerPhi(),
                _getFakeNVVMBuilderParameter(),
                _getFakeNVVMBuilderBlock()) == SLANG_E_NOT_AVAILABLE);
        SLANG_CHECK(gFakeNVVMBuilder.getIntegerConstantCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitIntegerPhiCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.addIntegerPhiIncomingCallCount == 0);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);

    // The three Slice 8 functions form one all-or-none prefix.
    gFakeNVVMBuilder.reset();
    gFakeNVVMBuilder.api = _makeFakeNVVMBuilderAPI();
    gFakeNVVMBuilder.apiV2 = _makeFakeNVVMBuilderAPIV2();
    gFakeNVVMBuilder.apiV2.structureSize =
        uint32_t(SLANG_NVVM_BUILDER_API_V2_SCALAR_CONTROL_FLOW_MIN_SIZE + 1);
    gFakeNVVMBuilder.omitAPIV2Symbol = false;
    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK(NVVMIRBuilder::load(String(), loader, builder) == SLANG_E_NO_INTERFACE);
        SLANG_CHECK(!builder.isInitialized());
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);

    gFakeNVVMBuilder.reset();
    gFakeNVVMBuilder.api = _makeFakeNVVMBuilderAPI();
    gFakeNVVMBuilder.apiV2 = _makeFakeNVVMBuilderAPIV2();
    gFakeNVVMBuilder.apiV2.addIntegerPhiIncoming = nullptr;
    gFakeNVVMBuilder.omitAPIV2Symbol = false;
    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK(NVVMIRBuilder::load(String(), loader, builder) == SLANG_E_NO_INTERFACE);
        SLANG_CHECK(!builder.isInitialized());
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);

    // The complete prefix forwards values and sanitizes provider-written outputs on failure.
    gFakeNVVMBuilder.reset();
    gFakeNVVMBuilder.api = _makeFakeNVVMBuilderAPI();
    gFakeNVVMBuilder.apiV2 = _makeFakeNVVMBuilderAPIV2();
    gFakeNVVMBuilder.omitAPIV2Symbol = false;
    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(NVVMIRBuilder::load(String(), loader, builder)));
        loader.setNull();
        SLANG_CHECK(builder.supportsScalarSSA());
        SLANG_CHECK(builder.getVersionString().indexOf("scalar-ssa=1") >= 0);

        SlangNVVMValueHandle_1 constant = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getIntegerConstant(
            _getFakeNVVMBuilderModule(),
            _getFakeNVVMBuilderIntegerType(),
            -17,
            constant)));
        SLANG_CHECK(constant == _getFakeNVVMBuilderIntegerConstant());

        SlangNVVMValueHandle_1 phi = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitIntegerPhi(
            _getFakeNVVMBuilderModule(),
            _getFakeNVVMBuilderBlock(1),
            _getFakeNVVMBuilderIntegerType(),
            phi)));
        SLANG_CHECK(phi == _getFakeNVVMBuilderIntegerPhi());
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.addIntegerPhiIncoming(
            _getFakeNVVMBuilderModule(),
            phi,
            constant,
            _getFakeNVVMBuilderBlock())));

        gFakeNVVMBuilder.failIntegerConstantAfterWrite = true;
        constant = _getFakeNVVMBuilderFunction();
        SLANG_CHECK(
            builder.getIntegerConstant(
                _getFakeNVVMBuilderModule(),
                _getFakeNVVMBuilderIntegerType(),
                5,
                constant) == SLANG_FAIL);
        SLANG_CHECK(constant == nullptr);
        gFakeNVVMBuilder.failIntegerConstantAfterWrite = false;

        gFakeNVVMBuilder.failIntegerPhiAfterWrite = true;
        phi = _getFakeNVVMBuilderFunction();
        SLANG_CHECK(
            builder.emitIntegerPhi(
                _getFakeNVVMBuilderModule(),
                _getFakeNVVMBuilderBlock(2),
                _getFakeNVVMBuilderIntegerType(),
                phi) == SLANG_FAIL);
        SLANG_CHECK(phi == nullptr);

        SLANG_CHECK(gFakeNVVMBuilder.integerConstantValues[0] == -17);
        SLANG_CHECK(gFakeNVVMBuilder.integerPhiTargetBlockIndices[0] == 1);
        SLANG_CHECK(gFakeNVVMBuilder.integerPhiIncomingPhiIndices[0] == 0);
        SLANG_CHECK(
            gFakeNVVMBuilder.integerPhiIncomingValueRefs[0].kind ==
            FakeNVVMBuilderValueKind::IntegerConstant);
        SLANG_CHECK(gFakeNVVMBuilder.integerPhiIncomingPredecessorBlockIndices[0] == 0);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmIRBuilderNegotiatesScalarFunctionAPI)
{
    // An exact Slice 8 provider remains valid, but the appended call/valued-return wrappers are
    // unavailable and must not dispatch through the shorter table.
    gFakeNVVMBuilder.reset();
    gFakeNVVMBuilder.api = _makeFakeNVVMBuilderAPI();
    gFakeNVVMBuilder.apiV2 = _makeFakeNVVMBuilderAPIV2();
    gFakeNVVMBuilder.apiV2.structureSize = uint32_t(SLANG_NVVM_BUILDER_API_V2_SCALAR_SSA_MIN_SIZE);
    gFakeNVVMBuilder.omitAPIV2Symbol = false;
    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(NVVMIRBuilder::load(String(), loader, builder)));
        loader.setNull();
        SLANG_CHECK(builder.supportsScalarSSA());
        SLANG_CHECK(!builder.supportsScalarFunctions());

        SlangNVVMValueHandle_1 result = _getFakeNVVMBuilderFunction();
        const SlangNVVMValueHandle_1 argument = _getFakeNVVMBuilderParameter();
        SLANG_CHECK(
            builder.emitIntegerCall(
                _getFakeNVVMBuilderModule(),
                _getFakeNVVMBuilderFunction(),
                &argument,
                1,
                result) == SLANG_E_NOT_AVAILABLE);
        SLANG_CHECK(result == nullptr);
        SLANG_CHECK(
            builder.emitIntegerReturn(
                _getFakeNVVMBuilderModule(),
                _getFakeNVVMBuilderParameter()) == SLANG_E_NOT_AVAILABLE);
        SLANG_CHECK(gFakeNVVMBuilder.emitIntegerCallCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitIntegerReturnCallCount == 0);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);

    // The two Slice 9 functions form one all-or-none prefix.
    gFakeNVVMBuilder.reset();
    gFakeNVVMBuilder.api = _makeFakeNVVMBuilderAPI();
    gFakeNVVMBuilder.apiV2 = _makeFakeNVVMBuilderAPIV2();
    gFakeNVVMBuilder.apiV2.structureSize =
        uint32_t(SLANG_NVVM_BUILDER_API_V2_SCALAR_SSA_MIN_SIZE + 1);
    gFakeNVVMBuilder.omitAPIV2Symbol = false;
    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK(NVVMIRBuilder::load(String(), loader, builder) == SLANG_E_NO_INTERFACE);
        SLANG_CHECK(!builder.isInitialized());
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);

    gFakeNVVMBuilder.reset();
    gFakeNVVMBuilder.api = _makeFakeNVVMBuilderAPI();
    gFakeNVVMBuilder.apiV2 = _makeFakeNVVMBuilderAPIV2();
    gFakeNVVMBuilder.apiV2.emitIntegerCall = nullptr;
    gFakeNVVMBuilder.omitAPIV2Symbol = false;
    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK(NVVMIRBuilder::load(String(), loader, builder) == SLANG_E_NO_INTERFACE);
        SLANG_CHECK(!builder.isInitialized());
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);

    gFakeNVVMBuilder.reset();
    gFakeNVVMBuilder.api = _makeFakeNVVMBuilderAPI();
    gFakeNVVMBuilder.apiV2 = _makeFakeNVVMBuilderAPIV2();
    gFakeNVVMBuilder.apiV2.emitIntegerReturn = nullptr;
    gFakeNVVMBuilder.omitAPIV2Symbol = false;
    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK(NVVMIRBuilder::load(String(), loader, builder) == SLANG_E_NO_INTERFACE);
        SLANG_CHECK(!builder.isInitialized());
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);

    // A complete prefix forwards the direct-call graph and clears a provider-written result when
    // the provider reports failure.
    gFakeNVVMBuilder.reset();
    gFakeNVVMBuilder.api = _makeFakeNVVMBuilderAPI();
    gFakeNVVMBuilder.apiV2 = _makeFakeNVVMBuilderAPIV2();
    gFakeNVVMBuilder.omitAPIV2Symbol = false;
    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(NVVMIRBuilder::load(String(), loader, builder)));
        loader.setNull();
        SLANG_CHECK(builder.supportsScalarFunctions());
        SLANG_CHECK(builder.getVersionString().indexOf("scalar-functions=1") >= 0);

        SlangNVVMTypeHandle_1 integerFunctionType = nullptr;
        const SlangNVVMTypeHandle_1 parameterType = _getFakeNVVMBuilderIntegerType();
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionType(
            _getFakeNVVMBuilderModule(),
            _getFakeNVVMBuilderIntegerType(),
            &parameterType,
            1,
            integerFunctionType)));
        SlangNVVMValueHandle_1 function = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
            _getFakeNVVMBuilderModule(),
            integerFunctionType,
            toSlice("fakeScalarHelper"),
            function)));
        SlangNVVMValueHandle_1 parameter = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            builder.getFunctionParameter(_getFakeNVVMBuilderModule(), function, 0, parameter)));
        SlangNVVMBlockHandle_1 block = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            builder.createBlock(_getFakeNVVMBuilderModule(), function, toSlice("entry"), block)));
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.setInsertBlock(_getFakeNVVMBuilderModule(), block)));

        SlangNVVMValueHandle_1 result = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            builder.emitIntegerCall(_getFakeNVVMBuilderModule(), function, &parameter, 1, result)));
        SLANG_CHECK(result == _getFakeNVVMBuilderCall());
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.emitIntegerReturn(_getFakeNVVMBuilderModule(), result)));

        gFakeNVVMBuilder.failIntegerCallAfterWrite = true;
        result = _getFakeNVVMBuilderFunction();
        SLANG_CHECK(
            builder.emitIntegerCall(_getFakeNVVMBuilderModule(), function, &parameter, 1, result) ==
            SLANG_FAIL);
        SLANG_CHECK(result == nullptr);
        gFakeNVVMBuilder.failIntegerReturn = true;
        SLANG_CHECK(
            builder.emitIntegerReturn(_getFakeNVVMBuilderModule(), parameter) == SLANG_FAIL);
        SLANG_CHECK(gFakeNVVMBuilder.emitIntegerCallCallCount == 2);
        SLANG_CHECK(gFakeNVVMBuilder.emitIntegerReturnCallCount == 2);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmIRBuilderNegotiatesScalarPointerArithmeticAPI)
{
    // An exact Slice 9 provider remains valid, but the appended pointer-offset wrapper cannot
    // dispatch beyond that frozen prefix and must clear its output locally.
    gFakeNVVMBuilder.reset();
    gFakeNVVMBuilder.api = _makeFakeNVVMBuilderAPI();
    gFakeNVVMBuilder.apiV2 = _makeFakeNVVMBuilderAPIV2();
    gFakeNVVMBuilder.apiV2.structureSize =
        uint32_t(SLANG_NVVM_BUILDER_API_V2_SCALAR_FUNCTION_MIN_SIZE);
    gFakeNVVMBuilder.omitAPIV2Symbol = false;
    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(NVVMIRBuilder::load(String(), loader, builder)));
        loader.setNull();
        SLANG_CHECK(builder.supportsScalarFunctions());
        SLANG_CHECK(!builder.supportsScalarPointerArithmetic());
        SLANG_CHECK(builder.getVersionString().indexOf("scalar-pointer-arithmetic=0") >= 0);

        SlangNVVMValueHandle_1 result = _getFakeNVVMBuilderFunction();
        SLANG_CHECK(
            builder.emitPointerOffset(
                _getFakeNVVMBuilderModule(),
                _getFakeNVVMBuilderParameter(),
                _getFakeNVVMBuilderParameter(1),
                result) == SLANG_E_NOT_AVAILABLE);
        SLANG_CHECK(result == nullptr);
        SLANG_CHECK(gFakeNVVMBuilder.emitPointerOffsetCallCount == 0);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);

    // No byte count inside the one-function suffix describes a coherent capability.
    gFakeNVVMBuilder.reset();
    gFakeNVVMBuilder.api = _makeFakeNVVMBuilderAPI();
    gFakeNVVMBuilder.apiV2 = _makeFakeNVVMBuilderAPIV2();
    gFakeNVVMBuilder.apiV2.structureSize =
        uint32_t(SLANG_NVVM_BUILDER_API_V2_SCALAR_FUNCTION_MIN_SIZE + 1);
    gFakeNVVMBuilder.omitAPIV2Symbol = false;
    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK(NVVMIRBuilder::load(String(), loader, builder) == SLANG_E_NO_INTERFACE);
        SLANG_CHECK(!builder.isInitialized());
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);

    // Claiming the complete suffix makes its only operation mandatory.
    gFakeNVVMBuilder.reset();
    gFakeNVVMBuilder.api = _makeFakeNVVMBuilderAPI();
    gFakeNVVMBuilder.apiV2 = _makeFakeNVVMBuilderAPIV2();
    gFakeNVVMBuilder.apiV2.emitPointerOffset = nullptr;
    gFakeNVVMBuilder.omitAPIV2Symbol = false;
    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK(NVVMIRBuilder::load(String(), loader, builder) == SLANG_E_NO_INTERFACE);
        SLANG_CHECK(!builder.isInitialized());
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);

    // A complete suffix forwards the exact base/index topology. The fake validates parameter
    // roles through their owning function type, and the host clears provider-written failures.
    gFakeNVVMBuilder.reset();
    gFakeNVVMBuilder.api = _makeFakeNVVMBuilderAPI();
    gFakeNVVMBuilder.apiV2 = _makeFakeNVVMBuilderAPIV2();
    gFakeNVVMBuilder.omitAPIV2Symbol = false;
    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(NVVMIRBuilder::load(String(), loader, builder)));
        loader.setNull();
        SLANG_CHECK(builder.supportsScalarPointerArithmetic());
        SLANG_CHECK(builder.getVersionString().indexOf("scalar-pointer-arithmetic=1") >= 0);

        ScopedNVVMBuilderModule scope;
        scope.builder = &builder;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            builder.createModule(toSlice("fake-pointer-offset-module"), scope.module)));
        SlangNVVMTypeHandle_1 voidType = nullptr;
        SlangNVVMTypeHandle_1 integerType = nullptr;
        SlangNVVMTypeHandle_1 pointerType = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(scope.module, voidType)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getIntegerType(scope.module, 32, integerType)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getPointerType(
            scope.module,
            integerType,
            SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
            pointerType)));
        const SlangNVVMTypeHandle_1 parameterTypes[] = {pointerType, pointerType, integerType};
        SlangNVVMTypeHandle_1 functionType = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionType(
            scope.module,
            voidType,
            parameterTypes,
            SLANG_COUNT_OF(parameterTypes),
            functionType)));
        SlangNVVMValueHandle_1 function = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
            scope.module,
            functionType,
            toSlice("fakePointerOffset"),
            function)));
        SlangNVVMValueHandle_1 destination = nullptr;
        SlangNVVMValueHandle_1 source = nullptr;
        SlangNVVMValueHandle_1 index = nullptr;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.getFunctionParameter(scope.module, function, 0, destination)));
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.getFunctionParameter(scope.module, function, 1, source)));
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.getFunctionParameter(scope.module, function, 2, index)));
        SlangNVVMBlockHandle_1 block = nullptr;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.createBlock(scope.module, function, toSlice("entry"), block)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(scope.module, block)));

        SlangNVVMValueHandle_1 result = nullptr;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.emitPointerOffset(scope.module, destination, index, result)));
        SLANG_CHECK(result == _getFakeNVVMBuilderPointerOffset());
        SLANG_CHECK(gFakeNVVMBuilder.pointerOffsetCallerBlockIndices[0] == 0);
        SLANG_CHECK(
            gFakeNVVMBuilder.pointerOffsetBaseValueRefs[0].kind ==
            FakeNVVMBuilderValueKind::Parameter);
        SLANG_CHECK(gFakeNVVMBuilder.pointerOffsetBaseValueRefs[0].functionIndex == 0);
        SLANG_CHECK(gFakeNVVMBuilder.pointerOffsetBaseValueRefs[0].index == 0);
        SLANG_CHECK(
            gFakeNVVMBuilder.pointerOffsetElementValueRefs[0].kind ==
            FakeNVVMBuilderValueKind::Parameter);
        SLANG_CHECK(gFakeNVVMBuilder.pointerOffsetElementValueRefs[0].functionIndex == 0);
        SLANG_CHECK(gFakeNVVMBuilder.pointerOffsetElementValueRefs[0].index == 2);

        result = _getFakeNVVMBuilderFunction();
        SLANG_CHECK(
            builder.emitPointerOffset(scope.module, index, index, result) == SLANG_E_INVALID_ARG);
        SLANG_CHECK(result == nullptr);
        result = _getFakeNVVMBuilderFunction();
        SLANG_CHECK(
            builder.emitPointerOffset(scope.module, source, source, result) == SLANG_E_INVALID_ARG);
        SLANG_CHECK(result == nullptr);

        gFakeNVVMBuilder.failPointerOffsetAfterWrite = true;
        result = _getFakeNVVMBuilderFunction();
        SLANG_CHECK(builder.emitPointerOffset(scope.module, source, index, result) == SLANG_FAIL);
        SLANG_CHECK(result == nullptr);
        SLANG_CHECK(gFakeNVVMBuilder.emitPointerOffsetCallCount == 4);
        SLANG_CHECK(gFakeNVVMBuilder.pointerOffsetBaseValueRefs.getCount() == 2);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmIRBuilderNegotiatesScalarArrayAddressingAPI)
{
    SLANG_CHECK(
        offsetof(SlangNVVMBuilderAPI_V2, emitIntegerMultiply) ==
        SLANG_NVVM_BUILDER_API_V2_SCALAR_ARRAY_MIN_SIZE);

    // The frozen Slice 10 prefix remains valid and cannot dispatch either array operation.
    gFakeNVVMBuilder.reset();
    gFakeNVVMBuilder.api = _makeFakeNVVMBuilderAPI();
    gFakeNVVMBuilder.apiV2 = _makeFakeNVVMBuilderAPIV2();
    gFakeNVVMBuilder.apiV2.structureSize =
        uint32_t(SLANG_NVVM_BUILDER_API_V2_SCALAR_POINTER_ARITHMETIC_MIN_SIZE);
    gFakeNVVMBuilder.omitAPIV2Symbol = false;
    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(NVVMIRBuilder::load(String(), loader, builder)));
        loader.setNull();
        SLANG_CHECK(builder.supportsScalarPointerArithmetic());
        SLANG_CHECK(!builder.supportsScalarArrayAddressing());
        SLANG_CHECK(builder.getVersionString().indexOf("scalar-array-addressing=0") >= 0);

        SlangNVVMTypeHandle_1 arrayType = _getFakeNVVMBuilderVoidType();
        SLANG_CHECK(
            builder.getArrayType(
                _getFakeNVVMBuilderModule(),
                _getFakeNVVMBuilderIntegerType(),
                4,
                arrayType) == SLANG_E_NOT_AVAILABLE);
        SLANG_CHECK(arrayType == nullptr);
        SlangNVVMValueHandle_1 elementPointer = _getFakeNVVMBuilderFunction();
        SLANG_CHECK(
            builder.emitArrayElementPointer(
                _getFakeNVVMBuilderModule(),
                _getFakeNVVMBuilderParameter(),
                _getFakeNVVMBuilderParameter(1),
                elementPointer) == SLANG_E_NOT_AVAILABLE);
        SLANG_CHECK(elementPointer == nullptr);
        SLANG_CHECK(gFakeNVVMBuilder.getArrayTypeCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitArrayElementPointerCallCount == 0);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);

    // A byte count ending inside either appended function pointer is not a coherent prefix.
    const uint32_t partialSizes[] = {
        uint32_t(SLANG_NVVM_BUILDER_API_V2_SCALAR_POINTER_ARITHMETIC_MIN_SIZE + 1),
        uint32_t(offsetof(SlangNVVMBuilderAPI_V2, emitArrayElementPointer) + 1),
    };
    for (uint32_t partialSize : partialSizes)
    {
        gFakeNVVMBuilder.reset();
        gFakeNVVMBuilder.api = _makeFakeNVVMBuilderAPI();
        gFakeNVVMBuilder.apiV2 = _makeFakeNVVMBuilderAPIV2();
        gFakeNVVMBuilder.apiV2.structureSize = partialSize;
        gFakeNVVMBuilder.omitAPIV2Symbol = false;
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK(NVVMIRBuilder::load(String(), loader, builder) == SLANG_E_NO_INTERFACE);
        SLANG_CHECK(!builder.isInitialized());
        loader.setNull();
        SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    }

    // Both operations are mandatory once the complete Slice 11 prefix is advertised.
    gFakeNVVMBuilder.reset();
    gFakeNVVMBuilder.api = _makeFakeNVVMBuilderAPI();
    gFakeNVVMBuilder.apiV2 = _makeFakeNVVMBuilderAPIV2();
    gFakeNVVMBuilder.apiV2.getArrayType = nullptr;
    gFakeNVVMBuilder.omitAPIV2Symbol = false;
    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK(NVVMIRBuilder::load(String(), loader, builder) == SLANG_E_NO_INTERFACE);
        SLANG_CHECK(!builder.isInitialized());
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);

    gFakeNVVMBuilder.reset();
    gFakeNVVMBuilder.api = _makeFakeNVVMBuilderAPI();
    gFakeNVVMBuilder.apiV2 = _makeFakeNVVMBuilderAPIV2();
    gFakeNVVMBuilder.apiV2.emitArrayElementPointer = nullptr;
    gFakeNVVMBuilder.omitAPIV2Symbol = false;
    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK(NVVMIRBuilder::load(String(), loader, builder) == SLANG_E_NO_INTERFACE);
        SLANG_CHECK(!builder.isInitialized());
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);

    // The complete suffix forwards exact type and value identities. Failed provider calls never
    // leak their written handles through the host wrappers.
    gFakeNVVMBuilder.reset();
    gFakeNVVMBuilder.api = _makeFakeNVVMBuilderAPI();
    gFakeNVVMBuilder.apiV2 = _makeFakeNVVMBuilderAPIV2();
    gFakeNVVMBuilder.omitAPIV2Symbol = false;
    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(NVVMIRBuilder::load(String(), loader, builder)));
        loader.setNull();
        SLANG_CHECK(builder.supportsScalarArrayAddressing());
        SLANG_CHECK(builder.getVersionString().indexOf("scalar-array-addressing=1") >= 0);

        ScopedNVVMBuilderModule scope;
        scope.builder = &builder;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            builder.createModule(toSlice("fake-array-addressing-module"), scope.module)));
        SlangNVVMTypeHandle_1 voidType = nullptr;
        SlangNVVMTypeHandle_1 integerType = nullptr;
        SlangNVVMTypeHandle_1 arrayType = nullptr;
        SlangNVVMTypeHandle_1 arrayPointerType = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(scope.module, voidType)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getIntegerType(scope.module, 32, integerType)));
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.getArrayType(scope.module, integerType, 4, arrayType)));
        SLANG_CHECK(arrayType == _getFakeNVVMBuilderArrayType());
        SLANG_CHECK(gFakeNVVMBuilder.arrayElementType == integerType);
        SLANG_CHECK(gFakeNVVMBuilder.arrayElementCount == 4);
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getPointerType(
            scope.module,
            arrayType,
            SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
            arrayPointerType)));
        SLANG_CHECK(arrayPointerType == _getFakeNVVMBuilderArrayPointerType());

        const SlangNVVMTypeHandle_1 parameterTypes[] = {
            arrayPointerType,
            arrayPointerType,
            integerType,
        };
        SlangNVVMTypeHandle_1 functionType = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionType(
            scope.module,
            voidType,
            parameterTypes,
            SLANG_COUNT_OF(parameterTypes),
            functionType)));
        SlangNVVMValueHandle_1 function = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
            scope.module,
            functionType,
            toSlice("fakeArrayAddressing"),
            function)));
        SlangNVVMValueHandle_1 destination = nullptr;
        SlangNVVMValueHandle_1 source = nullptr;
        SlangNVVMValueHandle_1 index = nullptr;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.getFunctionParameter(scope.module, function, 0, destination)));
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.getFunctionParameter(scope.module, function, 1, source)));
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.getFunctionParameter(scope.module, function, 2, index)));
        SlangNVVMBlockHandle_1 block = nullptr;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.createBlock(scope.module, function, toSlice("entry"), block)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(scope.module, block)));

        SlangNVVMValueHandle_1 destinationElement = nullptr;
        SlangNVVMValueHandle_1 sourceElement = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            builder.emitArrayElementPointer(scope.module, destination, index, destinationElement)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            builder.emitArrayElementPointer(scope.module, source, index, sourceElement)));
        SLANG_CHECK(destinationElement == _getFakeNVVMBuilderArrayElementPointer(0));
        SLANG_CHECK(sourceElement == _getFakeNVVMBuilderArrayElementPointer(1));
        SlangNVVMValueHandle_1 loadedValue = nullptr;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.emitLoad(scope.module, sourceElement, 4, loadedValue)));
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.emitStore(scope.module, loadedValue, destinationElement, 4)));
        SLANG_CHECK(
            gFakeNVVMBuilder.loadPointerValueRefs[0].kind ==
            FakeNVVMBuilderValueKind::ArrayElementPointer);
        SLANG_CHECK(gFakeNVVMBuilder.loadPointerValueRefs[0].index == 1);
        SLANG_CHECK(
            gFakeNVVMBuilder.storePointerValueRefs[0].kind ==
            FakeNVVMBuilderValueKind::ArrayElementPointer);
        SLANG_CHECK(gFakeNVVMBuilder.storePointerValueRefs[0].index == 0);

        SlangNVVMValueHandle_1 result = _getFakeNVVMBuilderFunction();
        SLANG_CHECK(
            builder.emitArrayElementPointer(scope.module, index, index, result) ==
            SLANG_E_INVALID_ARG);
        SLANG_CHECK(result == nullptr);
        result = _getFakeNVVMBuilderFunction();
        SLANG_CHECK(
            builder.emitArrayElementPointer(scope.module, source, source, result) ==
            SLANG_E_INVALID_ARG);
        SLANG_CHECK(result == nullptr);

        gFakeNVVMBuilder.failArrayTypeAfterWrite = true;
        SlangNVVMTypeHandle_1 failedArrayType = _getFakeNVVMBuilderVoidType();
        SLANG_CHECK(
            builder.getArrayType(scope.module, integerType, 4, failedArrayType) == SLANG_FAIL);
        SLANG_CHECK(failedArrayType == nullptr);
        gFakeNVVMBuilder.failArrayTypeAfterWrite = false;

        gFakeNVVMBuilder.returnNullArrayType = true;
        failedArrayType = _getFakeNVVMBuilderVoidType();
        SLANG_CHECK(
            builder.getArrayType(scope.module, integerType, 4, failedArrayType) == SLANG_FAIL);
        SLANG_CHECK(failedArrayType == nullptr);
        gFakeNVVMBuilder.returnNullArrayType = false;

        gFakeNVVMBuilder.returnNullArrayElementPointer = true;
        result = _getFakeNVVMBuilderFunction();
        SLANG_CHECK(
            builder.emitArrayElementPointer(scope.module, source, index, result) == SLANG_FAIL);
        SLANG_CHECK(result == nullptr);
        gFakeNVVMBuilder.returnNullArrayElementPointer = false;

        gFakeNVVMBuilder.failArrayElementPointerAfterWrite = true;
        result = _getFakeNVVMBuilderFunction();
        SLANG_CHECK(
            builder.emitArrayElementPointer(scope.module, source, index, result) == SLANG_FAIL);
        SLANG_CHECK(result == nullptr);
        SLANG_CHECK(gFakeNVVMBuilder.getArrayTypeCallCount == 3);
        SLANG_CHECK(gFakeNVVMBuilder.emitArrayElementPointerCallCount == 6);
        SLANG_CHECK(gFakeNVVMBuilder.arrayElementPointerBaseValueRefs.getCount() == 4);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
}

struct NVVMScalarBuilderAPICase
{
    uint32_t previousSize;
    uint32_t completeSize;
    size_t callbackOffset;
    size_t nextCallbackOffset;
    const char* versionFeature;
};

static NVVMScalarBuilderAPICase _getNVVMScalarBuilderAPICase(NVVMScalarTestOperation operation)
{
    switch (operation)
    {
    case NVVMScalarTestOperation::Multiply:
        return {
            uint32_t(SLANG_NVVM_BUILDER_API_V2_SCALAR_ARRAY_MIN_SIZE),
            uint32_t(SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_MULTIPLY_MIN_SIZE),
            offsetof(SlangNVVMBuilderAPI_V2, emitIntegerMultiply),
            offsetof(SlangNVVMBuilderAPI_V2, emitIntegerBitAnd),
            "scalar-integer-multiply"};
    case NVVMScalarTestOperation::BitAnd:
        return {
            uint32_t(SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_MULTIPLY_MIN_SIZE),
            uint32_t(SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_BIT_AND_MIN_SIZE),
            offsetof(SlangNVVMBuilderAPI_V2, emitIntegerBitAnd),
            offsetof(SlangNVVMBuilderAPI_V2, emitIntegerBitOr),
            "scalar-integer-bit-and"};
    case NVVMScalarTestOperation::BitOr:
        return {
            uint32_t(SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_BIT_AND_MIN_SIZE),
            uint32_t(SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_BIT_OR_MIN_SIZE),
            offsetof(SlangNVVMBuilderAPI_V2, emitIntegerBitOr),
            offsetof(SlangNVVMBuilderAPI_V2, emitIntegerBitXor),
            "scalar-integer-bit-or"};
    case NVVMScalarTestOperation::BitXor:
        return {
            uint32_t(SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_BIT_OR_MIN_SIZE),
            uint32_t(SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_BIT_XOR_MIN_SIZE),
            offsetof(SlangNVVMBuilderAPI_V2, emitIntegerBitXor),
            offsetof(SlangNVVMBuilderAPI_V2, emitIntegerBitNot),
            "scalar-integer-bit-xor"};
    case NVVMScalarTestOperation::BitNot:
        return {
            uint32_t(SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_BIT_XOR_MIN_SIZE),
            uint32_t(SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_BIT_NOT_MIN_SIZE),
            offsetof(SlangNVVMBuilderAPI_V2, emitIntegerBitNot),
            offsetof(SlangNVVMBuilderAPI_V2, emitIntegerNegate),
            "scalar-integer-bit-not"};
    case NVVMScalarTestOperation::Negate:
        return {
            uint32_t(SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_BIT_NOT_MIN_SIZE),
            uint32_t(SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_NEGATE_MIN_SIZE),
            offsetof(SlangNVVMBuilderAPI_V2, emitIntegerNegate),
            offsetof(SlangNVVMBuilderAPI_V2, emitRelaxedGlobalI32AtomicAdd),
            "scalar-integer-negate"};
    case NVVMScalarTestOperation::Equal:
        return {
            uint32_t(SLANG_NVVM_BUILDER_API_V2_RELAXED_GLOBAL_I32_ATOMIC_ADD_MIN_SIZE),
            uint32_t(SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_EQUAL_MIN_SIZE),
            offsetof(SlangNVVMBuilderAPI_V2, emitIntegerEqual),
            offsetof(SlangNVVMBuilderAPI_V2, emitIntegerNotEqual),
            "scalar-integer-equal"};
    case NVVMScalarTestOperation::NotEqual:
        return {
            uint32_t(SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_EQUAL_MIN_SIZE),
            uint32_t(SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_NOT_EQUAL_MIN_SIZE),
            offsetof(SlangNVVMBuilderAPI_V2, emitIntegerNotEqual),
            offsetof(SlangNVVMBuilderAPI_V2, emitIntegerSignedGreaterThan),
            "scalar-integer-not-equal"};
    case NVVMScalarTestOperation::SignedGreaterThan:
        return {
            uint32_t(SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_NOT_EQUAL_MIN_SIZE),
            uint32_t(SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_SIGNED_GREATER_THAN_MIN_SIZE),
            offsetof(SlangNVVMBuilderAPI_V2, emitIntegerSignedGreaterThan),
            offsetof(SlangNVVMBuilderAPI_V2, emitIntegerSignedLessEqual),
            "scalar-integer-signed-greater-than"};
    case NVVMScalarTestOperation::SignedLessEqual:
        return {
            uint32_t(SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_SIGNED_GREATER_THAN_MIN_SIZE),
            uint32_t(SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_SIGNED_LESS_EQUAL_MIN_SIZE),
            offsetof(SlangNVVMBuilderAPI_V2, emitIntegerSignedLessEqual),
            offsetof(SlangNVVMBuilderAPI_V2, emitIntegerSignedGreaterEqual),
            "scalar-integer-signed-less-equal"};
    case NVVMScalarTestOperation::SignedGreaterEqual:
        return {
            uint32_t(SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_SIGNED_LESS_EQUAL_MIN_SIZE),
            uint32_t(SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_SIGNED_GREATER_EQUAL_MIN_SIZE),
            offsetof(SlangNVVMBuilderAPI_V2, emitIntegerSignedGreaterEqual),
            offsetof(SlangNVVMBuilderAPI_V2, getRawRWStructuredBufferI32Type),
            "scalar-integer-signed-greater-equal"};
    }
    SLANG_UNEXPECTED("unknown NVVM scalar builder API operation");
}

static bool _supportsNVVMScalarBuilderOperation(
    const NVVMIRBuilder& builder,
    NVVMScalarTestOperation operation)
{
    switch (operation)
    {
    case NVVMScalarTestOperation::Multiply:
        return builder.supportsScalarIntegerMultiply();
    case NVVMScalarTestOperation::BitAnd:
        return builder.supportsScalarIntegerBitAnd();
    case NVVMScalarTestOperation::BitOr:
        return builder.supportsScalarIntegerBitOr();
    case NVVMScalarTestOperation::BitXor:
        return builder.supportsScalarIntegerBitXor();
    case NVVMScalarTestOperation::BitNot:
        return builder.supportsScalarIntegerBitNot();
    case NVVMScalarTestOperation::Negate:
        return builder.supportsScalarIntegerNegate();
    case NVVMScalarTestOperation::Equal:
        return builder.supportsScalarIntegerEqual();
    case NVVMScalarTestOperation::NotEqual:
        return builder.supportsScalarIntegerNotEqual();
    case NVVMScalarTestOperation::SignedGreaterThan:
        return builder.supportsScalarIntegerSignedGreaterThan();
    case NVVMScalarTestOperation::SignedLessEqual:
        return builder.supportsScalarIntegerSignedLessEqual();
    case NVVMScalarTestOperation::SignedGreaterEqual:
        return builder.supportsScalarIntegerSignedGreaterEqual();
    }
    return false;
}

static void _removeFakeNVVMScalarBuilderCallback(
    SlangNVVMBuilderAPI_V2& api,
    NVVMScalarTestOperation operation)
{
    switch (operation)
    {
    case NVVMScalarTestOperation::Multiply:
        api.emitIntegerMultiply = nullptr;
        break;
    case NVVMScalarTestOperation::BitAnd:
        api.emitIntegerBitAnd = nullptr;
        break;
    case NVVMScalarTestOperation::BitOr:
        api.emitIntegerBitOr = nullptr;
        break;
    case NVVMScalarTestOperation::BitXor:
        api.emitIntegerBitXor = nullptr;
        break;
    case NVVMScalarTestOperation::BitNot:
        api.emitIntegerBitNot = nullptr;
        break;
    case NVVMScalarTestOperation::Negate:
        api.emitIntegerNegate = nullptr;
        break;
    case NVVMScalarTestOperation::Equal:
        api.emitIntegerEqual = nullptr;
        break;
    case NVVMScalarTestOperation::NotEqual:
        api.emitIntegerNotEqual = nullptr;
        break;
    case NVVMScalarTestOperation::SignedGreaterThan:
        api.emitIntegerSignedGreaterThan = nullptr;
        break;
    case NVVMScalarTestOperation::SignedLessEqual:
        api.emitIntegerSignedLessEqual = nullptr;
        break;
    case NVVMScalarTestOperation::SignedGreaterEqual:
        api.emitIntegerSignedGreaterEqual = nullptr;
        break;
    }
}

static SlangResult _emitNVVMScalarBuilderOperation(
    NVVMIRBuilder& builder,
    NVVMScalarTestOperation operation,
    SlangNVVMModuleHandle_1 module,
    SlangNVVMValueHandle_1 left,
    SlangNVVMValueHandle_1 right,
    SlangNVVMValueHandle_1& outValue)
{
    switch (operation)
    {
    case NVVMScalarTestOperation::Multiply:
        return builder.emitIntegerMultiply(module, left, right, outValue);
    case NVVMScalarTestOperation::BitAnd:
        return builder.emitIntegerBitAnd(module, left, right, outValue);
    case NVVMScalarTestOperation::BitOr:
        return builder.emitIntegerBitOr(module, left, right, outValue);
    case NVVMScalarTestOperation::BitXor:
        return builder.emitIntegerBitXor(module, left, right, outValue);
    case NVVMScalarTestOperation::BitNot:
        return builder.emitIntegerBitNot(module, left, outValue);
    case NVVMScalarTestOperation::Negate:
        return builder.emitIntegerNegate(module, left, outValue);
    case NVVMScalarTestOperation::Equal:
        return builder.emitIntegerEqual(module, left, right, outValue);
    case NVVMScalarTestOperation::NotEqual:
        return builder.emitIntegerNotEqual(module, left, right, outValue);
    case NVVMScalarTestOperation::SignedGreaterThan:
        return builder.emitIntegerSignedGreaterThan(module, left, right, outValue);
    case NVVMScalarTestOperation::SignedLessEqual:
        return builder.emitIntegerSignedLessEqual(module, left, right, outValue);
    case NVVMScalarTestOperation::SignedGreaterEqual:
        return builder.emitIntegerSignedGreaterEqual(module, left, right, outValue);
    }
    return SLANG_E_INVALID_ARG;
}

static void _initializeFakeNVVMBuilderV2()
{
    gFakeNVVMBuilder.reset();
    gFakeNVVMBuilder.api = _makeFakeNVVMBuilderAPI();
    gFakeNVVMBuilder.apiV2 = _makeFakeNVVMBuilderAPIV2();
    gFakeNVVMBuilder.omitAPIV2Symbol = false;
}

static void _runNVVMScalarBuilderAPINegotiation(NVVMScalarTestOperation operation)
{
    const NVVMScalarTestCase& testCase = _getNVVMScalarTestCase(operation);
    const NVVMScalarBuilderAPICase apiCase = _getNVVMScalarBuilderAPICase(operation);
    const bool isUnary = testCase.key.family == FakeNVVMBuilderScalarFamily::Unary;
    const bool isCompare = testCase.key.family == FakeNVVMBuilderScalarFamily::Compare;

    SLANG_CHECK(apiCase.callbackOffset == apiCase.previousSize);
    SLANG_CHECK(apiCase.nextCallbackOffset == apiCase.completeSize);
    SLANG_CHECK(apiCase.completeSize == apiCase.previousSize + sizeof(void*));

    {
        NVVMIRBuilder builder;
        SlangNVVMValueHandle_1 result = _getFakeNVVMBuilderFunction();
        SLANG_CHECK(
            _emitNVVMScalarBuilderOperation(
                builder,
                operation,
                _getFakeNVVMBuilderModule(),
                _getFakeNVVMBuilderParameter(),
                _getFakeNVVMBuilderParameter(1),
                result) == SLANG_E_UNINITIALIZED);
        SLANG_CHECK(result == nullptr);
    }

    _initializeFakeNVVMBuilderV2();
    gFakeNVVMBuilder.apiV2.structureSize = apiCase.previousSize;
    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(NVVMIRBuilder::load(String(), loader, builder)));
        loader.setNull();
        SLANG_CHECK(!_supportsNVVMScalarBuilderOperation(builder, operation));
        StringBuilder feature;
        feature << apiCase.versionFeature << "=0";
        SLANG_CHECK(builder.getVersionString().indexOf(feature.getUnownedSlice()) >= 0);

        SlangNVVMValueHandle_1 result = _getFakeNVVMBuilderFunction();
        SLANG_CHECK(
            _emitNVVMScalarBuilderOperation(
                builder,
                operation,
                _getFakeNVVMBuilderModule(),
                _getFakeNVVMBuilderParameter(),
                _getFakeNVVMBuilderParameter(1),
                result) == SLANG_E_NOT_AVAILABLE);
        SLANG_CHECK(result == nullptr);
        SLANG_CHECK(
            _getFakeNVVMBuilderScalarOperationCallCount(
                testCase.key.family,
                testCase.key.operation) == 0);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);

    for (uint32_t partialSize = apiCase.previousSize + 1; partialSize < apiCase.completeSize;
         ++partialSize)
    {
        _initializeFakeNVVMBuilderV2();
        gFakeNVVMBuilder.apiV2.structureSize = partialSize;
        {
            ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
            NVVMIRBuilder builder;
            SLANG_CHECK(NVVMIRBuilder::load(String(), loader, builder) == SLANG_E_NO_INTERFACE);
            SLANG_CHECK(!builder.isInitialized());
        }
        SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    }

    _initializeFakeNVVMBuilderV2();
    _removeFakeNVVMScalarBuilderCallback(gFakeNVVMBuilder.apiV2, operation);
    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK(NVVMIRBuilder::load(String(), loader, builder) == SLANG_E_NO_INTERFACE);
        SLANG_CHECK(!builder.isInitialized());
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);

    _initializeFakeNVVMBuilderV2();
    gFakeNVVMBuilder.apiV2.structureSize = uint32_t(sizeof(SlangNVVMBuilderAPI_V2) + 16);
    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(NVVMIRBuilder::load(String(), loader, builder)));
        loader.setNull();
        SLANG_CHECK(_supportsNVVMScalarBuilderOperation(builder, operation));
        StringBuilder feature;
        feature << apiCase.versionFeature << "=1";
        SLANG_CHECK(builder.getVersionString().indexOf(feature.getUnownedSlice()) >= 0);
        SLANG_CHECK_ABORT(builder.getAPIV2() != nullptr);
        SLANG_CHECK(builder.getAPIV2()->structureSize == sizeof(SlangNVVMBuilderAPI_V2));
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);

    _initializeFakeNVVMBuilderV2();
    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(NVVMIRBuilder::load(String(), loader, builder)));
        loader.setNull();
        SLANG_CHECK(_supportsNVVMScalarBuilderOperation(builder, operation));

        ScopedNVVMBuilderModule scope;
        scope.builder = &builder;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.createModule(toSlice("fake-scalar-operation"), scope.module)));
        SlangNVVMTypeHandle_1 voidType = nullptr;
        SlangNVVMTypeHandle_1 integerType = nullptr;
        SlangNVVMTypeHandle_1 pointerType = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(scope.module, voidType)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getIntegerType(scope.module, 32, integerType)));
        if (!isCompare)
        {
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getPointerType(
                scope.module,
                integerType,
                SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
                pointerType)));
        }
        const SlangNVVMTypeHandle_1 parameterTypes[] = {
            isCompare ? integerType : pointerType,
            integerType,
            integerType,
        };
        const size_t parameterCount = isCompare ? 2 : (isUnary ? 2 : 3);
        SlangNVVMTypeHandle_1 functionType = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionType(
            scope.module,
            voidType,
            parameterTypes,
            parameterCount,
            functionType)));
        SlangNVVMValueHandle_1 function = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
            scope.module,
            functionType,
            toSlice("fakeScalarOperation"),
            function)));
        SlangNVVMValueHandle_1 parameters[3] = {};
        for (size_t i = 0; i < parameterCount; ++i)
        {
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                builder.getFunctionParameter(scope.module, function, i, parameters[i])));
        }
        SlangNVVMBlockHandle_1 block = nullptr;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.createBlock(scope.module, function, toSlice("entry"), block)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(scope.module, block)));

        SlangNVVMValueHandle_1 left = parameters[isCompare ? 0 : 1];
        SlangNVVMValueHandle_1 right = parameters[isCompare ? 1 : 2];
        SlangNVVMValueHandle_1 result = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_emitNVVMScalarBuilderOperation(
            builder,
            operation,
            scope.module,
            left,
            right,
            result)));
        SLANG_CHECK(result == _getFakeNVVMBuilderScalarOperation(0));
        SLANG_CHECK(gFakeNVVMBuilder.scalarOperations.getCount() == 1);
        const FakeNVVMBuilderScalarOperation& recorded = gFakeNVVMBuilder.scalarOperations[0];
        SLANG_CHECK(_isFakeNVVMBuilderScalarOperation(
            recorded.key,
            testCase.key.family,
            testCase.key.operation));
        SLANG_CHECK(recorded.callerBlockIndex == 0);
        SLANG_CHECK(recorded.operandCount == (isUnary ? 1 : 2));
        SLANG_CHECK(recorded.operands[0].kind == FakeNVVMBuilderValueKind::Parameter);
        SLANG_CHECK(recorded.operands[0].functionIndex == 0);
        SLANG_CHECK(recorded.operands[0].index == Index(isCompare ? 0 : 1));
        if (!isUnary)
        {
            SLANG_CHECK(recorded.operands[1].kind == FakeNVVMBuilderValueKind::Parameter);
            SLANG_CHECK(recorded.operands[1].functionIndex == 0);
            SLANG_CHECK(recorded.operands[1].index == Index(isCompare ? 1 : 2));
        }

        if (!isCompare)
        {
            result = _getFakeNVVMBuilderFunction();
            SLANG_CHECK(
                _emitNVVMScalarBuilderOperation(
                    builder,
                    operation,
                    scope.module,
                    parameters[0],
                    right,
                    result) == SLANG_E_INVALID_ARG);
            SLANG_CHECK(result == nullptr);
            if (!isUnary)
            {
                result = _getFakeNVVMBuilderFunction();
                SLANG_CHECK(
                    _emitNVVMScalarBuilderOperation(
                        builder,
                        operation,
                        scope.module,
                        left,
                        parameters[0],
                        result) == SLANG_E_INVALID_ARG);
                SLANG_CHECK(result == nullptr);
            }
        }

        _setFakeNVVMBuilderScalarOperationFailure(
            gFakeNVVMBuilder.returnNullScalarOperation,
            testCase.key.family,
            testCase.key.operation,
            true);
        result = _getFakeNVVMBuilderFunction();
        SLANG_CHECK(
            _emitNVVMScalarBuilderOperation(
                builder,
                operation,
                scope.module,
                left,
                right,
                result) == SLANG_FAIL);
        SLANG_CHECK(result == nullptr);
        _setFakeNVVMBuilderScalarOperationFailure(
            gFakeNVVMBuilder.returnNullScalarOperation,
            testCase.key.family,
            testCase.key.operation,
            false);

        _setFakeNVVMBuilderScalarOperationFailure(
            gFakeNVVMBuilder.failScalarOperationAfterWrite,
            testCase.key.family,
            testCase.key.operation,
            true);
        result = _getFakeNVVMBuilderFunction();
        SLANG_CHECK(
            _emitNVVMScalarBuilderOperation(
                builder,
                operation,
                scope.module,
                left,
                right,
                result) == SLANG_FAIL);
        SLANG_CHECK(result == nullptr);
        const int expectedCallCount = isCompare ? 3 : (isUnary ? 4 : 5);
        SLANG_CHECK(
            _getFakeNVVMBuilderScalarOperationCallCount(
                testCase.key.family,
                testCase.key.operation) == expectedCallCount);
        SLANG_CHECK(gFakeNVVMBuilder.scalarOperations.getCount() == 3);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
}

#define NVVM_SCALAR_BUILDER_API_TEST(NAME, OPERATION)                            \
    SLANG_UNIT_TEST(NAME)                                                        \
    {                                                                            \
        _runNVVMScalarBuilderAPINegotiation(NVVMScalarTestOperation::OPERATION); \
    }

NVVM_SCALAR_BUILDER_API_TEST(nvvmIRBuilderNegotiatesScalarIntegerMultiplyAPI, Multiply)
NVVM_SCALAR_BUILDER_API_TEST(nvvmIRBuilderNegotiatesScalarIntegerBitAndAPI, BitAnd)
NVVM_SCALAR_BUILDER_API_TEST(nvvmIRBuilderNegotiatesScalarIntegerBitOrAPI, BitOr)
NVVM_SCALAR_BUILDER_API_TEST(nvvmIRBuilderNegotiatesScalarIntegerBitXorAPI, BitXor)
NVVM_SCALAR_BUILDER_API_TEST(nvvmIRBuilderNegotiatesScalarIntegerBitNotAPI, BitNot)
NVVM_SCALAR_BUILDER_API_TEST(nvvmIRBuilderNegotiatesScalarIntegerNegateAPI, Negate)
SLANG_UNIT_TEST(nvvmIRBuilderNegotiatesRelaxedGlobalI32AtomicAddAPI)
{
    const uint32_t previousPrefixSize = sizeof(void*) == 8 ? 312u : 176u;
    const uint32_t atomicOperationEnd = sizeof(void*) == 8 ? 320u : 180u;
    const uint32_t completePrefixSize = sizeof(void*) == 8 ? 328u : 184u;
    SLANG_CHECK(
        offsetof(SlangNVVMBuilderAPI_V2, emitRelaxedGlobalI32AtomicAdd) ==
        SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_NEGATE_MIN_SIZE);
    SLANG_CHECK(
        offsetof(SlangNVVMBuilderAPI_V2, serializeNVVMIR20AssemblyWithDiagnostics) ==
        atomicOperationEnd);
    SLANG_CHECK(
        offsetof(SlangNVVMBuilderAPI_V2, emitIntegerEqual) ==
        SLANG_NVVM_BUILDER_API_V2_RELAXED_GLOBAL_I32_ATOMIC_ADD_MIN_SIZE);
    SLANG_CHECK(SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_NEGATE_MIN_SIZE == previousPrefixSize);
    SLANG_CHECK(
        SLANG_NVVM_BUILDER_API_V2_RELAXED_GLOBAL_I32_ATOMIC_ADD_MIN_SIZE == completePrefixSize);

    // An uninitialized wrapper must reject before dispatch and clear a stale result handle.
    {
        NVVMIRBuilder builder;
        SlangNVVMValueHandle_1 result = _getFakeNVVMBuilderFunction();
        SLANG_CHECK(
            builder.emitRelaxedGlobalI32AtomicAdd(
                _getFakeNVVMBuilderModule(),
                _getFakeNVVMBuilderParameter(),
                _getFakeNVVMBuilderParameter(1),
                result) == SLANG_E_UNINITIALIZED);
        SLANG_CHECK(result == nullptr);
    }

    // The exact Slice 17 prefix retains integer negate but cannot dispatch atomic add.
    gFakeNVVMBuilder.reset();
    gFakeNVVMBuilder.api = _makeFakeNVVMBuilderAPI();
    gFakeNVVMBuilder.apiV2 = _makeFakeNVVMBuilderAPIV2();
    gFakeNVVMBuilder.apiV2.structureSize =
        uint32_t(SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_NEGATE_MIN_SIZE);
    gFakeNVVMBuilder.omitAPIV2Symbol = false;
    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(NVVMIRBuilder::load(String(), loader, builder)));
        loader.setNull();
        SLANG_CHECK(builder.supportsScalarIntegerNegate());
        SLANG_CHECK(!builder.supportsNVVMIR20Assembly());
        SLANG_CHECK(!builder.supportsRelaxedGlobalI32AtomicAdd());
        SLANG_CHECK(builder.getVersionString().indexOf("nvvm-ir-2.0-assembly=0") >= 0);
        SLANG_CHECK(builder.getVersionString().indexOf("relaxed-global-i32-atomic-add=0") >= 0);

        SlangNVVMValueHandle_1 result = _getFakeNVVMBuilderFunction();
        SLANG_CHECK(
            builder.emitRelaxedGlobalI32AtomicAdd(
                _getFakeNVVMBuilderModule(),
                _getFakeNVVMBuilderParameter(),
                _getFakeNVVMBuilderParameter(1),
                result) == SLANG_E_NOT_AVAILABLE);
        SLANG_CHECK(result == nullptr);
        SLANG_CHECK(gFakeNVVMBuilder.emitRelaxedGlobalI32AtomicAddCallCount == 0);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);

    // No byte count inside the two-function suffix describes a coherent capability.
    for (uint32_t partialSize =
             uint32_t(SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_NEGATE_MIN_SIZE + 1);
         partialSize < uint32_t(SLANG_NVVM_BUILDER_API_V2_RELAXED_GLOBAL_I32_ATOMIC_ADD_MIN_SIZE);
         ++partialSize)
    {
        gFakeNVVMBuilder.reset();
        gFakeNVVMBuilder.api = _makeFakeNVVMBuilderAPI();
        gFakeNVVMBuilder.apiV2 = _makeFakeNVVMBuilderAPIV2();
        gFakeNVVMBuilder.apiV2.structureSize = partialSize;
        gFakeNVVMBuilder.omitAPIV2Symbol = false;
        {
            ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
            NVVMIRBuilder builder;
            SLANG_CHECK(NVVMIRBuilder::load(String(), loader, builder) == SLANG_E_NO_INTERFACE);
            SLANG_CHECK(!builder.isInitialized());
        }
        SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    }

    // Claiming the complete suffix makes both its operation and wire serializer mandatory.
    gFakeNVVMBuilder.reset();
    gFakeNVVMBuilder.api = _makeFakeNVVMBuilderAPI();
    gFakeNVVMBuilder.apiV2 = _makeFakeNVVMBuilderAPIV2();
    gFakeNVVMBuilder.apiV2.emitRelaxedGlobalI32AtomicAdd = nullptr;
    gFakeNVVMBuilder.omitAPIV2Symbol = false;
    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK(NVVMIRBuilder::load(String(), loader, builder) == SLANG_E_NO_INTERFACE);
        SLANG_CHECK(!builder.isInitialized());
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);

    gFakeNVVMBuilder.reset();
    gFakeNVVMBuilder.api = _makeFakeNVVMBuilderAPI();
    gFakeNVVMBuilder.apiV2 = _makeFakeNVVMBuilderAPIV2();
    gFakeNVVMBuilder.apiV2.serializeNVVMIR20AssemblyWithDiagnostics = nullptr;
    gFakeNVVMBuilder.omitAPIV2Symbol = false;
    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK(NVVMIRBuilder::load(String(), loader, builder) == SLANG_E_NO_INTERFACE);
        SLANG_CHECK(!builder.isInitialized());
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);

    // Future providers are accepted, advertise the operation, and are clamped to the known table.
    gFakeNVVMBuilder.reset();
    gFakeNVVMBuilder.api = _makeFakeNVVMBuilderAPI();
    gFakeNVVMBuilder.apiV2 = _makeFakeNVVMBuilderAPIV2();
    gFakeNVVMBuilder.apiV2.structureSize = uint32_t(sizeof(SlangNVVMBuilderAPI_V2) + 16);
    gFakeNVVMBuilder.omitAPIV2Symbol = false;
    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(NVVMIRBuilder::load(String(), loader, builder)));
        loader.setNull();
        SLANG_CHECK(builder.supportsNVVMIR20Assembly());
        SLANG_CHECK(builder.supportsRelaxedGlobalI32AtomicAdd());
        SLANG_CHECK(builder.getVersionString().indexOf("nvvm-ir-2.0-assembly=1") >= 0);
        SLANG_CHECK(builder.getVersionString().indexOf("relaxed-global-i32-atomic-add=1") >= 0);
        SLANG_CHECK_ABORT(builder.getAPIV2() != nullptr);
        SLANG_CHECK(builder.getAPIV2()->structureSize == sizeof(SlangNVVMBuilderAPI_V2));
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);

    // The complete suffix forwards exact pointer/value identities. The wrapper rejects success
    // without a handle and clears provider-written handles from failed calls.
    gFakeNVVMBuilder.reset();
    gFakeNVVMBuilder.api = _makeFakeNVVMBuilderAPI();
    gFakeNVVMBuilder.apiV2 = _makeFakeNVVMBuilderAPIV2();
    gFakeNVVMBuilder.omitAPIV2Symbol = false;
    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(NVVMIRBuilder::load(String(), loader, builder)));
        loader.setNull();
        SLANG_CHECK(builder.supportsNVVMIR20Assembly());
        SLANG_CHECK(builder.supportsRelaxedGlobalI32AtomicAdd());

        ScopedNVVMBuilderModule scope;
        scope.builder = &builder;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            builder.createModule(toSlice("fake-relaxed-global-i32-atomic-add"), scope.module)));
        SlangNVVMTypeHandle_1 voidType = nullptr;
        SlangNVVMTypeHandle_1 integerType = nullptr;
        SlangNVVMTypeHandle_1 pointerType = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(scope.module, voidType)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getIntegerType(scope.module, 32, integerType)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getPointerType(
            scope.module,
            integerType,
            SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
            pointerType)));
        const SlangNVVMTypeHandle_1 parameterTypes[] = {pointerType, integerType};
        SlangNVVMTypeHandle_1 functionType = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionType(
            scope.module,
            voidType,
            parameterTypes,
            SLANG_COUNT_OF(parameterTypes),
            functionType)));
        SlangNVVMValueHandle_1 function = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
            scope.module,
            functionType,
            toSlice("fakeRelaxedGlobalI32AtomicAdd"),
            function)));
        SlangNVVMValueHandle_1 destination = nullptr;
        SlangNVVMValueHandle_1 value = nullptr;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.getFunctionParameter(scope.module, function, 0, destination)));
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.getFunctionParameter(scope.module, function, 1, value)));
        SlangNVVMBlockHandle_1 block = nullptr;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.createBlock(scope.module, function, toSlice("entry"), block)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(scope.module, block)));

        SlangNVVMValueHandle_1 oldValue = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            builder.emitRelaxedGlobalI32AtomicAdd(scope.module, destination, value, oldValue)));
        SLANG_CHECK(oldValue == _getFakeNVVMBuilderRelaxedGlobalI32AtomicAdd());
        SLANG_CHECK(gFakeNVVMBuilder.relaxedGlobalI32AtomicAddCallerBlockIndices[0] == 0);
        SLANG_CHECK(
            gFakeNVVMBuilder.relaxedGlobalI32AtomicAddPointerValueRefs[0].kind ==
            FakeNVVMBuilderValueKind::Parameter);
        SLANG_CHECK(gFakeNVVMBuilder.relaxedGlobalI32AtomicAddPointerValueRefs[0].index == 0);
        SLANG_CHECK(
            gFakeNVVMBuilder.relaxedGlobalI32AtomicAddValueRefs[0].kind ==
            FakeNVVMBuilderValueKind::Parameter);
        SLANG_CHECK(gFakeNVVMBuilder.relaxedGlobalI32AtomicAddValueRefs[0].index == 1);

        oldValue = _getFakeNVVMBuilderFunction();
        SLANG_CHECK(
            builder.emitRelaxedGlobalI32AtomicAdd(scope.module, value, destination, oldValue) ==
            SLANG_E_INVALID_ARG);
        SLANG_CHECK(oldValue == nullptr);

        gFakeNVVMBuilder.returnNullRelaxedGlobalI32AtomicAdd = true;
        oldValue = _getFakeNVVMBuilderFunction();
        SLANG_CHECK(
            builder.emitRelaxedGlobalI32AtomicAdd(scope.module, destination, value, oldValue) ==
            SLANG_FAIL);
        SLANG_CHECK(oldValue == nullptr);
        gFakeNVVMBuilder.returnNullRelaxedGlobalI32AtomicAdd = false;

        gFakeNVVMBuilder.failRelaxedGlobalI32AtomicAddAfterWrite = true;
        oldValue = _getFakeNVVMBuilderFunction();
        SLANG_CHECK(
            builder.emitRelaxedGlobalI32AtomicAdd(scope.module, destination, value, oldValue) ==
            SLANG_FAIL);
        SLANG_CHECK(oldValue == nullptr);
        SLANG_CHECK(gFakeNVVMBuilder.emitRelaxedGlobalI32AtomicAddCallCount == 4);
        SLANG_CHECK(gFakeNVVMBuilder.relaxedGlobalI32AtomicAddValueRefs.getCount() == 3);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
}

NVVM_SCALAR_BUILDER_API_TEST(nvvmIRBuilderNegotiatesScalarIntegerEqualAPI, Equal)
NVVM_SCALAR_BUILDER_API_TEST(nvvmIRBuilderNegotiatesScalarIntegerNotEqualAPI, NotEqual)
NVVM_SCALAR_BUILDER_API_TEST(
    nvvmIRBuilderNegotiatesScalarIntegerSignedGreaterThanAPI,
    SignedGreaterThan)
NVVM_SCALAR_BUILDER_API_TEST(
    nvvmIRBuilderNegotiatesScalarIntegerSignedLessEqualAPI,
    SignedLessEqual)
NVVM_SCALAR_BUILDER_API_TEST(
    nvvmIRBuilderNegotiatesScalarIntegerSignedGreaterEqualAPI,
    SignedGreaterEqual)

#undef NVVM_SCALAR_BUILDER_API_TEST
SLANG_UNIT_TEST(nvvmIRBuilderNegotiatesRawRWStructuredBufferI32API)
{
    const uint32_t previousPrefixSize = sizeof(void*) == 8 ? 368u : 204u;
    const uint32_t typePrefixSize = sizeof(void*) == 8 ? 376u : 208u;
    const uint32_t completePrefixSize = sizeof(void*) == 8 ? 384u : 212u;
    SLANG_CHECK(
        offsetof(SlangNVVMBuilderAPI_V2, getRawRWStructuredBufferI32Type) ==
        SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_SIGNED_GREATER_EQUAL_MIN_SIZE);
    SLANG_CHECK(
        offsetof(SlangNVVMBuilderAPI_V2, emitRawRWStructuredBufferI32ElementPointer) ==
        typePrefixSize);
    SLANG_CHECK(
        SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_SIGNED_GREATER_EQUAL_MIN_SIZE ==
        previousPrefixSize);
    SLANG_CHECK(
        sizeof(SlangNVVMBuilderAPI_V2) ==
        SLANG_NVVM_BUILDER_API_V2_RAW_RW_STRUCTURED_BUFFER_I32_MIN_SIZE);
    SLANG_CHECK(
        SLANG_NVVM_BUILDER_API_V2_RAW_RW_STRUCTURED_BUFFER_I32_MIN_SIZE == completePrefixSize);

    // An uninitialized wrapper rejects before dispatch and clears stale handles.
    {
        NVVMIRBuilder builder;
        SlangNVVMTypeHandle_1 type = _getFakeNVVMBuilderIntegerType();
        SLANG_CHECK(
            builder.getRawRWStructuredBufferI32Type(_getFakeNVVMBuilderModule(), type) ==
            SLANG_E_UNINITIALIZED);
        SLANG_CHECK(type == nullptr);
        SlangNVVMValueHandle_1 pointer = _getFakeNVVMBuilderFunction();
        SLANG_CHECK(
            builder.emitRawRWStructuredBufferI32ElementPointer(
                _getFakeNVVMBuilderModule(),
                _getFakeNVVMBuilderParameter(),
                _getFakeNVVMBuilderParameter(1),
                pointer) == SLANG_E_UNINITIALIZED);
        SLANG_CHECK(pointer == nullptr);
    }

    // An exact Slice 25 provider remains usable, but does not advertise resource lowering.
    gFakeNVVMBuilder.reset();
    gFakeNVVMBuilder.api = _makeFakeNVVMBuilderAPI();
    gFakeNVVMBuilder.apiV2 = _makeFakeNVVMBuilderAPIV2();
    gFakeNVVMBuilder.apiV2.structureSize = previousPrefixSize;
    gFakeNVVMBuilder.omitAPIV2Symbol = false;
    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(NVVMIRBuilder::load(String(), loader, builder)));
        loader.setNull();
        SLANG_CHECK(builder.supportsScalarIntegerSignedGreaterEqual());
        SLANG_CHECK(!builder.supportsRawRWStructuredBufferI32());
        SLANG_CHECK(builder.getVersionString().indexOf("raw-rw-structured-buffer-i32=0") >= 0);
        SlangNVVMTypeHandle_1 type = _getFakeNVVMBuilderIntegerType();
        SLANG_CHECK(
            builder.getRawRWStructuredBufferI32Type(_getFakeNVVMBuilderModule(), type) ==
            SLANG_E_NOT_AVAILABLE);
        SLANG_CHECK(type == nullptr);
        SlangNVVMValueHandle_1 pointer = _getFakeNVVMBuilderFunction();
        SLANG_CHECK(
            builder.emitRawRWStructuredBufferI32ElementPointer(
                _getFakeNVVMBuilderModule(),
                _getFakeNVVMBuilderParameter(),
                _getFakeNVVMBuilderParameter(1),
                pointer) == SLANG_E_NOT_AVAILABLE);
        SLANG_CHECK(pointer == nullptr);
        SLANG_CHECK(gFakeNVVMBuilder.getRawRWStructuredBufferI32TypeCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitRawRWStructuredBufferI32ElementPointerCallCount == 0);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);

    // Neither a partial type pointer nor a complete type without its operation is coherent.
    for (uint32_t partialSize = previousPrefixSize + 1; partialSize < completePrefixSize;
         ++partialSize)
    {
        gFakeNVVMBuilder.reset();
        gFakeNVVMBuilder.api = _makeFakeNVVMBuilderAPI();
        gFakeNVVMBuilder.apiV2 = _makeFakeNVVMBuilderAPIV2();
        gFakeNVVMBuilder.apiV2.structureSize = partialSize;
        gFakeNVVMBuilder.omitAPIV2Symbol = false;
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK(NVVMIRBuilder::load(String(), loader, builder) == SLANG_E_NO_INTERFACE);
        SLANG_CHECK(!builder.isInitialized());
        loader.setNull();
        SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    }

    // Both callbacks are mandatory once the complete resource prefix is claimed.
    for (Index omittedCallback = 0; omittedCallback < 2; ++omittedCallback)
    {
        gFakeNVVMBuilder.reset();
        gFakeNVVMBuilder.api = _makeFakeNVVMBuilderAPI();
        gFakeNVVMBuilder.apiV2 = _makeFakeNVVMBuilderAPIV2();
        if (omittedCallback == 0)
            gFakeNVVMBuilder.apiV2.getRawRWStructuredBufferI32Type = nullptr;
        else
            gFakeNVVMBuilder.apiV2.emitRawRWStructuredBufferI32ElementPointer = nullptr;
        gFakeNVVMBuilder.omitAPIV2Symbol = false;
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK(NVVMIRBuilder::load(String(), loader, builder) == SLANG_E_NO_INTERFACE);
        SLANG_CHECK(!builder.isInitialized());
        loader.setNull();
        SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    }

    // A future provider is clamped to the local table and forwards exact identities.
    gFakeNVVMBuilder.reset();
    gFakeNVVMBuilder.api = _makeFakeNVVMBuilderAPI();
    gFakeNVVMBuilder.apiV2 = _makeFakeNVVMBuilderAPIV2();
    gFakeNVVMBuilder.apiV2.structureSize = uint32_t(sizeof(SlangNVVMBuilderAPI_V2) + 16);
    gFakeNVVMBuilder.omitAPIV2Symbol = false;
    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(NVVMIRBuilder::load(String(), loader, builder)));
        loader.setNull();
        SLANG_CHECK(builder.supportsRawRWStructuredBufferI32());
        SLANG_CHECK(builder.getVersionString().indexOf("raw-rw-structured-buffer-i32=1") >= 0);
        SLANG_CHECK_ABORT(builder.getAPIV2() != nullptr);
        SLANG_CHECK(builder.getAPIV2()->structureSize == sizeof(SlangNVVMBuilderAPI_V2));

        ScopedNVVMBuilderModule scope;
        scope.builder = &builder;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            builder.createModule(toSlice("fake-raw-rw-structured-buffer-i32"), scope.module)));
        SlangNVVMTypeHandle_1 voidType = nullptr;
        SlangNVVMTypeHandle_1 integerType = nullptr;
        SlangNVVMTypeHandle_1 resourceType = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(scope.module, voidType)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getIntegerType(scope.module, 32, integerType)));
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.getRawRWStructuredBufferI32Type(scope.module, resourceType)));
        SLANG_CHECK(resourceType == _getFakeNVVMBuilderRawRWStructuredBufferI32Type());
        const SlangNVVMTypeHandle_1 parameterTypes[] = {resourceType, integerType};
        SlangNVVMTypeHandle_1 functionType = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionType(
            scope.module,
            voidType,
            parameterTypes,
            SLANG_COUNT_OF(parameterTypes),
            functionType)));
        SlangNVVMValueHandle_1 function = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
            scope.module,
            functionType,
            toSlice("fakeRawRWStructuredBufferI32"),
            function)));
        SlangNVVMValueHandle_1 buffer = nullptr;
        SlangNVVMValueHandle_1 index = nullptr;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.getFunctionParameter(scope.module, function, 0, buffer)));
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.getFunctionParameter(scope.module, function, 1, index)));
        SlangNVVMBlockHandle_1 block = nullptr;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.createBlock(scope.module, function, toSlice("entry"), block)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(scope.module, block)));

        SlangNVVMValueHandle_1 pointer = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            builder
                .emitRawRWStructuredBufferI32ElementPointer(scope.module, buffer, index, pointer)));
        SLANG_CHECK(pointer == _getFakeNVVMBuilderRawRWStructuredBufferI32ElementPointer());
        SLANG_CHECK(
            gFakeNVVMBuilder.rawRWStructuredBufferI32ElementPointerCallerBlockIndices[0] == 0);
        SLANG_CHECK(
            gFakeNVVMBuilder.rawRWStructuredBufferI32ElementPointerBufferValueRefs[0].index == 0);
        SLANG_CHECK(
            gFakeNVVMBuilder.rawRWStructuredBufferI32ElementPointerIndexValueRefs[0].index == 1);

        gFakeNVVMBuilder.returnNullRawRWStructuredBufferI32Type = true;
        resourceType = _getFakeNVVMBuilderIntegerType();
        SLANG_CHECK(
            builder.getRawRWStructuredBufferI32Type(scope.module, resourceType) == SLANG_FAIL);
        SLANG_CHECK(resourceType == nullptr);
        gFakeNVVMBuilder.returnNullRawRWStructuredBufferI32Type = false;
        gFakeNVVMBuilder.failRawRWStructuredBufferI32TypeAfterWrite = true;
        resourceType = _getFakeNVVMBuilderIntegerType();
        SLANG_CHECK(
            builder.getRawRWStructuredBufferI32Type(scope.module, resourceType) == SLANG_FAIL);
        SLANG_CHECK(resourceType == nullptr);

        gFakeNVVMBuilder.returnNullRawRWStructuredBufferI32ElementPointer = true;
        pointer = _getFakeNVVMBuilderFunction();
        SLANG_CHECK(
            builder
                .emitRawRWStructuredBufferI32ElementPointer(scope.module, buffer, index, pointer) ==
            SLANG_FAIL);
        SLANG_CHECK(pointer == nullptr);
        gFakeNVVMBuilder.returnNullRawRWStructuredBufferI32ElementPointer = false;
        gFakeNVVMBuilder.failRawRWStructuredBufferI32ElementPointerAfterWrite = true;
        pointer = _getFakeNVVMBuilderFunction();
        SLANG_CHECK(
            builder
                .emitRawRWStructuredBufferI32ElementPointer(scope.module, buffer, index, pointer) ==
            SLANG_FAIL);
        SLANG_CHECK(pointer == nullptr);
        SLANG_CHECK(gFakeNVVMBuilder.getRawRWStructuredBufferI32TypeCallCount == 3);
        SLANG_CHECK(gFakeNVVMBuilder.emitRawRWStructuredBufferI32ElementPointerCallCount == 3);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmIRBuilderNegotiatesV2Diagnostics)
{
    gFakeNVVMBuilder.reset();
    {
        ComPtr<ISlangSharedLibrary> library(new FakeNVVMBuilderLibrary);
        NVVMIRBuilder rejectedBuilder;

        SlangNVVMBuilderAPI_V2 invalidAPI = _makeFakeNVVMBuilderAPIV2();
        invalidAPI.structureSize = uint32_t(SLANG_NVVM_BUILDER_API_V2_MIN_SIZE - 1);
        SLANG_CHECK(
            NVVMIRBuilder::initialize(invalidAPI, library, rejectedBuilder) ==
            SLANG_E_NO_INTERFACE);

        invalidAPI = _makeFakeNVVMBuilderAPIV2();
        invalidAPI.abiVersion += 1;
        SLANG_CHECK(
            NVVMIRBuilder::initialize(invalidAPI, library, rejectedBuilder) ==
            SLANG_E_NO_INTERFACE);

        invalidAPI = _makeFakeNVVMBuilderAPIV2();
        invalidAPI.serializeModuleWithDiagnostics = nullptr;
        SLANG_CHECK(
            NVVMIRBuilder::initialize(invalidAPI, library, rejectedBuilder) ==
            SLANG_E_NO_INTERFACE);

        invalidAPI = _makeFakeNVVMBuilderAPIV2();
        invalidAPI.baseAPI.llvmVersionMajor = 15;
        SLANG_CHECK(
            NVVMIRBuilder::initialize(invalidAPI, library, rejectedBuilder) ==
            SLANG_E_NO_INTERFACE);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVMBuilder.destroyedLibraryCount == 1);

    gFakeNVVMBuilder.reset();
    gFakeNVVMBuilder.api = _makeFakeNVVMBuilderAPI();
    gFakeNVVMBuilder.apiV2 = _makeFakeNVVMBuilderAPIV2();
    // A provider may append fields to V2. The host accepts its known prefix without reading the
    // unknown tail, then clamps the retained table size to the local prefix it actually stores.
    gFakeNVVMBuilder.apiV2.structureSize = uint32_t(sizeof(SlangNVVMBuilderAPI_V2) + 16);
    gFakeNVVMBuilder.omitAPIV2Symbol = false;

    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(NVVMIRBuilder::load(String(), loader, builder)));
        loader.setNull();
        SLANG_CHECK(builder.isInitialized());
        SLANG_CHECK(builder.supportsSerializationDiagnostics());
        SLANG_CHECK(builder.supportsScalarArrayAddressing());
        SLANG_CHECK(builder.getVersionString().indexOf("scalar-array-addressing=1") >= 0);
        SLANG_CHECK_ABORT(builder.getAPIV2() != nullptr);
        SLANG_CHECK(builder.getAPIV2()->structureSize == sizeof(SlangNVVMBuilderAPI_V2));
        SLANG_CHECK(builder.getAPI().llvmVersionMajor == 14);

        ScopedNVVMBuilderModule scope;
        scope.builder = &builder;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.createModule(toSlice("fake-v2-module"), scope.module)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            _populateEmptyNVVMKernel(builder, scope.module, toSlice("fakeV2Kernel"))));

        ComPtr<ISlangBlob> bitcode;
        String diagnostics = "stale diagnostics";
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(
            scope.module,
            SLANG_NVVM_SERIALIZATION_FORMAT_BITCODE,
            bitcode,
            diagnostics)));
        SLANG_CHECK_ABORT(bitcode != nullptr);
        SLANG_CHECK_ABORT(bitcode->getBufferSize() == sizeof(kFakeNVVMBuilderBitcode));
        SLANG_CHECK(
            ::memcmp(
                bitcode->getBufferPointer(),
                kFakeNVVMBuilderBitcode,
                sizeof(kFakeNVVMBuilderBitcode)) == 0);
        SLANG_CHECK(diagnostics.getLength() == 0);

        // Make both result buffers non-empty, then undersize only the diagnostic buffer. Atomic
        // failure means the otherwise sufficient serialization buffer must retain its sentinels.
        gFakeNVVMBuilder.verificationDiagnostic = "fake valid verification note";
        size_t requiredSerializedSize = 0;
        size_t requiredDiagnosticSize = 0;
        SlangNVVMVerificationStatus_2 verificationStatus = SLANG_NVVM_VERIFICATION_NOT_RUN;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(gFakeNVVMBuilder.apiV2.serializeModuleWithDiagnostics(
            scope.module,
            SLANG_NVVM_SERIALIZATION_FORMAT_BITCODE,
            nullptr,
            0,
            &requiredSerializedSize,
            nullptr,
            0,
            &requiredDiagnosticSize,
            &verificationStatus)));
        SLANG_CHECK(requiredSerializedSize == sizeof(kFakeNVVMBuilderBitcode));
        SLANG_CHECK(requiredDiagnosticSize > 1);
        SLANG_CHECK(verificationStatus == SLANG_NVVM_VERIFICATION_VALID);

        List<uint8_t> serializedSentinels;
        serializedSentinels.setCount(Index(requiredSerializedSize));
        for (auto& value : serializedSentinels)
            value = 0xa5;
        uint8_t diagnosticSentinel = 0x5a;
        size_t reportedSerializedSize = 0;
        size_t reportedDiagnosticSize = 0;
        verificationStatus = SLANG_NVVM_VERIFICATION_NOT_RUN;
        SLANG_CHECK(
            gFakeNVVMBuilder.apiV2.serializeModuleWithDiagnostics(
                scope.module,
                SLANG_NVVM_SERIALIZATION_FORMAT_BITCODE,
                serializedSentinels.getBuffer(),
                size_t(serializedSentinels.getCount()),
                &reportedSerializedSize,
                &diagnosticSentinel,
                1,
                &reportedDiagnosticSize,
                &verificationStatus) == SLANG_E_BUFFER_TOO_SMALL);
        SLANG_CHECK(reportedSerializedSize == requiredSerializedSize);
        SLANG_CHECK(reportedDiagnosticSize == requiredDiagnosticSize);
        SLANG_CHECK(verificationStatus == SLANG_NVVM_VERIFICATION_VALID);
        for (const auto value : serializedSentinels)
            SLANG_CHECK(value == 0xa5);
        SLANG_CHECK(diagnosticSentinel == 0x5a);

        diagnostics = "stale diagnostics";
        bitcode.setNull();
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(
            scope.module,
            SLANG_NVVM_SERIALIZATION_FORMAT_BITCODE,
            bitcode,
            diagnostics)));
        SLANG_CHECK_ABORT(bitcode != nullptr);
        SLANG_CHECK(diagnostics == gFakeNVVMBuilder.verificationDiagnostic);

        gFakeNVVMBuilder.verificationStatus = SLANG_NVVM_VERIFICATION_INVALID;
        static const char kEmbeddedDiagnostic[] =
            {'f', 'a', 'k', 'e', 0, 'L', 'L', 'V', 'M', ' ', 'e', 'r', 'r', 'o', 'r'};
        gFakeNVVMBuilder.verificationDiagnostic =
            String(UnownedStringSlice(kEmbeddedDiagnostic, SLANG_COUNT_OF(kEmbeddedDiagnostic)));
        diagnostics = "stale diagnostics";
        bitcode.setNull();
        SLANG_CHECK(
            builder.serializeModule(
                scope.module,
                SLANG_NVVM_SERIALIZATION_FORMAT_BITCODE,
                bitcode,
                diagnostics) == SLANG_FAIL);
        SLANG_CHECK(bitcode == nullptr);
        SLANG_CHECK_ABORT(diagnostics.getLength() == SLANG_COUNT_OF(kEmbeddedDiagnostic));
        SLANG_CHECK(
            ::memcmp(
                diagnostics.getBuffer(),
                kEmbeddedDiagnostic,
                SLANG_COUNT_OF(kEmbeddedDiagnostic)) == 0);

        gFakeNVVMBuilder.verificationStatus = SLANG_NVVM_VERIFICATION_NOT_RUN;
        gFakeNVVMBuilder.verificationDiagnostic = String();
        diagnostics = "stale diagnostics";
        SLANG_CHECK(
            builder.serializeModule(
                scope.module,
                SLANG_NVVM_SERIALIZATION_FORMAT_BITCODE,
                bitcode,
                diagnostics) == SLANG_FAIL);
        SLANG_CHECK(bitcode == nullptr);
        SLANG_CHECK(diagnostics.getLength() == 0);

        // Malformed successful responses are rejected after the query, before the wrapper asks the
        // provider to write either buffer.
        gFakeNVVMBuilder.verificationStatus = SLANG_NVVM_VERIFICATION_INVALID;
        const int emptyInvalidQueryCount = gFakeNVVMBuilder.serializeWithDiagnosticsQueryCallCount;
        const int emptyInvalidWriteCount = gFakeNVVMBuilder.serializeWithDiagnosticsWriteCallCount;
        diagnostics = "stale diagnostics";
        SLANG_CHECK(
            builder.serializeModule(
                scope.module,
                SLANG_NVVM_SERIALIZATION_FORMAT_BITCODE,
                bitcode,
                diagnostics) == SLANG_FAIL);
        SLANG_CHECK(bitcode == nullptr);
        SLANG_CHECK(diagnostics.getLength() == 0);
        SLANG_CHECK(
            gFakeNVVMBuilder.serializeWithDiagnosticsQueryCallCount == emptyInvalidQueryCount + 1);
        SLANG_CHECK(
            gFakeNVVMBuilder.serializeWithDiagnosticsWriteCallCount == emptyInvalidWriteCount);

        gFakeNVVMBuilder.verificationStatus = SLANG_NVVM_VERIFICATION_VALID;
        gFakeNVVMBuilder.omitValidSerializedOutput = true;
        const int emptyValidQueryCount = gFakeNVVMBuilder.serializeWithDiagnosticsQueryCallCount;
        const int emptyValidWriteCount = gFakeNVVMBuilder.serializeWithDiagnosticsWriteCallCount;
        diagnostics = "stale diagnostics";
        SLANG_CHECK(
            builder.serializeModule(
                scope.module,
                SLANG_NVVM_SERIALIZATION_FORMAT_BITCODE,
                bitcode,
                diagnostics) == SLANG_FAIL);
        SLANG_CHECK(bitcode == nullptr);
        SLANG_CHECK(diagnostics.getLength() == 0);
        SLANG_CHECK(
            gFakeNVVMBuilder.serializeWithDiagnosticsQueryCallCount == emptyValidQueryCount + 1);
        SLANG_CHECK(
            gFakeNVVMBuilder.serializeWithDiagnosticsWriteCallCount == emptyValidWriteCount);
        gFakeNVVMBuilder.omitValidSerializedOutput = false;

        gFakeNVVMBuilder.serializationWithDiagnosticsResult = SLANG_FAIL;
        diagnostics = "stale diagnostics";
        SLANG_CHECK(
            builder.serializeModule(
                scope.module,
                SLANG_NVVM_SERIALIZATION_FORMAT_BITCODE,
                bitcode,
                diagnostics) == SLANG_FAIL);
        SLANG_CHECK(bitcode == nullptr);
        SLANG_CHECK(diagnostics.getLength() == 0);
        gFakeNVVMBuilder.serializationWithDiagnosticsResult = SLANG_OK;
        gFakeNVVMBuilder.verificationStatus = SLANG_NVVM_VERIFICATION_VALID;

        gFakeNVVMBuilder.reportMismatchedSerializedDiagnosticWriteSize = true;
        SLANG_CHECK(
            builder.serializeModule(
                scope.module,
                SLANG_NVVM_SERIALIZATION_FORMAT_BITCODE,
                bitcode,
                diagnostics) == SLANG_FAIL);
        SLANG_CHECK(bitcode == nullptr);
        gFakeNVVMBuilder.reportMismatchedSerializedDiagnosticWriteSize = false;

        gFakeNVVMBuilder.verificationStatus = SLANG_NVVM_VERIFICATION_INVALID;
        gFakeNVVMBuilder.verificationDiagnostic = "fake stable diagnostic";
        gFakeNVVMBuilder.reportMismatchedVerificationDiagnosticWriteSize = true;
        diagnostics = "stale diagnostics";
        SLANG_CHECK(
            builder.serializeModule(
                scope.module,
                SLANG_NVVM_SERIALIZATION_FORMAT_BITCODE,
                bitcode,
                diagnostics) == SLANG_FAIL);
        SLANG_CHECK(bitcode == nullptr);
        SLANG_CHECK(diagnostics.getLength() == 0);
        gFakeNVVMBuilder.reportMismatchedVerificationDiagnosticWriteSize = false;

        gFakeNVVMBuilder.reportMismatchedVerificationStatus = true;
        diagnostics = "stale diagnostics";
        SLANG_CHECK(
            builder.serializeModule(
                scope.module,
                SLANG_NVVM_SERIALIZATION_FORMAT_BITCODE,
                bitcode,
                diagnostics) == SLANG_FAIL);
        SLANG_CHECK(bitcode == nullptr);
        SLANG_CHECK(diagnostics.getLength() == 0);
        gFakeNVVMBuilder.reportMismatchedVerificationStatus = false;

        SLANG_CHECK(gFakeNVVMBuilder.serializeWithDiagnosticsQueryCallCount > 0);
        SLANG_CHECK(gFakeNVVMBuilder.serializeWithDiagnosticsWriteCallCount > 0);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVMBuilder.destroyedLibraryCount == 1);
}

SLANG_UNIT_TEST(nvvmIRBuilderSerializesEmptyKernel)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);

    const SlangNVVMBuilderAPI_V1& api = builder.getAPI();
    SLANG_CHECK(api.llvmVersionMajor == 14);
    SLANG_CHECK(api.llvmVersionMinor == 0);
    SLANG_CHECK(api.llvmVersionPatch == 6);
    SLANG_CHECK(api.nvvmIRVersionMajor == 2);
    SLANG_CHECK(api.nvvmIRVersionMinor == 0);
    SLANG_CHECK(api.pointerModel == SLANG_NVVM_POINTER_MODEL_TYPED);

    static const char kKernelName[] = "slangSlice3aEmpty";
    ComPtr<ISlangBlob> assemblyBlob;
    ComPtr<ISlangBlob> bitcodeBlob;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        _buildEmptyNVVMKernel(builder, toSlice(kKernelName), assemblyBlob, bitcodeBlob)));
    SLANG_CHECK_ABORT(assemblyBlob != nullptr);
    SLANG_CHECK_ABORT(bitcodeBlob != nullptr);

    const String assembly(UnownedStringSlice(
        static_cast<const char*>(assemblyBlob->getBufferPointer()),
        assemblyBlob->getBufferSize()));
    static const char kExpectedDataLayout[] =
        "target datalayout = \"e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-"
        "i64:64:64-i128:128:128-f32:32:32-f64:64:64-v16:16:16-v32:32:32-v64:64:64-"
        "v128:128:128-n16:32:64\"";
    SLANG_CHECK(assembly.indexOf(kExpectedDataLayout) >= 0);
    SLANG_CHECK(assembly.indexOf("target triple = \"nvptx64-nvidia-cuda\"") >= 0);
    SLANG_CHECK(assembly.indexOf("define void @slangSlice3aEmpty()") >= 0);
    SLANG_CHECK(assembly.indexOf("!nvvmir.version") >= 0);
    SLANG_CHECK(assembly.indexOf("!nvvm.annotations") >= 0);
    SLANG_CHECK(assembly.indexOf("void ()* @slangSlice3aEmpty") >= 0);
    SLANG_CHECK(assembly.indexOf("!\"kernel\", i32 1") >= 0);

    SLANG_CHECK(bitcodeBlob->getBufferSize() > 4);
    static const uint8_t kBitcodeMagic[] = {0x42, 0x43, 0xc0, 0xde};
    SLANG_CHECK(
        ::memcmp(bitcodeBlob->getBufferPointer(), kBitcodeMagic, sizeof(kBitcodeMagic)) == 0);
    bool hasEmbeddedNull = false;
    const uint8_t* bitcodeBytes = static_cast<const uint8_t*>(bitcodeBlob->getBufferPointer());
    for (size_t i = 0; i < bitcodeBlob->getBufferSize(); ++i)
        hasEmbeddedNull = hasEmbeddedNull || bitcodeBytes[i] == 0;
    SLANG_CHECK(hasEmbeddedNull);
}

SLANG_UNIT_TEST(nvvmIRBuilderRejectsUnknownV3OperationsWithoutMutation)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    const SlangNVVMBuilderAPI_V3* api = builder.getAPIV3();
    SLANG_CHECK_ABORT(api != nullptr);

    ScopedNVVMBuilderModule scope;
    scope.builder = &builder;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.createModule(toSlice("unknown-v3-operations"), scope.module)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        _populateEmptyNVVMKernel(builder, scope.module, toSlice("unknownV3Operations"))));

    ComPtr<ISlangBlob> before;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.serializeModule(scope.module, SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY, before)));

    SlangNVVMValueHandle_1 output = reinterpret_cast<SlangNVVMValueHandle_1>(uintptr_t(1));
    SLANG_CHECK(
        api->emitIntegerUnary(scope.module, SlangNVVMIntegerUnaryOp_3(99), nullptr, &output) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(output == nullptr);
    output = reinterpret_cast<SlangNVVMValueHandle_1>(uintptr_t(1));
    SLANG_CHECK(
        api->emitIntegerBinary(
            scope.module,
            SlangNVVMIntegerBinaryOp_3(99),
            nullptr,
            nullptr,
            &output) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(output == nullptr);
    output = reinterpret_cast<SlangNVVMValueHandle_1>(uintptr_t(1));
    SLANG_CHECK(
        api->emitIntegerCompare(
            scope.module,
            SlangNVVMIntegerCompareOp_3(99),
            nullptr,
            nullptr,
            &output) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(output == nullptr);
    output = reinterpret_cast<SlangNVVMValueHandle_1>(uintptr_t(1));
    SLANG_CHECK(
        api->emitFloatingBinary(
            scope.module,
            SlangNVVMFloatingBinaryOp_3(99),
            nullptr,
            nullptr,
            &output) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(output == nullptr);

    ComPtr<ISlangBlob> after;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.serializeModule(scope.module, SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY, after)));
    SLANG_CHECK_ABORT(before != nullptr && after != nullptr);
    SLANG_CHECK(before->getBufferSize() == after->getBufferSize());
    SLANG_CHECK(
        ::memcmp(before->getBufferPointer(), after->getBufferPointer(), before->getBufferSize()) ==
        0);
}

SLANG_UNIT_TEST(nvvmIRBuilderRejectsInvalidFloat32Operations)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    const SlangNVVMBuilderAPI_V3* api = builder.getAPIV3();
    SLANG_CHECK_ABORT(api != nullptr);

    ScopedNVVMBuilderModule scope;
    ScopedNVVMBuilderModule foreignScope;
    scope.builder = &builder;
    foreignScope.builder = &builder;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.createModule(toSlice("invalid-float32-main"), scope.module)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createModule(toSlice("invalid-float32-foreign"), foreignScope.module)));

    SlangNVVMTypeHandle_1 invalidType = reinterpret_cast<SlangNVVMTypeHandle_1>(uintptr_t(1));
    SLANG_CHECK(api->getFloatingPointType(scope.module, 16, &invalidType) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(invalidType == nullptr);

    SlangNVVMTypeHandle_1 voidType = nullptr;
    SlangNVVMTypeHandle_1 floatType = nullptr;
    SlangNVVMTypeHandle_1 integerType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(scope.module, voidType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFloatingPointType(scope.module, 32, floatType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getIntegerType(scope.module, 32, integerType)));
    const SlangNVVMTypeHandle_1 parameterTypes[] = {floatType, floatType, integerType};
    SlangNVVMTypeHandle_1 functionType = nullptr;
    SlangNVVMValueHandle_1 function = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionType(
        scope.module,
        voidType,
        parameterTypes,
        SLANG_COUNT_OF(parameterTypes),
        functionType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.declareFunction(scope.module, functionType, toSlice("invalidFloat32"), function)));
    SlangNVVMValueHandle_1 left = nullptr;
    SlangNVVMValueHandle_1 right = nullptr;
    SlangNVVMValueHandle_1 integer = nullptr;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(scope.module, function, 0, left)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(scope.module, function, 1, right)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(scope.module, function, 2, integer)));
    SlangNVVMBlockHandle_1 entryBlock = nullptr;
    SlangNVVMBlockHandle_1 laterBlock = nullptr;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.createBlock(scope.module, function, toSlice("entry"), entryBlock)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.createBlock(scope.module, function, toSlice("later"), laterBlock)));

    SlangNVVMTypeHandle_1 foreignVoidType = nullptr;
    SlangNVVMTypeHandle_1 foreignFloatType = nullptr;
    SlangNVVMTypeHandle_1 foreignFunctionType = nullptr;
    SlangNVVMValueHandle_1 foreignFunction = nullptr;
    SlangNVVMValueHandle_1 foreignValue = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(foreignScope.module, foreignVoidType)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFloatingPointType(foreignScope.module, 32, foreignFloatType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionType(
        foreignScope.module,
        foreignVoidType,
        &foreignFloatType,
        1,
        foreignFunctionType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
        foreignScope.module,
        foreignFunctionType,
        toSlice("foreignFloat32"),
        foreignFunction)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionParameter(foreignScope.module, foreignFunction, 0, foreignValue)));

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(scope.module, laterBlock)));
    SlangNVVMValueHandle_1 laterValue = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitFloatingBinary(
        scope.module,
        SLANG_NVVM_FLOATING_BINARY_OP_ADD,
        left,
        right,
        laterValue)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitReturnVoid(scope.module)));

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(scope.module, entryBlock)));
    const SlangNVVMValueHandle_1 invalidOperands[][2] = {
        {integer, integer},
        {left, integer},
        {left, foreignValue},
        {laterValue, left},
        {nullptr, right},
    };
    for (const auto& operands : invalidOperands)
    {
        SlangNVVMValueHandle_1 output = reinterpret_cast<SlangNVVMValueHandle_1>(uintptr_t(1));
        SLANG_CHECK(
            api->emitFloatingBinary(
                scope.module,
                SLANG_NVVM_FLOATING_BINARY_OP_ADD,
                operands[0],
                operands[1],
                &output) == SLANG_E_INVALID_ARG);
        SLANG_CHECK(output == nullptr);
    }

    SlangNVVMValueHandle_1 sum = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitFloatingBinary(
        scope.module,
        SLANG_NVVM_FLOATING_BINARY_OP_ADD,
        left,
        right,
        sum)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitReturnVoid(scope.module)));
    SlangNVVMValueHandle_1 output = reinterpret_cast<SlangNVVMValueHandle_1>(uintptr_t(1));
    SLANG_CHECK(
        api->emitFloatingBinary(
            scope.module,
            SLANG_NVVM_FLOATING_BINARY_OP_ADD,
            left,
            right,
            &output) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(output == nullptr);

    ComPtr<ISlangBlob> assemblyBlob;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(
        scope.module,
        SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
        assemblyBlob)));
    SLANG_CHECK_ABORT(assemblyBlob != nullptr);
    const UnownedStringSlice assembly(
        static_cast<const char*>(assemblyBlob->getBufferPointer()),
        assemblyBlob->getBufferSize());
    SLANG_CHECK(_countOccurrences(assembly, toSlice("fadd float")) == 2);
}

static void _runNVVMIRBuilderBuildsFloat32BinaryKernel(
    UnitTestContext* unitTestContext,
    NVVMFloat32BinaryTestOperation testOperation)
{
    const NVVMFloat32BinaryTestCase& testCase = _getNVVMFloat32BinaryTestCase(testOperation);
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    SLANG_CHECK_ABORT(builder.supportsFeature(testCase.feature));

    ScopedNVVMBuilderModule scope;
    scope.builder = &builder;
    StringBuilder moduleName;
    moduleName << testCase.diagnosticName << "-module";
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.createModule(moduleName.getUnownedSlice(), scope.module)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_populateFloat32BinaryKernel(
        builder,
        scope.module,
        UnownedStringSlice(testCase.kernelName),
        testCase.operation)));

    ComPtr<ISlangBlob> llvmAssembly;
    ComPtr<ISlangBlob> nvvmAssembly;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(
        scope.module,
        SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
        llvmAssembly)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(
        scope.module,
        SLANG_NVVM_SERIALIZATION_FORMAT_NVVM_IR_2_0_ASSEMBLY,
        nvvmAssembly)));
    SLANG_CHECK_ABORT(llvmAssembly != nullptr && nvvmAssembly != nullptr);

    const String llvmText(UnownedStringSlice(
        static_cast<const char*>(llvmAssembly->getBufferPointer()),
        llvmAssembly->getBufferSize()));
    const String nvvmText(UnownedStringSlice(
        static_cast<const char*>(nvvmAssembly->getBufferPointer()),
        nvvmAssembly->getBufferSize()));
    const String texts[] = {llvmText, nvvmText};
    for (const String& text : texts)
    {
        StringBuilder signature;
        signature << "define void @" << testCase.kernelName << "(float addrspace(1)*";
        SLANG_CHECK(text.indexOf(signature.getUnownedSlice()) >= 0);
        for (const auto& binaryCase : kNVVMFloat32BinaryTestCases)
        {
            StringBuilder instruction;
            instruction << binaryCase.llvmOpcode << " float";
            SLANG_CHECK(
                _countOccurrences(text.getUnownedSlice(), instruction.getUnownedSlice()) ==
                (&binaryCase == &testCase ? 1 : 0));
        }
        SLANG_CHECK(text.indexOf("store float") >= 0);
        SLANG_CHECK(text.indexOf("align 4") >= 0);
        SLANG_CHECK(text.indexOf("fast") < 0);
    }
    SLANG_CHECK(nvvmText.indexOf("!nvvmir.version") >= 0);
    SLANG_CHECK(nvvmText.indexOf("!\"kernel\", i32 1") >= 0);
}

#define NVVM_FLOAT32_BINARY_BUILDER_TEST(NAME, OPERATION) \
    SLANG_UNIT_TEST(NAME)                                 \
    {                                                     \
        _runNVVMIRBuilderBuildsFloat32BinaryKernel(       \
            unitTestContext,                              \
            NVVMFloat32BinaryTestOperation::OPERATION);   \
    }

NVVM_FLOAT32_BINARY_BUILDER_TEST(nvvmIRBuilderBuildsFloat32AddKernel, Add)
NVVM_FLOAT32_BINARY_BUILDER_TEST(nvvmIRBuilderBuildsFloat32SubtractKernel, Subtract)
NVVM_FLOAT32_BINARY_BUILDER_TEST(nvvmIRBuilderBuildsFloat32MultiplyKernel, Multiply)

#undef NVVM_FLOAT32_BINARY_BUILDER_TEST

SLANG_UNIT_TEST(nvvmIRBuilderBuildsFloat32CopyKernel)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    SLANG_CHECK_ABORT(builder.supportsFeature(SLANG_NVVM_BUILDER_FEATURE_SCALAR_FLOAT32_ADD));

    ScopedNVVMBuilderModule scope;
    scope.builder = &builder;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.createModule(toSlice("float32-copy-module"), scope.module)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(_populateFloat32CopyKernel(builder, scope.module, toSlice("float32Copy"))));

    ComPtr<ISlangBlob> llvmAssembly;
    ComPtr<ISlangBlob> nvvmAssembly;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(
        scope.module,
        SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
        llvmAssembly)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(
        scope.module,
        SLANG_NVVM_SERIALIZATION_FORMAT_NVVM_IR_2_0_ASSEMBLY,
        nvvmAssembly)));
    SLANG_CHECK_ABORT(llvmAssembly != nullptr && nvvmAssembly != nullptr);

    const String llvmText(UnownedStringSlice(
        static_cast<const char*>(llvmAssembly->getBufferPointer()),
        llvmAssembly->getBufferSize()));
    const String nvvmText(UnownedStringSlice(
        static_cast<const char*>(nvvmAssembly->getBufferPointer()),
        nvvmAssembly->getBufferSize()));
    const String texts[] = {llvmText, nvvmText};
    for (const String& text : texts)
    {
        SLANG_CHECK(text.indexOf("define void @float32Copy(float addrspace(1)*") >= 0);
        SLANG_CHECK(_countOccurrences(text.getUnownedSlice(), toSlice("float addrspace(1)*")) >= 4);
        SLANG_CHECK(_countOccurrences(text.getUnownedSlice(), toSlice("load float")) == 1);
        SLANG_CHECK(_countOccurrences(text.getUnownedSlice(), toSlice("store float")) == 1);
        SLANG_CHECK(_countOccurrences(text.getUnownedSlice(), toSlice("align 4")) == 2);
        SLANG_CHECK(text.indexOf("fadd float") < 0);
    }
    SLANG_CHECK(nvvmText.indexOf("!nvvmir.version") >= 0);
    SLANG_CHECK(nvvmText.indexOf("!\"kernel\", i32 1") >= 0);
}

SLANG_UNIT_TEST(nvvmIRBuilderRealProviderPreservesShortBuffers)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    SLANG_CHECK_ABORT(builder.supportsSerializationDiagnostics());

    ScopedNVVMBuilderModule scope;
    scope.builder = &builder;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.createModule(toSlice("real-short-buffer"), scope.module)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        _populateEmptyNVVMKernel(builder, scope.module, toSlice("realShortBufferKernel"))));

    size_t requiredLegacySize = 0;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getAPI().serializeModule(
        scope.module,
        SLANG_NVVM_SERIALIZATION_FORMAT_BITCODE,
        nullptr,
        0,
        &requiredLegacySize)));
    uint8_t legacySentinels[8];
    ::memset(legacySentinels, 0xa5, sizeof(legacySentinels));
    size_t reportedLegacySize = 0;
    SLANG_CHECK(requiredLegacySize > sizeof(legacySentinels));
    SLANG_CHECK(
        builder.getAPI().serializeModule(
            scope.module,
            SLANG_NVVM_SERIALIZATION_FORMAT_BITCODE,
            legacySentinels,
            sizeof(legacySentinels),
            &reportedLegacySize) == SLANG_E_BUFFER_TOO_SMALL);
    SLANG_CHECK(reportedLegacySize == requiredLegacySize);
    for (const auto value : legacySentinels)
        SLANG_CHECK(value == 0xa5);

    const SlangNVVMBuilderAPI_V2* v2API = builder.getAPIV2();
    SLANG_CHECK_ABORT(v2API != nullptr);
    SLANG_CHECK_ABORT(v2API->serializeModuleWithDiagnostics != nullptr);

    size_t requiredSerializedSize = 0;
    size_t requiredDiagnosticSize = 0;
    SlangNVVMVerificationStatus_2 verificationStatus = SLANG_NVVM_VERIFICATION_NOT_RUN;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(v2API->serializeModuleWithDiagnostics(
        scope.module,
        SLANG_NVVM_SERIALIZATION_FORMAT_BITCODE,
        nullptr,
        0,
        &requiredSerializedSize,
        nullptr,
        0,
        &requiredDiagnosticSize,
        &verificationStatus)));
    SLANG_CHECK(requiredSerializedSize == requiredLegacySize);
    SLANG_CHECK(requiredDiagnosticSize == 0);
    SLANG_CHECK(verificationStatus == SLANG_NVVM_VERIFICATION_VALID);

    // A query has both destinations null. Supplying even an otherwise-unneeded diagnostic
    // destination makes this a write, so omitting the non-empty serialized destination must fail
    // without touching the diagnostic sentinel.
    uint8_t mixedDiagnosticSentinel = 0x3c;
    size_t mixedSerializedSize = 0;
    size_t mixedDiagnosticSize = 1;
    verificationStatus = SLANG_NVVM_VERIFICATION_NOT_RUN;
    SLANG_CHECK(
        v2API->serializeModuleWithDiagnostics(
            scope.module,
            SLANG_NVVM_SERIALIZATION_FORMAT_BITCODE,
            nullptr,
            0,
            &mixedSerializedSize,
            &mixedDiagnosticSentinel,
            1,
            &mixedDiagnosticSize,
            &verificationStatus) == SLANG_E_BUFFER_TOO_SMALL);
    SLANG_CHECK(mixedSerializedSize == requiredSerializedSize);
    SLANG_CHECK(mixedDiagnosticSize == requiredDiagnosticSize);
    SLANG_CHECK(verificationStatus == SLANG_NVVM_VERIFICATION_VALID);
    SLANG_CHECK(mixedDiagnosticSentinel == 0x3c);

    uint8_t serializedSentinels[8];
    ::memset(serializedSentinels, 0x5a, sizeof(serializedSentinels));
    size_t reportedSerializedSize = 0;
    size_t reportedDiagnosticSize = 1;
    verificationStatus = SLANG_NVVM_VERIFICATION_NOT_RUN;
    SLANG_CHECK(requiredSerializedSize > sizeof(serializedSentinels));
    SLANG_CHECK(
        v2API->serializeModuleWithDiagnostics(
            scope.module,
            SLANG_NVVM_SERIALIZATION_FORMAT_BITCODE,
            serializedSentinels,
            sizeof(serializedSentinels),
            &reportedSerializedSize,
            nullptr,
            0,
            &reportedDiagnosticSize,
            &verificationStatus) == SLANG_E_BUFFER_TOO_SMALL);
    SLANG_CHECK(reportedSerializedSize == requiredSerializedSize);
    SLANG_CHECK(reportedDiagnosticSize == requiredDiagnosticSize);
    SLANG_CHECK(verificationStatus == SLANG_NVVM_VERIFICATION_VALID);
    for (const auto value : serializedSentinels)
        SLANG_CHECK(value == 0x5a);

    ComPtr<ISlangBlob> bitcode;
    String diagnostics = "stale diagnostics";
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(
        scope.module,
        SLANG_NVVM_SERIALIZATION_FORMAT_BITCODE,
        bitcode,
        diagnostics)));
    SLANG_CHECK_ABORT(bitcode != nullptr);
    SLANG_CHECK(bitcode->getBufferSize() == requiredSerializedSize);
    SLANG_CHECK(diagnostics.getLength() == 0);
}

// Exercise the module boundary's rejected input shapes with handles produced by the real LLVM 14
// implementation. These checks keep malformed LLVM IR from becoming a libNVVM diagnostic later.
SLANG_UNIT_TEST(nvvmIRBuilderRejectsInvalidOperations)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);

    ScopedNVVMBuilderModule firstModule;
    firstModule.builder = &builder;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.createModule(toSlice("first-module"), firstModule.module)));
    ScopedNVVMBuilderModule secondModule;
    secondModule.builder = &builder;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.createModule(toSlice("second-module"), secondModule.module)));

    SlangNVVMTypeHandle_1 firstVoidType = nullptr;
    SlangNVVMTypeHandle_1 secondVoidType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(firstModule.module, firstVoidType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(secondModule.module, secondVoidType)));

    SlangNVVMTypeHandle_1 invalidFunctionType = nullptr;
    SLANG_CHECK(
        builder.getFunctionType(
            firstModule.module,
            firstVoidType,
            &firstVoidType,
            1,
            invalidFunctionType) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(invalidFunctionType == nullptr);
    SLANG_CHECK(
        builder
            .getFunctionType(firstModule.module, secondVoidType, nullptr, 0, invalidFunctionType) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(invalidFunctionType == nullptr);

    SlangNVVMTypeHandle_1 firstFunctionType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionType(firstModule.module, firstVoidType, nullptr, 0, firstFunctionType)));
    SlangNVVMValueHandle_1 firstFunction = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
        firstModule.module,
        firstFunctionType,
        toSlice("uniqueKernel"),
        firstFunction)));

    SlangNVVMValueHandle_1 invalidFunction = nullptr;
    SLANG_CHECK(
        builder.declareFunction(
            firstModule.module,
            firstFunctionType,
            toSlice("uniqueKernel"),
            invalidFunction) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(invalidFunction == nullptr);
    SLANG_CHECK(
        builder.declareFunction(
            secondModule.module,
            firstFunctionType,
            toSlice("foreignType"),
            invalidFunction) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(invalidFunction == nullptr);

    SlangNVVMBlockHandle_1 invalidBlock = nullptr;
    SLANG_CHECK(
        builder.createBlock(
            secondModule.module,
            firstFunction,
            toSlice("foreignFunction"),
            invalidBlock) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(invalidBlock == nullptr);

    SlangNVVMBlockHandle_1 firstBlock = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(firstModule.module, firstFunction, toSlice("entry"), firstBlock)));
    SLANG_CHECK(builder.setInsertBlock(secondModule.module, firstBlock) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(
        builder.markFunctionAsKernel(secondModule.module, firstFunction) == SLANG_E_INVALID_ARG);

    const SlangNVVMSerializationFormat_1 unknownFormat =
        SlangNVVMSerializationFormat_1(SLANG_NVVM_SERIALIZATION_FORMAT_NVVM_IR_2_0_ASSEMBLY + 1);
    size_t legacyUnknownFormatSize = 1;
    SLANG_CHECK(
        builder.getAPI().serializeModule(
            firstModule.module,
            unknownFormat,
            nullptr,
            0,
            &legacyUnknownFormatSize) == SLANG_FAIL);
    SLANG_CHECK(legacyUnknownFormatSize == 1);

    const SlangNVVMBuilderAPI_V2* v2API = builder.getAPIV2();
    SLANG_CHECK_ABORT(v2API != nullptr);
    SLANG_CHECK_ABORT(v2API->serializeModuleWithDiagnostics != nullptr);

    size_t v2UnknownFormatSerializedSize = 1;
    size_t v2UnknownFormatDiagnosticSize = 1;
    SlangNVVMVerificationStatus_2 v2UnknownFormatStatus = SLANG_NVVM_VERIFICATION_VALID;
    SLANG_CHECK(
        v2API->serializeModuleWithDiagnostics(
            firstModule.module,
            unknownFormat,
            nullptr,
            0,
            &v2UnknownFormatSerializedSize,
            nullptr,
            0,
            &v2UnknownFormatDiagnosticSize,
            &v2UnknownFormatStatus) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(v2UnknownFormatSerializedSize == 0);
    SLANG_CHECK(v2UnknownFormatDiagnosticSize == 0);
    SLANG_CHECK(v2UnknownFormatStatus == SLANG_NVVM_VERIFICATION_NOT_RUN);

    size_t invalidSerializedSize = 1;
    size_t invalidDiagnosticSize = 0;
    SlangNVVMVerificationStatus_2 invalidStatus = SLANG_NVVM_VERIFICATION_NOT_RUN;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(v2API->serializeModuleWithDiagnostics(
        firstModule.module,
        SLANG_NVVM_SERIALIZATION_FORMAT_BITCODE,
        nullptr,
        0,
        &invalidSerializedSize,
        nullptr,
        0,
        &invalidDiagnosticSize,
        &invalidStatus)));
    SLANG_CHECK(invalidSerializedSize == 0);
    SLANG_CHECK(invalidDiagnosticSize > 0);
    SLANG_CHECK(invalidStatus == SLANG_NVVM_VERIFICATION_INVALID);

    uint8_t invalidSerializedSentinel = 0xa5;
    uint8_t invalidDiagnosticSentinel = 0x5a;
    size_t reportedInvalidSerializedSize = 1;
    size_t reportedInvalidDiagnosticSize = 0;
    invalidStatus = SLANG_NVVM_VERIFICATION_NOT_RUN;
    SLANG_CHECK(
        v2API->serializeModuleWithDiagnostics(
            firstModule.module,
            SLANG_NVVM_SERIALIZATION_FORMAT_BITCODE,
            &invalidSerializedSentinel,
            1,
            &reportedInvalidSerializedSize,
            &invalidDiagnosticSentinel,
            1,
            &reportedInvalidDiagnosticSize,
            &invalidStatus) == SLANG_E_BUFFER_TOO_SMALL);
    SLANG_CHECK(reportedInvalidSerializedSize == 0);
    SLANG_CHECK(reportedInvalidDiagnosticSize == invalidDiagnosticSize);
    SLANG_CHECK(invalidStatus == SLANG_NVVM_VERIFICATION_INVALID);
    SLANG_CHECK(invalidSerializedSentinel == 0xa5);
    SLANG_CHECK(invalidDiagnosticSentinel == 0x5a);

    ComPtr<ISlangBlob> invalidBitcode;
    SLANG_CHECK(
        builder.serializeModule(
            firstModule.module,
            SLANG_NVVM_SERIALIZATION_FORMAT_BITCODE,
            invalidBitcode) == SLANG_FAIL);
    SLANG_CHECK(invalidBitcode == nullptr);

    String verifierDiagnostics = "stale diagnostics";
    SLANG_CHECK(
        builder.serializeModule(
            firstModule.module,
            SLANG_NVVM_SERIALIZATION_FORMAT_BITCODE,
            invalidBitcode,
            verifierDiagnostics) == SLANG_FAIL);
    SLANG_CHECK(invalidBitcode == nullptr);
    SLANG_CHECK(verifierDiagnostics.indexOf("does not have terminator") >= 0);
    SLANG_CHECK(verifierDiagnostics.indexOf("uniqueKernel") >= 0);

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(firstModule.module, firstBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitReturnVoid(firstModule.module)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.markFunctionAsKernel(firstModule.module, firstFunction)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(
        firstModule.module,
        SLANG_NVVM_SERIALIZATION_FORMAT_BITCODE,
        invalidBitcode)));
    SLANG_CHECK(invalidBitcode != nullptr);

    invalidBitcode.setNull();
    verifierDiagnostics = "stale diagnostics";
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(
        firstModule.module,
        SLANG_NVVM_SERIALIZATION_FORMAT_BITCODE,
        invalidBitcode,
        verifierDiagnostics)));
    SLANG_CHECK(invalidBitcode != nullptr);
    SLANG_CHECK(verifierDiagnostics.getLength() == 0);
}

// Scalar-memory calls reject malformed module-owned shapes before they can insert LLVM IR.
SLANG_UNIT_TEST(nvvmIRBuilderRejectsInvalidScalarOperations)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    SLANG_CHECK_ABORT(builder.supportsScalarOperations());

    ScopedNVVMBuilderModule firstModule;
    firstModule.builder = &builder;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.createModule(toSlice("invalid-scalar-first"), firstModule.module)));
    ScopedNVVMBuilderModule secondModule;
    secondModule.builder = &builder;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createModule(toSlice("invalid-scalar-second"), secondModule.module)));

    SlangNVVMTypeHandle_1 firstVoidType = nullptr;
    SlangNVVMTypeHandle_1 firstIntegerType = nullptr;
    SlangNVVMTypeHandle_1 firstGlobalPointerType = nullptr;
    SlangNVVMTypeHandle_1 firstConstantPointerType = nullptr;
    SlangNVVMTypeHandle_1 secondVoidType = nullptr;
    SlangNVVMTypeHandle_1 secondIntegerType = nullptr;
    SlangNVVMTypeHandle_1 secondGlobalPointerType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(firstModule.module, firstVoidType)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getIntegerType(firstModule.module, 32, firstIntegerType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getPointerType(
        firstModule.module,
        firstIntegerType,
        SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
        firstGlobalPointerType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getPointerType(
        firstModule.module,
        firstIntegerType,
        SLANG_NVVM_ADDRESS_SPACE_CONSTANT,
        firstConstantPointerType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(secondModule.module, secondVoidType)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getIntegerType(secondModule.module, 32, secondIntegerType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getPointerType(
        secondModule.module,
        secondIntegerType,
        SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
        secondGlobalPointerType)));

    const SlangNVVMBuilderAPI_V2* scalarAPI = builder.getAPIV2();
    SLANG_CHECK_ABORT(scalarAPI != nullptr);
    SLANG_CHECK_ABORT(scalarAPI->getIntegerType != nullptr);
    SLANG_CHECK_ABORT(scalarAPI->getPointerType != nullptr);
    SLANG_CHECK_ABORT(scalarAPI->getFunctionParameter != nullptr);
    SLANG_CHECK_ABORT(scalarAPI->emitLoad != nullptr);
    SLANG_CHECK(scalarAPI->getIntegerType(firstModule.module, 32, nullptr) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(
        scalarAPI->getPointerType(
            firstModule.module,
            firstIntegerType,
            SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
            nullptr) == SLANG_E_INVALID_ARG);

    SlangNVVMTypeHandle_1 rejectedType = firstVoidType;
    SLANG_CHECK(builder.getIntegerType(firstModule.module, 0, rejectedType) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedType == nullptr);
    static const uint32_t kMaximumIntegerBitWidth = 1u << 23;
    SlangNVVMTypeHandle_1 maximumIntegerType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getIntegerType(firstModule.module, kMaximumIntegerBitWidth, maximumIntegerType)));
    SLANG_CHECK(maximumIntegerType != nullptr);
    rejectedType = firstVoidType;
    SLANG_CHECK(
        builder.getIntegerType(firstModule.module, kMaximumIntegerBitWidth + 1, rejectedType) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedType == nullptr);
    rejectedType = firstVoidType;
    SLANG_CHECK(
        builder.getPointerType(
            firstModule.module,
            firstVoidType,
            SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
            rejectedType) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedType == nullptr);
    rejectedType = firstVoidType;
    SLANG_CHECK(
        builder.getPointerType(
            firstModule.module,
            firstIntegerType,
            SlangNVVMAddressSpace_2(2),
            rejectedType) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedType == nullptr);
    rejectedType = firstVoidType;
    SLANG_CHECK(
        builder.getPointerType(
            firstModule.module,
            secondIntegerType,
            SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
            rejectedType) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedType == nullptr);

    const SlangNVVMTypeHandle_1 firstParameterTypes[] = {
        firstGlobalPointerType,
        firstIntegerType,
        firstConstantPointerType,
    };
    SlangNVVMTypeHandle_1 firstFunctionType = nullptr;
    SlangNVVMValueHandle_1 firstFunction = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionType(
        firstModule.module,
        firstVoidType,
        firstParameterTypes,
        SLANG_COUNT_OF(firstParameterTypes),
        firstFunctionType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
        firstModule.module,
        firstFunctionType,
        toSlice("rejectInvalidScalarOperations"),
        firstFunction)));

    SlangNVVMValueHandle_1 firstDestination = nullptr;
    SlangNVVMValueHandle_1 firstValue = nullptr;
    SlangNVVMValueHandle_1 firstConstantDestination = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionParameter(firstModule.module, firstFunction, 0, firstDestination)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionParameter(firstModule.module, firstFunction, 1, firstValue)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder
            .getFunctionParameter(firstModule.module, firstFunction, 2, firstConstantDestination)));
    SLANG_CHECK(
        scalarAPI->getFunctionParameter(firstModule.module, firstFunction, 0, nullptr) ==
        SLANG_E_INVALID_ARG);

    const SlangNVVMTypeHandle_1 secondParameterTypes[] = {
        secondGlobalPointerType,
        secondIntegerType,
    };
    SlangNVVMTypeHandle_1 secondFunctionType = nullptr;
    SlangNVVMValueHandle_1 secondFunction = nullptr;
    SlangNVVMValueHandle_1 secondDestination = nullptr;
    SlangNVVMValueHandle_1 secondValue = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionType(
        secondModule.module,
        secondVoidType,
        secondParameterTypes,
        SLANG_COUNT_OF(secondParameterTypes),
        secondFunctionType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
        secondModule.module,
        secondFunctionType,
        toSlice("foreignScalarFunction"),
        secondFunction)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionParameter(secondModule.module, secondFunction, 0, secondDestination)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionParameter(secondModule.module, secondFunction, 1, secondValue)));

    SlangNVVMValueHandle_1 rejectedValue = firstFunction;
    SLANG_CHECK(
        builder.getFunctionParameter(
            firstModule.module,
            firstFunction,
            SLANG_COUNT_OF(firstParameterTypes),
            rejectedValue) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    rejectedValue = firstFunction;
    SLANG_CHECK(
        builder.getFunctionParameter(firstModule.module, secondFunction, 0, rejectedValue) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);

    // A declared function has no insertion block yet. Both operations must fail without mutation.
    rejectedValue = firstFunction;
    SLANG_CHECK(
        builder.emitLoad(firstModule.module, firstDestination, 4, rejectedValue) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    SLANG_CHECK(
        builder.emitStore(firstModule.module, firstValue, firstDestination, 4) ==
        SLANG_E_INVALID_ARG);

    SlangNVVMBlockHandle_1 firstBlock = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(firstModule.module, firstFunction, toSlice("entry"), firstBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(firstModule.module, firstBlock)));
    SLANG_CHECK(
        scalarAPI->emitLoad(firstModule.module, firstDestination, 4, nullptr) ==
        SLANG_E_INVALID_ARG);

    rejectedValue = firstFunction;
    SLANG_CHECK(
        builder.emitLoad(firstModule.module, firstValue, 4, rejectedValue) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    rejectedValue = firstFunction;
    SLANG_CHECK(
        builder.emitLoad(firstModule.module, secondDestination, 4, rejectedValue) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    static const uint32_t kInvalidAlignments[] = {0u, 3u};
    for (uint32_t invalidAlignment : kInvalidAlignments)
    {
        rejectedValue = firstFunction;
        SLANG_CHECK(
            builder
                .emitLoad(firstModule.module, firstDestination, invalidAlignment, rejectedValue) ==
            SLANG_E_INVALID_ARG);
        SLANG_CHECK(rejectedValue == nullptr);
        SLANG_CHECK(
            builder.emitStore(firstModule.module, firstValue, firstDestination, invalidAlignment) ==
            SLANG_E_INVALID_ARG);
    }
    SLANG_CHECK(
        builder.emitStore(firstModule.module, firstDestination, firstDestination, 4) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(
        builder.emitStore(firstModule.module, firstValue, firstValue, 4) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(
        builder.emitStore(firstModule.module, secondValue, firstDestination, 4) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(
        builder.emitStore(firstModule.module, firstValue, secondDestination, 4) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(
        builder.emitStore(firstModule.module, firstValue, firstConstantDestination, 4) ==
        SLANG_E_INVALID_ARG);

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitReturnVoid(firstModule.module)));
    rejectedValue = firstFunction;
    SLANG_CHECK(
        builder.emitLoad(firstModule.module, firstDestination, 4, rejectedValue) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    SLANG_CHECK(
        builder.emitStore(firstModule.module, firstValue, firstDestination, 4) ==
        SLANG_E_INVALID_ARG);

    ComPtr<ISlangBlob> assemblyBlob;
    String diagnostics = "stale diagnostics";
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(
        firstModule.module,
        SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
        assemblyBlob,
        diagnostics)));
    SLANG_CHECK_ABORT(assemblyBlob != nullptr);
    SLANG_CHECK(diagnostics.getLength() == 0);
    const String assembly = _getBlobText(assemblyBlob);
    SLANG_CHECK(assembly.indexOf(" = load ") < 0);
    SLANG_CHECK(assembly.indexOf("\n  store ") < 0);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("ret void")) == 1);
}

SLANG_UNIT_TEST(nvvmIRBuilderRejectsInvalidScalarControlOperations)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    SLANG_CHECK_ABORT(builder.supportsScalarControlFlow());

    ScopedNVVMBuilderModule firstModule;
    firstModule.builder = &builder;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createModule(toSlice("invalid-control-first"), firstModule.module)));
    ScopedNVVMBuilderModule foreignModule;
    foreignModule.builder = &builder;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createModule(toSlice("invalid-control-foreign"), foreignModule.module)));

    SlangNVVMTypeHandle_1 voidType = nullptr;
    SlangNVVMTypeHandle_1 integerType = nullptr;
    SlangNVVMTypeHandle_1 pointerType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(firstModule.module, voidType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getIntegerType(firstModule.module, 32, integerType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getPointerType(
        firstModule.module,
        integerType,
        SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
        pointerType)));
    const SlangNVVMTypeHandle_1 parameterTypes[] = {pointerType, integerType, integerType};
    SlangNVVMTypeHandle_1 functionType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionType(
        firstModule.module,
        voidType,
        parameterTypes,
        SLANG_COUNT_OF(parameterTypes),
        functionType)));

    auto declareFunction = [&](const char* name, SlangNVVMValueHandle_1& outFunction)
    {
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
            firstModule.module,
            functionType,
            UnownedStringSlice(name),
            outFunction)));
    };
    SlangNVVMValueHandle_1 firstFunction = nullptr;
    SlangNVVMValueHandle_1 secondFunction = nullptr;
    declareFunction("firstControlFunction", firstFunction);
    declareFunction("secondControlFunction", secondFunction);

    SlangNVVMValueHandle_1 firstDestination = nullptr;
    SlangNVVMValueHandle_1 firstX = nullptr;
    SlangNVVMValueHandle_1 firstY = nullptr;
    SlangNVVMValueHandle_1 secondDestination = nullptr;
    SlangNVVMValueHandle_1 secondX = nullptr;
    SlangNVVMValueHandle_1 secondY = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionParameter(firstModule.module, firstFunction, 0, firstDestination)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionParameter(firstModule.module, firstFunction, 1, firstX)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionParameter(firstModule.module, firstFunction, 2, firstY)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionParameter(firstModule.module, secondFunction, 0, secondDestination)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionParameter(firstModule.module, secondFunction, 1, secondX)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionParameter(firstModule.module, secondFunction, 2, secondY)));

    SlangNVVMTypeHandle_1 foreignVoidType = nullptr;
    SlangNVVMTypeHandle_1 foreignIntegerType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(foreignModule.module, foreignVoidType)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getIntegerType(foreignModule.module, 32, foreignIntegerType)));
    const SlangNVVMTypeHandle_1 foreignParameterTypes[] = {
        foreignIntegerType,
        foreignIntegerType,
    };
    SlangNVVMTypeHandle_1 foreignFunctionType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionType(
        foreignModule.module,
        foreignVoidType,
        foreignParameterTypes,
        SLANG_COUNT_OF(foreignParameterTypes),
        foreignFunctionType)));
    SlangNVVMValueHandle_1 foreignFunction = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
        foreignModule.module,
        foreignFunctionType,
        toSlice("foreignControlFunction"),
        foreignFunction)));
    SlangNVVMValueHandle_1 foreignX = nullptr;
    SlangNVVMValueHandle_1 foreignY = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionParameter(foreignModule.module, foreignFunction, 0, foreignX)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionParameter(foreignModule.module, foreignFunction, 1, foreignY)));
    SlangNVVMBlockHandle_1 foreignBlock = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.createBlock(
        foreignModule.module,
        foreignFunction,
        toSlice("foreign-entry"),
        foreignBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(foreignModule.module, foreignBlock)));
    SlangNVVMValueHandle_1 foreignCondition = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitIntegerSignedLessThan(
        foreignModule.module,
        foreignX,
        foreignY,
        foreignCondition)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitReturnVoid(foreignModule.module)));

    SlangNVVMBlockHandle_1 entryBlock = nullptr;
    SlangNVVMBlockHandle_1 trueBlock = nullptr;
    SlangNVVMBlockHandle_1 falseBlock = nullptr;
    SlangNVVMBlockHandle_1 mergeBlock = nullptr;
    SlangNVVMBlockHandle_1 secondBlock = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(firstModule.module, firstFunction, toSlice("entry"), entryBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(firstModule.module, firstFunction, toSlice("true"), trueBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(firstModule.module, firstFunction, toSlice("false"), falseBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(firstModule.module, firstFunction, toSlice("merge"), mergeBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.createBlock(
        firstModule.module,
        secondFunction,
        toSlice("second-entry"),
        secondBlock)));

    SlangNVVMValueHandle_1 rejectedValue = firstFunction;
    SLANG_CHECK(
        builder.emitIntegerBinary(
            firstModule.module,
            SLANG_NVVM_INTEGER_BINARY_OP_ADD,
            firstX,
            firstY,
            rejectedValue) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    SLANG_CHECK(builder.emitBranch(firstModule.module, mergeBlock) == SLANG_E_INVALID_ARG);

    // Produce a live i1 in a second function, then prove that values and blocks from that function
    // cannot be consumed at the first function's insertion point.
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(firstModule.module, secondBlock)));
    SlangNVVMValueHandle_1 secondCondition = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.emitIntegerSignedLessThan(firstModule.module, secondX, secondY, secondCondition)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitReturnVoid(firstModule.module)));

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(firstModule.module, entryBlock)));
    const SlangNVVMBuilderAPI_V2* controlAPI = builder.getAPIV2();
    SLANG_CHECK_ABORT(controlAPI != nullptr);
    SLANG_CHECK(
        controlAPI->emitIntegerBinary(
            firstModule.module,
            SLANG_NVVM_INTEGER_BINARY_OP_ADD,
            firstX,
            firstY,
            nullptr) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(
        controlAPI->emitIntegerSignedLessThan(firstModule.module, firstX, firstY, nullptr) ==
        SLANG_E_INVALID_ARG);

    // Context ownership is stricter than function ownership: values, conditions, and blocks from
    // another provider module must be rejected before any first-module instruction is created.
    rejectedValue = firstFunction;
    SLANG_CHECK(
        builder.emitIntegerBinary(
            firstModule.module,
            SLANG_NVVM_INTEGER_BINARY_OP_ADD,
            firstX,
            foreignX,
            rejectedValue) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    rejectedValue = firstFunction;
    SLANG_CHECK(
        builder.emitIntegerSignedLessThan(firstModule.module, firstY, foreignY, rejectedValue) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    SLANG_CHECK(
        builder
            .emitConditionalBranch(firstModule.module, foreignCondition, trueBlock, falseBlock) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(builder.emitBranch(firstModule.module, foreignBlock) == SLANG_E_INVALID_ARG);

    rejectedValue = firstFunction;
    SLANG_CHECK(
        builder.emitIntegerBinary(
            firstModule.module,
            SlangNVVMIntegerBinaryOp_2(SLANG_NVVM_INTEGER_BINARY_OP_SUB + 1),
            firstX,
            firstY,
            rejectedValue) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    rejectedValue = firstFunction;
    SLANG_CHECK(
        builder.emitIntegerBinary(
            firstModule.module,
            SLANG_NVVM_INTEGER_BINARY_OP_ADD,
            firstX,
            firstDestination,
            rejectedValue) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    rejectedValue = firstFunction;
    SLANG_CHECK(
        builder.emitIntegerBinary(
            firstModule.module,
            SLANG_NVVM_INTEGER_BINARY_OP_ADD,
            firstX,
            secondX,
            rejectedValue) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    rejectedValue = firstFunction;
    SLANG_CHECK(
        builder.emitIntegerSignedLessThan(firstModule.module, firstX, secondY, rejectedValue) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    SLANG_CHECK(
        builder.emitStore(firstModule.module, secondX, firstDestination, 4) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(
        builder.emitLoad(firstModule.module, secondDestination, 4, rejectedValue) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    SLANG_CHECK(
        builder.emitConditionalBranch(firstModule.module, firstX, trueBlock, falseBlock) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(
        builder.emitConditionalBranch(firstModule.module, secondCondition, trueBlock, falseBlock) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(builder.emitBranch(firstModule.module, secondBlock) == SLANG_E_INVALID_ARG);

    // Every rejected call above must leave the entry block untouched, so one valid graph still
    // verifies and contains exactly the instructions deliberately emitted below.
    SlangNVVMValueHandle_1 condition = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.emitIntegerSignedLessThan(firstModule.module, firstX, firstY, condition)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.emitConditionalBranch(firstModule.module, condition, trueBlock, falseBlock)));

    rejectedValue = firstFunction;
    SLANG_CHECK(
        builder.emitIntegerBinary(
            firstModule.module,
            SLANG_NVVM_INTEGER_BINARY_OP_ADD,
            firstX,
            firstY,
            rejectedValue) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    SLANG_CHECK(builder.emitBranch(firstModule.module, mergeBlock) == SLANG_E_INVALID_ARG);

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(firstModule.module, trueBlock)));
    SlangNVVMValueHandle_1 sum = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitIntegerBinary(
        firstModule.module,
        SLANG_NVVM_INTEGER_BINARY_OP_ADD,
        firstX,
        firstY,
        sum)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.emitStore(firstModule.module, sum, firstDestination, 4)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitBranch(firstModule.module, mergeBlock)));

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(firstModule.module, falseBlock)));
    // `sum` belongs to the sibling true block and does not dominate this insertion point. Both
    // instruction-producing and side-effecting consumers must reject it without changing the
    // false block.
    rejectedValue = firstFunction;
    SLANG_CHECK(
        builder.emitIntegerBinary(
            firstModule.module,
            SLANG_NVVM_INTEGER_BINARY_OP_ADD,
            sum,
            firstX,
            rejectedValue) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    SLANG_CHECK(
        builder.emitStore(firstModule.module, sum, firstDestination, 4) == SLANG_E_INVALID_ARG);

    SlangNVVMValueHandle_1 difference = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitIntegerBinary(
        firstModule.module,
        SLANG_NVVM_INTEGER_BINARY_OP_SUB,
        firstX,
        firstY,
        difference)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.emitStore(firstModule.module, difference, firstDestination, 4)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitBranch(firstModule.module, mergeBlock)));

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(firstModule.module, mergeBlock)));
    // The merge is reachable without executing the true block as well, so the same value remains
    // unavailable here. The final assembly counts below prove these failures added no instructions.
    rejectedValue = firstFunction;
    SLANG_CHECK(
        builder.emitIntegerBinary(
            firstModule.module,
            SLANG_NVVM_INTEGER_BINARY_OP_SUB,
            sum,
            firstY,
            rejectedValue) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    SLANG_CHECK(
        builder.emitStore(firstModule.module, sum, firstDestination, 4) == SLANG_E_INVALID_ARG);
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitReturnVoid(firstModule.module)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.markFunctionAsKernel(firstModule.module, firstFunction)));

    ComPtr<ISlangBlob> assemblyBlob;
    String diagnostics;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(
        firstModule.module,
        SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
        assemblyBlob,
        diagnostics)));
    SLANG_CHECK_ABORT(assemblyBlob != nullptr);
    SLANG_CHECK(diagnostics.getLength() == 0);
    const String assembly = _getBlobText(assemblyBlob);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("icmp slt i32")) == 2);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("add i32")) == 1);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("sub i32")) == 1);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("store i32")) == 2);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("br i1")) == 1);
}

SLANG_UNIT_TEST(nvvmIRBuilderRejectsInvalidScalarSSAOperations)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    SLANG_CHECK_ABORT(builder.supportsScalarSSA());

    ScopedNVVMBuilderModule module;
    module.builder = &builder;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.createModule(toSlice("invalid-scalar-ssa"), module.module)));
    ScopedNVVMBuilderModule foreignModule;
    foreignModule.builder = &builder;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createModule(toSlice("invalid-scalar-ssa-foreign"), foreignModule.module)));

    SlangNVVMTypeHandle_1 voidType = nullptr;
    SlangNVVMTypeHandle_1 integerType = nullptr;
    SlangNVVMTypeHandle_1 pointerType = nullptr;
    SlangNVVMTypeHandle_1 foreignIntegerType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(module.module, voidType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getIntegerType(module.module, 32, integerType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getPointerType(
        module.module,
        integerType,
        SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
        pointerType)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getIntegerType(foreignModule.module, 32, foreignIntegerType)));

    const SlangNVVMBuilderAPI_V2* ssaAPI = builder.getAPIV2();
    SLANG_CHECK_ABORT(ssaAPI != nullptr);
    SLANG_CHECK(
        ssaAPI->getIntegerConstant(module.module, integerType, 0, nullptr) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(
        ssaAPI->emitIntegerPhi(module.module, nullptr, integerType, nullptr) ==
        SLANG_E_INVALID_ARG);

    SlangNVVMValueHandle_1 rejectedValue = reinterpret_cast<SlangNVVMValueHandle_1>(uintptr_t(1));
    SLANG_CHECK(
        builder.getIntegerConstant(module.module, voidType, 1, rejectedValue) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    rejectedValue = reinterpret_cast<SlangNVVMValueHandle_1>(uintptr_t(1));
    SLANG_CHECK(
        builder.getIntegerConstant(module.module, foreignIntegerType, 1, rejectedValue) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    static const int64_t kOutOfI32Range[] = {INT64_C(2147483648), -INT64_C(2147483649)};
    for (int64_t value : kOutOfI32Range)
    {
        rejectedValue = reinterpret_cast<SlangNVVMValueHandle_1>(uintptr_t(1));
        SLANG_CHECK(
            builder.getIntegerConstant(module.module, integerType, value, rejectedValue) ==
            SLANG_E_INVALID_ARG);
        SLANG_CHECK(rejectedValue == nullptr);
    }
    SlangNVVMValueHandle_1 minimum = nullptr;
    SlangNVVMValueHandle_1 maximum = nullptr;
    SlangNVVMValueHandle_1 foreignOne = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getIntegerConstant(module.module, integerType, -INT64_C(2147483647) - 1, minimum)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getIntegerConstant(module.module, integerType, INT64_C(2147483647), maximum)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getIntegerConstant(foreignModule.module, foreignIntegerType, 1, foreignOne)));

    const SlangNVVMTypeHandle_1 parameterTypes[] = {pointerType, integerType, integerType};
    SlangNVVMTypeHandle_1 functionType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionType(
        module.module,
        voidType,
        parameterTypes,
        SLANG_COUNT_OF(parameterTypes),
        functionType)));
    SlangNVVMValueHandle_1 function = nullptr;
    SlangNVVMValueHandle_1 secondFunction = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.declareFunction(module.module, functionType, toSlice("latePhiKernel"), function)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
        module.module,
        functionType,
        toSlice("sameModuleForeignFunction"),
        secondFunction)));

    SlangNVVMValueHandle_1 destination = nullptr;
    SlangNVVMValueHandle_1 x = nullptr;
    SlangNVVMValueHandle_1 y = nullptr;
    SlangNVVMValueHandle_1 secondX = nullptr;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, function, 0, destination)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, function, 1, x)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, function, 2, y)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, secondFunction, 1, secondX)));

    SlangNVVMBlockHandle_1 entryBlock = nullptr;
    SlangNVVMBlockHandle_1 trueBlock = nullptr;
    SlangNVVMBlockHandle_1 falseBlock = nullptr;
    SlangNVVMBlockHandle_1 mergeBlock = nullptr;
    SlangNVVMBlockHandle_1 orphanBlock = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, function, toSlice("entry"), entryBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, function, toSlice("if.true"), trueBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, function, toSlice("if.false"), falseBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, function, toSlice("if.merge"), mergeBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, function, toSlice("orphan"), orphanBlock)));

    rejectedValue = reinterpret_cast<SlangNVVMValueHandle_1>(uintptr_t(1));
    SLANG_CHECK(
        builder.emitIntegerPhi(foreignModule.module, mergeBlock, integerType, rejectedValue) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    rejectedValue = reinterpret_cast<SlangNVVMValueHandle_1>(uintptr_t(1));
    SLANG_CHECK(
        builder.emitIntegerPhi(module.module, mergeBlock, voidType, rejectedValue) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, entryBlock)));
    SlangNVVMValueHandle_1 condition = nullptr;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.emitIntegerSignedLessThan(module.module, x, y, condition)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.emitConditionalBranch(module.module, condition, trueBlock, falseBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, trueBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitBranch(module.module, mergeBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, falseBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitBranch(module.module, mergeBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, orphanBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitReturnVoid(module.module)));

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, mergeBlock)));
    SlangNVVMValueHandle_1 sum = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.emitIntegerBinary(module.module, SLANG_NVVM_INTEGER_BINARY_OP_ADD, x, y, sum)));
    SlangNVVMValueHandle_1 phi = nullptr;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.emitIntegerPhi(module.module, mergeBlock, integerType, phi)));
    // Incoming validation requires the complete CFG; merge has no terminator yet.
    SLANG_CHECK(
        builder.addIntegerPhiIncoming(module.module, phi, x, trueBlock) == SLANG_E_INVALID_ARG);
    // The explicit target permits late phi insertion and must preserve the current insertion state.
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitStore(module.module, phi, destination, 4)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitReturnVoid(module.module)));

    // All blocks in the phi function are now terminated. Invalid incoming calls must not mutate it.
    SLANG_CHECK(
        builder.addIntegerPhiIncoming(module.module, phi, condition, trueBlock) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(
        builder.addIntegerPhiIncoming(module.module, phi, secondX, trueBlock) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(
        builder.addIntegerPhiIncoming(module.module, phi, foreignOne, trueBlock) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(
        builder.addIntegerPhiIncoming(module.module, phi, x, orphanBlock) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(
        builder.addIntegerPhiIncoming(module.module, phi, sum, trueBlock) == SLANG_E_INVALID_ARG);
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.addIntegerPhiIncoming(module.module, phi, x, trueBlock)));
    SLANG_CHECK(
        builder.addIntegerPhiIncoming(module.module, phi, y, trueBlock) == SLANG_E_INVALID_ARG);
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.addIntegerPhiIncoming(module.module, phi, y, falseBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.markFunctionAsKernel(module.module, function)));

    ComPtr<ISlangBlob> assemblyBlob;
    String diagnostics;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(
        module.module,
        SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
        assemblyBlob,
        diagnostics)));
    SLANG_CHECK_ABORT(assemblyBlob != nullptr);
    SLANG_CHECK(diagnostics.getLength() == 0);
    const String assembly = _getBlobText(assemblyBlob);
    const Index phiIndex = assembly.indexOf("phi i32");
    const Index addIndex = assembly.indexOf("add i32");
    const Index storeIndex = assembly.indexOf("store i32");
    SLANG_CHECK_ABORT(phiIndex >= 0);
    SLANG_CHECK(addIndex > phiIndex);
    SLANG_CHECK(storeIndex > addIndex);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("phi i32")) == 1);
    UnownedStringSlice phiLine = assembly.getUnownedSlice().tail(phiIndex);
    const Index phiLineEnd = phiLine.indexOf(toSlice("\n"));
    if (phiLineEnd >= 0)
        phiLine = phiLine.head(phiLineEnd);
    SLANG_CHECK(_countOccurrences(phiLine, toSlice("[")) == 2);
    SLANG_CHECK(phiLine.indexOf(toSlice("%if.true")) >= 0);
    SLANG_CHECK(phiLine.indexOf(toSlice("%if.false")) >= 0);
}

SLANG_UNIT_TEST(nvvmIRBuilderRejectsInvalidScalarFunctionOperations)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    SLANG_CHECK_ABORT(builder.supportsScalarFunctions());

    ScopedNVVMBuilderModule module;
    module.builder = &builder;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.createModule(toSlice("invalid-scalar-function"), module.module)));
    ScopedNVVMBuilderModule foreignModule;
    foreignModule.builder = &builder;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createModule(toSlice("invalid-scalar-function-foreign"), foreignModule.module)));

    SlangNVVMTypeHandle_1 voidType = nullptr;
    SlangNVVMTypeHandle_1 integerType = nullptr;
    SlangNVVMTypeHandle_1 foreignIntegerType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(module.module, voidType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getIntegerType(module.module, 32, integerType)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getIntegerType(foreignModule.module, 32, foreignIntegerType)));

    SlangNVVMTypeHandle_1 helperType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionType(module.module, integerType, &integerType, 1, helperType)));
    SlangNVVMValueHandle_1 helper = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.declareFunction(module.module, helperType, toSlice("invalidCallHelper"), helper)));
    SlangNVVMValueHandle_1 helperValue = nullptr;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, helper, 0, helperValue)));

    const SlangNVVMTypeHandle_1 callerParameterTypes[] = {integerType, integerType};
    SlangNVVMTypeHandle_1 callerType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionType(
        module.module,
        integerType,
        callerParameterTypes,
        SLANG_COUNT_OF(callerParameterTypes),
        callerType)));
    SlangNVVMValueHandle_1 caller = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.declareFunction(module.module, callerType, toSlice("invalidCallCaller"), caller)));
    SlangNVVMValueHandle_1 x = nullptr;
    SlangNVVMValueHandle_1 y = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, caller, 0, x)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, caller, 1, y)));

    SlangNVVMTypeHandle_1 voidFunctionType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionType(module.module, voidType, &integerType, 1, voidFunctionType)));
    SlangNVVMValueHandle_1 voidFunction = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
        module.module,
        voidFunctionType,
        toSlice("invalidCallVoid"),
        voidFunction)));
    SlangNVVMValueHandle_1 voidFunctionValue = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionParameter(module.module, voidFunction, 0, voidFunctionValue)));

    SlangNVVMTypeHandle_1 foreignHelperType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionType(
        foreignModule.module,
        foreignIntegerType,
        &foreignIntegerType,
        1,
        foreignHelperType)));
    SlangNVVMValueHandle_1 foreignHelper = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
        foreignModule.module,
        foreignHelperType,
        toSlice("foreignCallHelper"),
        foreignHelper)));
    SlangNVVMValueHandle_1 foreignValue = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionParameter(foreignModule.module, foreignHelper, 0, foreignValue)));

    // This module has no insertion block yet. Both operations must reject without creating an
    // instruction or selecting function ownership implicitly.
    const SlangNVVMValueHandle_1 noInsertionArguments[] = {x};
    SlangNVVMValueHandle_1 noInsertionResult =
        reinterpret_cast<SlangNVVMValueHandle_1>(uintptr_t(1));
    SLANG_CHECK(
        builder.emitIntegerCall(
            module.module,
            helper,
            noInsertionArguments,
            SLANG_COUNT_OF(noInsertionArguments),
            noInsertionResult) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(noInsertionResult == nullptr);
    SLANG_CHECK(builder.emitIntegerReturn(module.module, x) == SLANG_E_INVALID_ARG);

    SlangNVVMBlockHandle_1 helperBlock = nullptr;
    SlangNVVMBlockHandle_1 callerEntry = nullptr;
    SlangNVVMBlockHandle_1 callerOther = nullptr;
    SlangNVVMBlockHandle_1 voidBlock = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, helper, toSlice("helper.entry"), helperBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, caller, toSlice("caller.entry"), callerEntry)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, caller, toSlice("caller.other"), callerOther)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, voidFunction, toSlice("void.entry"), voidBlock)));

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, helperBlock)));
    SlangNVVMValueHandle_1 one = nullptr;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getIntegerConstant(module.module, integerType, 1, one)));
    SlangNVVMValueHandle_1 helperResult = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitIntegerBinary(
        module.module,
        SLANG_NVVM_INTEGER_BINARY_OP_ADD,
        helperValue,
        one,
        helperResult)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitIntegerReturn(module.module, helperResult)));

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, callerOther)));
    SlangNVVMValueHandle_1 nonDominatingValue = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitIntegerBinary(
        module.module,
        SLANG_NVVM_INTEGER_BINARY_OP_ADD,
        x,
        y,
        nonDominatingValue)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.emitIntegerReturn(module.module, nonDominatingValue)));

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, voidBlock)));
    SLANG_CHECK(builder.emitIntegerReturn(module.module, voidFunctionValue) == SLANG_E_INVALID_ARG);
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitReturnVoid(module.module)));

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, callerEntry)));
    SlangNVVMValueHandle_1 condition = nullptr;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.emitIntegerSignedLessThan(module.module, x, y, condition)));

    const SlangNVVMValueHandle_1 xArgument[] = {x};
    const SlangNVVMValueHandle_1 conditionArgument[] = {condition};
    const SlangNVVMValueHandle_1 helperArgument[] = {helperValue};
    const SlangNVVMValueHandle_1 foreignArgument[] = {foreignValue};
    const SlangNVVMValueHandle_1 nonDominatingArgument[] = {nonDominatingValue};
    const SlangNVVMValueHandle_1 tooManyArguments[] = {x, y};
    SlangNVVMValueHandle_1 rejectedValue = reinterpret_cast<SlangNVVMValueHandle_1>(uintptr_t(1));
    SLANG_CHECK(
        builder.getAPIV2()->emitIntegerCall(module.module, helper, xArgument, 1, nullptr) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(
        builder.emitIntegerCall(module.module, x, xArgument, 1, rejectedValue) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    rejectedValue = reinterpret_cast<SlangNVVMValueHandle_1>(uintptr_t(1));
    SLANG_CHECK(
        builder.emitIntegerCall(module.module, foreignHelper, xArgument, 1, rejectedValue) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    rejectedValue = reinterpret_cast<SlangNVVMValueHandle_1>(uintptr_t(1));
    SLANG_CHECK(
        builder.emitIntegerCall(module.module, voidFunction, xArgument, 1, rejectedValue) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    rejectedValue = reinterpret_cast<SlangNVVMValueHandle_1>(uintptr_t(1));
    SLANG_CHECK(
        builder.emitIntegerCall(module.module, helper, nullptr, 1, rejectedValue) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    rejectedValue = reinterpret_cast<SlangNVVMValueHandle_1>(uintptr_t(1));
    SLANG_CHECK(
        builder.emitIntegerCall(module.module, helper, nullptr, 0, rejectedValue) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    rejectedValue = reinterpret_cast<SlangNVVMValueHandle_1>(uintptr_t(1));
    SLANG_CHECK(
        builder.emitIntegerCall(
            module.module,
            helper,
            tooManyArguments,
            SLANG_COUNT_OF(tooManyArguments),
            rejectedValue) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    rejectedValue = reinterpret_cast<SlangNVVMValueHandle_1>(uintptr_t(1));
    SLANG_CHECK(
        builder.emitIntegerCall(module.module, helper, conditionArgument, 1, rejectedValue) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    rejectedValue = reinterpret_cast<SlangNVVMValueHandle_1>(uintptr_t(1));
    SLANG_CHECK(
        builder.emitIntegerCall(module.module, helper, helperArgument, 1, rejectedValue) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    rejectedValue = reinterpret_cast<SlangNVVMValueHandle_1>(uintptr_t(1));
    SLANG_CHECK(
        builder.emitIntegerCall(module.module, helper, foreignArgument, 1, rejectedValue) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    rejectedValue = reinterpret_cast<SlangNVVMValueHandle_1>(uintptr_t(1));
    SLANG_CHECK(
        builder.emitIntegerCall(module.module, helper, nonDominatingArgument, 1, rejectedValue) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);

    // Invalid valued returns must likewise leave caller.entry unterminated for the valid graph.
    SLANG_CHECK(builder.emitIntegerReturn(module.module, condition) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(builder.emitIntegerReturn(module.module, helperValue) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(builder.emitIntegerReturn(module.module, foreignValue) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(
        builder.emitIntegerReturn(module.module, nonDominatingValue) == SLANG_E_INVALID_ARG);

    SlangNVVMValueHandle_1 callResult = nullptr;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.emitIntegerCall(module.module, helper, xArgument, 1, callResult)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitIntegerReturn(module.module, callResult)));
    rejectedValue = reinterpret_cast<SlangNVVMValueHandle_1>(uintptr_t(1));
    SLANG_CHECK(
        builder.emitIntegerCall(module.module, helper, xArgument, 1, rejectedValue) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    SLANG_CHECK(builder.emitIntegerReturn(module.module, x) == SLANG_E_INVALID_ARG);

    ComPtr<ISlangBlob> assemblyBlob;
    String diagnostics;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(
        module.module,
        SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
        assemblyBlob,
        diagnostics)));
    SLANG_CHECK_ABORT(assemblyBlob != nullptr);
    SLANG_CHECK(diagnostics.getLength() == 0);
    const String assembly = _getBlobText(assemblyBlob);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("call i32")) == 1);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("ret i32")) == 3);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("ret void")) == 1);
}

SLANG_UNIT_TEST(nvvmIRBuilderRejectsInvalidPointerOffsetOperations)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    SLANG_CHECK_ABORT(builder.supportsScalarPointerArithmetic());

    ScopedNVVMBuilderModule module;
    module.builder = &builder;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.createModule(toSlice("invalid-pointer-offset"), module.module)));
    ScopedNVVMBuilderModule foreignModule;
    foreignModule.builder = &builder;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createModule(toSlice("invalid-pointer-offset-foreign"), foreignModule.module)));

    SlangNVVMTypeHandle_1 voidType = nullptr;
    SlangNVVMTypeHandle_1 integerType = nullptr;
    SlangNVVMTypeHandle_1 pointerType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(module.module, voidType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getIntegerType(module.module, 32, integerType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getPointerType(
        module.module,
        integerType,
        SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
        pointerType)));

    // The opaque ABI has no aggregate/opaque type constructor, and its only unsized exposed type
    // cannot form a pointer. This pins the construction boundary without forging provider handles.
    SlangNVVMTypeHandle_1 rejectedUnsizedPointer =
        reinterpret_cast<SlangNVVMTypeHandle_1>(uintptr_t(1));
    SLANG_CHECK(
        builder.getPointerType(
            module.module,
            voidType,
            SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
            rejectedUnsizedPointer) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedUnsizedPointer == nullptr);

    const SlangNVVMTypeHandle_1 parameterTypes[] = {pointerType, pointerType, integerType};
    SlangNVVMTypeHandle_1 functionType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionType(
        module.module,
        voidType,
        parameterTypes,
        SLANG_COUNT_OF(parameterTypes),
        functionType)));
    SlangNVVMValueHandle_1 function = nullptr;
    SlangNVVMValueHandle_1 otherFunction = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
        module.module,
        functionType,
        toSlice("invalidPointerOffset"),
        function)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
        module.module,
        functionType,
        toSlice("otherPointerOffset"),
        otherFunction)));

    SlangNVVMValueHandle_1 destination = nullptr;
    SlangNVVMValueHandle_1 source = nullptr;
    SlangNVVMValueHandle_1 index = nullptr;
    SlangNVVMValueHandle_1 otherDestination = nullptr;
    SlangNVVMValueHandle_1 otherIndex = nullptr;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, function, 0, destination)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, function, 1, source)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, function, 2, index)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionParameter(module.module, otherFunction, 0, otherDestination)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, otherFunction, 2, otherIndex)));

    SlangNVVMTypeHandle_1 foreignVoidType = nullptr;
    SlangNVVMTypeHandle_1 foreignIntegerType = nullptr;
    SlangNVVMTypeHandle_1 foreignPointerType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(foreignModule.module, foreignVoidType)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getIntegerType(foreignModule.module, 32, foreignIntegerType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getPointerType(
        foreignModule.module,
        foreignIntegerType,
        SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
        foreignPointerType)));
    const SlangNVVMTypeHandle_1 foreignParameterTypes[] = {
        foreignPointerType,
        foreignIntegerType,
    };
    SlangNVVMTypeHandle_1 foreignFunctionType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionType(
        foreignModule.module,
        foreignVoidType,
        foreignParameterTypes,
        SLANG_COUNT_OF(foreignParameterTypes),
        foreignFunctionType)));
    SlangNVVMValueHandle_1 foreignFunction = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
        foreignModule.module,
        foreignFunctionType,
        toSlice("foreignPointerOffset"),
        foreignFunction)));
    SlangNVVMValueHandle_1 foreignPointer = nullptr;
    SlangNVVMValueHandle_1 foreignIndex = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionParameter(foreignModule.module, foreignFunction, 0, foreignPointer)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionParameter(foreignModule.module, foreignFunction, 1, foreignIndex)));

    auto expectRejectedOffset = [&](SlangNVVMModuleHandle_1 targetModule,
                                    SlangNVVMValueHandle_1 base,
                                    SlangNVVMValueHandle_1 offset)
    {
        SlangNVVMValueHandle_1 rejected = reinterpret_cast<SlangNVVMValueHandle_1>(uintptr_t(1));
        SLANG_CHECK(
            builder.emitPointerOffset(targetModule, base, offset, rejected) == SLANG_E_INVALID_ARG);
        SLANG_CHECK(rejected == nullptr);
    };

    // No insertion point and module ownership failures must be rejected before any instruction is
    // created or a function is inferred from the values.
    expectRejectedOffset(module.module, destination, index);
    expectRejectedOffset(nullptr, destination, index);
    expectRejectedOffset(foreignModule.module, destination, index);

    SlangNVVMBlockHandle_1 entryBlock = nullptr;
    SlangNVVMBlockHandle_1 producerBlock = nullptr;
    SlangNVVMBlockHandle_1 consumerBlock = nullptr;
    SlangNVVMBlockHandle_1 mergeBlock = nullptr;
    SlangNVVMBlockHandle_1 otherBlock = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, function, toSlice("entry"), entryBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, function, toSlice("producer"), producerBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, function, toSlice("consumer"), consumerBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, function, toSlice("merge"), mergeBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, otherFunction, toSlice("other.entry"), otherBlock)));

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, entryBlock)));
    SlangNVVMValueHandle_1 condition = nullptr;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.emitIntegerSignedLessThan(module.module, index, index, condition)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.emitConditionalBranch(module.module, condition, producerBlock, consumerBlock)));

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, producerBlock)));
    SlangNVVMValueHandle_1 producerPointer = nullptr;
    SlangNVVMValueHandle_1 producerInteger = nullptr;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.emitPointerOffset(module.module, source, index, producerPointer)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitIntegerBinary(
        module.module,
        SLANG_NVVM_INTEGER_BINARY_OP_ADD,
        index,
        index,
        producerInteger)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitBranch(module.module, mergeBlock)));

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, consumerBlock)));
    SLANG_CHECK(
        builder.getAPIV2()->emitPointerOffset(module.module, destination, index, nullptr) ==
        SLANG_E_INVALID_ARG);
    expectRejectedOffset(module.module, index, index);
    expectRejectedOffset(module.module, destination, source);
    expectRejectedOffset(module.module, foreignPointer, index);
    expectRejectedOffset(module.module, destination, foreignIndex);
    expectRejectedOffset(module.module, otherDestination, index);
    expectRejectedOffset(module.module, destination, otherIndex);
    expectRejectedOffset(module.module, producerPointer, index);
    expectRejectedOffset(module.module, destination, producerInteger);

    SlangNVVMValueHandle_1 consumerPointer = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.emitPointerOffset(module.module, destination, index, consumerPointer)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitBranch(module.module, mergeBlock)));

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, mergeBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitReturnVoid(module.module)));
    expectRejectedOffset(module.module, destination, index);

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, otherBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitReturnVoid(module.module)));

    ComPtr<ISlangBlob> assemblyBlob;
    String diagnostics;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(
        module.module,
        SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
        assemblyBlob,
        diagnostics)));
    SLANG_CHECK_ABORT(assemblyBlob != nullptr);
    SLANG_CHECK(diagnostics.getLength() == 0);
    const String assembly = _getBlobText(assemblyBlob);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("getelementptr i32")) == 2);
    SLANG_CHECK(assembly.indexOf("getelementptr inbounds") < 0);
}

SLANG_UNIT_TEST(nvvmIRBuilderRejectsInvalidArrayAddressingOperations)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    SLANG_CHECK_ABORT(builder.supportsScalarArrayAddressing());

    ScopedNVVMBuilderModule module;
    module.builder = &builder;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.createModule(toSlice("invalid-array-addressing"), module.module)));
    ScopedNVVMBuilderModule foreignModule;
    foreignModule.builder = &builder;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createModule(toSlice("invalid-array-addressing-foreign"), foreignModule.module)));

    SlangNVVMTypeHandle_1 voidType = nullptr;
    SlangNVVMTypeHandle_1 integerType = nullptr;
    SlangNVVMTypeHandle_1 arrayType = nullptr;
    SlangNVVMTypeHandle_1 arrayPointerType = nullptr;
    SlangNVVMTypeHandle_1 scalarPointerType = nullptr;
    SlangNVVMTypeHandle_1 foreignIntegerType = nullptr;
    SlangNVVMTypeHandle_1 foreignArrayType = nullptr;
    SlangNVVMTypeHandle_1 foreignArrayPointerType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(module.module, voidType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getIntegerType(module.module, 32, integerType)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getArrayType(module.module, integerType, 4, arrayType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getPointerType(
        module.module,
        arrayType,
        SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
        arrayPointerType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getPointerType(
        module.module,
        integerType,
        SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
        scalarPointerType)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getIntegerType(foreignModule.module, 32, foreignIntegerType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getArrayType(foreignModule.module, foreignIntegerType, 4, foreignArrayType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getPointerType(
        foreignModule.module,
        foreignArrayType,
        SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
        foreignArrayPointerType)));

    SLANG_CHECK(
        builder.getAPIV2()->getArrayType(module.module, integerType, 4, nullptr) ==
        SLANG_E_INVALID_ARG);
    SlangNVVMTypeHandle_1 rawRejectedType = reinterpret_cast<SlangNVVMTypeHandle_1>(uintptr_t(1));
    SLANG_CHECK(
        builder.getAPIV2()->getArrayType(module.module, voidType, 4, &rawRejectedType) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(rawRejectedType == nullptr);
    auto expectRejectedArrayType =
        [&](SlangNVVMModuleHandle_1 targetModule, SlangNVVMTypeHandle_1 elementType, uint32_t count)
    {
        SlangNVVMTypeHandle_1 rejected = reinterpret_cast<SlangNVVMTypeHandle_1>(uintptr_t(1));
        SLANG_CHECK(
            builder.getArrayType(targetModule, elementType, count, rejected) ==
            SLANG_E_INVALID_ARG);
        SLANG_CHECK(rejected == nullptr);
    };
    expectRejectedArrayType(nullptr, integerType, 4);
    expectRejectedArrayType(foreignModule.module, integerType, 4);
    expectRejectedArrayType(module.module, foreignIntegerType, 4);
    // Void is the only unsized type exposed by this provider ABI.
    expectRejectedArrayType(module.module, voidType, 4);
    expectRejectedArrayType(module.module, integerType, 0);

    const SlangNVVMTypeHandle_1 parameterTypes[] = {
        arrayPointerType,
        arrayPointerType,
        scalarPointerType,
        integerType,
    };
    SlangNVVMTypeHandle_1 functionType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionType(
        module.module,
        voidType,
        parameterTypes,
        SLANG_COUNT_OF(parameterTypes),
        functionType)));
    SlangNVVMValueHandle_1 function = nullptr;
    SlangNVVMValueHandle_1 otherFunction = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
        module.module,
        functionType,
        toSlice("invalidArrayAddressing"),
        function)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
        module.module,
        functionType,
        toSlice("otherArrayAddressing"),
        otherFunction)));

    SlangNVVMValueHandle_1 destination = nullptr;
    SlangNVVMValueHandle_1 source = nullptr;
    SlangNVVMValueHandle_1 scalarPointer = nullptr;
    SlangNVVMValueHandle_1 index = nullptr;
    SlangNVVMValueHandle_1 otherDestination = nullptr;
    SlangNVVMValueHandle_1 otherIndex = nullptr;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, function, 0, destination)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, function, 1, source)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, function, 2, scalarPointer)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, function, 3, index)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionParameter(module.module, otherFunction, 0, otherDestination)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, otherFunction, 3, otherIndex)));

    const SlangNVVMTypeHandle_1 foreignParameterTypes[] = {
        foreignArrayPointerType,
        foreignIntegerType,
    };
    SlangNVVMTypeHandle_1 foreignFunctionType = nullptr;
    SlangNVVMValueHandle_1 foreignFunction = nullptr;
    SlangNVVMValueHandle_1 foreignBase = nullptr;
    SlangNVVMValueHandle_1 foreignIndex = nullptr;
    SlangNVVMTypeHandle_1 foreignVoidType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(foreignModule.module, foreignVoidType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionType(
        foreignModule.module,
        foreignVoidType,
        foreignParameterTypes,
        SLANG_COUNT_OF(foreignParameterTypes),
        foreignFunctionType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
        foreignModule.module,
        foreignFunctionType,
        toSlice("foreignArrayAddressing"),
        foreignFunction)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionParameter(foreignModule.module, foreignFunction, 0, foreignBase)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionParameter(foreignModule.module, foreignFunction, 1, foreignIndex)));

    auto expectRejectedElement = [&](SlangNVVMModuleHandle_1 targetModule,
                                     SlangNVVMValueHandle_1 base,
                                     SlangNVVMValueHandle_1 elementIndex)
    {
        SlangNVVMValueHandle_1 rejected = reinterpret_cast<SlangNVVMValueHandle_1>(uintptr_t(1));
        SLANG_CHECK(
            builder.emitArrayElementPointer(targetModule, base, elementIndex, rejected) ==
            SLANG_E_INVALID_ARG);
        SLANG_CHECK(rejected == nullptr);
    };

    expectRejectedElement(module.module, destination, index);
    SlangNVVMValueHandle_1 rawRejectedElement =
        reinterpret_cast<SlangNVVMValueHandle_1>(uintptr_t(1));
    SLANG_CHECK(
        builder.getAPIV2()
            ->emitArrayElementPointer(module.module, destination, index, &rawRejectedElement) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(rawRejectedElement == nullptr);
    expectRejectedElement(nullptr, destination, index);
    expectRejectedElement(foreignModule.module, destination, index);
    expectRejectedElement(module.module, nullptr, index);
    expectRejectedElement(module.module, destination, nullptr);

    SlangNVVMBlockHandle_1 entryBlock = nullptr;
    SlangNVVMBlockHandle_1 producerBlock = nullptr;
    SlangNVVMBlockHandle_1 consumerBlock = nullptr;
    SlangNVVMBlockHandle_1 mergeBlock = nullptr;
    SlangNVVMBlockHandle_1 otherBlock = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, function, toSlice("entry"), entryBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, function, toSlice("producer"), producerBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, function, toSlice("consumer"), consumerBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, function, toSlice("merge"), mergeBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, otherFunction, toSlice("other.entry"), otherBlock)));

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, entryBlock)));
    SlangNVVMValueHandle_1 condition = nullptr;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.emitIntegerSignedLessThan(module.module, index, index, condition)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.emitConditionalBranch(module.module, condition, producerBlock, consumerBlock)));

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, producerBlock)));
    SlangNVVMValueHandle_1 nonDominatingIndex = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitIntegerBinary(
        module.module,
        SLANG_NVVM_INTEGER_BINARY_OP_ADD,
        index,
        index,
        nonDominatingIndex)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitBranch(module.module, mergeBlock)));

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, consumerBlock)));
    SLANG_CHECK(
        builder.getAPIV2()->emitArrayElementPointer(module.module, destination, index, nullptr) ==
        SLANG_E_INVALID_ARG);
    expectRejectedElement(module.module, scalarPointer, index);
    expectRejectedElement(module.module, destination, source);
    expectRejectedElement(module.module, foreignBase, index);
    expectRejectedElement(module.module, destination, foreignIndex);
    expectRejectedElement(module.module, otherDestination, index);
    expectRejectedElement(module.module, destination, otherIndex);
    expectRejectedElement(module.module, destination, nonDominatingIndex);

    SlangNVVMValueHandle_1 destinationElement = nullptr;
    SlangNVVMValueHandle_1 sourceElement = nullptr;
    SlangNVVMValueHandle_1 value = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.emitArrayElementPointer(module.module, destination, index, destinationElement)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.emitArrayElementPointer(module.module, source, index, sourceElement)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitLoad(module.module, sourceElement, 4, value)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.emitStore(module.module, value, destinationElement, 4)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitBranch(module.module, mergeBlock)));

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, mergeBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitReturnVoid(module.module)));
    expectRejectedElement(module.module, destination, index);
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, otherBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitReturnVoid(module.module)));

    ComPtr<ISlangBlob> assemblyBlob;
    String diagnostics;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(
        module.module,
        SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
        assemblyBlob,
        diagnostics)));
    SLANG_CHECK_ABORT(assemblyBlob != nullptr);
    SLANG_CHECK(diagnostics.getLength() == 0);
    const String assembly = _getBlobText(assemblyBlob);
    SLANG_CHECK(
        _countOccurrences(assembly.getUnownedSlice(), toSlice("getelementptr [4 x i32]")) == 2);
    SLANG_CHECK(
        _countOccurrences(assembly.getUnownedSlice(), toSlice("i32 0, i32 %slangParameter3")) == 2);
    SLANG_CHECK(assembly.indexOf("getelementptr inbounds") < 0);
    SLANG_CHECK(assembly.indexOf("addrspacecast") < 0);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("load i32")) == 1);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("store i32")) == 1);
}

SLANG_UNIT_TEST(nvvmIRBuilderRejectsInvalidRawRWStructuredBufferI32Operations)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    SLANG_CHECK_ABORT(builder.supportsRawRWStructuredBufferI32());

    ScopedNVVMBuilderModule module;
    module.builder = &builder;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createModule(toSlice("invalid-raw-rw-structured-buffer-i32"), module.module)));
    ScopedNVVMBuilderModule foreignModule;
    foreignModule.builder = &builder;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.createModule(
        toSlice("invalid-raw-rw-structured-buffer-i32-foreign"),
        foreignModule.module)));

    SlangNVVMTypeHandle_1 voidType = nullptr;
    SlangNVVMTypeHandle_1 integerType = nullptr;
    SlangNVVMTypeHandle_1 wideIntegerType = nullptr;
    SlangNVVMTypeHandle_1 resourceType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(module.module, voidType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getIntegerType(module.module, 32, integerType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getIntegerType(module.module, 64, wideIntegerType)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getRawRWStructuredBufferI32Type(module.module, resourceType)));

    SlangNVVMTypeHandle_1 rejectedType = reinterpret_cast<SlangNVVMTypeHandle_1>(uintptr_t(1));
    SLANG_CHECK(
        builder.getAPIV2()->getRawRWStructuredBufferI32Type(nullptr, &rejectedType) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedType == nullptr);
    SLANG_CHECK(
        builder.getAPIV2()->getRawRWStructuredBufferI32Type(module.module, nullptr) ==
        SLANG_E_INVALID_ARG);

    const SlangNVVMTypeHandle_1 parameterTypes[] = {
        resourceType,
        integerType,
        wideIntegerType,
    };
    SlangNVVMTypeHandle_1 functionType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionType(
        module.module,
        voidType,
        parameterTypes,
        SLANG_COUNT_OF(parameterTypes),
        functionType)));
    SlangNVVMValueHandle_1 function = nullptr;
    SlangNVVMValueHandle_1 otherFunction = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
        module.module,
        functionType,
        toSlice("invalidRawRWStructuredBufferI32"),
        function)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
        module.module,
        functionType,
        toSlice("otherRawRWStructuredBufferI32"),
        otherFunction)));

    SlangNVVMValueHandle_1 buffer = nullptr;
    SlangNVVMValueHandle_1 index = nullptr;
    SlangNVVMValueHandle_1 wideIndex = nullptr;
    SlangNVVMValueHandle_1 otherBuffer = nullptr;
    SlangNVVMValueHandle_1 otherIndex = nullptr;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, function, 0, buffer)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, function, 1, index)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, function, 2, wideIndex)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionParameter(module.module, otherFunction, 0, otherBuffer)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, otherFunction, 1, otherIndex)));

    SlangNVVMTypeHandle_1 foreignVoidType = nullptr;
    SlangNVVMTypeHandle_1 foreignIntegerType = nullptr;
    SlangNVVMTypeHandle_1 foreignResourceType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(foreignModule.module, foreignVoidType)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getIntegerType(foreignModule.module, 32, foreignIntegerType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getRawRWStructuredBufferI32Type(foreignModule.module, foreignResourceType)));
    const SlangNVVMTypeHandle_1 foreignParameterTypes[] = {
        foreignResourceType,
        foreignIntegerType,
    };
    SlangNVVMTypeHandle_1 foreignFunctionType = nullptr;
    SlangNVVMValueHandle_1 foreignFunction = nullptr;
    SlangNVVMValueHandle_1 foreignBuffer = nullptr;
    SlangNVVMValueHandle_1 foreignIndex = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionType(
        foreignModule.module,
        foreignVoidType,
        foreignParameterTypes,
        SLANG_COUNT_OF(foreignParameterTypes),
        foreignFunctionType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
        foreignModule.module,
        foreignFunctionType,
        toSlice("foreignRawRWStructuredBufferI32"),
        foreignFunction)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionParameter(foreignModule.module, foreignFunction, 0, foreignBuffer)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionParameter(foreignModule.module, foreignFunction, 1, foreignIndex)));

    SlangNVVMBlockHandle_1 block = nullptr;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.createBlock(module.module, function, toSlice("entry"), block)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, block)));

    String diagnostics;

    auto expectRejected = [&](SlangNVVMModuleHandle_1 targetModule,
                              SlangNVVMValueHandle_1 targetBuffer,
                              SlangNVVMValueHandle_1 targetIndex)
    {
        SlangNVVMValueHandle_1 rejected = reinterpret_cast<SlangNVVMValueHandle_1>(uintptr_t(1));
        SLANG_CHECK(
            builder.emitRawRWStructuredBufferI32ElementPointer(
                targetModule,
                targetBuffer,
                targetIndex,
                rejected) == SLANG_E_INVALID_ARG);
        SLANG_CHECK(rejected == nullptr);
    };
    SLANG_CHECK(
        builder.getAPIV2()
            ->emitRawRWStructuredBufferI32ElementPointer(module.module, buffer, index, nullptr) ==
        SLANG_E_INVALID_ARG);
    expectRejected(nullptr, buffer, index);
    expectRejected(foreignModule.module, buffer, index);
    expectRejected(module.module, nullptr, index);
    expectRejected(module.module, buffer, nullptr);
    expectRejected(module.module, index, index);
    expectRejected(module.module, buffer, buffer);
    expectRejected(module.module, buffer, wideIndex);
    expectRejected(module.module, otherBuffer, index);
    expectRejected(module.module, buffer, otherIndex);
    expectRejected(module.module, foreignBuffer, index);
    expectRejected(module.module, buffer, foreignIndex);

    SlangNVVMValueHandle_1 elementPointer = nullptr;
    SlangNVVMValueHandle_1 value = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitRawRWStructuredBufferI32ElementPointer(
        module.module,
        buffer,
        index,
        elementPointer)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getIntegerConstant(module.module, integerType, 42, value)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitStore(module.module, value, elementPointer, 4)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitReturnVoid(module.module)));

    ComPtr<ISlangBlob> completeBlob;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(
        module.module,
        SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
        completeBlob,
        diagnostics)));
    const String complete = _getBlobText(completeBlob);
    expectRejected(module.module, buffer, index);
    ComPtr<ISlangBlob> afterTerminatedBlob;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(
        module.module,
        SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
        afterTerminatedBlob,
        diagnostics)));
    SLANG_CHECK(_getBlobText(afterTerminatedBlob) == complete);

    SLANG_CHECK(
        complete.indexOf("define void @invalidRawRWStructuredBufferI32({ i32 addrspace(1)*, i64 } "
                         "%slangParameter0, i32 %slangParameter1, i64 %slangParameter2)") >= 0);
    SLANG_CHECK(_countOccurrences(complete.getUnownedSlice(), toSlice("extractvalue")) == 1);
    SLANG_CHECK(_countOccurrences(complete.getUnownedSlice(), toSlice("getelementptr i32")) == 1);
    SLANG_CHECK(complete.indexOf("getelementptr inbounds") < 0);
    SLANG_CHECK(_countOccurrences(complete.getUnownedSlice(), toSlice("store i32 42")) == 1);
}

static SlangResult _emitRawNVVMScalarBuilderOperation(
    const SlangNVVMBuilderAPI_V2* api,
    NVVMScalarTestOperation operation,
    SlangNVVMModuleHandle_1 module,
    SlangNVVMValueHandle_1 left,
    SlangNVVMValueHandle_1 right,
    SlangNVVMValueHandle_1* outValue)
{
    switch (operation)
    {
    case NVVMScalarTestOperation::Multiply:
        return api->emitIntegerMultiply(module, left, right, outValue);
    case NVVMScalarTestOperation::BitAnd:
        return api->emitIntegerBitAnd(module, left, right, outValue);
    case NVVMScalarTestOperation::BitOr:
        return api->emitIntegerBitOr(module, left, right, outValue);
    case NVVMScalarTestOperation::BitXor:
        return api->emitIntegerBitXor(module, left, right, outValue);
    case NVVMScalarTestOperation::BitNot:
        return api->emitIntegerBitNot(module, left, outValue);
    case NVVMScalarTestOperation::Negate:
        return api->emitIntegerNegate(module, left, outValue);
    case NVVMScalarTestOperation::Equal:
        return api->emitIntegerEqual(module, left, right, outValue);
    case NVVMScalarTestOperation::NotEqual:
        return api->emitIntegerNotEqual(module, left, right, outValue);
    case NVVMScalarTestOperation::SignedGreaterThan:
        return api->emitIntegerSignedGreaterThan(module, left, right, outValue);
    case NVVMScalarTestOperation::SignedLessEqual:
        return api->emitIntegerSignedLessEqual(module, left, right, outValue);
    case NVVMScalarTestOperation::SignedGreaterEqual:
        return api->emitIntegerSignedGreaterEqual(module, left, right, outValue);
    }
    return SLANG_E_INVALID_ARG;
}

static void _runNVVMScalarInvalidOperations(
    UnitTestContext* unitTestContext,
    NVVMScalarTestOperation operation)
{
    const NVVMScalarTestCase& testCase = _getNVVMScalarTestCase(operation);
    const bool isUnary = testCase.key.family == FakeNVVMBuilderScalarFamily::Unary;
    const bool isCompare = testCase.key.family == FakeNVVMBuilderScalarFamily::Compare;

    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    SLANG_CHECK_ABORT(_supportsNVVMScalarBuilderOperation(builder, operation));

    ScopedNVVMBuilderModule module;
    module.builder = &builder;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.createModule(toSlice("invalid-scalar-operation"), module.module)));
    ScopedNVVMBuilderModule foreignModule;
    foreignModule.builder = &builder;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createModule(toSlice("invalid-scalar-operation-foreign"), foreignModule.module)));

    SlangNVVMTypeHandle_1 voidType = nullptr;
    SlangNVVMTypeHandle_1 integerType = nullptr;
    SlangNVVMTypeHandle_1 wideIntegerType = nullptr;
    SlangNVVMTypeHandle_1 pointerType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(module.module, voidType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getIntegerType(module.module, 32, integerType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getIntegerType(module.module, 64, wideIntegerType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getPointerType(
        module.module,
        integerType,
        SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
        pointerType)));

    const SlangNVVMTypeHandle_1 parameterTypes[] = {
        pointerType,
        integerType,
        integerType,
        wideIntegerType,
    };
    SlangNVVMTypeHandle_1 functionType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionType(
        module.module,
        voidType,
        parameterTypes,
        SLANG_COUNT_OF(parameterTypes),
        functionType)));
    SlangNVVMValueHandle_1 function = nullptr;
    SlangNVVMValueHandle_1 otherFunction = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
        module.module,
        functionType,
        toSlice("invalidScalarOperation"),
        function)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
        module.module,
        functionType,
        toSlice("otherScalarOperation"),
        otherFunction)));

    SlangNVVMValueHandle_1 destination = nullptr;
    SlangNVVMValueHandle_1 left = nullptr;
    SlangNVVMValueHandle_1 right = nullptr;
    SlangNVVMValueHandle_1 wide = nullptr;
    SlangNVVMValueHandle_1 otherLeft = nullptr;
    SlangNVVMValueHandle_1 otherRight = nullptr;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, function, 0, destination)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, function, 1, left)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, function, 2, right)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, function, 3, wide)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, otherFunction, 1, otherLeft)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, otherFunction, 2, otherRight)));

    SlangNVVMTypeHandle_1 foreignVoidType = nullptr;
    SlangNVVMTypeHandle_1 foreignIntegerType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(foreignModule.module, foreignVoidType)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getIntegerType(foreignModule.module, 32, foreignIntegerType)));
    const SlangNVVMTypeHandle_1 foreignParameterTypes[] = {
        foreignIntegerType,
        foreignIntegerType,
    };
    SlangNVVMTypeHandle_1 foreignFunctionType = nullptr;
    SlangNVVMValueHandle_1 foreignFunction = nullptr;
    SlangNVVMValueHandle_1 foreignLeft = nullptr;
    SlangNVVMValueHandle_1 foreignRight = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionType(
        foreignModule.module,
        foreignVoidType,
        foreignParameterTypes,
        SLANG_COUNT_OF(foreignParameterTypes),
        foreignFunctionType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
        foreignModule.module,
        foreignFunctionType,
        toSlice("foreignScalarOperation"),
        foreignFunction)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionParameter(foreignModule.module, foreignFunction, 0, foreignLeft)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionParameter(foreignModule.module, foreignFunction, 1, foreignRight)));

    auto expectRejected = [&](SlangNVVMModuleHandle_1 targetModule,
                              SlangNVVMValueHandle_1 candidateLeft,
                              SlangNVVMValueHandle_1 candidateRight)
    {
        SlangNVVMValueHandle_1 rejected = reinterpret_cast<SlangNVVMValueHandle_1>(uintptr_t(1));
        SLANG_CHECK(
            _emitNVVMScalarBuilderOperation(
                builder,
                operation,
                targetModule,
                candidateLeft,
                candidateRight,
                rejected) == SLANG_E_INVALID_ARG);
        SLANG_CHECK(rejected == nullptr);
    };

    expectRejected(module.module, left, right);
    SlangNVVMValueHandle_1 rawRejected = reinterpret_cast<SlangNVVMValueHandle_1>(uintptr_t(1));
    SLANG_CHECK(
        _emitRawNVVMScalarBuilderOperation(
            builder.getAPIV2(),
            operation,
            module.module,
            left,
            right,
            &rawRejected) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(rawRejected == nullptr);
    expectRejected(nullptr, left, right);
    expectRejected(foreignModule.module, left, right);

    SlangNVVMBlockHandle_1 entryBlock = nullptr;
    SlangNVVMBlockHandle_1 producerBlock = nullptr;
    SlangNVVMBlockHandle_1 consumerBlock = nullptr;
    SlangNVVMBlockHandle_1 trueBlock = nullptr;
    SlangNVVMBlockHandle_1 falseBlock = nullptr;
    SlangNVVMBlockHandle_1 mergeBlock = nullptr;
    SlangNVVMBlockHandle_1 otherBlock = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, function, toSlice("entry"), entryBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, function, toSlice("producer"), producerBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, function, toSlice("consumer"), consumerBlock)));
    if (isCompare)
    {
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            builder.createBlock(module.module, function, toSlice("true"), trueBlock)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            builder.createBlock(module.module, function, toSlice("false"), falseBlock)));
    }
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, function, toSlice("merge"), mergeBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, otherFunction, toSlice("other.entry"), otherBlock)));

    SlangNVVMValueHandle_1 zero = nullptr;
    SlangNVVMValueHandle_1 one = nullptr;
    if (isCompare)
    {
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.getIntegerConstant(module.module, integerType, 0, zero)));
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.getIntegerConstant(module.module, integerType, 1, one)));
    }

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, entryBlock)));
    SlangNVVMValueHandle_1 scaffoldCondition = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.emitIntegerSignedLessThan(module.module, left, right, scaffoldCondition)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitConditionalBranch(
        module.module,
        scaffoldCondition,
        producerBlock,
        consumerBlock)));

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, producerBlock)));
    SlangNVVMValueHandle_1 nonDominating = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitIntegerBinary(
        module.module,
        SLANG_NVVM_INTEGER_BINARY_OP_ADD,
        left,
        right,
        nonDominating)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitBranch(module.module, mergeBlock)));

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, consumerBlock)));
    SLANG_CHECK(
        _emitRawNVVMScalarBuilderOperation(
            builder.getAPIV2(),
            operation,
            module.module,
            left,
            right,
            nullptr) == SLANG_E_INVALID_ARG);
    expectRejected(module.module, nullptr, right);
    if (!isUnary)
        expectRejected(module.module, left, nullptr);
    expectRejected(module.module, destination, right);
    if (!isUnary)
        expectRejected(module.module, left, destination);
    if (!isUnary)
        expectRejected(module.module, wide, right);
    if (!isUnary)
        expectRejected(module.module, left, wide);
    expectRejected(module.module, foreignLeft, right);
    if (!isUnary)
        expectRejected(module.module, left, foreignRight);
    expectRejected(module.module, otherLeft, right);
    if (!isUnary)
        expectRejected(module.module, left, otherRight);
    expectRejected(module.module, nonDominating, right);
    if (!isUnary)
        expectRejected(module.module, left, nonDominating);

    SlangNVVMValueHandle_1 value = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        _emitNVVMScalarBuilderOperation(builder, operation, module.module, left, right, value)));
    if (isCompare)
    {
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            builder.emitConditionalBranch(module.module, value, trueBlock, falseBlock)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, trueBlock)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitStore(module.module, one, destination, 4)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitBranch(module.module, mergeBlock)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, falseBlock)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitStore(module.module, zero, destination, 4)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitBranch(module.module, mergeBlock)));
    }
    else
    {
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitStore(module.module, value, destination, 4)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitBranch(module.module, mergeBlock)));
    }

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, mergeBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitReturnVoid(module.module)));
    expectRejected(module.module, left, right);

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, otherBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitReturnVoid(module.module)));

    ComPtr<ISlangBlob> assemblyBlob;
    String diagnostics;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(
        module.module,
        SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
        assemblyBlob,
        diagnostics)));
    SLANG_CHECK_ABORT(assemblyBlob != nullptr);
    SLANG_CHECK(diagnostics.getLength() == 0);
    const String assembly = _getBlobText(assemblyBlob);
    StringBuilder instruction32;
    instruction32 << testCase.llvmOpcode << " i32";
    StringBuilder instruction64;
    instruction64 << testCase.llvmOpcode << " i64";
    SLANG_CHECK(
        _countOccurrences(assembly.getUnownedSlice(), instruction32.getUnownedSlice()) == 1);
    SLANG_CHECK(
        _countOccurrences(assembly.getUnownedSlice(), instruction64.getUnownedSlice()) == 0);
    SLANG_CHECK(
        _countOccurrences(assembly.getUnownedSlice(), toSlice("store i32")) == (isCompare ? 2 : 1));
    const Index operationIndex = assembly.indexOf(instruction32.getUnownedSlice());
    SLANG_CHECK_ABORT(operationIndex >= 0);
    if (isCompare)
    {
        const Index conditionalIndex =
            assembly.getUnownedSlice().tail(operationIndex).indexOf(toSlice("br i1"));
        SLANG_CHECK(conditionalIndex > 0);
    }
    else
    {
        const Index storeIndex = assembly.indexOf("store i32");
        SLANG_CHECK(storeIndex > operationIndex);
    }
}

#define NVVM_SCALAR_INVALID_TEST(NAME, OPERATION)                                             \
    SLANG_UNIT_TEST(NAME)                                                                     \
    {                                                                                         \
        _runNVVMScalarInvalidOperations(unitTestContext, NVVMScalarTestOperation::OPERATION); \
    }

NVVM_SCALAR_INVALID_TEST(nvvmIRBuilderRejectsInvalidIntegerMultiplyOperations, Multiply)
NVVM_SCALAR_INVALID_TEST(nvvmIRBuilderRejectsInvalidIntegerBitAndOperations, BitAnd)
NVVM_SCALAR_INVALID_TEST(nvvmIRBuilderRejectsInvalidIntegerBitOrOperations, BitOr)
NVVM_SCALAR_INVALID_TEST(nvvmIRBuilderRejectsInvalidIntegerBitXorOperations, BitXor)
NVVM_SCALAR_INVALID_TEST(nvvmIRBuilderRejectsInvalidIntegerBitNotOperations, BitNot)
NVVM_SCALAR_INVALID_TEST(nvvmIRBuilderRejectsInvalidIntegerNegateOperations, Negate)
SLANG_UNIT_TEST(nvvmIRBuilderRejectsInvalidRelaxedGlobalI32AtomicAddOperations)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    SLANG_CHECK_ABORT(builder.supportsRelaxedGlobalI32AtomicAdd());

    ScopedNVVMBuilderModule module;
    module.builder = &builder;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createModule(toSlice("invalid-relaxed-global-i32-atomic-add"), module.module)));
    ScopedNVVMBuilderModule foreignModule;
    foreignModule.builder = &builder;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.createModule(
        toSlice("invalid-relaxed-global-i32-atomic-add-foreign"),
        foreignModule.module)));

    SlangNVVMTypeHandle_1 voidType = nullptr;
    SlangNVVMTypeHandle_1 i32Type = nullptr;
    SlangNVVMTypeHandle_1 i64Type = nullptr;
    SlangNVVMTypeHandle_1 globalI32PointerType = nullptr;
    SlangNVVMTypeHandle_1 sharedI32PointerType = nullptr;
    SlangNVVMTypeHandle_1 constantI32PointerType = nullptr;
    SlangNVVMTypeHandle_1 genericI32PointerType = nullptr;
    SlangNVVMTypeHandle_1 localI32PointerType = nullptr;
    SlangNVVMTypeHandle_1 globalI64PointerType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(module.module, voidType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getIntegerType(module.module, 32, i32Type)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getIntegerType(module.module, 64, i64Type)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getPointerType(
        module.module,
        i32Type,
        SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
        globalI32PointerType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getPointerType(
        module.module,
        i32Type,
        SLANG_NVVM_ADDRESS_SPACE_SHARED,
        sharedI32PointerType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getPointerType(
        module.module,
        i32Type,
        SLANG_NVVM_ADDRESS_SPACE_CONSTANT,
        constantI32PointerType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getPointerType(
        module.module,
        i32Type,
        SLANG_NVVM_ADDRESS_SPACE_GENERIC,
        genericI32PointerType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getPointerType(
        module.module,
        i32Type,
        SLANG_NVVM_ADDRESS_SPACE_LOCAL,
        localI32PointerType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getPointerType(
        module.module,
        i64Type,
        SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
        globalI64PointerType)));

    const SlangNVVMTypeHandle_1 parameterTypes[] = {
        globalI32PointerType,
        globalI32PointerType,
        i32Type,
        sharedI32PointerType,
        constantI32PointerType,
        genericI32PointerType,
        localI32PointerType,
        globalI64PointerType,
        i64Type,
    };
    SlangNVVMTypeHandle_1 functionType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionType(
        module.module,
        voidType,
        parameterTypes,
        SLANG_COUNT_OF(parameterTypes),
        functionType)));
    SlangNVVMValueHandle_1 function = nullptr;
    SlangNVVMValueHandle_1 otherFunction = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
        module.module,
        functionType,
        toSlice("rejectInvalidRelaxedGlobalI32AtomicAdd"),
        function)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
        module.module,
        functionType,
        toSlice("otherRelaxedGlobalI32AtomicAdd"),
        otherFunction)));

    SlangNVVMValueHandle_1 destination = nullptr;
    SlangNVVMValueHandle_1 oldValueDestination = nullptr;
    SlangNVVMValueHandle_1 value = nullptr;
    SlangNVVMValueHandle_1 sharedDestination = nullptr;
    SlangNVVMValueHandle_1 constantDestination = nullptr;
    SlangNVVMValueHandle_1 genericDestination = nullptr;
    SlangNVVMValueHandle_1 localDestination = nullptr;
    SlangNVVMValueHandle_1 wideDestination = nullptr;
    SlangNVVMValueHandle_1 wideValue = nullptr;
    SlangNVVMValueHandle_1 otherDestination = nullptr;
    SlangNVVMValueHandle_1 otherValue = nullptr;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, function, 0, destination)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionParameter(module.module, function, 1, oldValueDestination)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, function, 2, value)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionParameter(module.module, function, 3, sharedDestination)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionParameter(module.module, function, 4, constantDestination)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionParameter(module.module, function, 5, genericDestination)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionParameter(module.module, function, 6, localDestination)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, function, 7, wideDestination)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, function, 8, wideValue)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionParameter(module.module, otherFunction, 0, otherDestination)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, otherFunction, 2, otherValue)));

    SlangNVVMTypeHandle_1 foreignVoidType = nullptr;
    SlangNVVMTypeHandle_1 foreignI32Type = nullptr;
    SlangNVVMTypeHandle_1 foreignGlobalI32PointerType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(foreignModule.module, foreignVoidType)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getIntegerType(foreignModule.module, 32, foreignI32Type)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getPointerType(
        foreignModule.module,
        foreignI32Type,
        SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
        foreignGlobalI32PointerType)));
    const SlangNVVMTypeHandle_1 foreignParameterTypes[] = {
        foreignGlobalI32PointerType,
        foreignI32Type,
    };
    SlangNVVMTypeHandle_1 foreignFunctionType = nullptr;
    SlangNVVMValueHandle_1 foreignFunction = nullptr;
    SlangNVVMValueHandle_1 foreignDestination = nullptr;
    SlangNVVMValueHandle_1 foreignValue = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionType(
        foreignModule.module,
        foreignVoidType,
        foreignParameterTypes,
        SLANG_COUNT_OF(foreignParameterTypes),
        foreignFunctionType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
        foreignModule.module,
        foreignFunctionType,
        toSlice("foreignRelaxedGlobalI32AtomicAdd"),
        foreignFunction)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder
            .getFunctionParameter(foreignModule.module, foreignFunction, 0, foreignDestination)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionParameter(foreignModule.module, foreignFunction, 1, foreignValue)));
    SlangNVVMBlockHandle_1 foreignBlock = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder
            .createBlock(foreignModule.module, foreignFunction, toSlice("entry"), foreignBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(foreignModule.module, foreignBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitReturnVoid(foreignModule.module)));

    auto expectRejected = [&](SlangNVVMModuleHandle_1 targetModule,
                              SlangNVVMValueHandle_1 pointer,
                              SlangNVVMValueHandle_1 addend)
    {
        SlangNVVMValueHandle_1 rejected = reinterpret_cast<SlangNVVMValueHandle_1>(uintptr_t(1));
        SLANG_CHECK(
            builder.emitRelaxedGlobalI32AtomicAdd(targetModule, pointer, addend, rejected) ==
            SLANG_E_INVALID_ARG);
        SLANG_CHECK(rejected == nullptr);
    };

    // No insertion point exists for the selected function. Rejections must not infer ownership or
    // create an atomic instruction in some other function's current block.
    expectRejected(module.module, destination, value);
    expectRejected(nullptr, destination, value);
    expectRejected(foreignModule.module, destination, value);

    SlangNVVMBlockHandle_1 entryBlock = nullptr;
    SlangNVVMBlockHandle_1 producerBlock = nullptr;
    SlangNVVMBlockHandle_1 consumerBlock = nullptr;
    SlangNVVMBlockHandle_1 mergeBlock = nullptr;
    SlangNVVMBlockHandle_1 otherBlock = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, function, toSlice("entry"), entryBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, function, toSlice("producer"), producerBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, function, toSlice("consumer"), consumerBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, function, toSlice("merge"), mergeBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, otherFunction, toSlice("other.entry"), otherBlock)));

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, otherBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitReturnVoid(module.module)));

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, entryBlock)));
    SlangNVVMValueHandle_1 condition = nullptr;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.emitIntegerSignedLessThan(module.module, value, value, condition)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.emitConditionalBranch(module.module, condition, producerBlock, consumerBlock)));

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, producerBlock)));
    SlangNVVMValueHandle_1 nonDominatingValue = nullptr;
    SlangNVVMValueHandle_1 nonDominatingPointer = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitIntegerBinary(
        module.module,
        SLANG_NVVM_INTEGER_BINARY_OP_ADD,
        value,
        value,
        nonDominatingValue)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.emitPointerOffset(module.module, destination, value, nonDominatingPointer)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitBranch(module.module, mergeBlock)));

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, consumerBlock)));
    const SlangNVVMBuilderAPI_V2* api = builder.getAPIV2();
    SLANG_CHECK_ABORT(api != nullptr);
    SLANG_CHECK(
        api->emitRelaxedGlobalI32AtomicAdd(module.module, destination, value, nullptr) ==
        SLANG_E_INVALID_ARG);
    expectRejected(module.module, nullptr, value);
    expectRejected(module.module, destination, nullptr);
    expectRejected(module.module, value, value);
    expectRejected(module.module, destination, destination);
    expectRejected(module.module, sharedDestination, value);
    expectRejected(module.module, constantDestination, value);
    expectRejected(module.module, genericDestination, value);
    expectRejected(module.module, localDestination, value);
    expectRejected(module.module, wideDestination, value);
    expectRejected(module.module, destination, wideValue);
    expectRejected(module.module, otherDestination, value);
    expectRejected(module.module, destination, otherValue);
    expectRejected(module.module, foreignDestination, value);
    expectRejected(module.module, destination, foreignValue);
    expectRejected(module.module, nonDominatingPointer, value);
    expectRejected(module.module, destination, nonDominatingValue);

    SlangNVVMValueHandle_1 oldValue = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.emitRelaxedGlobalI32AtomicAdd(module.module, destination, value, oldValue)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.emitStore(module.module, oldValue, oldValueDestination, 4)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitBranch(module.module, mergeBlock)));

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, mergeBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitReturnVoid(module.module)));
    expectRejected(module.module, destination, value);

    ComPtr<ISlangBlob> assemblyBlob;
    String diagnostics;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(
        module.module,
        SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
        assemblyBlob,
        diagnostics)));
    SLANG_CHECK_ABORT(assemblyBlob != nullptr);
    SLANG_CHECK(diagnostics.getLength() == 0);
    const String assembly = _getBlobText(assemblyBlob);
    const UnownedStringSlice assemblySlice = assembly.getUnownedSlice();
    SLANG_CHECK(_countOccurrences(assemblySlice, toSlice("atomicrmw add i32 addrspace(1)*")) == 1);
    SLANG_CHECK(_countOccurrences(assemblySlice, toSlice("atomicrmw add i64")) == 0);
    SLANG_CHECK(_countOccurrences(assemblySlice, toSlice("monotonic")) == 1);
    SLANG_CHECK(_countOccurrences(assemblySlice, toSlice("align 4")) == 2);
    SLANG_CHECK(assembly.indexOf("monotonic, align 4") >= 0);
    SLANG_CHECK(assembly.indexOf("syncscope(") < 0);
    SLANG_CHECK(assembly.indexOf("acquire") < 0);
    SLANG_CHECK(assembly.indexOf("release") < 0);
    SLANG_CHECK(assembly.indexOf("seq_cst") < 0);
    SLANG_CHECK(_countOccurrences(assemblySlice, toSlice("store i32")) == 1);
    const Index atomicIndex = assembly.indexOf("atomicrmw add i32 addrspace(1)*");
    const Index storeIndex = assembly.indexOf("store i32");
    SLANG_CHECK_ABORT(atomicIndex >= 0);
    SLANG_CHECK(storeIndex > atomicIndex);
}

NVVM_SCALAR_INVALID_TEST(nvvmIRBuilderRejectsInvalidIntegerEqualOperations, Equal)
NVVM_SCALAR_INVALID_TEST(nvvmIRBuilderRejectsInvalidIntegerNotEqualOperations, NotEqual)
NVVM_SCALAR_INVALID_TEST(
    nvvmIRBuilderRejectsInvalidIntegerSignedGreaterThanOperations,
    SignedGreaterThan)
NVVM_SCALAR_INVALID_TEST(
    nvvmIRBuilderRejectsInvalidIntegerSignedLessEqualOperations,
    SignedLessEqual)
NVVM_SCALAR_INVALID_TEST(
    nvvmIRBuilderRejectsInvalidIntegerSignedGreaterEqualOperations,
    SignedGreaterEqual)

#undef NVVM_SCALAR_INVALID_TEST
SLANG_UNIT_TEST(nvvmIRBuilderBuildsScalarReferenceKernels)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    SLANG_CHECK_ABORT(builder.supportsScalarOperations());

    ComPtr<ISlangBlob> assemblyBlob;
    ComPtr<ISlangBlob> bitcodeBlob;
    String assemblyDiagnostics = "stale assembly diagnostics";
    String bitcodeDiagnostics = "stale bitcode diagnostics";
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_buildScalarReferenceModule(
        builder,
        assemblyBlob,
        assemblyDiagnostics,
        bitcodeBlob,
        bitcodeDiagnostics)));
    SLANG_CHECK_ABORT(assemblyBlob != nullptr);
    SLANG_CHECK_ABORT(bitcodeBlob != nullptr);
    SLANG_CHECK(assemblyDiagnostics.getLength() == 0);
    SLANG_CHECK(bitcodeDiagnostics.getLength() == 0);

    const String assembly = _getBlobText(assemblyBlob);
    SLANG_CHECK(assembly.indexOf("target triple = \"nvptx64-nvidia-cuda\"") >= 0);
    SLANG_CHECK(assembly.indexOf("define void @writeScalar(i32 addrspace(1)*") >= 0);
    SLANG_CHECK(assembly.indexOf("define void @copyScalar(i32 addrspace(1)*") >= 0);
    SLANG_CHECK(assembly.indexOf(", i32 addrspace(1)*") >= 0);
    SLANG_CHECK(
        assembly.indexOf(
            "store i32 %slangParameter1, i32 addrspace(1)* %slangParameter0, align 4") >= 0);
    SLANG_CHECK(assembly.indexOf("load i32, i32 addrspace(1)* %slangParameter1, align 4") >= 0);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("store i32")) == 2);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("load i32")) == 1);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("align 4")) == 3);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("ret void")) == 2);
    SLANG_CHECK(assembly.indexOf("addrspacecast") < 0);
    SLANG_CHECK(assembly.indexOf("!nvvm.annotations") >= 0);
    SLANG_CHECK(assembly.indexOf("!nvvmir.version") >= 0);
    SLANG_CHECK(assembly.indexOf("@writeScalar, !\"kernel\", i32 1") >= 0);
    SLANG_CHECK(assembly.indexOf("@copyScalar, !\"kernel\", i32 1") >= 0);

    static const uint8_t kBitcodeMagic[] = {0x42, 0x43, 0xc0, 0xde};
    SLANG_CHECK(bitcodeBlob->getBufferSize() > SLANG_COUNT_OF(kBitcodeMagic));
    SLANG_CHECK(
        ::memcmp(bitcodeBlob->getBufferPointer(), kBitcodeMagic, SLANG_COUNT_OF(kBitcodeMagic)) ==
        0);
}

SLANG_UNIT_TEST(nvvmIRBuilderBuildsScalarConditionalKernel)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    SLANG_CHECK_ABORT(builder.supportsScalarControlFlow());

    ComPtr<ISlangBlob> assemblyBlob;
    ComPtr<ISlangBlob> bitcodeBlob;
    String assemblyDiagnostics = "stale assembly diagnostics";
    String bitcodeDiagnostics = "stale bitcode diagnostics";
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_buildScalarConditionalModule(
        builder,
        assemblyBlob,
        assemblyDiagnostics,
        bitcodeBlob,
        bitcodeDiagnostics)));
    SLANG_CHECK_ABORT(assemblyBlob != nullptr);
    SLANG_CHECK_ABORT(bitcodeBlob != nullptr);
    SLANG_CHECK(assemblyDiagnostics.getLength() == 0);
    SLANG_CHECK(bitcodeDiagnostics.getLength() == 0);

    const String assembly = _getBlobText(assemblyBlob);
    SLANG_CHECK(assembly.indexOf("define void @chooseScalar(i32 addrspace(1)*") >= 0);
    SLANG_CHECK(assembly.indexOf("icmp slt i32") >= 0);
    SLANG_CHECK(assembly.indexOf("add i32") >= 0);
    SLANG_CHECK(assembly.indexOf("sub i32") >= 0);
    SLANG_CHECK(assembly.indexOf("br i1") >= 0);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("store i32")) == 2);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("ret void")) == 1);
    SLANG_CHECK(assembly.indexOf("@chooseScalar, !\"kernel\", i32 1") >= 0);

    static const uint8_t kBitcodeMagic[] = {0x42, 0x43, 0xc0, 0xde};
    SLANG_CHECK(bitcodeBlob->getBufferSize() > SLANG_COUNT_OF(kBitcodeMagic));
    SLANG_CHECK(
        ::memcmp(bitcodeBlob->getBufferPointer(), kBitcodeMagic, SLANG_COUNT_OF(kBitcodeMagic)) ==
        0);
}

SLANG_UNIT_TEST(nvvmIRBuilderBuildsScalarSSALoopKernel)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    SLANG_CHECK_ABORT(builder.supportsScalarSSA());

    ComPtr<ISlangBlob> assemblyBlob;
    ComPtr<ISlangBlob> bitcodeBlob;
    String assemblyDiagnostics = "stale assembly diagnostics";
    String bitcodeDiagnostics = "stale bitcode diagnostics";
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_buildScalarSSALoopModule(
        builder,
        assemblyBlob,
        assemblyDiagnostics,
        bitcodeBlob,
        bitcodeDiagnostics)));
    SLANG_CHECK_ABORT(assemblyBlob != nullptr);
    SLANG_CHECK_ABORT(bitcodeBlob != nullptr);
    SLANG_CHECK(assemblyDiagnostics.getLength() == 0);
    SLANG_CHECK(bitcodeDiagnostics.getLength() == 0);

    const String assembly = _getBlobText(assemblyBlob);
    SLANG_CHECK(assembly.indexOf("define void @sumToLimit(i32 addrspace(1)*") >= 0);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("phi i32")) == 2);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("icmp slt i32")) == 1);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("add i32")) == 2);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("br i1")) == 1);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("store i32")) == 1);
    SLANG_CHECK(assembly.indexOf("i32 0") >= 0);
    SLANG_CHECK(assembly.indexOf("i32 1") >= 0);
    SLANG_CHECK(assembly.indexOf("%entry") >= 0);
    SLANG_CHECK(assembly.indexOf("%loop.body") >= 0);
    SLANG_CHECK(assembly.indexOf("%loop.continue") >= 0);
    SLANG_CHECK(assembly.indexOf("@sumToLimit, !\"kernel\", i32 1") >= 0);

    static const uint8_t kBitcodeMagic[] = {0x42, 0x43, 0xc0, 0xde};
    SLANG_CHECK(bitcodeBlob->getBufferSize() > SLANG_COUNT_OF(kBitcodeMagic));
    SLANG_CHECK(
        ::memcmp(bitcodeBlob->getBufferPointer(), kBitcodeMagic, SLANG_COUNT_OF(kBitcodeMagic)) ==
        0);
}

SLANG_UNIT_TEST(nvvmIRBuilderBuildsScalarFunctionKernel)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    SLANG_CHECK_ABORT(builder.supportsScalarFunctions());

    ComPtr<ISlangBlob> assemblyBlob;
    ComPtr<ISlangBlob> bitcodeBlob;
    String assemblyDiagnostics = "stale assembly diagnostics";
    String bitcodeDiagnostics = "stale bitcode diagnostics";
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_buildScalarFunctionModule(
        builder,
        assemblyBlob,
        assemblyDiagnostics,
        bitcodeBlob,
        bitcodeDiagnostics)));
    SLANG_CHECK_ABORT(assemblyBlob != nullptr);
    SLANG_CHECK_ABORT(bitcodeBlob != nullptr);
    SLANG_CHECK(assemblyDiagnostics.getLength() == 0);
    SLANG_CHECK(bitcodeDiagnostics.getLength() == 0);

    const String assembly = _getBlobText(assemblyBlob);
    SLANG_CHECK(assembly.indexOf("define i32 @incrementScalar(i32") >= 0);
    SLANG_CHECK(assembly.indexOf("define void @callScalar(i32 addrspace(1)*") >= 0);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("call i32")) == 1);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("ret i32")) == 1);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("ret void")) == 1);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("store i32")) == 1);
    SLANG_CHECK(assembly.indexOf("@callScalar, !\"kernel\", i32 1") >= 0);
    SLANG_CHECK(assembly.indexOf("@incrementScalar, !\"kernel\"") < 0);

    static const uint8_t kBitcodeMagic[] = {0x42, 0x43, 0xc0, 0xde};
    SLANG_CHECK(bitcodeBlob->getBufferSize() > SLANG_COUNT_OF(kBitcodeMagic));
    SLANG_CHECK(
        ::memcmp(bitcodeBlob->getBufferPointer(), kBitcodeMagic, SLANG_COUNT_OF(kBitcodeMagic)) ==
        0);
}

SLANG_UNIT_TEST(nvvmIRBuilderBuildsPointerOffsetKernel)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    SLANG_CHECK_ABORT(builder.supportsScalarPointerArithmetic());

    ComPtr<ISlangBlob> assemblyBlob;
    ComPtr<ISlangBlob> bitcodeBlob;
    String assemblyDiagnostics = "stale assembly diagnostics";
    String bitcodeDiagnostics = "stale bitcode diagnostics";
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_buildPointerOffsetModule(
        builder,
        assemblyBlob,
        assemblyDiagnostics,
        bitcodeBlob,
        bitcodeDiagnostics)));
    SLANG_CHECK_ABORT(assemblyBlob != nullptr);
    SLANG_CHECK_ABORT(bitcodeBlob != nullptr);
    SLANG_CHECK(assemblyDiagnostics.getLength() == 0);
    SLANG_CHECK(bitcodeDiagnostics.getLength() == 0);

    const String assembly = _getBlobText(assemblyBlob);
    SLANG_CHECK(
        assembly.indexOf(
            "define void @copyIndexed(i32 addrspace(1)* %slangParameter0, i32 addrspace(1)* "
            "%slangParameter1, i32 %slangParameter2)") >= 0);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("getelementptr i32")) == 2);
    SLANG_CHECK(assembly.indexOf("getelementptr inbounds") < 0);
    SLANG_CHECK(assembly.indexOf("load i32, i32 addrspace(1)*") >= 0);
    SLANG_CHECK(assembly.indexOf("store i32") >= 0 && assembly.indexOf("i32 addrspace(1)*") >= 0);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("align 4")) == 2);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("ret void")) == 1);
    SLANG_CHECK(assembly.indexOf("@copyIndexed, !\"kernel\", i32 1") >= 0);

    static const uint8_t kBitcodeMagic[] = {0x42, 0x43, 0xc0, 0xde};
    SLANG_CHECK(bitcodeBlob->getBufferSize() > SLANG_COUNT_OF(kBitcodeMagic));
    SLANG_CHECK(
        ::memcmp(bitcodeBlob->getBufferPointer(), kBitcodeMagic, SLANG_COUNT_OF(kBitcodeMagic)) ==
        0);
}

SLANG_UNIT_TEST(nvvmIRBuilderBuildsArrayElementKernel)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    SLANG_CHECK_ABORT(builder.supportsScalarArrayAddressing());

    ComPtr<ISlangBlob> assemblyBlob;
    ComPtr<ISlangBlob> bitcodeBlob;
    String assemblyDiagnostics = "stale assembly diagnostics";
    String bitcodeDiagnostics = "stale bitcode diagnostics";
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_buildArrayElementModule(
        builder,
        assemblyBlob,
        assemblyDiagnostics,
        bitcodeBlob,
        bitcodeDiagnostics)));
    SLANG_CHECK_ABORT(assemblyBlob != nullptr);
    SLANG_CHECK_ABORT(bitcodeBlob != nullptr);
    SLANG_CHECK(assemblyDiagnostics.getLength() == 0);
    SLANG_CHECK(bitcodeDiagnostics.getLength() == 0);

    const String assembly = _getBlobText(assemblyBlob);
    SLANG_CHECK(
        assembly.indexOf(
            "define void @copyArrayElement([4 x i32] addrspace(1)* %slangParameter0, [4 x i32] "
            "addrspace(1)* %slangParameter1, i32 %slangParameter2)") >= 0);
    SLANG_CHECK(
        _countOccurrences(assembly.getUnownedSlice(), toSlice("getelementptr [4 x i32]")) == 2);
    SLANG_CHECK(
        _countOccurrences(assembly.getUnownedSlice(), toSlice("i32 0, i32 %slangParameter2")) == 2);
    SLANG_CHECK(assembly.indexOf("getelementptr inbounds") < 0);
    SLANG_CHECK(assembly.indexOf("addrspacecast") < 0);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("load i32")) == 1);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("store i32")) == 1);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("align 4")) == 2);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("ret void")) == 1);
    SLANG_CHECK(assembly.indexOf("@copyArrayElement, !\"kernel\", i32 1") >= 0);

    static const uint8_t kBitcodeMagic[] = {0x42, 0x43, 0xc0, 0xde};
    SLANG_CHECK(bitcodeBlob->getBufferSize() > SLANG_COUNT_OF(kBitcodeMagic));
    SLANG_CHECK(
        ::memcmp(bitcodeBlob->getBufferPointer(), kBitcodeMagic, SLANG_COUNT_OF(kBitcodeMagic)) ==
        0);
}

static void _runNVVMScalarBuilderKernel(
    UnitTestContext* unitTestContext,
    NVVMScalarTestOperation operation)
{
    const NVVMScalarTestCase& testCase = _getNVVMScalarTestCase(operation);
    const bool isCompare = testCase.key.family == FakeNVVMBuilderScalarFamily::Compare;

    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    SLANG_CHECK_ABORT(_supportsNVVMScalarBuilderOperation(builder, operation));

    ComPtr<ISlangBlob> assemblyBlob;
    ComPtr<ISlangBlob> bitcodeBlob;
    String assemblyDiagnostics = "stale assembly diagnostics";
    String bitcodeDiagnostics = "stale bitcode diagnostics";
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_buildNVVMScalarTestModule(
        builder,
        operation,
        assemblyBlob,
        assemblyDiagnostics,
        bitcodeBlob,
        bitcodeDiagnostics)));
    SLANG_CHECK_ABORT(assemblyBlob != nullptr);
    SLANG_CHECK_ABORT(bitcodeBlob != nullptr);
    SLANG_CHECK(assemblyDiagnostics.getLength() == 0);
    SLANG_CHECK(bitcodeDiagnostics.getLength() == 0);

    const String assembly = _getBlobText(assemblyBlob);
    StringBuilder instruction32;
    instruction32 << testCase.llvmOpcode << " i32";
    StringBuilder instruction64;
    instruction64 << testCase.llvmOpcode << " i64";
    SLANG_CHECK(
        _countOccurrences(assembly.getUnownedSlice(), instruction32.getUnownedSlice()) == 1);
    SLANG_CHECK(
        _countOccurrences(assembly.getUnownedSlice(), instruction64.getUnownedSlice()) == 0);
    SLANG_CHECK(
        _countOccurrences(assembly.getUnownedSlice(), toSlice("store i32")) == (isCompare ? 2 : 1));
    SLANG_CHECK(
        _countOccurrences(assembly.getUnownedSlice(), toSlice("br i1")) == (isCompare ? 1 : 0));
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("ret void")) == 1);
    StringBuilder kernelMetadata;
    kernelMetadata << "@" << testCase.kernelName << ", !\"kernel\", i32 1";
    SLANG_CHECK(assembly.indexOf(kernelMetadata.getUnownedSlice()) >= 0);

    const Index operationIndex = assembly.indexOf(instruction32.getUnownedSlice());
    SLANG_CHECK_ABORT(operationIndex >= 0);
    if (!isCompare)
    {
        const Index storeIndex = assembly.indexOf("store i32");
        SLANG_CHECK(storeIndex > operationIndex);
        SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("align 4")) == 1);
    }
    if (operation == NVVMScalarTestOperation::BitNot)
    {
        SLANG_CHECK(assembly.indexOf("-1") >= 0);
    }
    if (operation == NVVMScalarTestOperation::Negate)
    {
        SLANG_CHECK(assembly.indexOf("sub i32 0, %slangParameter1") >= 0);
        SLANG_CHECK(assembly.indexOf("sub nsw") < 0);
        SLANG_CHECK(assembly.indexOf("sub nuw") < 0);
    }

    static const uint8_t kBitcodeMagic[] = {0x42, 0x43, 0xc0, 0xde};
    SLANG_CHECK(bitcodeBlob->getBufferSize() > SLANG_COUNT_OF(kBitcodeMagic));
    SLANG_CHECK(
        ::memcmp(bitcodeBlob->getBufferPointer(), kBitcodeMagic, SLANG_COUNT_OF(kBitcodeMagic)) ==
        0);
}

#define NVVM_SCALAR_BUILDER_KERNEL_TEST(NAME, OPERATION)                                  \
    SLANG_UNIT_TEST(NAME)                                                                 \
    {                                                                                     \
        _runNVVMScalarBuilderKernel(unitTestContext, NVVMScalarTestOperation::OPERATION); \
    }

NVVM_SCALAR_BUILDER_KERNEL_TEST(nvvmIRBuilderBuildsIntegerMultiplyKernel, Multiply)
NVVM_SCALAR_BUILDER_KERNEL_TEST(nvvmIRBuilderBuildsIntegerBitAndKernel, BitAnd)
NVVM_SCALAR_BUILDER_KERNEL_TEST(nvvmIRBuilderBuildsIntegerBitOrKernel, BitOr)
NVVM_SCALAR_BUILDER_KERNEL_TEST(nvvmIRBuilderBuildsIntegerBitXorKernel, BitXor)
NVVM_SCALAR_BUILDER_KERNEL_TEST(nvvmIRBuilderBuildsIntegerBitNotKernel, BitNot)
NVVM_SCALAR_BUILDER_KERNEL_TEST(nvvmIRBuilderBuildsIntegerNegateKernel, Negate)
SLANG_UNIT_TEST(nvvmIRBuilderBuildsRelaxedGlobalI32AtomicAddKernel)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    SLANG_CHECK_ABORT(builder.supportsRelaxedGlobalI32AtomicAdd());

    ComPtr<ISlangBlob> assemblyBlob;
    ComPtr<ISlangBlob> bitcodeBlob;
    String assemblyDiagnostics = "stale assembly diagnostics";
    String bitcodeDiagnostics = "stale bitcode diagnostics";
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_buildRelaxedGlobalI32AtomicAddModule(
        builder,
        assemblyBlob,
        assemblyDiagnostics,
        bitcodeBlob,
        bitcodeDiagnostics)));
    SLANG_CHECK_ABORT(assemblyBlob != nullptr);
    SLANG_CHECK_ABORT(bitcodeBlob != nullptr);
    SLANG_CHECK(assemblyDiagnostics.getLength() == 0);
    SLANG_CHECK(bitcodeDiagnostics.getLength() == 0);

    const String assembly = _getBlobText(assemblyBlob);
    const UnownedStringSlice assemblySlice = assembly.getUnownedSlice();
    SLANG_CHECK(
        assembly.indexOf(
            "define void @relaxedGlobalI32AtomicAdd(i32 addrspace(1)* %slangParameter0, i32 "
            "addrspace(1)* %slangParameter1, i32 %slangParameter2)") >= 0);
    SLANG_CHECK(_countOccurrences(assemblySlice, toSlice("atomicrmw add i32 addrspace(1)*")) == 1);
    SLANG_CHECK(_countOccurrences(assemblySlice, toSlice("atomicrmw add i64")) == 0);
    SLANG_CHECK(_countOccurrences(assemblySlice, toSlice("monotonic")) == 1);
    SLANG_CHECK(assembly.indexOf("syncscope(") < 0);
    SLANG_CHECK(assembly.indexOf("acquire") < 0);
    SLANG_CHECK(assembly.indexOf("release") < 0);
    SLANG_CHECK(assembly.indexOf("seq_cst") < 0);
    SLANG_CHECK(_countOccurrences(assemblySlice, toSlice("store i32")) == 1);
    SLANG_CHECK(_countOccurrences(assemblySlice, toSlice("align 4")) == 1);
    SLANG_CHECK(assembly.indexOf("monotonic, align") < 0);
    SLANG_CHECK(_countOccurrences(assemblySlice, toSlice("ret void")) == 1);
    SLANG_CHECK(assembly.indexOf("@relaxedGlobalI32AtomicAdd, !\"kernel\", i32 1") >= 0);
    const Index atomicIndex = assembly.indexOf("atomicrmw add i32 addrspace(1)*");
    const Index storeIndex = assembly.indexOf("store i32");
    SLANG_CHECK_ABORT(atomicIndex >= 0);
    SLANG_CHECK(storeIndex > atomicIndex);

    static const uint8_t kBitcodeMagic[] = {0x42, 0x43, 0xc0, 0xde};
    SLANG_CHECK(bitcodeBlob->getBufferSize() > SLANG_COUNT_OF(kBitcodeMagic));
    SLANG_CHECK(
        ::memcmp(bitcodeBlob->getBufferPointer(), kBitcodeMagic, SLANG_COUNT_OF(kBitcodeMagic)) ==
        0);
}

NVVM_SCALAR_BUILDER_KERNEL_TEST(nvvmIRBuilderBuildsIntegerEqualKernel, Equal)
NVVM_SCALAR_BUILDER_KERNEL_TEST(nvvmIRBuilderBuildsIntegerNotEqualKernel, NotEqual)
NVVM_SCALAR_BUILDER_KERNEL_TEST(
    nvvmIRBuilderBuildsIntegerSignedGreaterThanKernel,
    SignedGreaterThan)
NVVM_SCALAR_BUILDER_KERNEL_TEST(nvvmIRBuilderBuildsIntegerSignedLessEqualKernel, SignedLessEqual)
NVVM_SCALAR_BUILDER_KERNEL_TEST(
    nvvmIRBuilderBuildsIntegerSignedGreaterEqualKernel,
    SignedGreaterEqual)

#undef NVVM_SCALAR_BUILDER_KERNEL_TEST
SLANG_UNIT_TEST(nvvmIRBuilderDifferentialScalarPTX)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    SLANG_CHECK_ABORT(builder.supportsScalarOperations());

    ComPtr<ISlangBlob> assemblyBlob;
    ComPtr<ISlangBlob> bitcodeBlob;
    String assemblyDiagnostics;
    String bitcodeDiagnostics;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_buildScalarReferenceModule(
        builder,
        assemblyBlob,
        assemblyDiagnostics,
        bitcodeBlob,
        bitcodeDiagnostics)));
    SLANG_CHECK_ABORT(bitcodeBlob != nullptr);
    SLANG_CHECK(assemblyDiagnostics.getLength() == 0);
    SLANG_CHECK(bitcodeDiagnostics.getLength() == 0);

    ComPtr<IArtifact> nvvmArtifact;
    const SlangResult nvvmResult = _compileRealNVVMBitcode(
        String(),
        bitcodeBlob->getBufferPointer(),
        bitcodeBlob->getBufferSize(),
        nvvmArtifact);
    if (nvvmResult == SLANG_E_NOT_FOUND)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring scalar PTX differential test because libNVVM was not found.");
        SLANG_IGNORE_TEST;
    }
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(nvvmResult));
    SLANG_CHECK_ABORT(nvvmArtifact != nullptr);

    ComPtr<IArtifact> nvrtcArtifact;
    const SlangResult nvrtcResult =
        _compileRealNVRTCSource(toSlice(kScalarReferenceCUDASource), nvrtcArtifact);
    if (nvrtcResult == SLANG_E_NOT_FOUND)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring scalar PTX differential test because NVRTC was not found.");
        SLANG_IGNORE_TEST;
    }
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(nvrtcResult));
    SLANG_CHECK_ABORT(nvrtcArtifact != nullptr);

    String nvvmPTX;
    String nvrtcPTX;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_loadPTXText(nvvmArtifact, nvvmPTX)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_loadPTXText(nvrtcArtifact, nvrtcPTX)));
    SLANG_CHECK(nvvmPTX.indexOf(".address_size 64") >= 0);
    SLANG_CHECK(nvrtcPTX.indexOf(".address_size 64") >= 0);

    PTXEntrySummary nvvmWrite;
    PTXEntrySummary nvvmCopy;
    PTXEntrySummary nvrtcWrite;
    PTXEntrySummary nvrtcCopy;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        _summarizePTXEntry(nvvmPTX.getUnownedSlice(), toSlice(kWriteScalarKernelName), nvvmWrite)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        _summarizePTXEntry(nvvmPTX.getUnownedSlice(), toSlice(kCopyScalarKernelName), nvvmCopy)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_summarizePTXEntry(
        nvrtcPTX.getUnownedSlice(),
        toSlice(kWriteScalarKernelName),
        nvrtcWrite)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        _summarizePTXEntry(nvrtcPTX.getUnownedSlice(), toSlice(kCopyScalarKernelName), nvrtcCopy)));

    static const uint32_t kWriteParameterWidths[] = {64, 32};
    static const uint32_t kCopyParameterWidths[] = {64, 64};
    SLANG_CHECK(_hasPTXParameterWidths(
        nvvmWrite,
        kWriteParameterWidths,
        SLANG_COUNT_OF(kWriteParameterWidths)));
    SLANG_CHECK(_hasPTXParameterWidths(
        nvrtcWrite,
        kWriteParameterWidths,
        SLANG_COUNT_OF(kWriteParameterWidths)));
    SLANG_CHECK(_hasPTXParameterWidths(
        nvvmCopy,
        kCopyParameterWidths,
        SLANG_COUNT_OF(kCopyParameterWidths)));
    SLANG_CHECK(_hasPTXParameterWidths(
        nvrtcCopy,
        kCopyParameterWidths,
        SLANG_COUNT_OF(kCopyParameterWidths)));
    SLANG_CHECK(_haveEqualPTXParameterWidths(nvvmWrite, nvrtcWrite));
    SLANG_CHECK(_haveEqualPTXParameterWidths(nvvmCopy, nvrtcCopy));

    SLANG_CHECK(nvvmWrite.hasGlobalStore32);
    SLANG_CHECK(nvrtcWrite.hasGlobalStore32);
    SLANG_CHECK(nvvmCopy.hasGlobalLoad32);
    SLANG_CHECK(nvvmCopy.hasGlobalStore32);
    SLANG_CHECK(nvrtcCopy.hasGlobalLoad32);
    SLANG_CHECK(nvrtcCopy.hasGlobalStore32);
}

SLANG_UNIT_TEST(nvvmIRBuilderCompilesScalarBitcodeThroughRegistry)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    SLANG_CHECK_ABORT(builder.supportsScalarOperations());

    ComPtr<ISlangBlob> assemblyBlob;
    ComPtr<ISlangBlob> bitcodeBlob;
    String assemblyDiagnostics;
    String bitcodeDiagnostics;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_buildScalarReferenceModule(
        builder,
        assemblyBlob,
        assemblyDiagnostics,
        bitcodeBlob,
        bitcodeDiagnostics)));
    SLANG_CHECK_ABORT(bitcodeBlob != nullptr);
    SLANG_CHECK(assemblyDiagnostics.getLength() == 0);
    SLANG_CHECK(bitcodeDiagnostics.getLength() == 0);

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    auto session = static_cast<Slang::Session*>(globalSession.get());
    if (SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVVM)))
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring registered NVVM handoff test because no CUDA toolkit was discovered.");
        SLANG_IGNORE_TEST;
    }
    IDownstreamCompiler* compiler = session->m_downstreamCompilers[int(PassThroughMode::NVVM)];
    SLANG_CHECK_ABORT(compiler != nullptr);
    SLANG_CHECK_ABORT(compiler->getDesc().type == SLANG_PASS_THROUGH_NVVM);

    ComPtr<IArtifact> sourceArtifact =
        _createNVVMBitcodeArtifact(bitcodeBlob->getBufferPointer(), bitcodeBlob->getBufferSize());
    ComPtr<IArtifact> outputArtifact;
    CompileSettings settings;
    const SlangResult compileResult =
        _compileNVVM(compiler, sourceArtifact, settings, outputArtifact.writeRef());
    IArtifactDiagnostics* diagnostics = _findDiagnostics(outputArtifact);
    if (SLANG_FAILED(compileResult) || !diagnostics || SLANG_FAILED(diagnostics->getResult()))
        _reportArtifactDiagnostics(outputArtifact);

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(compileResult));
    SLANG_CHECK_ABORT(outputArtifact != nullptr);
    SLANG_CHECK(
        outputArtifact->getDesc() ==
        ArtifactDesc::make(ArtifactKind::ObjectCode, ArtifactPayload::PTX, ArtifactStyle::Kernel));
    SLANG_CHECK_ABORT(diagnostics != nullptr);
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(diagnostics->getResult()));
    SLANG_CHECK(_ptxContainsEntry(outputArtifact, toSlice(kWriteScalarKernelName)));
    SLANG_CHECK(_ptxContainsEntry(outputArtifact, toSlice(kCopyScalarKernelName)));
}

SLANG_UNIT_TEST(nvvmIRBuilderCompilesEmptyKernel)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);

    static const char kKernelName[] = "slangSlice3aCompiledEmpty";
    ComPtr<ISlangBlob> assemblyBlob;
    ComPtr<ISlangBlob> bitcodeBlob;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        _buildEmptyNVVMKernel(builder, toSlice(kKernelName), assemblyBlob, bitcodeBlob)));

    ComPtr<IArtifact> outputArtifact;
    const SlangResult compileResult = _compileRealNVVMBitcode(
        String(),
        bitcodeBlob->getBufferPointer(),
        bitcodeBlob->getBufferSize(),
        outputArtifact);
    if (compileResult == SLANG_E_NOT_FOUND)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring generated-bitcode compile test because no CUDA toolkit was discovered.");
        SLANG_IGNORE_TEST;
    }
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(compileResult));
    SLANG_CHECK_ABORT(outputArtifact != nullptr);
    SLANG_CHECK(
        outputArtifact->getDesc() ==
        ArtifactDesc::make(ArtifactKind::ObjectCode, ArtifactPayload::PTX, ArtifactStyle::Kernel));
    SLANG_CHECK(_ptxContainsEntry(outputArtifact, toSlice(kKernelName)));
}

SLANG_UNIT_TEST(nvvmIRBuilderCoexistsWithLLVM21)
{
    StringBuilder childOrderBuilder;
    if (SLANG_SUCCEEDED(PlatformUtil::getEnvironmentVariable(
            toSlice(kNVVMCoexistenceChildEnv),
            childOrderBuilder)) &&
        childOrderBuilder.getLength())
    {
        const String childOrder = childOrderBuilder.produceString();
        NVVMLLVMLoadOrder order = NVVMLLVMLoadOrder::LLVMFirst;
        if (childOrder == "llvm-first")
            order = NVVMLLVMLoadOrder::LLVMFirst;
        else if (childOrder == "nvvm-first")
            order = NVVMLLVMLoadOrder::NVVMFirst;
        else
        {
            getTestReporter()->message(
                TestMessageType::TestFailure,
                "Unknown NVVM/LLVM coexistence child order.");
            SLANG_CHECK_ABORT(false);
        }

        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_exerciseNVVMLLVMCoexistence(unitTestContext, order)));
        return;
    }

    // Establish availability in the parent so dependency absence ignores this test instead of
    // becoming an apparently successful ignored child. Loading here cannot affect either worker:
    // each child invocation below executes the probe in its own fully isolated test-server.
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);
    const SlangResult llvmResult = _queryLLVM21(unitTestContext);
    if (llvmResult == SLANG_E_NOT_FOUND)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring LLVM coexistence test because slang-llvm was not found.");
        SLANG_IGNORE_TEST;
    }
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(llvmResult));

    struct OrderCase
    {
        const char* name;
        const char* environmentValue;
    };
    static const OrderCase kOrders[] = {
        {"LLVM 21 then LLVM 14 NVVM", "llvm-first"},
        {"LLVM 14 NVVM then LLVM 21", "nvvm-first"},
    };

    for (const auto& order : kOrders)
    {
        CommandLine commandLine;
        commandLine.setExecutableLocation(
            ExecutableLocation(unitTestContext->executableDirectory, "slang-test"));
        commandLine.addArg("-use-fully-isolated-test-server");
        commandLine.addArg("-server-count");
        commandLine.addArg("1");
        commandLine.addArg("-disable-retries");
        commandLine.addArg("-skip-api-detection");
        commandLine.addArg(kNVVMCoexistenceTestName);

        ExecuteResult childResult;
        childResult.init();
        SlangResult executeResult = SLANG_FAIL;
        {
            SlangUnitTest::ScopedEnvVar childOrder(
                kNVVMCoexistenceChildEnv,
                order.environmentValue);
            executeResult = ProcessUtil::execute(commandLine, childResult);
        }
        const bool reportedOnePassingTest =
            childResult.standardOutput.indexOf("100% of tests passed (1/1)") >= 0;
        if (SLANG_FAILED(executeResult) || childResult.resultCode != 0 || !reportedOnePassingTest)
        {
            _reportCoexistenceChildFailure(order.name, executeResult, childResult);
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(executeResult));
        SLANG_CHECK(childResult.resultCode == 0);
        SLANG_CHECK(reportedOnePassingTest);
    }
}
