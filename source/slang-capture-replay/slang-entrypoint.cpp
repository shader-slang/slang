#include "capture_utility.h"
#include "slang-entrypoint.h"

namespace SlangCapture
{
    EntryPointCapture::EntryPointCapture(slang::IEntryPoint* entryPoint)
        : m_actualEntryPoint(entryPoint)
    {
        assert(m_actualEntryPoint != nullptr);
        slangCaptureLog(LogLevel::Verbose, "%s: %p\n", __PRETTY_FUNCTION__, entryPoint);
    }

    EntryPointCapture::~EntryPointCapture()
    {
        m_actualEntryPoint->release();
    }

    ISlangUnknown* EntryPointCapture::getInterface(const Guid& guid)
    {
        if(guid == EntryPointCapture::getTypeGuid())
            return static_cast<ISlangUnknown*>(this);
        else
            return nullptr;
    }

    SLANG_NO_THROW slang::ISession* EntryPointCapture::getSession()
	{
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);
        slang::ISession* res = m_actualEntryPoint->getSession();
        return res;
    }

    SLANG_NO_THROW slang::ProgramLayout* EntryPointCapture::getLayout(
        SlangInt    targetIndex,
        slang::IBlob**     outDiagnostics)
	{
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);
        slang::ProgramLayout* res = m_actualEntryPoint->getLayout(targetIndex, outDiagnostics);
        return res;
    }

    SLANG_NO_THROW SlangInt EntryPointCapture::getSpecializationParamCount()
	{
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);
        SlangInt res = m_actualEntryPoint->getSpecializationParamCount();
        return res;
    }

    SLANG_NO_THROW SlangResult EntryPointCapture::getEntryPointCode(
        SlangInt    entryPointIndex,
        SlangInt    targetIndex,
        slang::IBlob**     outCode,
        slang::IBlob**     outDiagnostics)
	{
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);
        SlangResult res = m_actualEntryPoint->getEntryPointCode(entryPointIndex, targetIndex, outCode, outDiagnostics);
        return res;
    }

    SLANG_NO_THROW SlangResult EntryPointCapture::getResultAsFileSystem(
        SlangInt    entryPointIndex,
        SlangInt    targetIndex,
        ISlangMutableFileSystem** outFileSystem)
	{
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);
        SlangResult res = m_actualEntryPoint->getResultAsFileSystem(entryPointIndex, targetIndex, outFileSystem);
        return res;
    }

    SLANG_NO_THROW void EntryPointCapture::getEntryPointHash(
        SlangInt    entryPointIndex,
        SlangInt    targetIndex,
        slang::IBlob**     outHash)
	{
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);
        m_actualEntryPoint->getEntryPointHash(entryPointIndex, targetIndex, outHash);
    }

    SLANG_NO_THROW SlangResult EntryPointCapture::specialize(
        slang::SpecializationArg const*    specializationArgs,
        SlangInt                    specializationArgCount,
        slang::IComponentType**            outSpecializedComponentType,
        ISlangBlob**                outDiagnostics)
	{
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);
        SlangResult res = m_actualEntryPoint->specialize(specializationArgs, specializationArgCount, outSpecializedComponentType, outDiagnostics);
        return res;
    }

    SLANG_NO_THROW SlangResult EntryPointCapture::link(
        slang::IComponentType**            outLinkedComponentType,
        ISlangBlob**                outDiagnostics)
	{
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);
        SlangResult res = m_actualEntryPoint->link(outLinkedComponentType, outDiagnostics);
        return res;
    }

    SLANG_NO_THROW SlangResult EntryPointCapture::getEntryPointHostCallable(
        int                     entryPointIndex,
        int                     targetIndex,
        ISlangSharedLibrary**   outSharedLibrary,
        slang::IBlob**          outDiagnostics)
	{
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);
        SlangResult res = m_actualEntryPoint->getEntryPointHostCallable(entryPointIndex, targetIndex, outSharedLibrary, outDiagnostics);
        return res;
    }

    SLANG_NO_THROW SlangResult EntryPointCapture::renameEntryPoint(
        const char* newName, IComponentType** outEntryPoint)
	{
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);
        SlangResult res = m_actualEntryPoint->renameEntryPoint(newName, outEntryPoint);
        return res;
    }

    SLANG_NO_THROW SlangResult EntryPointCapture::linkWithOptions(
        IComponentType** outLinkedComponentType,
        uint32_t compilerOptionEntryCount,
        slang::CompilerOptionEntry* compilerOptionEntries,
        ISlangBlob** outDiagnostics)
	{
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);
        SlangResult res = m_actualEntryPoint->linkWithOptions(outLinkedComponentType, compilerOptionEntryCount, compilerOptionEntries, outDiagnostics);
        return res;
    }
}
