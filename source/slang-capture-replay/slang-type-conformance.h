#ifndef SLANG_TYPE_CONFORMANCE_H
#define SLANG_TYPE_CONFORMANCE_H

#include "../../slang-com-ptr.h"
#include "../../slang.h"
#include "../../slang-com-helper.h"
#include "../core/slang-smart-pointer.h"
#include "../core/slang-dictionary.h"
#include "../slang/slang-compiler.h"

namespace SlangCapture
{
    using namespace Slang;
    class TypeConformanceCapture: public slang::ITypeConformance, public RefObject
    {
    public:
        SLANG_COM_INTERFACE(0x0e67d05d, 0xee0a, 0x41e1, { 0xb5, 0xa3, 0x23, 0xe3, 0xb0, 0xec, 0x33, 0xf1 })

        SLANG_REF_OBJECT_IUNKNOWN_ALL
        ISlangUnknown* getInterface(const Guid& guid);

        explicit TypeConformanceCapture(slang::ITypeConformance* typeConformance);
        ~TypeConformanceCapture();

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

        slang::ITypeConformance* getActualTypeConformance() const { return m_actualTypeConformance; }
    private:
        Slang::ComPtr<slang::ITypeConformance> m_actualTypeConformance;
    };
}
#endif // SLANG_TYPE_CONFORMANCE_H
