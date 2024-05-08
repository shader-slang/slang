#include "capture_utility.h"
#include "slang-session.h"

namespace SlangCapture
{

    SessionCapture::SessionCapture(slang::ISession* session)
        : m_actualSession(session)
    {
        assert(m_actualSession);
        slangCaptureLog(LogLevel::Verbose, "%s: %p\n", "Session", session);
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
        slangCaptureLog(LogLevel::Verbose, "%s\n", __func__);
        slang::IGlobalSession* pGlobalSession = m_actualSession->getGlobalSession();
        return pGlobalSession;
    }

    SLANG_NO_THROW slang::IModule* SessionCapture::loadModule(
        const char* moduleName,
        slang::IBlob**     outDiagnostics)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __func__);
        slang::IModule* pModule = m_actualSession->loadModule(moduleName, outDiagnostics);
        return pModule;
    }

    SLANG_NO_THROW slang::IModule* SessionCapture::loadModuleFromIRBlob(
        const char* moduleName,
        const char* path,
        slang::IBlob* source,
        slang::IBlob** outDiagnostics)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __func__);
        slang::IModule* pModule = m_actualSession->loadModuleFromIRBlob(moduleName, path, source, outDiagnostics);
        return pModule;
    }

    SLANG_NO_THROW slang::IModule* SessionCapture::loadModuleFromSource(
        const char* moduleName,
        const char* path,
        slang::IBlob* source,
        slang::IBlob** outDiagnostics)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __func__);
        slang::IModule* pModule = m_actualSession->loadModuleFromSource(moduleName, path, source, outDiagnostics);
        return pModule;
    }

    SLANG_NO_THROW slang::IModule* SessionCapture::loadModuleFromSourceString(
        const char* moduleName,
        const char* path,
        const char* string,
        slang::IBlob** outDiagnostics)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __func__);
        slang::IModule* pModule = m_actualSession->loadModuleFromSourceString(moduleName, path, string, outDiagnostics);
        return pModule;
    }

    SLANG_NO_THROW SlangResult SessionCapture::createCompositeComponentType(
        slang::IComponentType* const*   componentTypes,
        SlangInt                        componentTypeCount,
        slang::IComponentType**         outCompositeComponentType,
        ISlangBlob**                    outDiagnostics)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __func__);
        SlangResult result = m_actualSession->createCompositeComponentType(componentTypes, componentTypeCount, outCompositeComponentType, outDiagnostics);
        return result;
    }

    SLANG_NO_THROW slang::TypeReflection* SessionCapture::specializeType(
        slang::TypeReflection*          type,
        slang::SpecializationArg const* specializationArgs,
        SlangInt                        specializationArgCount,
        ISlangBlob**                    outDiagnostics)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __func__);
        slang::TypeReflection* pTypeReflection = m_actualSession->specializeType(type, specializationArgs, specializationArgCount, outDiagnostics);
        return pTypeReflection;
    }

    SLANG_NO_THROW slang::TypeLayoutReflection* SessionCapture::getTypeLayout(
        slang::TypeReflection* type,
        SlangInt               targetIndex,
        slang::LayoutRules     rules,
        ISlangBlob**    outDiagnostics)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __func__);
        slang::TypeLayoutReflection* pTypeLayoutReflection = m_actualSession->getTypeLayout(type, targetIndex, rules, outDiagnostics);
        return pTypeLayoutReflection;
    }

    SLANG_NO_THROW slang::TypeReflection* SessionCapture::getContainerType(
        slang::TypeReflection* elementType,
        slang::ContainerType containerType,
        ISlangBlob** outDiagnostics)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __func__);
        slang::TypeReflection* pTypeReflection = m_actualSession->getContainerType(elementType, containerType, outDiagnostics);
        return pTypeReflection;
    }

    SLANG_NO_THROW slang::TypeReflection* SessionCapture::getDynamicType()
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __func__);
        slang::TypeReflection* pTypeReflection = m_actualSession->getDynamicType();
        return pTypeReflection;
    }

    SLANG_NO_THROW SlangResult SessionCapture::getTypeRTTIMangledName(
        slang::TypeReflection* type,
        ISlangBlob** outNameBlob)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __func__);
        SlangResult result = m_actualSession->getTypeRTTIMangledName(type, outNameBlob);
        return result;
    }

    SLANG_NO_THROW SlangResult SessionCapture::getTypeConformanceWitnessMangledName(
        slang::TypeReflection* type,
        slang::TypeReflection* interfaceType,
        ISlangBlob** outNameBlob)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __func__);
        SlangResult result = m_actualSession->getTypeConformanceWitnessMangledName(type, interfaceType, outNameBlob);
        return result;
    }

    SLANG_NO_THROW SlangResult SessionCapture::getTypeConformanceWitnessSequentialID(
        slang::TypeReflection* type,
        slang::TypeReflection* interfaceType,
        uint32_t*              outId)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __func__);
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
        slangCaptureLog(LogLevel::Verbose, "%s\n", __func__);
        SlangResult result = m_actualSession->createTypeConformanceComponentType(type, interfaceType, outConformance, conformanceIdOverride, outDiagnostics);
        return result;
    }

    SLANG_NO_THROW SlangResult SessionCapture::createCompileRequest(
        SlangCompileRequest**   outCompileRequest)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __func__);
        SlangResult result = m_actualSession->createCompileRequest(outCompileRequest);
        return result;
    }

    SLANG_NO_THROW SlangInt SessionCapture::getLoadedModuleCount()
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __func__);
        SlangInt count = m_actualSession->getLoadedModuleCount();
        return count;
    }

    SLANG_NO_THROW slang::IModule* SessionCapture::getLoadedModule(SlangInt index)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __func__);
        slang::IModule* pModule = m_actualSession->getLoadedModule(index);
        return pModule;
    }

    SLANG_NO_THROW bool SessionCapture::isBinaryModuleUpToDate(const char* modulePath, slang::IBlob* binaryModuleBlob)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __func__);
        bool result = m_actualSession->isBinaryModuleUpToDate(modulePath, binaryModuleBlob);
        return result;
    }

} // namespace SlangCapture
