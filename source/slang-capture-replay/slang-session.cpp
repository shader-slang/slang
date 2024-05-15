#include <cassert>
#include "capture_utility.h"
#include "slang-session.h"
#include "slang-entrypoint.h"
#include "slang-composite-component-type.h"
#include "slang-type-conformance.h"

namespace SlangCapture
{

    SessionCapture::SessionCapture(slang::ISession* session)
        : m_actualSession(session)
    {
        assert(m_actualSession);
        slangCaptureLog(LogLevel::Verbose, "%s: %p\n", "SessionCapture create:", session);
    }

    SessionCapture::~SessionCapture()
    {
        m_actualSession->release();
    }

    ISlangUnknown* SessionCapture::getInterface(const Guid& guid)
    {
        if(guid == ISlangUnknown::getTypeGuid() || guid == ISession::getTypeGuid())
            return asExternal(this);

        return nullptr;
    }

    SLANG_NO_THROW slang::IGlobalSession* SessionCapture::getGlobalSession()
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);
        slang::IGlobalSession* pGlobalSession = m_actualSession->getGlobalSession();
        return pGlobalSession;
    }

    SLANG_NO_THROW slang::IModule* SessionCapture::loadModule(
        const char* moduleName,
        slang::IBlob**     outDiagnostics)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);
        slang::IModule* pModule = m_actualSession->loadModule(moduleName, outDiagnostics);
        ModuleCapture* pModuleCapture = getModuleCapture(pModule);
        return static_cast<slang::IModule*>(pModuleCapture);
    }

    SLANG_NO_THROW slang::IModule* SessionCapture::loadModuleFromIRBlob(
        const char* moduleName,
        const char* path,
        slang::IBlob* source,
        slang::IBlob** outDiagnostics)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);
        slang::IModule* pModule = m_actualSession->loadModuleFromIRBlob(moduleName, path, source, outDiagnostics);
        ModuleCapture* pModuleCapture = getModuleCapture(pModule);
        return static_cast<slang::IModule*>(pModuleCapture);
    }

    SLANG_NO_THROW slang::IModule* SessionCapture::loadModuleFromSource(
        const char* moduleName,
        const char* path,
        slang::IBlob* source,
        slang::IBlob** outDiagnostics)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);
        slang::IModule* pModule = m_actualSession->loadModuleFromSource(moduleName, path, source, outDiagnostics);
        ModuleCapture* pModuleCapture = getModuleCapture(pModule);
        return static_cast<slang::IModule*>(pModuleCapture);
    }

    SLANG_NO_THROW slang::IModule* SessionCapture::loadModuleFromSourceString(
        const char* moduleName,
        const char* path,
        const char* string,
        slang::IBlob** outDiagnostics)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);
        slang::IModule* pModule = m_actualSession->loadModuleFromSourceString(moduleName, path, string, outDiagnostics);
        ModuleCapture* pModuleCapture = getModuleCapture(pModule);
        return static_cast<slang::IModule*>(pModuleCapture);
    }

    SLANG_NO_THROW SlangResult SessionCapture::createCompositeComponentType(
        slang::IComponentType* const*   componentTypes,
        SlangInt                        componentTypeCount,
        slang::IComponentType**         outCompositeComponentType,
        ISlangBlob**                    outDiagnostics)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        Slang::List<slang::IComponentType*> componentTypeList;

        // get the actual component types from our capture wrappers
        assert(SLANG_OK == getActualComponentTypes(componentTypes, componentTypeCount, componentTypeList));

        slang::IComponentType* compositeComponentType = nullptr;
        SlangResult result = m_actualSession->createCompositeComponentType(
                componentTypeList.getBuffer(), componentTypeCount, &compositeComponentType, outDiagnostics);

        CompositeComponentTypeCapture* compositeComponentTypeCapture = new CompositeComponentTypeCapture(compositeComponentType);
        Slang::ComPtr<CompositeComponentTypeCapture> resultCapture(compositeComponentTypeCapture);
        *outCompositeComponentType = resultCapture.detach();
        return result;
    }

    SLANG_NO_THROW slang::TypeReflection* SessionCapture::specializeType(
        slang::TypeReflection*          type,
        slang::SpecializationArg const* specializationArgs,
        SlangInt                        specializationArgCount,
        ISlangBlob**                    outDiagnostics)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);
        slang::TypeReflection* pTypeReflection = m_actualSession->specializeType(type, specializationArgs, specializationArgCount, outDiagnostics);
        return pTypeReflection;
    }

    SLANG_NO_THROW slang::TypeLayoutReflection* SessionCapture::getTypeLayout(
        slang::TypeReflection* type,
        SlangInt               targetIndex,
        slang::LayoutRules     rules,
        ISlangBlob**    outDiagnostics)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);
        slang::TypeLayoutReflection* pTypeLayoutReflection = m_actualSession->getTypeLayout(type, targetIndex, rules, outDiagnostics);
        return pTypeLayoutReflection;
    }

    SLANG_NO_THROW slang::TypeReflection* SessionCapture::getContainerType(
        slang::TypeReflection* elementType,
        slang::ContainerType containerType,
        ISlangBlob** outDiagnostics)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);
        slang::TypeReflection* pTypeReflection = m_actualSession->getContainerType(elementType, containerType, outDiagnostics);
        return pTypeReflection;
    }

    SLANG_NO_THROW slang::TypeReflection* SessionCapture::getDynamicType()
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);
        slang::TypeReflection* pTypeReflection = m_actualSession->getDynamicType();
        return pTypeReflection;
    }

    SLANG_NO_THROW SlangResult SessionCapture::getTypeRTTIMangledName(
        slang::TypeReflection* type,
        ISlangBlob** outNameBlob)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);
        SlangResult result = m_actualSession->getTypeRTTIMangledName(type, outNameBlob);
        return result;
    }

    SLANG_NO_THROW SlangResult SessionCapture::getTypeConformanceWitnessMangledName(
        slang::TypeReflection* type,
        slang::TypeReflection* interfaceType,
        ISlangBlob** outNameBlob)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);
        SlangResult result = m_actualSession->getTypeConformanceWitnessMangledName(type, interfaceType, outNameBlob);
        return result;
    }

    SLANG_NO_THROW SlangResult SessionCapture::getTypeConformanceWitnessSequentialID(
        slang::TypeReflection* type,
        slang::TypeReflection* interfaceType,
        uint32_t*              outId)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);
        SlangResult result = m_actualSession->getTypeConformanceWitnessSequentialID(type, interfaceType, outId);
        return result;
    }

    SLANG_NO_THROW SlangResult SessionCapture::createTypeConformanceComponentType(
        slang::TypeReflection* type,
        slang::TypeReflection* interfaceType,
        slang::ITypeConformance** outConformance,
        SlangInt conformanceIdOverride,
        ISlangBlob** outDiagnostics)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        slang::ITypeConformance* pConformance = nullptr;
        SlangResult result = m_actualSession->createTypeConformanceComponentType(type, interfaceType, &pConformance, conformanceIdOverride, outDiagnostics);

        TypeConformanceCapture* conformanceCapture = new TypeConformanceCapture(pConformance);
        Slang::ComPtr<TypeConformanceCapture> resultCapture(conformanceCapture);
        *outConformance = resultCapture.detach();

        return result;
    }

    SLANG_NO_THROW SlangResult SessionCapture::createCompileRequest(
        SlangCompileRequest**   outCompileRequest)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);
        SlangResult result = m_actualSession->createCompileRequest(outCompileRequest);
        return result;
    }

    SLANG_NO_THROW SlangInt SessionCapture::getLoadedModuleCount()
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);
        SlangInt count = m_actualSession->getLoadedModuleCount();
        return count;
    }

    SLANG_NO_THROW slang::IModule* SessionCapture::getLoadedModule(SlangInt index)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);
        slang::IModule* pModule = m_actualSession->getLoadedModule(index);

        if (pModule)
        {
            ModuleCapture* moduleCapture = m_mapModuleToCapture.tryGetValue(pModule);
            if (!moduleCapture)
            {
                assert(!"Module not found in mapModuleToCapture");
            }
            return static_cast<slang::IModule*>(moduleCapture);
        }

        return pModule;
    }

    SLANG_NO_THROW bool SessionCapture::isBinaryModuleUpToDate(const char* modulePath, slang::IBlob* binaryModuleBlob)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);
        bool result = m_actualSession->isBinaryModuleUpToDate(modulePath, binaryModuleBlob);
        return result;
    }

    ModuleCapture* SessionCapture::getModuleCapture(slang::IModule* module)
    {
        ModuleCapture* moduleCapture = nullptr;
        moduleCapture = m_mapModuleToCapture.tryGetValue(module);
        if (!moduleCapture)
        {
            moduleCapture = new ModuleCapture(module);
            Slang::ComPtr<ModuleCapture> result(moduleCapture);
            m_mapModuleToCapture.add(module, *result.detach());
        }
        return moduleCapture;
    }

    SlangResult SessionCapture::getActualComponentTypes(
            slang::IComponentType* const*   componentTypes,
            SlangInt                        componentTypeCount,
            List<slang::IComponentType*>&   outActualComponentTypes)
    {
        for (SlangInt i = 0; i < componentTypeCount; i++)
        {
            slang::IComponentType* const& componentType = componentTypes[i];
            void* outObj = nullptr;

            if (componentType->queryInterface(ModuleCapture::getTypeGuid(), &outObj) == SLANG_OK)
            {
                ModuleCapture* moduleCapture = static_cast<ModuleCapture*>(outObj);
                outActualComponentTypes.add(moduleCapture->getActualModule());
            }
            else if (componentType->queryInterface(EntryPointCapture::getTypeGuid(), &outObj) == SLANG_OK)
            {
                EntryPointCapture* entrypointCapture = static_cast<EntryPointCapture*>(outObj);
                outActualComponentTypes.add(entrypointCapture->getActualEntryPoint());
            }
            // will fall back to the actual component type, it means that we didn't capture this type.
            else
            {
                outActualComponentTypes.add(componentType);
            }
        }

        if (componentTypeCount == outActualComponentTypes.getCount())
        {
            return SLANG_OK;
        }
        return SLANG_FAIL;
    }
} // namespace SlangCapture
