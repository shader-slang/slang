#ifndef SLANG_SESSION_H
#define SLANG_SESSION_H

#include "../../slang-com-ptr.h"
#include "../../slang.h"
#include "../../slang-com-helper.h"
#include "../core/slang-smart-pointer.h"
#include "../slang/slang-compiler.h"

namespace SlangCapture
{
    using namespace Slang;
    class SessionCapture: public RefObject, public slang::ISession
    {
    public:
        SLANG_REF_OBJECT_IUNKNOWN_ALL
        ISlangUnknown* getInterface(const Guid& guid);

        explicit SessionCapture(slang::ISession* session);
        ~SessionCapture();

        SLANG_NO_THROW slang::IGlobalSession* getGlobalSession() override;
        SLANG_NO_THROW slang::IModule* loadModule(
            const char* moduleName,
            slang::IBlob**     outDiagnostics = nullptr) override;
        slang::IModule* loadModuleFromBlob(
            const char* moduleName,
            const char* path,
            slang::IBlob* source,
            ModuleBlobType blobType,
            slang::IBlob** outDiagnostics = nullptr);
        SLANG_NO_THROW slang::IModule* loadModuleFromIRBlob(
            const char* moduleName,
            const char* path,
            slang::IBlob* source,
            slang::IBlob** outDiagnostics = nullptr) override;
        SLANG_NO_THROW slang::IModule* loadModuleFromSource(
            const char* moduleName,
            const char* path,
            slang::IBlob* source,
            slang::IBlob** outDiagnostics = nullptr) override;
        SLANG_NO_THROW slang::IModule* loadModuleFromSourceString(
            const char* moduleName,
            const char* path,
            const char* string,
            slang::IBlob** outDiagnostics = nullptr) override;
        SLANG_NO_THROW SlangResult createCompositeComponentType(
            slang::IComponentType* const*   componentTypes,
            SlangInt                        componentTypeCount,
            slang::IComponentType**         outCompositeComponentType,
            ISlangBlob**                    outDiagnostics = nullptr) override;
        SLANG_NO_THROW slang::TypeReflection* specializeType(
            slang::TypeReflection*          type,
            slang::SpecializationArg const* specializationArgs,
            SlangInt                        specializationArgCount,
            ISlangBlob**                    outDiagnostics = nullptr) override;
        SLANG_NO_THROW slang::TypeLayoutReflection* getTypeLayout(
            slang::TypeReflection* type,
            SlangInt               targetIndex = 0,
            slang::LayoutRules     rules = slang::LayoutRules::Default,
            ISlangBlob**    outDiagnostics = nullptr) override;
        SLANG_NO_THROW slang::TypeReflection* getContainerType(
            slang::TypeReflection* elementType,
            slang::ContainerType containerType,
            ISlangBlob** outDiagnostics = nullptr) override;
        SLANG_NO_THROW slang::TypeReflection* getDynamicType() override;
        SLANG_NO_THROW SlangResult getTypeRTTIMangledName(
            slang::TypeReflection* type,
            ISlangBlob** outNameBlob) override;
        SLANG_NO_THROW SlangResult getTypeConformanceWitnessMangledName(
            slang::TypeReflection* type,
            slang::TypeReflection* interfaceType,
            ISlangBlob** outNameBlob) override;
        SLANG_NO_THROW SlangResult getTypeConformanceWitnessSequentialID(
            slang::TypeReflection* type,
            slang::TypeReflection* interfaceType,
            uint32_t*              outId) override;
        SLANG_NO_THROW SlangResult createTypeConformanceComponentType(
            slang::TypeReflection* type,
            slang::TypeReflection* interfaceType,
            slang::ITypeConformance** outConformance,
            SlangInt conformanceIdOverride,
            ISlangBlob** outDiagnostics) override;
        SLANG_NO_THROW SlangResult createCompileRequest(
            SlangCompileRequest**   outCompileRequest) override;
        SLANG_NO_THROW SlangInt getLoadedModuleCount() override;
        SLANG_NO_THROW slang::IModule* getLoadedModule(SlangInt index) override;
        SLANG_NO_THROW bool isBinaryModuleUpToDate(const char* modulePath, slang::IBlob* binaryModuleBlob) override;

    private:
        SLANG_FORCE_INLINE slang::ISession* asExternal(SessionCapture* session)
        {
            return static_cast<slang::ISession*>(session);
        }
        slang::ISession* m_actualSession = nullptr;
    };
}

#endif // SLANG_SESSION_H
