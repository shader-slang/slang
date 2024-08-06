#ifndef SLANG_ENTRY_POINT_H
#define SLANG_ENTRY_POINT_H

#include "slang-com-ptr.h"
#include "slang.h"
#include "slang-com-helper.h"
#include "../../core/slang-smart-pointer.h"
#include "../../core/slang-dictionary.h"
#include "../../slang/slang-compiler.h"
#include "record-manager.h"

namespace SlangRecord
{
    using namespace Slang;
    class EntryPointRecorder : public slang::IEntryPoint, public RefObject
    {
    public:
        SLANG_COM_INTERFACE(0xf4c1e23d, 0xb321, 0x4931, { 0x8f, 0x37, 0xf1, 0x22, 0x6a, 0xf9, 0x20, 0x85 })

        SLANG_REF_OBJECT_IUNKNOWN_ALL
        ISlangUnknown* getInterface(const Guid& guid);

        explicit EntryPointRecorder(slang::IEntryPoint* entryPoint, RecordManager* recordManager);
        ~EntryPointRecorder();

        // Interfaces for `IComponentType`
        virtual SLANG_NO_THROW slang::ISession* SLANG_MCALL getSession() override;
        virtual SLANG_NO_THROW slang::ProgramLayout* SLANG_MCALL getLayout(
            SlangInt    targetIndex = 0,
            slang::IBlob**     outDiagnostics = nullptr) override;
        virtual SLANG_NO_THROW SlangInt SLANG_MCALL getSpecializationParamCount() override;
        virtual SLANG_NO_THROW SlangResult SLANG_MCALL getEntryPointCode(
            SlangInt    entryPointIndex,
            SlangInt    targetIndex,
            slang::IBlob**     outCode,
            slang::IBlob**     outDiagnostics = nullptr) override;
        virtual SLANG_NO_THROW SlangResult SLANG_MCALL getTargetCode(
            SlangInt    targetIndex,
            slang::IBlob** outCode,
            slang::IBlob** outDiagnostics = nullptr) override;
        virtual SLANG_NO_THROW SlangResult SLANG_MCALL getResultAsFileSystem(
            SlangInt    entryPointIndex,
            SlangInt    targetIndex,
            ISlangMutableFileSystem** outFileSystem) override;
        virtual SLANG_NO_THROW void SLANG_MCALL getEntryPointHash(
            SlangInt    entryPointIndex,
            SlangInt    targetIndex,
            slang::IBlob**     outHash) override;
        virtual SLANG_NO_THROW SlangResult SLANG_MCALL specialize(
            slang::SpecializationArg const*    specializationArgs,
            SlangInt                    specializationArgCount,
            slang::IComponentType**            outSpecializedComponentType,
            ISlangBlob**                outDiagnostics = nullptr) override;
        virtual SLANG_NO_THROW SlangResult SLANG_MCALL link(
            slang::IComponentType**            outLinkedComponentType,
            ISlangBlob**                outDiagnostics = nullptr) override;
        virtual SLANG_NO_THROW SlangResult SLANG_MCALL getEntryPointHostCallable(
            int                     entryPointIndex,
            int                     targetIndex,
            ISlangSharedLibrary**   outSharedLibrary,
            slang::IBlob**          outDiagnostics = 0) override;
        virtual SLANG_NO_THROW SlangResult SLANG_MCALL renameEntryPoint(
            const char* newName, IComponentType** outEntryPoint) override;
        virtual SLANG_NO_THROW SlangResult SLANG_MCALL linkWithOptions(
            IComponentType** outLinkedComponentType,
            uint32_t compilerOptionEntryCount,
            slang::CompilerOptionEntry* compilerOptionEntries,
            ISlangBlob** outDiagnostics = nullptr) override;
        virtual SLANG_NO_THROW slang::FunctionReflection* SLANG_MCALL getFunctionReflection() override;
        slang::IEntryPoint* getActualEntryPoint() const { return m_actualEntryPoint; }
    private:
        Slang::ComPtr<slang::IEntryPoint>   m_actualEntryPoint;
        uint64_t                            m_entryPointHandle = 0;
        RecordManager*                     m_recordManager = nullptr;
    };
}
#endif // SLANG_ENTRY_POINT_H
