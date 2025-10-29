#include "slang-com-helper.h"

#include "llvm/ExecutionEngine/Orc/LLJIT.h"

#include <core/slang-com-object.h>

namespace slang_llvm
{

/* This implementation uses atomic ref counting to ensure the shared libraries lifetime can outlive
the LLVMDownstreamCompileResult and the compilation that created it */
class LLVMJITSharedLibrary : public Slang::ComBaseObject, public ISlangSharedLibrary
{
public:
    // ISlangUnknown
    SLANG_COM_BASE_IUNKNOWN_ALL

    /// ICastable
    virtual SLANG_NO_THROW void* SLANG_MCALL castAs(const Slang::Guid& guid) SLANG_OVERRIDE;

    // ISlangSharedLibrary impl
    virtual SLANG_NO_THROW void* SLANG_MCALL findSymbolAddressByName(char const* name)
        SLANG_OVERRIDE;

    LLVMJITSharedLibrary(std::unique_ptr<llvm::orc::LLJIT> jit)
        : m_jit(std::move(jit))
    {
    }

protected:
    ISlangUnknown* getInterface(const SlangUUID& uuid);
    void* getObject(const SlangUUID& uuid);

    std::unique_ptr<llvm::orc::LLJIT> m_jit;
};

} // namespace slang_llvm
