#include "slang-llvm-jit-shared-library.h"

#if SLANG_WINDOWS_FAMILY && SLANG_PTR_IS_64
#include "llvm/Config/llvm-config.h"
#include "llvm/ExecutionEngine/Orc/RTDyldObjectLinkingLayer.h"
#include "llvm/ExecutionEngine/RTDyldMemoryManager.h"
#include "llvm/Support/Memory.h"
#include "llvm/Support/Process.h"
#endif
#include "llvm/ExecutionEngine/Orc/JITTargetMachineBuilder.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/TargetParser/Triple.h"

#include <core/slang-platform.h>
#include <core/slang-string.h>

namespace slang_llvm
{

#if SLANG_WINDOWS_FAMILY && SLANG_PTR_IS_64
namespace
{

// TODO: LLVM 23 switches LLJIT's default object layer from RuntimeDyld to JITLink, whose
// InProcessMemoryManager should not need this Windows SectionMemoryManager workaround.
static_assert(
    LLVM_VERSION_MAJOR < 23,
    "Remove OrderedRTDyldMemoryManager when upgrading to LLVM 23 or later; LLJIT uses JITLink's "
    "InProcessMemoryManager by default");

// Places each JIT object's code, read-only data, and writable data in one contiguous mapping in
// that order. This keeps every COFF ADDR32NB target at a non-negative 32-bit offset from the
// object's image base.
class OrderedRTDyldMemoryManager : public llvm::RTDyldMemoryManager
{
public:
    ~OrderedRTDyldMemoryManager()
    {
        if (m_memory.base())
            llvm::sys::Memory::releaseMappedMemory(m_memory);
    }

    bool needsToReserveAllocationSpace() override { return true; }

    void reserveAllocationSpace(
        uintptr_t codeSize,
        llvm::Align codeAlign,
        uintptr_t readOnlyDataSize,
        llvm::Align readOnlyDataAlign,
        uintptr_t readWriteDataSize,
        llvm::Align readWriteDataAlign) override
    {
        SLANG_ASSERT(!m_memory.base());

        const uintptr_t pageSize = llvm::sys::Process::getPageSizeEstimate();
        const uintptr_t codeAlignment = std::max(pageSize, codeAlign.value());
        const uintptr_t readOnlyDataAlignment = std::max(pageSize, readOnlyDataAlign.value());
        const uintptr_t readWriteDataAlignment = std::max(pageSize, readWriteDataAlign.value());
        const uintptr_t allocationAlignment =
            std::max(codeAlignment, std::max(readOnlyDataAlignment, readWriteDataAlignment));

        // ADDR32NB can only represent positions within the first 4 GiB of the image. Checking each
        // layout step against that limit also makes the additions in _alignUp safe on a 64-bit
        // host.
        if (codeSize > UINT32_MAX || readOnlyDataSize > UINT32_MAX ||
            readWriteDataSize > UINT32_MAX || allocationAlignment > UINT32_MAX)
        {
            m_error = "JIT object is too large for 32-bit image-relative relocations";
            return;
        }

        const uintptr_t readOnlyDataOffset = _alignUp(codeSize, readOnlyDataAlignment);
        if (readOnlyDataOffset > UINT32_MAX || readOnlyDataSize > UINT32_MAX - readOnlyDataOffset)
        {
            m_error = "JIT object is too large for 32-bit image-relative relocations";
            return;
        }
        const uintptr_t readWriteDataOffset =
            _alignUp(readOnlyDataOffset + readOnlyDataSize, readWriteDataAlignment);
        if (readWriteDataOffset > UINT32_MAX ||
            readWriteDataSize > UINT32_MAX - readWriteDataOffset)
        {
            m_error = "JIT object is too large for 32-bit image-relative relocations";
            return;
        }
        const uintptr_t imageSize = _alignUp(readWriteDataOffset + readWriteDataSize, pageSize);

        // allocateMappedMemory only guarantees allocation-granularity alignment. Reserve enough
        // prefix space to align the image for any larger section alignment too.
        const uintptr_t allocationSize = imageSize + allocationAlignment - 1;
        std::error_code error;
        m_memory = llvm::sys::Memory::allocateMappedMemory(
            allocationSize,
            nullptr,
            llvm::sys::Memory::MF_READ | llvm::sys::Memory::MF_WRITE,
            error);
        if (error)
        {
            m_error = error.message();
            return;
        }

        uint8_t* imageBase =
            reinterpret_cast<uint8_t*>(_alignUp(uintptr_t(m_memory.base()), allocationAlignment));
        uint8_t* readOnlyDataBase = imageBase + readOnlyDataOffset;
        uint8_t* readWriteDataBase = imageBase + readWriteDataOffset;
        m_code = {imageBase, imageBase, readOnlyDataBase, codeAlign.value()};
        m_readOnlyData =
            {readOnlyDataBase, readOnlyDataBase, readWriteDataBase, readOnlyDataAlign.value()};
        m_readWriteData = {
            readWriteDataBase,
            readWriteDataBase,
            imageBase + imageSize,
            readWriteDataAlign.value()};
    }

