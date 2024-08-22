#include "../util/record-utility.h"
#include "slang-session.h"
#include "slang-entrypoint.h"
#include "slang-composite-component-type.h"
#include "slang-type-conformance.h"

namespace SlangRecord
{

    SessionRecorder::SessionRecorder(slang::ISession* session, RecordManager* recordManager)
        : m_actualSession(session),
          m_recordManager(recordManager)
    {
        SLANG_RECORD_ASSERT(m_actualSession);
        SLANG_RECORD_ASSERT(m_recordManager);
        m_sessionHandle = reinterpret_cast<uint64_t>(m_actualSession.get());
        slangRecordLog(LogLevel::Verbose, "%s: %p\n", "SessionRecorder create:", session);
    }

    ISlangUnknown* SessionRecorder::getInterface(const Guid& guid)
    {
        if(guid == ISlangUnknown::getTypeGuid() || guid == ISession::getTypeGuid())
            return asExternal(this);

        return nullptr;
    }

    SLANG_NO_THROW slang::IGlobalSession* SessionRecorder::getGlobalSession()
    {
        // No need to record this function.
        slangRecordLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);
        slang::IGlobalSession* pGlobalSession = m_actualSession->getGlobalSession();
        return pGlobalSession;
    }

    SLANG_NO_THROW slang::IModule* SessionRecorder::loadModule(
        const char* moduleName,
        slang::IBlob**     outDiagnostics)
    {
        slangRecordLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterRecorder* recorder {};
        {
            recorder = m_recordManager->beginMethodRecord(ApiCallId::ISession_loadModule, m_sessionHandle);
            recorder->recordString(moduleName);
            recorder = m_recordManager->endMethodRecord();
        }

        slang::IModule* pModule = m_actualSession->loadModule(moduleName, outDiagnostics);

        {
            recorder->recordAddress(outDiagnostics ? *outDiagnostics : nullptr);
            recorder->recordAddress(pModule);
            m_recordManager->apendOutput();
        }

        ModuleRecorder* pModuleRecorder = getModuleRecorder(pModule);
        return static_cast<slang::IModule*>(pModuleRecorder);
    }

    SLANG_NO_THROW slang::IModule* SessionRecorder::loadModuleFromIRBlob(
        const char* moduleName,
        const char* path,
        slang::IBlob* source,
        slang::IBlob** outDiagnostics)
    {
        slangRecordLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterRecorder* recorder {};
        {
            recorder = m_recordManager->beginMethodRecord(ApiCallId::ISession_loadModuleFromIRBlob, m_sessionHandle);
            recorder->recordString(moduleName);
            recorder->recordString(path);
            recorder->recordPointer(source);
            recorder = m_recordManager->endMethodRecord();
        }

        slang::IModule* pModule = m_actualSession->loadModuleFromIRBlob(moduleName, path, source, outDiagnostics);

        {
            recorder->recordAddress(outDiagnostics ? *outDiagnostics : nullptr);
            recorder->recordAddress(pModule);
            m_recordManager->apendOutput();
        }

        ModuleRecorder* pModuleRecorder = getModuleRecorder(pModule);
        return static_cast<slang::IModule*>(pModuleRecorder);
    }

    SLANG_NO_THROW slang::IModule* SessionRecorder::loadModuleFromSource(
        const char* moduleName,
        const char* path,
        slang::IBlob* source,
        slang::IBlob** outDiagnostics)
    {
        slangRecordLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterRecorder* recorder {};
        {
            recorder = m_recordManager->beginMethodRecord(ApiCallId::ISession_loadModuleFromSource, m_sessionHandle);
            recorder->recordString(moduleName);
            recorder->recordString(path);
            recorder->recordPointer(source);
            recorder = m_recordManager->endMethodRecord();
        }

        slang::IModule* pModule = m_actualSession->loadModuleFromSource(moduleName, path, source, outDiagnostics);

        {
            recorder->recordAddress(outDiagnostics ? *outDiagnostics : nullptr);
            recorder->recordAddress(pModule);
            m_recordManager->apendOutput();
        }

        ModuleRecorder* pModuleRecorder = getModuleRecorder(pModule);
        return static_cast<slang::IModule*>(pModuleRecorder);
    }

    SLANG_NO_THROW slang::IModule* SessionRecorder::loadModuleFromSourceString(
        const char* moduleName,
        const char* path,
        const char* string,
        slang::IBlob** outDiagnostics)
    {
        slangRecordLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterRecorder* recorder {};
        {
            recorder = m_recordManager->beginMethodRecord(ApiCallId::ISession_loadModuleFromSourceString, m_sessionHandle);
            recorder->recordString(moduleName);
            recorder->recordString(path);
            recorder->recordString(string);
            recorder = m_recordManager->endMethodRecord();
        }

        slang::IModule* pModule = m_actualSession->loadModuleFromSourceString(moduleName, path, string, outDiagnostics);

        {
            // TODO: Not sure if we need to record the diagnostics blob.
            recorder->recordAddress(outDiagnostics ? *outDiagnostics : nullptr);
            recorder->recordAddress(pModule);
            m_recordManager->apendOutput();
        }

        ModuleRecorder* pModuleRecorder = getModuleRecorder(pModule);
        return static_cast<slang::IModule*>(pModuleRecorder);
    }

    SLANG_NO_THROW SlangResult SessionRecorder::createCompositeComponentType(
        slang::IComponentType* const*   componentTypes,
        SlangInt                        componentTypeCount,
        slang::IComponentType**         outCompositeComponentType,
        ISlangBlob**                    outDiagnostics)
    {
        slangRecordLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        Slang::List<slang::IComponentType*> componentTypeList;

        // get the actual component types from our record wrappers
        if(SLANG_OK != getActualComponentTypes(componentTypes, componentTypeCount, componentTypeList))
        {
            SLANG_RECORD_ASSERT(!"Failed to get actual component types");
        }

        ParameterRecorder* recorder {};
        {
            recorder = m_recordManager->beginMethodRecord(ApiCallId::ISession_createCompositeComponentType, m_sessionHandle);
            recorder->recordAddressArray(componentTypeList.getBuffer(), componentTypeCount);
            recorder = m_recordManager->endMethodRecord();
        }

        SlangResult result = m_actualSession->createCompositeComponentType(
                componentTypeList.getBuffer(), componentTypeCount, outCompositeComponentType, outDiagnostics);

        {
            recorder->recordAddress(*outCompositeComponentType);
            recorder->recordAddress(outDiagnostics ? *outDiagnostics : nullptr);
            m_recordManager->apendOutput();
        }

        if (SLANG_OK == result)
        {
            CompositeComponentTypeRecorder* compositeComponentTypeRecord =
                new CompositeComponentTypeRecorder(*outCompositeComponentType, m_recordManager);
            Slang::ComPtr<CompositeComponentTypeRecorder> resultRecord(compositeComponentTypeRecord);
            *outCompositeComponentType = resultRecord.detach();
        }

        return result;
    }

    SLANG_NO_THROW slang::TypeReflection* SessionRecorder::specializeType(
        slang::TypeReflection*          type,
        slang::SpecializationArg const* specializationArgs,
        SlangInt                        specializationArgCount,
        ISlangBlob**                    outDiagnostics)
    {
        slangRecordLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterRecorder* recorder {};
        {
            recorder = m_recordManager->beginMethodRecord(ApiCallId::ISession_specializeType, m_sessionHandle);
            recorder->recordAddress(type);
            recorder->recordStructArray(specializationArgs, specializationArgCount);
            recorder = m_recordManager->endMethodRecord();
        }

        slang::TypeReflection* pTypeReflection = m_actualSession->specializeType(type, specializationArgs, specializationArgCount, outDiagnostics);

        {
            recorder->recordAddress(outDiagnostics ? *outDiagnostics : nullptr);
            recorder->recordAddress(pTypeReflection);
            m_recordManager->apendOutput();
        }

        return pTypeReflection;
    }

    SLANG_NO_THROW slang::TypeLayoutReflection* SessionRecorder::getTypeLayout(
        slang::TypeReflection* type,
        SlangInt               targetIndex,
        slang::LayoutRules     rules,
        ISlangBlob**    outDiagnostics)
    {
        slangRecordLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterRecorder* recorder {};
        {
            recorder = m_recordManager->beginMethodRecord(ApiCallId::ISession_getTypeLayout, m_sessionHandle);
            recorder->recordAddress(type);
            recorder->recordInt64(targetIndex);
            recorder->recordEnumValue(rules);
            recorder = m_recordManager->endMethodRecord();
        }

        slang::TypeLayoutReflection* pTypeLayoutReflection = m_actualSession->getTypeLayout(type, targetIndex, rules, outDiagnostics);

        {
            recorder->recordAddress(outDiagnostics ? *outDiagnostics : nullptr);
            recorder->recordAddress(pTypeLayoutReflection);
            m_recordManager->apendOutput();
        }

        return pTypeLayoutReflection;
    }

    SLANG_NO_THROW slang::TypeReflection* SessionRecorder::getContainerType(
        slang::TypeReflection* elementType,
        slang::ContainerType containerType,
        ISlangBlob** outDiagnostics)
    {
        slangRecordLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterRecorder* recorder {};
        {
            recorder = m_recordManager->beginMethodRecord(ApiCallId::ISession_getContainerType, m_sessionHandle);
            recorder->recordAddress(elementType);
            recorder->recordEnumValue(containerType);
            recorder = m_recordManager->endMethodRecord();
        }

        slang::TypeReflection* pTypeReflection = m_actualSession->getContainerType(elementType, containerType, outDiagnostics);

        {
            recorder->recordAddress(outDiagnostics ? *outDiagnostics : nullptr);
            recorder->recordAddress(pTypeReflection);
            m_recordManager->apendOutput();
        }

        return pTypeReflection;
    }

    SLANG_NO_THROW slang::TypeReflection* SessionRecorder::getDynamicType()
    {
        slangRecordLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterRecorder* recorder {};
        {
            recorder = m_recordManager->beginMethodRecord(ApiCallId::ISession_getDynamicType, m_sessionHandle);
            recorder = m_recordManager->endMethodRecord();
        }

        slang::TypeReflection* pTypeReflection = m_actualSession->getDynamicType();

        {
            recorder->recordAddress(pTypeReflection);
            m_recordManager->apendOutput();
        }

        return pTypeReflection;
    }

    SLANG_NO_THROW SlangResult SessionRecorder::getTypeRTTIMangledName(
        slang::TypeReflection* type,
        ISlangBlob** outNameBlob)
    {
        slangRecordLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterRecorder* recorder {};
        {
            recorder = m_recordManager->beginMethodRecord(ApiCallId::ISession_getTypeRTTIMangledName, m_sessionHandle);
            recorder->recordAddress(type);
            recorder = m_recordManager->endMethodRecord();
        }

        SlangResult result = m_actualSession->getTypeRTTIMangledName(type, outNameBlob);

        {
            recorder->recordAddress(outNameBlob);
            m_recordManager->apendOutput();
        }

        return result;
    }

    SLANG_NO_THROW SlangResult SessionRecorder::getTypeConformanceWitnessMangledName(
        slang::TypeReflection* type,
        slang::TypeReflection* interfaceType,
        ISlangBlob** outNameBlob)
    {
        slangRecordLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterRecorder* recorder {};
        {
            recorder = m_recordManager->beginMethodRecord(ApiCallId::ISession_getTypeConformanceWitnessMangledName, m_sessionHandle);
            recorder->recordAddress(type);
            recorder->recordAddress(interfaceType);
            recorder = m_recordManager->endMethodRecord();
        }

        SlangResult result = m_actualSession->getTypeConformanceWitnessMangledName(type, interfaceType, outNameBlob);

        {
            recorder->recordAddress(outNameBlob);
            m_recordManager->apendOutput();
        }

        return result;
    }

    SLANG_NO_THROW SlangResult SessionRecorder::getTypeConformanceWitnessSequentialID(
        slang::TypeReflection* type,
        slang::TypeReflection* interfaceType,
        uint32_t*              outId)
    {
        slangRecordLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterRecorder* recorder {};
        {
            recorder = m_recordManager->beginMethodRecord(ApiCallId::ISession_getTypeConformanceWitnessSequentialID, m_sessionHandle);
            recorder->recordAddress(type);
            recorder->recordAddress(interfaceType);
            recorder = m_recordManager->endMethodRecord();
        }

        SlangResult result = m_actualSession->getTypeConformanceWitnessSequentialID(type, interfaceType, outId);

        // No need to record outId, it's not slang allocation
        return result;
    }

    SLANG_NO_THROW SlangResult SessionRecorder::createTypeConformanceComponentType(
        slang::TypeReflection* type,
        slang::TypeReflection* interfaceType,
        slang::ITypeConformance** outConformance,
        SlangInt conformanceIdOverride,
        ISlangBlob** outDiagnostics)
    {
        slangRecordLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterRecorder* recorder {};
        {
            recorder = m_recordManager->beginMethodRecord(ApiCallId::ISession_createTypeConformanceComponentType, m_sessionHandle);
            recorder->recordAddress(type);
            recorder->recordAddress(interfaceType);
            recorder->recordInt64(conformanceIdOverride);
            recorder = m_recordManager->endMethodRecord();
        }

        SlangResult result = m_actualSession->createTypeConformanceComponentType(type, interfaceType, outConformance, conformanceIdOverride, outDiagnostics);

        {
            recorder->recordAddress(*outConformance);
            recorder->recordAddress(outDiagnostics ? *outDiagnostics : nullptr);
            m_recordManager->apendOutput();
        }

        if (SLANG_OK != result)
        {
            TypeConformanceRecorder* conformanceRecord = new TypeConformanceRecorder(*outConformance, m_recordManager);
            Slang::ComPtr<TypeConformanceRecorder> resultRecord(conformanceRecord);
            *outConformance = resultRecord.detach();
        }

        return result;
    }

    SLANG_NO_THROW SlangResult SessionRecorder::createCompileRequest(
        SlangCompileRequest**   outCompileRequest)
    {
        slangRecordLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterRecorder* recorder {};
        {
            recorder = m_recordManager->beginMethodRecord(ApiCallId::ISession_createCompileRequest, m_sessionHandle);
            recorder = m_recordManager->endMethodRecord();
        }

        SlangResult result = m_actualSession->createCompileRequest(outCompileRequest);

        {
            recorder->recordAddress(*outCompileRequest);
            m_recordManager->apendOutput();
        }

        return result;
    }

    SLANG_NO_THROW SlangInt SessionRecorder::getLoadedModuleCount()
    {
        // No need to record this function, it's just a query.
        slangRecordLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);
        SlangInt count = m_actualSession->getLoadedModuleCount();
        return count;
    }

    SLANG_NO_THROW slang::IModule* SessionRecorder::getLoadedModule(SlangInt index)
    {
        slangRecordLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        ParameterRecorder* recorder {};
        {
            recorder = m_recordManager->beginMethodRecord(ApiCallId::ISession_getLoadedModule, m_sessionHandle);
            recorder->recordInt64(index);
            recorder = m_recordManager->endMethodRecord();
        }

        slang::IModule* pModule = m_actualSession->getLoadedModule(index);

        {
            recorder->recordAddress(pModule);
            m_recordManager->apendOutput();
        }

        if (pModule)
        {
            ModuleRecorder* moduleRecord = m_mapModuleToRecord.tryGetValue(pModule);
            if (!moduleRecord)
            {
                SLANG_RECORD_ASSERT(!"Module not found in mapModuleToRecord");
            }
            return static_cast<slang::IModule*>(moduleRecord);
        }

        return pModule;
    }

    SLANG_NO_THROW bool SessionRecorder::isBinaryModuleUpToDate(const char* modulePath, slang::IBlob* binaryModuleBlob)
    {
        // No need to record this function, it's a query function and doesn't impact slang internal state.
        slangRecordLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);
        bool result = m_actualSession->isBinaryModuleUpToDate(modulePath, binaryModuleBlob);
        return result;
    }

    ModuleRecorder* SessionRecorder::getModuleRecorder(slang::IModule* module)
    {
        ModuleRecorder* moduleRecord = nullptr;
        moduleRecord = m_mapModuleToRecord.tryGetValue(module);
        if (!moduleRecord)
        {
            moduleRecord = new ModuleRecorder(module, m_recordManager);
            Slang::ComPtr<ModuleRecorder> result(moduleRecord);
            m_mapModuleToRecord.add(module, *result.detach());
        }
        return moduleRecord;
    }

    SlangResult SessionRecorder::getActualComponentTypes(
            slang::IComponentType* const*   componentTypes,
            SlangInt                        componentTypeCount,
            List<slang::IComponentType*>&   outActualComponentTypes)
    {
        for (SlangInt i = 0; i < componentTypeCount; i++)
        {
            slang::IComponentType* const& componentType = componentTypes[i];
            void* outObj = nullptr;

            if (componentType->queryInterface(ModuleRecorder::getTypeGuid(), &outObj) == SLANG_OK)
            {
                ModuleRecorder* moduleRecord = static_cast<ModuleRecorder*>(outObj);
                outActualComponentTypes.add(moduleRecord->getActualModule());
            }
            else if (componentType->queryInterface(EntryPointRecorder::getTypeGuid(), &outObj) == SLANG_OK)
            {
                EntryPointRecorder* entrypointRecord = static_cast<EntryPointRecorder*>(outObj);
                outActualComponentTypes.add(entrypointRecord->getActualEntryPoint());
            }
            // will fall back to the actual component type, it means that we didn't record this type.
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
} // namespace SlangRecord
