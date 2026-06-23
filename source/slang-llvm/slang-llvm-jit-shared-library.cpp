#include "slang-llvm-jit-shared-library.h"

#include "llvm/ExecutionEngine/Orc/JITTargetMachineBuilder.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/TargetParser/Triple.h"

#include <core/slang-platform.h>
#include <core/slang-string.h>

namespace slang_llvm
{

void disableAVX512ForJIT(llvm::orc::LLJITBuilder& jitBuilder)
{
    // Opt-in mitigation: only subtract AVX-512 from the JIT TargetMachine
    // when SLANG_DISABLE_AVX512=1 in the environment. Default is to leave
    // AVX-512 alone, so production builds keep AVX-512 codegen on capable
    // hosts. CI workflows that hit #11062 set the env var on the test
    // step. When LLVM 22 lands (#11017) and its host detection no longer
    // mis-reports AVX-512 on the GitHub-Azure runners, the env var can
    // be dropped from the workflows and this whole helper becomes dead
    // code.
    Slang::StringBuilder envValue;
    if (SLANG_FAILED(Slang::PlatformUtil::getEnvironmentVariable(
            Slang::UnownedStringSlice("SLANG_DISABLE_AVX512"),
            envValue)) ||
        envValue.getUnownedSlice() != Slang::UnownedStringSlice::fromLiteral("1"))
        return;

    llvm::Expected<llvm::orc::JITTargetMachineBuilder> expectJTMB =
        llvm::orc::JITTargetMachineBuilder::detectHost();
    if (!expectJTMB)
    {
        // detectHost() failed (e.g. unsupported triple). Consume the Error so
        // the Expected destructor doesn't fire report_fatal_error in LLVM
        // builds with LLVM_ENABLE_ABI_BREAKING_CHECKS, log loudly so a future
        // SIGILL recurrence here is traceable to this branch via grep, and
        // leave the LLJITBuilder at its default. The default will run
        // detectHost() again inside LLJIT::Create, so if that path produces
        // a JTMB with AVX-512 enabled on a CPU that can't execute it, the
        // SIGILL we're trying to neutralise will reappear. Practically
        // unreachable on the x86_64-linux runners we care about (#11062),
        // but worth flagging if it ever does fire.
        llvm::errs() << "slang-llvm[#11062]: JITTargetMachineBuilder::detectHost() failed: "
                     << llvm::toString(expectJTMB.takeError())
                     << " — leaving LLJITBuilder at default; AVX-512 mitigation NOT applied\n";
        return;
    }
    if (expectJTMB->getTargetTriple().getArch() != llvm::Triple::x86_64)
    {
        // No AVX-512 to worry about on non-x86_64 hosts.
        return;
    }
    // Mitigation: pin the JIT CPU to the baseline "x86-64"  (SSE2 only,
    // no AVX/AVX2/AVX-512/FMA/BMI/etc.). Feature-subtraction alone
    // (`-avx512f`, `-avx`, …) was insufficient to workaround the problem
    // on GitHub CPU runners. Forcing the baseline x86-64 CPU sidesteps.
    expectJTMB->setCPU("x86-64");
    jitBuilder.setJITTargetMachineBuilder(std::move(*expectJTMB));
}

llvm::Expected<std::unique_ptr<llvm::orc::LLJIT>> createAVX512SafeLLJIT()
{
    llvm::orc::LLJITBuilder jitBuilder;
    disableAVX512ForJIT(jitBuilder);
    return jitBuilder.create();
}

ISlangUnknown* LLVMJITSharedLibrary::getInterface(const SlangUUID& guid)
{
    if (guid == ISlangUnknown::getTypeGuid() || guid == ISlangCastable::getTypeGuid() ||
        guid == ISlangSharedLibrary::getTypeGuid())
    {
        return static_cast<ISlangSharedLibrary*>(this);
    }
    return nullptr;
}

void* LLVMJITSharedLibrary::getObject(const SlangUUID& uuid)
{
    SLANG_UNUSED(uuid);
    return nullptr;
}

void* LLVMJITSharedLibrary::castAs(const Slang::Guid& guid)
{
    if (auto ptr = getInterface(guid))
    {
        return ptr;
    }
    return getObject(guid);
}

void* LLVMJITSharedLibrary::findSymbolAddressByName(char const* name)
{
    auto fn = m_jit->lookup(name);
    return fn ? (void*)fn.get().getValue() : nullptr;
}

} // namespace slang_llvm