    uint8_t* allocateCodeSection(uintptr_t size, unsigned alignment, unsigned, llvm::StringRef)
        override
    {
        return _allocate(m_code, size, alignment);
    }

    uint8_t* allocateDataSection(
        uintptr_t size,
        unsigned alignment,
        unsigned,
        llvm::StringRef,
        bool isReadOnly) override
    {
        return _allocate(isReadOnly ? m_readOnlyData : m_readWriteData, size, alignment);
    }

    bool finalizeMemory(std::string* errorMessage) override
    {
        if (!m_error.empty())
        {
            if (errorMessage)
                *errorMessage = m_error;
            return true;
        }

        if (_protect(m_code, llvm::sys::Memory::MF_READ | llvm::sys::Memory::MF_EXEC, errorMessage))
            return true;
        if (_protect(m_readOnlyData, llvm::sys::Memory::MF_READ, errorMessage))
            return true;

        llvm::sys::Memory::InvalidateInstructionCache(
            m_code.begin,
            uintptr_t(m_code.cursor - m_code.begin));
        return false;
    }

private:
    struct Segment
    {
        uint8_t* begin = nullptr;
        uint8_t* cursor = nullptr;
        uint8_t* end = nullptr;
        uintptr_t alignment = 1;
    };

    static uintptr_t _alignUp(uintptr_t value, uintptr_t alignment)
    {
        SLANG_ASSERT(alignment && !(alignment & (alignment - 1)));
        return (value + alignment - 1) & ~(alignment - 1);
    }

    uint8_t* _allocate(Segment& segment, uintptr_t size, uintptr_t alignment)
    {
        if (!m_error.empty() || !segment.begin)
            return nullptr;

        if (!alignment)
            alignment = 16;
        SLANG_ASSERT(!(alignment & (alignment - 1)));
        SLANG_ASSERT(alignment <= segment.alignment);

        uintptr_t address = _alignUp(uintptr_t(segment.cursor), alignment);
        if (address > uintptr_t(segment.end) || size > uintptr_t(segment.end) - address)
        {
            m_error = "LLVM requested more JIT section memory than it reserved";
            return nullptr;
        }

        segment.cursor = reinterpret_cast<uint8_t*>(address + size);
        return reinterpret_cast<uint8_t*>(address);
    }

    static bool _protect(Segment& segment, unsigned permissions, std::string* errorMessage)
    {
        if (segment.begin == segment.end)
            return false;

        llvm::sys::MemoryBlock block(segment.begin, uintptr_t(segment.end - segment.begin));
        if (std::error_code error = llvm::sys::Memory::protectMappedMemory(block, permissions))
        {
            if (errorMessage)
                *errorMessage = error.message();
            return true;
        }
        return false;
    }

    llvm::sys::MemoryBlock m_memory;
    Segment m_code;
    Segment m_readOnlyData;
    Segment m_readWriteData;
    std::string m_error;
};

} // namespace
#endif

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

static void configureRTDyldForWindows64(llvm::orc::LLJITBuilder& jitBuilder)
{
#if SLANG_WINDOWS_FAMILY && SLANG_PTR_IS_64
    // LLVM 21's default LLJIT layer uses RuntimeDyld with a separate SectionMemoryManager for
    // each object. SectionMemoryManager makes separate virtual-memory allocations for code,
    // read-only data, and writable data. Windows can place a later allocation below the first
    // one, but COFF ADDR32NB relocations require every target to have a non-negative 32-bit offset
    // from the image base.
    //
    // Keep RuntimeDyld's established COFF behavior, but give each object one contiguous allocation
    // in the order required by its relocation model: code, read-only data, then writable data.
    jitBuilder.setObjectLinkingLayerCreator(
        [](llvm::orc::ExecutionSession& executionSession)
            -> llvm::Expected<std::unique_ptr<llvm::orc::ObjectLayer>>
        {
            auto getMemoryManager = [](const llvm::MemoryBuffer&)
            { return std::make_unique<OrderedRTDyldMemoryManager>(); };
            auto layer = std::make_unique<llvm::orc::RTDyldObjectLinkingLayer>(
                executionSession,
                std::move(getMemoryManager));
            layer->setOverrideObjectFlagsWithResponsibilityFlags(true);
            layer->setAutoClaimResponsibilityForObjectSymbols(true);
            return std::unique_ptr<llvm::orc::ObjectLayer>(std::move(layer));
        });
#else
    SLANG_UNUSED(jitBuilder);
#endif
}

llvm::Expected<std::unique_ptr<llvm::orc::LLJIT>> createSlangLLJIT()
{
    llvm::orc::LLJITBuilder jitBuilder;
    disableAVX512ForJIT(jitBuilder);
    configureRTDyldForWindows64(jitBuilder);
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
