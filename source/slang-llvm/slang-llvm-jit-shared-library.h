#include "slang-com-helper.h"

#include "llvm/ExecutionEngine/Orc/LLJIT.h"

#include <core/slang-com-object.h>

namespace slang_llvm
{

/// Disable the AVX-512 feature family on the JIT TargetMachine before LLJIT
/// construction, **only when the SLANG_DISABLE_AVX512 environment variable
/// is set to "1"**. Default is a no-op so production builds keep AVX-512
/// codegen on capable hosts. CI workflows that hit #11062 set the env
/// var on the test step.
///
/// On x86_64 with the var set, builds an explicit JITTargetMachineBuilder
/// via detectHost() and subtracts every AVX-512 feature LLVM might
/// recognise, then hands it to the LLJITBuilder. On non-x86_64 hosts (or
/// without the env var) this is a no-op.
///
/// Prefer `createSlangLLJIT()` over calling this helper directly: it
/// pairs the disable step with `LLJITBuilder::create()` so a future caller
/// can't accidentally construct an LLJIT without the mitigation. See
/// https://github.com/shader-slang/slang/issues/11062.
///
/// TODO(#11017): once the LLVM 22 bump lands, the underlying detectHost()
/// mis-reporting may go away — drop SLANG_DISABLE_AVX512 from the CI
/// workflows, observe whether the merge queue stays clean, and if so
/// remove this helper entirely.
void disableAVX512ForJIT(llvm::orc::LLJITBuilder& jitBuilder);

/// Construct an LLJIT using Slang's platform configuration.
///
/// `disableAVX512ForJIT` only fires when SLANG_DISABLE_AVX512=1 is set in
/// the environment. On 64-bit Windows, RuntimeDyld uses one ordered allocation
/// per object so COFF image-relative relocations always have valid offsets.
/// Use this from every LLJIT construction site in slang-llvm so neither
/// configuration can be forgotten.
llvm::Expected<std::unique_ptr<llvm::orc::LLJIT>> createSlangLLJIT();

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
