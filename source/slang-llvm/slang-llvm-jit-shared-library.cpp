#include "slang-llvm-jit-shared-library.h"

#include "llvm/ExecutionEngine/Orc/JITTargetMachineBuilder.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/TargetParser/Triple.h"

#include <core/slang-platform.h>
#include <core/slang-string.h>

namespace slang_llvm
{

void disableHostSpecificFeaturesForJIT(llvm::orc::LLJITBuilder& jitBuilder)
{
    // Opt-in mitigation: avoid host-specific features in the JIT TargetMachine
    // when SLANG_DISABLE_AVX512=1 in the environment. Default is to leave
    // host codegen alone, so production builds keep native codegen on capable
    // hosts. CI workflows that hit #11062 set the env var on the test step.
    // When LLVM 22 lands (#11017) and its host detection no longer mis-reports
    // unsupported features on the GitHub-Azure runners, the env var can be
    // dropped from the workflows and this whole helper becomes dead code.
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
        // a JTMB with unsupported features enabled, the SIGILL we're trying
        // to neutralise will reappear. Practically
        // unreachable on the x86_64-linux runners we care about (#11062),
        // but worth flagging if it ever does fire.
        llvm::errs()
            << "slang-llvm[#11062]: JITTargetMachineBuilder::detectHost() failed: "
            << llvm::toString(expectJTMB.takeError())
            << " -- leaving LLJITBuilder at default; host-feature mitigation NOT applied\n";
        return;
    }
    if (expectJTMB->getTargetTriple().getArch() != llvm::Triple::x86_64)
    {
        // This mitigation is only needed for the x86_64 GitHub-Azure runners.
        return;
    }
    // Do not try to subtract a growing list of feature names from a host CPU
    // model. Some features are implied by the CPU name, and LLVM can learn new
    // feature spellings. Use a baseline x86-64 target for this CI-only path.
    expectJTMB->setCPU("x86-64");
    expectJTMB->setFeatures("");
    jitBuilder.setJITTargetMachineBuilder(std::move(*expectJTMB));
}

llvm::Expected<std::unique_ptr<llvm::orc::LLJIT>> createAVX512SafeLLJIT()
{
    llvm::orc::LLJITBuilder jitBuilder;
    disableHostSpecificFeaturesForJIT(jitBuilder);
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
