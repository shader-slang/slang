#include "slang-llvm-jit-shared-library.h"

#include "llvm/ExecutionEngine/Orc/JITTargetMachineBuilder.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/TargetParser/Triple.h"

namespace slang_llvm
{

void disableAVX512ForJIT(llvm::orc::LLJITBuilder& jitBuilder)
{
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
    // Disable the AVX-512 family on x86_64. We list the family extensions
    // explicitly rather than relying on -avx512f to imply them, so that if a
    // future LLVM weakens the implication chain the mitigation still holds.
    // LLVM-22-only feature names (avx512er, avx512pf, avx512_4fmaps,
    // avx512_4vnniw — all KNL/KNM Xeon Phi) are deliberately omitted: on
    // LLVM 21 they hit the unknown-feature warning path and pollute stderr
    // with one line per JIT compilation (~hundreds of lines per CI run).
    // Any chip that would need those is unreachable from our deployment, so
    // the perfect-coverage gain isn't worth the log noise.
    expectJTMB->addFeatures({
        "-avx512f",
        "-avx512cd",
        "-avx512dq",
        "-avx512bw",
        "-avx512vl",
        "-avx512vbmi",
        "-avx512vbmi2",
        "-avx512vnni",
        "-avx512bitalg",
        "-avx512vpopcntdq",
        "-avx512ifma",
        "-avx512vp2intersect",
        "-avx512fp16",
        "-avx512bf16",
    });
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
