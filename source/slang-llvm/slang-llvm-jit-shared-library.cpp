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
        // builds with LLVM_ENABLE_ABI_BREAKING_CHECKS, log a hint that a
        // future SIGILL recurrence here points back to this branch, and
        // leave the LLJITBuilder at its default. The default will run
        // detectHost() again inside LLJIT::Create, so if that path produces
        // a JTMB with AVX-512 enabled on a CPU that can't execute it, the
        // SIGILL we're trying to neutralise will reappear. Practically
        // unreachable on the GH-hosted x86_64-linux runners we care about
        // (#11062), but worth flagging if it ever does fire.
        llvm::errs() << "slang-llvm: JITTargetMachineBuilder::detectHost() failed: "
                     << llvm::toString(expectJTMB.takeError())
                     << " — leaving LLJITBuilder at default; #11062 mitigation will not apply\n";
        return;
    }
    if (expectJTMB->getTargetTriple().getArch() != llvm::Triple::x86_64)
    {
        // No AVX-512 to worry about on non-x86_64 hosts.
        return;
    }
    // Subtract every AVX-512 family feature LLVM exposes. The base feature
    // (avx512f) is enough to neutralise the family because higher extensions
    // imply it, but we list everything explicitly to be defensive against
    // version-specific implication tables. LLVM silently ignores feature
    // names it doesn't recognise (e.g. avx512_4fmaps on LLVM < 22).
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
        "-avx512er",
        "-avx512pf",
        "-avx512_4fmaps",
        "-avx512_4vnniw",
    });
    jitBuilder.setJITTargetMachineBuilder(std::move(*expectJTMB));
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
