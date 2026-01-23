#ifndef SLANG_COMPONENT_TYPE2_H
#define SLANG_COMPONENT_TYPE2_H

#include "../../core/slang-smart-pointer.h"
#include "../../slang/slang-compiler.h"
#include "../util/record-utility.h"
#include "record-manager.h"
#include "slang-com-helper.h"
#include "slang-com-ptr.h"
#include "slang.h"

namespace SlangRecord
{
using namespace Slang;

class IComponentType2Recorder : public slang::IComponentType2, public RefObject
{
public:
    SLANG_COM_INTERFACE(
        0x0c23c81d,
        0x7e08,
        0x4a71,
        {0xa3, 0x0e, 0x90, 0xa2, 0xd7, 0x8a, 0xe4, 0x87})

    SLANG_REF_OBJECT_IUNKNOWN_ALL
    ISlangUnknown* getInterface(const Guid& guid);

    explicit IComponentType2Recorder(
        slang::IComponentType2* componentType,
        RecordManager* recordManager);

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL getTargetHostCallable(
        int targetIndex,
        ISlangSharedLibrary** outSharedLibrary,
        slang::IBlob** outDiagnostics = nullptr) override;
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL getTargetCompileResult(
        SlangInt targetIndex,
        slang::ICompileResult** outCompileResult,
        slang::IBlob** outDiagnostics = nullptr) override;
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL getEntryPointCompileResult(
        SlangInt entryPointIndex,
        SlangInt targetIndex,
        slang::ICompileResult** outCompileResult,
        slang::IBlob** outDiagnostics = nullptr) override;

    slang::IComponentType2* getActualComponentType() const { return m_actualComponentType2; }

protected:
    virtual ApiClassId getClassId() { return ApiClassId::Class_IComponentType2; }

    Slang::ComPtr<slang::IComponentType2> m_actualComponentType2;
    uint64_t m_componentType2Handle = 0;
    RecordManager* m_recordManager = nullptr;
};
} // namespace SlangRecord

#endif // #ifndef SLANG_COMPONENT_TYPE2_H
