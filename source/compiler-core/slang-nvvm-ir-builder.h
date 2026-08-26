#ifndef SLANG_NVVM_IR_BUILDER_H
#define SLANG_NVVM_IR_BUILDER_H

#include "core/slang-shared-library.h"
#include "slang-com-ptr.h"
#include "slang-nvvm-ir-builder-api.h"

namespace Slang
{

/// Owns and validates the optional LLVM 14 NVVM builder ABI.
///
/// This wrapper and its retained provider library must outlive every module handle created through
/// it. Individual modules are thread-confined by the underlying ABI.
class NVVMIRBuilder
{
public:
    /// Loads `slang-llvm-nvvm` from an explicit path or by its logical name.
    static SlangResult load(
        const String& path,
        ISlangSharedLibraryLoader* loader,
        NVVMIRBuilder& outBuilder);

    /// Validates an already queried API table and retains its non-null owning library.
    static SlangResult initialize(
        const SlangNVVMBuilderAPI_V1& api,
        ISlangSharedLibrary* library,
        NVVMIRBuilder& outBuilder);

    /// Validates the V2 diagnostic table and its nested V1 API, then retains its owning library.
    static SlangResult initialize(
        const SlangNVVMBuilderAPI_V2& api,
        ISlangSharedLibrary* library,
        NVVMIRBuilder& outBuilder);

    bool isInitialized() const { return m_api.createModule != nullptr; }
    bool supportsSerializationDiagnostics() const
    {
        return m_apiV2.serializeModuleWithDiagnostics != nullptr;
    }
    const SlangNVVMBuilderAPI_V1& getAPI() const { return m_api; }
    /// Returns the locally supported V2 prefix, with `structureSize` clamped to that prefix.
    const SlangNVVMBuilderAPI_V2* getAPIV2() const
    {
        return supportsSerializationDiagnostics() ? &m_apiV2 : nullptr;
    }

    /// Creates a module whose LLVM objects remain owned by the returned module handle.
    SlangResult createModule(
        const UnownedStringSlice& moduleName,
        SlangNVVMModuleHandle_1& outModule) const;

    /// Destroys a module and every opaque handle created from it.
    void destroyModule(SlangNVVMModuleHandle_1 module) const;

    /// Gets the module's context-owned void type.
    SlangResult getVoidType(SlangNVVMModuleHandle_1 module, SlangNVVMTypeHandle_1& outType) const;

    /// Creates a non-variadic function type from module-owned type handles.
    SlangResult getFunctionType(
        SlangNVVMModuleHandle_1 module,
        SlangNVVMTypeHandle_1 resultType,
        const SlangNVVMTypeHandle_1* parameterTypes,
        size_t parameterCount,
        SlangNVVMTypeHandle_1& outType) const;

    /// Declares a function in the module with the exact caller-provided name.
    SlangResult declareFunction(
        SlangNVVMModuleHandle_1 module,
        SlangNVVMTypeHandle_1 functionType,
        const UnownedStringSlice& name,
        SlangNVVMValueHandle_1& outFunction) const;

    /// Appends a basic block to a function owned by the module.
    SlangResult createBlock(
        SlangNVVMModuleHandle_1 module,
        SlangNVVMValueHandle_1 function,
        const UnownedStringSlice& name,
        SlangNVVMBlockHandle_1& outBlock) const;

    /// Selects a module-owned block as the destination for subsequent instructions.
    SlangResult setInsertBlock(SlangNVVMModuleHandle_1 module, SlangNVVMBlockHandle_1 block) const;

    /// Terminates the current void-returning insertion block.
    SlangResult emitReturnVoid(SlangNVVMModuleHandle_1 module) const;

    /// Adds the NVVM kernel annotation for a module-owned function.
    SlangResult markFunctionAsKernel(
        SlangNVVMModuleHandle_1 module,
        SlangNVVMValueHandle_1 function) const;

    /// Serializes into a host-owned blob using the ABI's size-query/write protocol.
    SlangResult serializeModule(
        SlangNVVMModuleHandle_1 module,
        SlangNVVMSerializationFormat_1 format,
        ComPtr<ISlangBlob>& outBlob) const;

    /// Serializes through V2 and copies the corresponding verifier bytes into host storage.
    ///
    /// Returns `SLANG_E_NOT_AVAILABLE` for a V1-only provider. LLVM verification failure returns
    /// `SLANG_FAIL`, leaves `outBlob` null, and preserves the verifier text in `outDiagnostics`.
    SlangResult serializeModule(
        SlangNVVMModuleHandle_1 module,
        SlangNVVMSerializationFormat_1 format,
        ComPtr<ISlangBlob>& outBlob,
        String& outDiagnostics) const;

private:
    SlangNVVMBuilderAPI_V1 m_api = {};
    SlangNVVMBuilderAPI_V2 m_apiV2 = {};
    ComPtr<ISlangSharedLibrary> m_library;
};

} // namespace Slang

#endif
