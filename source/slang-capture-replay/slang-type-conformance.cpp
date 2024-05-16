#include "capture_utility.h"
#include "slang-type-conformance.h"

namespace SlangCapture
{
    TypeConformanceCapture::TypeConformanceCapture(slang::ITypeConformance* typeConformance)
        : m_actualTypeConformance(typeConformance)
    {
        SLANG_CAPTURE_ASSERT(m_actualTypeConformance != nullptr);
        slangCaptureLog(LogLevel::Verbose, "%s: %p\n", __PRETTY_FUNCTION__, typeConformance);
    }
    TypeConformanceCapture::~TypeConformanceCapture()
    {
        m_actualTypeConformance->release();
    }

    ISlangUnknown* TypeConformanceCapture::getInterface(const Guid& guid)
    {
        if (guid == TypeConformanceCapture::getTypeGuid())
        {
            return static_cast<ISlangUnknown*>(this);
        }
        else
        {
            return nullptr;
        }
    }

    SLANG_NO_THROW slang::ISession* TypeConformanceCapture::getSession()
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);
        slang::ISession* res = m_actualTypeConformance->getSession();
        return res;
    }

    SLANG_NO_THROW slang::ProgramLayout* TypeConformanceCapture::getLayout(
        SlangInt    targetIndex,
        slang::IBlob**     outDiagnostics)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);
        slang::ProgramLayout* res = m_actualTypeConformance->getLayout(targetIndex, outDiagnostics);
        return res;
    }

    SLANG_NO_THROW SlangInt TypeConformanceCapture::getSpecializationParamCount()
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);
        SlangInt res = m_actualTypeConformance->getSpecializationParamCount();
        return res;
    }

    SLANG_NO_THROW SlangResult TypeConformanceCapture::getEntryPointCode(
        SlangInt    entryPointIndex,
        SlangInt    targetIndex,
        slang::IBlob**     outCode,
        slang::IBlob**     outDiagnostics)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);
        SlangResult res = m_actualTypeConformance->getEntryPointCode(entryPointIndex, targetIndex, outCode, outDiagnostics);
        return res;
    }

    SLANG_NO_THROW SlangResult TypeConformanceCapture::getResultAsFileSystem(
        SlangInt    entryPointIndex,
        SlangInt    targetIndex,
        ISlangMutableFileSystem** outFileSystem)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);
        SlangResult res = m_actualTypeConformance->getResultAsFileSystem(entryPointIndex, targetIndex, outFileSystem);
        return res;
    }

    SLANG_NO_THROW void TypeConformanceCapture::getEntryPointHash(
        SlangInt    entryPointIndex,
        SlangInt    targetIndex,
        slang::IBlob**     outHash)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);
        m_actualTypeConformance->getEntryPointHash(entryPointIndex, targetIndex, outHash);
    }

    SLANG_NO_THROW SlangResult TypeConformanceCapture::specialize(
        slang::SpecializationArg const*    specializationArgs,
        SlangInt                    specializationArgCount,
        slang::IComponentType**            outSpecializedComponentType,
        ISlangBlob**                outDiagnostics)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);
        SlangResult res = m_actualTypeConformance->specialize(specializationArgs, specializationArgCount, outSpecializedComponentType, outDiagnostics);
        return res;
    }

    SLANG_NO_THROW SlangResult TypeConformanceCapture::link(
        slang::IComponentType**            outLinkedComponentType,
        ISlangBlob**                outDiagnostics)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);
        SlangResult res = m_actualTypeConformance->link(outLinkedComponentType, outDiagnostics);
        return res;
    }

    SLANG_NO_THROW SlangResult TypeConformanceCapture::getEntryPointHostCallable(
        int                     entryPointIndex,
        int                     targetIndex,
        ISlangSharedLibrary**   outSharedLibrary,
        slang::IBlob**          outDiagnostics)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);
        SlangResult res = m_actualTypeConformance->getEntryPointHostCallable(entryPointIndex, targetIndex, outSharedLibrary, outDiagnostics);
        return res;
    }

    SLANG_NO_THROW SlangResult TypeConformanceCapture::renameEntryPoint(
        const char* newName, IComponentType** outEntryPoint)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);
        SlangResult res = m_actualTypeConformance->renameEntryPoint(newName, outEntryPoint);
        return res;
    }

    SLANG_NO_THROW SlangResult TypeConformanceCapture::linkWithOptions(
        IComponentType** outLinkedComponentType,
        uint32_t compilerOptionEntryCount,
        slang::CompilerOptionEntry* compilerOptionEntries,
        ISlangBlob** outDiagnostics)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);
        SlangResult res = m_actualTypeConformance->linkWithOptions(outLinkedComponentType, compilerOptionEntryCount, compilerOptionEntries, outDiagnostics);
        return res;
    }
}
