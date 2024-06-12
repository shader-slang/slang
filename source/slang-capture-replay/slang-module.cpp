#include "capture_utility.h"
#include "slang-module.h"

namespace SlangCapture
{
    ModuleCapture::ModuleCapture(slang::IModule* module)
        : m_actualModule(module)
    {
        SLANG_CAPTURE_ASSERT(m_actualModule != nullptr);
        slangCaptureLog(LogLevel::Verbose, "%s: %p\n", __PRETTY_FUNCTION__, module);
    }

    ModuleCapture::~ModuleCapture()
    {
        m_actualModule->release();
    }

    ISlangUnknown* ModuleCapture::getInterface(const Guid& guid)
    {
        if(guid == ModuleCapture::getTypeGuid())
            return static_cast<ISlangUnknown*>(this);
        else
            return nullptr;
    }

    SLANG_NO_THROW SlangResult ModuleCapture::findEntryPointByName(
        char const*     name,
        slang::IEntryPoint**   outEntryPoint)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        SlangResult res = m_actualModule->findEntryPointByName(name, outEntryPoint);

        if (SLANG_OK == res)
        {
            EntryPointCapture* entryPointCapture = getEntryPointCapture(*outEntryPoint);
            *outEntryPoint = static_cast<slang::IEntryPoint*>(entryPointCapture);
        }
        return res;
    }

    SLANG_NO_THROW SlangInt32 ModuleCapture::getDefinedEntryPointCount()
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);
        SlangInt32 res = m_actualModule->getDefinedEntryPointCount();
        return res;
    }

    SLANG_NO_THROW SlangResult ModuleCapture::getDefinedEntryPoint(SlangInt32 index, slang::IEntryPoint** outEntryPoint)
    {
        // This call is to find the existing entry point, so it has been created already. Therefore, we don't create a new one
        // and assert the error if it is not found in our map.
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);
        SlangResult res = m_actualModule->getDefinedEntryPoint(index, outEntryPoint);

        if (*outEntryPoint)
        {
            EntryPointCapture* entryPointCapture = m_mapEntryPointToCapture.tryGetValue(*outEntryPoint);
            if (!entryPointCapture)
            {
                SLANG_CAPTURE_ASSERT(!"Entrypoint not found in mapEntryPointToCapture");
            }
            *outEntryPoint = static_cast<slang::IEntryPoint*>(entryPointCapture);
        }
        else
            *outEntryPoint = nullptr;

        return res;
    }

    SLANG_NO_THROW SlangResult ModuleCapture::serialize(ISlangBlob** outSerializedBlob)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);
        SlangResult res = m_actualModule->serialize(outSerializedBlob);
        return res;
    }

    SLANG_NO_THROW SlangResult ModuleCapture::writeToFile(char const* fileName)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);
        SlangResult res = m_actualModule->writeToFile(fileName);
        return res;
    }

    SLANG_NO_THROW const char* ModuleCapture::getName()
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);
        const char* res = m_actualModule->getName();
        return res;
    }

    SLANG_NO_THROW const char* ModuleCapture::getFilePath()
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);
        const char* res = m_actualModule->getFilePath();
        return res;
    }

    SLANG_NO_THROW const char* ModuleCapture::getUniqueIdentity()
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);
        const char* res = m_actualModule->getUniqueIdentity();
        return res;
    }

    SLANG_NO_THROW SlangResult ModuleCapture::findAndCheckEntryPoint(
        char const* name,
        SlangStage stage,
        slang::IEntryPoint** outEntryPoint,
        ISlangBlob** outDiagnostics)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);

        SlangResult res = m_actualModule->findAndCheckEntryPoint(name, stage, outEntryPoint, outDiagnostics);

        if (SLANG_OK == res)
        {
            EntryPointCapture* entryPointCapture = getEntryPointCapture(*outEntryPoint);
            *outEntryPoint = static_cast<slang::IEntryPoint*>(entryPointCapture);
        }
        return res;
    }

    SLANG_NO_THROW slang::ISession* ModuleCapture::getSession()
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);
        slang::ISession* session = m_actualModule->getSession();
        return session;
    }

    SLANG_NO_THROW slang::ProgramLayout* ModuleCapture::getLayout(
        SlangInt    targetIndex,
        slang::IBlob**     outDiagnostics)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);
        slang::ProgramLayout* programLayout = m_actualModule->getLayout(targetIndex, outDiagnostics);
        return programLayout;
    }

    SLANG_NO_THROW SlangInt ModuleCapture::getSpecializationParamCount()
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);
        SlangInt res = m_actualModule->getSpecializationParamCount();
        return res;
    }

    SLANG_NO_THROW SlangResult ModuleCapture::getEntryPointCode(
        SlangInt    entryPointIndex,
        SlangInt    targetIndex,
        slang::IBlob**     outCode,
        slang::IBlob**     outDiagnostics)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);
        SlangResult res = m_actualModule->getEntryPointCode(entryPointIndex, targetIndex, outCode, outDiagnostics);
        return res;
    }

    SLANG_NO_THROW SlangResult ModuleCapture::getTargetCode(
        SlangInt    targetIndex,
        slang::IBlob** outCode,
        slang::IBlob** outDiagnostics)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);
        SlangResult res = m_actualModule->getTargetCode(targetIndex, outCode, outDiagnostics);
        return res;
    }

    SLANG_NO_THROW SlangResult ModuleCapture::getResultAsFileSystem(
        SlangInt    entryPointIndex,
        SlangInt    targetIndex,
        ISlangMutableFileSystem** outFileSystem)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);
        SlangResult res = m_actualModule->getResultAsFileSystem(entryPointIndex, targetIndex, outFileSystem);
        return res;
    }

    SLANG_NO_THROW void ModuleCapture::getEntryPointHash(
        SlangInt    entryPointIndex,
        SlangInt    targetIndex,
        slang::IBlob**     outHash)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);
        m_actualModule->getEntryPointHash(entryPointIndex, targetIndex, outHash);
    }

    SLANG_NO_THROW SlangResult ModuleCapture::specialize(
        slang::SpecializationArg const*    specializationArgs,
        SlangInt                    specializationArgCount,
        slang::IComponentType**            outSpecializedComponentType,
        ISlangBlob**                outDiagnostics)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);
        SlangResult res = m_actualModule->specialize(specializationArgs, specializationArgCount, outSpecializedComponentType, outDiagnostics);
        return res;
    }

    SLANG_NO_THROW SlangResult ModuleCapture::link(
        IComponentType**            outLinkedComponentType,
        ISlangBlob**                outDiagnostics)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);
        SlangResult res = m_actualModule->link(outLinkedComponentType, outDiagnostics);
        return res;
    }

    SLANG_NO_THROW SlangResult ModuleCapture::getEntryPointHostCallable(
        int                     entryPointIndex,
        int                     targetIndex,
        ISlangSharedLibrary**   outSharedLibrary,
        slang::IBlob**          outDiagnostics)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);
        SlangResult res = m_actualModule->getEntryPointHostCallable(entryPointIndex, targetIndex, outSharedLibrary, outDiagnostics);
        return res;
    }

    SLANG_NO_THROW SlangResult ModuleCapture::renameEntryPoint(
        const char* newName, IComponentType** outEntryPoint)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);
        SlangResult res = m_actualModule->renameEntryPoint(newName, outEntryPoint);
        return res;
    }

    SLANG_NO_THROW SlangResult ModuleCapture::linkWithOptions(
        IComponentType** outLinkedComponentType,
        uint32_t compilerOptionEntryCount,
        slang::CompilerOptionEntry* compilerOptionEntries,
        ISlangBlob** outDiagnostics)
    {
        slangCaptureLog(LogLevel::Verbose, "%s\n", __PRETTY_FUNCTION__);
        SlangResult res = m_actualModule->linkWithOptions(outLinkedComponentType, compilerOptionEntryCount, compilerOptionEntries, outDiagnostics);
        return res;
    }

    EntryPointCapture* ModuleCapture::getEntryPointCapture(slang::IEntryPoint* entryPoint)
    {
        EntryPointCapture* entryPointCapture = nullptr;
        entryPointCapture = m_mapEntryPointToCapture.tryGetValue(entryPoint);
        if (!entryPointCapture)
        {
            entryPointCapture = new EntryPointCapture(entryPoint);
            Slang::ComPtr<EntryPointCapture> result(entryPointCapture);
            m_mapEntryPointToCapture.add(entryPoint, *result.detach());
        }
        return entryPointCapture;
    }
}
