#include "capture_utility.h"
#include "slang-composite-component-type.h"

namespace SlangCapture
{
    CompositeComponentTypeCapture::CompositeComponentTypeCapture(slang::IComponentType* componentType)
        : m_actualCompositeComponentType(componentType)
    {
        assert(m_actualCompositeComponentType != nullptr);
        slangCaptureLog(LogLevel::Verbose, "%s: %p\n", __PRETTY_FUNCTION__, m_actualCompositeComponentType);
    }

    CompositeComponentTypeCapture::~CompositeComponentTypeCapture()
    {
        m_actualCompositeComponentType->release();
    }

    ISlangUnknown* CompositeComponentTypeCapture::getInterface(const Guid& guid)
    {
        if (guid == IComponentType::getTypeGuid())
        {
            return static_cast<ISlangUnknown*>(this);
        }
        return nullptr;
    }

    SLANG_NO_THROW slang::ISession* CompositeComponentTypeCapture::getSession()
	{
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);
        slang::ISession* res = m_actualCompositeComponentType->getSession();
        return res;
    }

    SLANG_NO_THROW slang::ProgramLayout* CompositeComponentTypeCapture::getLayout(
        SlangInt    targetIndex,
        slang::IBlob**     outDiagnostics)
	{
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);
        slang::ProgramLayout* res = m_actualCompositeComponentType->getLayout(targetIndex, outDiagnostics);
        return res;
    }

    SLANG_NO_THROW SlangInt CompositeComponentTypeCapture::getSpecializationParamCount()
	{
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);
        SlangInt res = m_actualCompositeComponentType->getSpecializationParamCount();
        return res;
    }

    SLANG_NO_THROW SlangResult CompositeComponentTypeCapture::getEntryPointCode(
        SlangInt    entryPointIndex,
        SlangInt    targetIndex,
        slang::IBlob**     outCode,
        slang::IBlob**     outDiagnostics)
	{
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);
        SlangResult res = m_actualCompositeComponentType->getEntryPointCode(entryPointIndex, targetIndex, outCode, outDiagnostics);
        return res;
    }

    SLANG_NO_THROW SlangResult CompositeComponentTypeCapture::getResultAsFileSystem(
        SlangInt    entryPointIndex,
        SlangInt    targetIndex,
        ISlangMutableFileSystem** outFileSystem)
	{
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);
        SlangResult res = m_actualCompositeComponentType->getResultAsFileSystem(entryPointIndex, targetIndex, outFileSystem);
        return res;
    }

    SLANG_NO_THROW void CompositeComponentTypeCapture::getEntryPointHash(
        SlangInt    entryPointIndex,
        SlangInt    targetIndex,
        slang::IBlob**     outHash)
	{
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);
        m_actualCompositeComponentType->getEntryPointHash(entryPointIndex, targetIndex, outHash);
    }

    SLANG_NO_THROW SlangResult CompositeComponentTypeCapture::specialize(
        slang::SpecializationArg const*    specializationArgs,
        SlangInt                    specializationArgCount,
        slang::IComponentType**            outSpecializedComponentType,
        ISlangBlob**                outDiagnostics)
	{
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);
        SlangResult res = m_actualCompositeComponentType->specialize(specializationArgs, specializationArgCount, outSpecializedComponentType, outDiagnostics);
        return res;
    }

    SLANG_NO_THROW SlangResult CompositeComponentTypeCapture::link(
        slang::IComponentType**            outLinkedComponentType,
        ISlangBlob**                outDiagnostics)
	{
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);
        SlangResult res = m_actualCompositeComponentType->link(outLinkedComponentType, outDiagnostics);
        return res;
    }

    SLANG_NO_THROW SlangResult CompositeComponentTypeCapture::getEntryPointHostCallable(
        int                     entryPointIndex,
        int                     targetIndex,
        ISlangSharedLibrary**   outSharedLibrary,
        slang::IBlob**          outDiagnostics)
	{
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);
        SlangResult res = m_actualCompositeComponentType->getEntryPointHostCallable(entryPointIndex, targetIndex, outSharedLibrary, outDiagnostics);
        return res;
    }

    SLANG_NO_THROW SlangResult CompositeComponentTypeCapture::renameEntryPoint(
        const char* newName, IComponentType** outEntryPoint)
	{
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);
        SlangResult res = m_actualCompositeComponentType->renameEntryPoint(newName, outEntryPoint);
        return res;
    }

    SLANG_NO_THROW SlangResult CompositeComponentTypeCapture::linkWithOptions(
        IComponentType** outLinkedComponentType,
        uint32_t compilerOptionEntryCount,
        slang::CompilerOptionEntry* compilerOptionEntries,
        ISlangBlob** outDiagnostics)
	{
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);
        SlangResult res = m_actualCompositeComponentType->linkWithOptions(outLinkedComponentType, compilerOptionEntryCount, compilerOptionEntries, outDiagnostics);
        return res;
    }
}
