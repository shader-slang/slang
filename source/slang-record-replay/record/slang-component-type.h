#ifndef SLANG_COMPONENT_TYPE_H
#define SLANG_COMPONENT_TYPE_H

#include "slang-com-ptr.h"
#include "slang.h"
#include "slang-com-helper.h"
#include "../../core/slang-smart-pointer.h"
#include "../../slang/slang-compiler.h"
#include "record-manager.h"
#include "../util/record-utility.h"

namespace SlangRecord
{
    using namespace Slang;
    class IComponentTypeRecorder: public slang::IComponentType
    {
    public:
        explicit IComponentTypeRecorder(slang::IComponentType* componentType, RecordManager* recordManager);

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
        virtual SLANG_NO_THROW SlangResult SLANG_MCALL getTargetCode(
            SlangInt    targetIndex,
            slang::IBlob** outCode,
            slang::IBlob** outDiagnostics = nullptr) override;
    protected:
        virtual ApiClassId getClassId() = 0;
        Slang::ComPtr<slang::IComponentType> m_actualComponentType;
        uint64_t                             m_componentHandle = 0;
        RecordManager*                       m_recordManager = nullptr;
    private:

        IComponentTypeRecorder* getComponentTypeRecorder(slang::IComponentType* componentTypes);

        Dictionary<slang::IComponentType*, IComponentTypeRecorder*>   m_mapComponentTypeToRecorder;
        List<ComPtr<IComponentTypeRecorder>>                          m_componentTypeRecorderAlloation;
    };


    class ComponentTypeRecorder: public IComponentTypeRecorder, public RefObject
    {
    public:
        SLANG_COM_INTERFACE(0x3d7c9db8, 0x11ad, 0x457e, { 0xbc, 0x46, 0x7e, 0x6e, 0xb9, 0x9a, 0x00, 0x1e })

        SLANG_REF_OBJECT_IUNKNOWN_ALL
        ISlangUnknown* getInterface(const Guid& guid)
        {
            if (guid == ComponentTypeRecorder::getTypeGuid())
            {
                return static_cast<ISlangUnknown*>(this);
            }
            return nullptr;
        }

        explicit ComponentTypeRecorder(slang::IComponentType* componentType, RecordManager* recordManager)
            : IComponentTypeRecorder(componentType, recordManager)
        {
            slangRecordLog(LogLevel::Verbose, "%s: %p\n", __PRETTY_FUNCTION__, componentType);
        }

        slang::IComponentType* getActualComponentType() const { return m_actualComponentType; }
    protected:
        virtual ApiClassId getClassId() override
        {
            return ApiClassId::Class_IComponentType;
        }
    };
}

#endif // #ifndef SLANG_COMPONENT_TYPE_H
