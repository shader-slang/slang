#ifndef SLANG_NVVM_IR_BUILDER_API_H
#define SLANG_NVVM_IR_BUILDER_API_H

#include <stddef.h>
#include <stdint.h>

#define SLANG_NVVM_BUILDER_ABI_VERSION_1 1u
#define SLANG_NVVM_BUILDER_GET_API_V1_NAME "slang_getNVVMBuilderAPI_V1"

#if defined(_MSC_VER)
#define SLANG_NVVM_CALL __stdcall
#elif defined(_WIN32) && defined(__GNUC__)
#define SLANG_NVVM_CALL __attribute__((stdcall))
#else
#define SLANG_NVVM_CALL
#endif

#if defined(SLANG_NVVM_BUILDER_EXPORTS)
#if defined(_MSC_VER)
#define SLANG_NVVM_BUILDER_API __declspec(dllexport)
#elif defined(_WIN32)
#define SLANG_NVVM_BUILDER_API __attribute__((dllexport)) __attribute__((visibility("default")))
#else
#define SLANG_NVVM_BUILDER_API __attribute__((visibility("default")))
#endif
#else
#define SLANG_NVVM_BUILDER_API
#endif

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct SlangNVVMModule_1* SlangNVVMModuleHandle_1;
    typedef struct SlangNVVMType_1* SlangNVVMTypeHandle_1;
    typedef struct SlangNVVMValue_1* SlangNVVMValueHandle_1;
    typedef struct SlangNVVMBlock_1* SlangNVVMBlockHandle_1;

    /** Uses Slang's signed 32-bit result convention: negative values fail, other values succeed. */
    typedef int32_t SlangNVVMResult_1;

    typedef uint32_t SlangNVVMPointerModel_1;
#define SLANG_NVVM_POINTER_MODEL_TYPED ((SlangNVVMPointerModel_1)1u)

    typedef uint32_t SlangNVVMSerializationFormat_1;
#define SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY ((SlangNVVMSerializationFormat_1)0u)
#define SLANG_NVVM_SERIALIZATION_FORMAT_BITCODE ((SlangNVVMSerializationFormat_1)1u)

    /**
     * Version 1 of the private ABI between Slang and its optional LLVM-backed NVVM IR module.
     *
     * The caller initializes `structureSize` and `abiVersion` before passing this structure to
     * `slang_getNVVMBuilderAPI_V1`. All LLVM objects remain owned by their module and cross the ABI
     * only as opaque handles. `serializeModule` uses a two-call protocol: query the required size
     * with a null destination, then provide caller-owned storage of at least that size.
     *
     * Every non-null handle passed to a function must still be live and must have been returned for
     * the same module. Destroying a module invalidates all of its type, value, and block handles.
     * Calls that mutate or serialize one module are thread-confined and must not run concurrently.
     */
    typedef struct SlangNVVMBuilderAPI_V1
    {
        uint32_t structureSize;
        uint32_t abiVersion;

        uint32_t llvmVersionMajor;
        uint32_t llvmVersionMinor;
        uint32_t llvmVersionPatch;
        uint32_t nvvmIRVersionMajor;
        uint32_t nvvmIRVersionMinor;
        uint32_t pointerModel;

        SlangNVVMResult_1(SLANG_NVVM_CALL* createModule)(
            const char* moduleName,
            size_t moduleNameSize,
            SlangNVVMModuleHandle_1* outModule);
        void(SLANG_NVVM_CALL* destroyModule)(SlangNVVMModuleHandle_1 module);

        SlangNVVMResult_1(SLANG_NVVM_CALL* getVoidType)(
            SlangNVVMModuleHandle_1 module,
            SlangNVVMTypeHandle_1* outType);
        SlangNVVMResult_1(SLANG_NVVM_CALL* getFunctionType)(
            SlangNVVMModuleHandle_1 module,
            SlangNVVMTypeHandle_1 resultType,
            const SlangNVVMTypeHandle_1* parameterTypes,
            size_t parameterCount,
            SlangNVVMTypeHandle_1* outType);
        SlangNVVMResult_1(SLANG_NVVM_CALL* declareFunction)(
            SlangNVVMModuleHandle_1 module,
            SlangNVVMTypeHandle_1 functionType,
            const char* name,
            size_t nameSize,
            SlangNVVMValueHandle_1* outFunction);

        SlangNVVMResult_1(SLANG_NVVM_CALL* createBlock)(
            SlangNVVMModuleHandle_1 module,
            SlangNVVMValueHandle_1 function,
            const char* name,
            size_t nameSize,
            SlangNVVMBlockHandle_1* outBlock);
        SlangNVVMResult_1(SLANG_NVVM_CALL* setInsertBlock)(
            SlangNVVMModuleHandle_1 module,
            SlangNVVMBlockHandle_1 block);
        SlangNVVMResult_1(SLANG_NVVM_CALL* emitReturnVoid)(SlangNVVMModuleHandle_1 module);
        SlangNVVMResult_1(SLANG_NVVM_CALL* markFunctionAsKernel)(
            SlangNVVMModuleHandle_1 module,
            SlangNVVMValueHandle_1 function);

        SlangNVVMResult_1(SLANG_NVVM_CALL* serializeModule)(
            SlangNVVMModuleHandle_1 module,
            SlangNVVMSerializationFormat_1 format,
            void* destination,
            size_t destinationSize,
            size_t* outSerializedSize);
    } SlangNVVMBuilderAPI_V1;

    typedef SlangNVVMResult_1(SLANG_NVVM_CALL* SlangGetNVVMBuilderAPI_V1)(
        SlangNVVMBuilderAPI_V1* outAPI);

    SLANG_NVVM_BUILDER_API SlangNVVMResult_1 SLANG_NVVM_CALL
    slang_getNVVMBuilderAPI_V1(SlangNVVMBuilderAPI_V1* outAPI);

#ifdef __cplusplus
}
#endif

#endif
