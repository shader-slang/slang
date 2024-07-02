#include "capture-utility.h"
#include "slang-session.h"
#include "slang-entrypoint.h"
#include "slang-composite-component-type.h"
#include "slang-type-conformance.h"

namespace SlangCapture
{

    SessionCapture::SessionCapture(slang::ISession* session, CaptureManager* captureManager)
        : m_actualSession(session),
          m_captureManager(captureManager)
    {
        SLANG_CAPTURE_ASSERT(m_actualSession);
        SLANG_CAPTURE_ASSERT(m_captureManager);
        m_sessionHandle = reinterpret_cast<uint64_t>(m_actualSession.get());
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
        // No need to capture this function.
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);
        slang::IGlobalSession* pGlobalSession = m_actualSession->getGlobalSession();
        return pGlobalSession;
    }

    SLANG_NO_THROW slang::IModule* SessionCapture::loadModule(
        const char* moduleName,
        slang::IBlob**     outDiagnostics)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterEncoder* encoder {};
        {
            encoder = m_captureManager->beginMethodCapture(ApiCallId::ISession_loadModule, m_sessionHandle);
            encoder->encodeString(moduleName);
            encoder = m_captureManager->endMethodCapture();
        }

        slang::IModule* pModule = m_actualSession->loadModule(moduleName, outDiagnostics);

        {
            encoder->encodeAddress(*outDiagnostics);
            encoder->encodeAddress(pModule);
            m_captureManager->endMethodCaptureAppendOutput();
        }

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

        ParameterEncoder* encoder {};
        {
            encoder = m_captureManager->beginMethodCapture(ApiCallId::ISession_loadModuleFromIRBlob, m_sessionHandle);
            encoder->encodeString(moduleName);
            encoder->encodeString(path);
            encoder->encodePointer(source);
            encoder = m_captureManager->endMethodCapture();
        }

        slang::IModule* pModule = m_actualSession->loadModuleFromIRBlob(moduleName, path, source, outDiagnostics);

        {
            encoder->encodeAddress(*outDiagnostics);
            encoder->encodeAddress(pModule);
            m_captureManager->endMethodCaptureAppendOutput();
        }

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

        ParameterEncoder* encoder {};
        {
            encoder = m_captureManager->beginMethodCapture(ApiCallId::ISession_loadModuleFromSource, m_sessionHandle);
            encoder->encodeString(moduleName);
            encoder->encodeString(path);
            encoder->encodePointer(source);
            encoder = m_captureManager->endMethodCapture();
        }

        slang::IModule* pModule = m_actualSession->loadModuleFromSource(moduleName, path, source, outDiagnostics);

        {
            encoder->encodeAddress(*outDiagnostics);
            encoder->encodeAddress(pModule);
            m_captureManager->endMethodCaptureAppendOutput();
        }

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

        ParameterEncoder* encoder {};
        {
            encoder = m_captureManager->beginMethodCapture(ApiCallId::ISession_loadModuleFromSourceString, m_sessionHandle);
            encoder->encodeString(moduleName);
            encoder->encodeString(path);
            encoder->encodeString(string);
            encoder = m_captureManager->endMethodCapture();
        }

        slang::IModule* pModule = m_actualSession->loadModuleFromSourceString(moduleName, path, string, outDiagnostics);

        {
            // TODO: Not sure if we need to capture the diagnostics blob.
            encoder->encodeAddress(*outDiagnostics);
            encoder->encodeAddress(pModule);
            m_captureManager->endMethodCaptureAppendOutput();
        }

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
        if(SLANG_OK != getActualComponentTypes(componentTypes, componentTypeCount, componentTypeList))
        {
            SLANG_CAPTURE_ASSERT(!"Failed to get actual component types");
        }

        ParameterEncoder* encoder {};
        {
            encoder = m_captureManager->beginMethodCapture(ApiCallId::ISession_createCompositeComponentType, m_sessionHandle);
            encoder->encodeAddressArray(componentTypeList.getBuffer(), componentTypeCount);
            encoder = m_captureManager->endMethodCapture();
        }

        SlangResult result = m_actualSession->createCompositeComponentType(
                componentTypeList.getBuffer(), componentTypeCount, outCompositeComponentType, outDiagnostics);

        {
            encoder->encodeAddress(*outCompositeComponentType);
            encoder->encodeAddress(*outDiagnostics);
            m_captureManager->endMethodCaptureAppendOutput();
        }

        if (SLANG_OK == result)
        {
            CompositeComponentTypeCapture* compositeComponentTypeCapture =
                new CompositeComponentTypeCapture(*outCompositeComponentType, m_captureManager);
            Slang::ComPtr<CompositeComponentTypeCapture> resultCapture(compositeComponentTypeCapture);
            *outCompositeComponentType = resultCapture.detach();
        }

        return result;
    }

    SLANG_NO_THROW slang::TypeReflection* SessionCapture::specializeType(
        slang::TypeReflection*          type,
        slang::SpecializationArg const* specializationArgs,
        SlangInt                        specializationArgCount,
        ISlangBlob**                    outDiagnostics)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterEncoder* encoder {};
        {
            encoder = m_captureManager->beginMethodCapture(ApiCallId::ISession_specializeType, m_sessionHandle);
            encoder->encodeAddress(type);
            encoder->encodeStructArray(specializationArgs, specializationArgCount);
            encoder = m_captureManager->endMethodCapture();
        }

        slang::TypeReflection* pTypeReflection = m_actualSession->specializeType(type, specializationArgs, specializationArgCount, outDiagnostics);

        {
            encoder->encodeAddress(*outDiagnostics);
            encoder->encodeAddress(pTypeReflection);
            m_captureManager->endMethodCaptureAppendOutput();
        }

        return pTypeReflection;
    }

    SLANG_NO_THROW slang::TypeLayoutReflection* SessionCapture::getTypeLayout(
        slang::TypeReflection* type,
        SlangInt               targetIndex,
        slang::LayoutRules     rules,
        ISlangBlob**    outDiagnostics)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterEncoder* encoder {};
        {
            encoder = m_captureManager->beginMethodCapture(ApiCallId::ISession_getTypeLayout, m_sessionHandle);
            encoder->encodeAddress(type);
            encoder->encodeInt64(targetIndex);
            encoder->encodeEnumValue(rules);
            encoder = m_captureManager->endMethodCapture();
        }

        slang::TypeLayoutReflection* pTypeLayoutReflection = m_actualSession->getTypeLayout(type, targetIndex, rules, outDiagnostics);

        {
            encoder->encodeAddress(*outDiagnostics);
            encoder->encodeAddress(pTypeLayoutReflection);
            m_captureManager->endMethodCaptureAppendOutput();
        }

        return pTypeLayoutReflection;
    }

    SLANG_NO_THROW slang::TypeReflection* SessionCapture::getContainerType(
        slang::TypeReflection* elementType,
        slang::ContainerType containerType,
        ISlangBlob** outDiagnostics)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterEncoder* encoder {};
        {
            encoder = m_captureManager->beginMethodCapture(ApiCallId::ISession_getContainerType, m_sessionHandle);
            encoder->encodeAddress(elementType);
            encoder->encodeEnumValue(containerType);
            encoder = m_captureManager->endMethodCapture();
        }

        slang::TypeReflection* pTypeReflection = m_actualSession->getContainerType(elementType, containerType, outDiagnostics);

        {
            encoder->encodeAddress(*outDiagnostics);
            encoder->encodeAddress(pTypeReflection);
            m_captureManager->endMethodCaptureAppendOutput();
        }

        return pTypeReflection;
    }

    SLANG_NO_THROW slang::TypeReflection* SessionCapture::getDynamicType()
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterEncoder* encoder {};
        {
            encoder = m_captureManager->beginMethodCapture(ApiCallId::ISession_getDynamicType, m_sessionHandle);
            encoder = m_captureManager->endMethodCapture();
        }

        slang::TypeReflection* pTypeReflection = m_actualSession->getDynamicType();

        {
            encoder->encodeAddress(pTypeReflection);
            m_captureManager->endMethodCaptureAppendOutput();
        }

        return pTypeReflection;
    }

    SLANG_NO_THROW SlangResult SessionCapture::getTypeRTTIMangledName(
        slang::TypeReflection* type,
        ISlangBlob** outNameBlob)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterEncoder* encoder {};
        {
            encoder = m_captureManager->beginMethodCapture(ApiCallId::ISession_getTypeRTTIMangledName, m_sessionHandle);
            encoder->encodeAddress(type);
            encoder = m_captureManager->endMethodCapture();
        }

        SlangResult result = m_actualSession->getTypeRTTIMangledName(type, outNameBlob);

        {
            encoder->encodeAddress(outNameBlob);
            m_captureManager->endMethodCaptureAppendOutput();
        }

        return result;
    }

    SLANG_NO_THROW SlangResult SessionCapture::getTypeConformanceWitnessMangledName(
        slang::TypeReflection* type,
        slang::TypeReflection* interfaceType,
        ISlangBlob** outNameBlob)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterEncoder* encoder {};
        {
            encoder = m_captureManager->beginMethodCapture(ApiCallId::ISession_getTypeConformanceWitnessMangledName, m_sessionHandle);
            encoder->encodeAddress(type);
            encoder->encodeAddress(interfaceType);
            encoder = m_captureManager->endMethodCapture();
        }

        SlangResult result = m_actualSession->getTypeConformanceWitnessMangledName(type, interfaceType, outNameBlob);

        {
            encoder->encodeAddress(outNameBlob);
            m_captureManager->endMethodCaptureAppendOutput();
        }

        return result;
    }

    SLANG_NO_THROW SlangResult SessionCapture::getTypeConformanceWitnessSequentialID(
        slang::TypeReflection* type,
        slang::TypeReflection* interfaceType,
        uint32_t*              outId)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterEncoder* encoder {};
        {
            encoder = m_captureManager->beginMethodCapture(ApiCallId::ISession_getTypeConformanceWitnessSequentialID, m_sessionHandle);
            encoder->encodeAddress(type);
            encoder->encodeAddress(interfaceType);
            encoder = m_captureManager->endMethodCapture();
        }

        SlangResult result = m_actualSession->getTypeConformanceWitnessSequentialID(type, interfaceType, outId);

        // No need to capture outId, it's not slang allocation
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

        ParameterEncoder* encoder {};
        {
            encoder = m_captureManager->beginMethodCapture(ApiCallId::ISession_createTypeConformanceComponentType, m_sessionHandle);
            encoder->encodeAddress(type);
            encoder->encodeAddress(interfaceType);
            encoder->encodeInt64(conformanceIdOverride);
            encoder = m_captureManager->endMethodCapture();
        }

        SlangResult result = m_actualSession->createTypeConformanceComponentType(type, interfaceType, outConformance, conformanceIdOverride, outDiagnostics);

        {
            encoder->encodeAddress(*outConformance);
            encoder->encodeAddress(*outDiagnostics);
            m_captureManager->endMethodCaptureAppendOutput();
        }

        if (SLANG_OK != result)
        {
            TypeConformanceCapture* conformanceCapture = new TypeConformanceCapture(*outConformance, m_captureManager);
            Slang::ComPtr<TypeConformanceCapture> resultCapture(conformanceCapture);
            *outConformance = resultCapture.detach();
        }

        return result;
    }

    SLANG_NO_THROW SlangResult SessionCapture::createCompileRequest(
        SlangCompileRequest**   outCompileRequest)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterEncoder* encoder {};
        {
            encoder = m_captureManager->beginMethodCapture(ApiCallId::ISession_createCompileRequest, m_sessionHandle);
            encoder = m_captureManager->endMethodCapture();
        }

        SlangResult result = m_actualSession->createCompileRequest(outCompileRequest);

        {
            encoder->encodeAddress(*outCompileRequest);
            m_captureManager->endMethodCaptureAppendOutput();
        }

        return result;
    }

    SLANG_NO_THROW SlangInt SessionCapture::getLoadedModuleCount()
    {
        // No need to capture this function, it's just a query.
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);
        SlangInt count = m_actualSession->getLoadedModuleCount();
        return count;
    }

    SLANG_NO_THROW slang::IModule* SessionCapture::getLoadedModule(SlangInt index)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterEncoder* encoder {};
        {
            encoder = m_captureManager->beginMethodCapture(ApiCallId::ISession_getLoadedModule, m_sessionHandle);
            encoder->encodeInt64(index);
            encoder = m_captureManager->endMethodCapture();
        }

        slang::IModule* pModule = m_actualSession->getLoadedModule(index);

        {
            encoder->encodeAddress(pModule);
            m_captureManager->endMethodCaptureAppendOutput();
        }

        if (pModule)
        {
            ModuleCapture* moduleCapture = m_mapModuleToCapture.tryGetValue(pModule);
            if (!moduleCapture)
            {
                SLANG_CAPTURE_ASSERT(!"Module not found in mapModuleToCapture");
            }
            return static_cast<slang::IModule*>(moduleCapture);
        }

        return pModule;
    }

    SLANG_NO_THROW bool SessionCapture::isBinaryModuleUpToDate(const char* modulePath, slang::IBlob* binaryModuleBlob)
    {
        // No need to capture this function, it's a query function and doesn't impact slang internal state.
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
            moduleCapture = new ModuleCapture(module, m_captureManager);
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
