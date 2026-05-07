#include "slang-com-helper.h"

#include "llvm/ExecutionEngine/Orc/LLJIT.h"

#include <core/slang-com-object.h>

namespace slang_llvm
{

/// Disable the AVX-512 feature family on the JIT TargetMachine before LLJIT
/// construction. On x86_64, builds an explicit JITTargetMachineBuilder via
/// detectHost() and subtracts every AVX-512 feature LLVM might recognise, then
/// hands it to the LLJITBuilder. On non-x86_64 hosts this is a no-op.
///
/// Prefer `createAVX512SafeLLJIT()` over calling this helper directly: it
/// pairs the disable step with `LLJITBuilder::create()` so a future caller
/// can't accidentally construct an LLJIT with AVX-512 enabled. See
/// https://github.com/shader-slang/slang/issues/11062.
///
/// TODO(#11070): remove once the CPUID+xgetbv-based defensive probe lands.
/// That follow-up only subtracts AVX-512 when the host genuinely can't run
/// it, restoring AVX-512 codegen on capable hosts.
void disableAVX512ForJIT(llvm::orc::LLJITBuilder& jitBuilder);

/// Construct an LLJIT with AVX-512 disabled in its JIT TargetMachine.
/// Equivalent to:
///
///   LLJITBuilder b;
///   disableAVX512ForJIT(b);
///   return b.create();
///
/// Use this from every LLJIT construction site in slang-llvm so the AVX-512
/// mitigation can't be forgotten. See #11062.
llvm::Expected<std::unique_ptr<llvm::orc::LLJIT>> createAVX512SafeLLJIT();

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
