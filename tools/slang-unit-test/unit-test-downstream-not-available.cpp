#include "compiler-core/slang-downstream-compiler-util.h"
#include "compiler-core/slang-glslang-compiler.h"
#include "compiler-core/slang-tint-compiler.h"
#include "core/slang-shared-library.h"
#include "slang-com-helper.h"
#include "slang-com-ptr.h"
#include "slang-glslang/slang-glslang.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

// `validate` distinguishes "this compiler cannot validate at all" (`SLANG_E_NOT_AVAILABLE`) from
// "the module was examined and rejected" (`SLANG_FAIL`). The disassembly entry points need the same
// distinction for the same reason: `init` accepts a `slang-glslang` that exports no disassembler,
// so a caller that collapses both into one failure reports a valid module as broken.
//
// These tests pin the exact result codes. A test asserting only `SLANG_FAILED(result)` would pass
// whichever code the implementation returned, so it could not detect the regression these guard
// against.

namespace
{

enum class FakeDisassemblerState
{
    Absent,
    PresentFailing,
    PresentSucceeding,
};

// Set by the fake disassemblers so a test can tell "returned false" from "was never called".
bool gFakeDisassemblerWasCalled = false;

bool fakeDisassembleReturningFalse(const uint32_t* contents, int contentsSize)
{
    SLANG_UNUSED(contents);
    SLANG_UNUSED(contentsSize);
    gFakeDisassemblerWasCalled = true;
    return false;
}

bool fakeDisassembleReturningTrue(const uint32_t* contents, int contentsSize)
{
    SLANG_UNUSED(contents);
    SLANG_UNUSED(contentsSize);
    gFakeDisassemblerWasCalled = true;
    return true;
}

bool fakeDisassembleWithResultReturningFalse(
    const uint32_t* contents,
    int contentsSize,
    char** outString)
{
    SLANG_UNUSED(contents);
    SLANG_UNUSED(contentsSize);
    SLANG_UNUSED(outString);
    gFakeDisassemblerWasCalled = true;
    return false;
}

// Returned to the compiler as the disassembly text. Static storage keeps it valid without a
// matching `glslang_freeDisassembly` export, whose absence the compiler already tolerates.
char gFakeDisassemblyText[] = "; fake disassembly";

bool fakeDisassembleWithResultReturningTrue(
    const uint32_t* contents,
    int contentsSize,
    char** outString)
{
    SLANG_UNUSED(contents);
    SLANG_UNUSED(contentsSize);
    gFakeDisassemblerWasCalled = true;
    *outString = gFakeDisassemblyText;
    return true;
}

int fakeCompileIdentity(glslang_CompileRequest_1_3* request)
{
    if (!request || !request->outputFunc)
    {
        return 1;
    }
    const char* begin = (const char*)request->inputBegin;
    const char* end = (const char*)request->inputEnd;
    request->outputFunc(begin, size_t(end - begin), request->outputUserData);
    return 0;
}

// A shared library that exists only as a symbol table, so a test can choose exactly which
// capabilities the loaded `slang-glslang` appears to have.
class FakeGlslangLibrary : public RefObject, public ISlangSharedLibrary
{
public:
    SLANG_REF_OBJECT_IUNKNOWN_ALL

    explicit FakeGlslangLibrary(FakeDisassemblerState disassemblerState)
        : m_disassemblerState(disassemblerState)
    {
    }

    virtual SLANG_NO_THROW void* SLANG_MCALL castAs(const SlangUUID& guid) SLANG_OVERRIDE
    {
        return getInterface(guid);
    }

    virtual SLANG_NO_THROW void* SLANG_MCALL findSymbolAddressByName(char const* name)
        SLANG_OVERRIDE
    {
        UnownedStringSlice symbol(name);

        // At least one compile entry point must resolve, otherwise `init` rejects the library
        // outright and we would be testing the load-failure path instead.
        if (symbol == "glslang_compile_1_3")
        {
            return (void*)fakeCompileIdentity;
        }
        if (symbol == "glslang_disassembleSPIRV")
        {
            switch (m_disassemblerState)
            {
            case FakeDisassemblerState::PresentFailing:
                return (void*)fakeDisassembleReturningFalse;
            case FakeDisassemblerState::PresentSucceeding:
                return (void*)fakeDisassembleReturningTrue;
            default:
                return nullptr;
            }
        }
        if (symbol == "glslang_disassembleSPIRVWithResult")
        {
            switch (m_disassemblerState)
            {
            case FakeDisassemblerState::PresentFailing:
                return (void*)fakeDisassembleWithResultReturningFalse;
            case FakeDisassemblerState::PresentSucceeding:
                return (void*)fakeDisassembleWithResultReturningTrue;
            default:
                return nullptr;
            }
        }
        return nullptr;
    }

protected:
    void* getInterface(const Guid& guid)
    {
        return (guid == ISlangUnknown::getTypeGuid() || guid == ICastable::getTypeGuid() ||
                guid == ISlangSharedLibrary::getTypeGuid())
                   ? static_cast<ISlangSharedLibrary*>(this)
                   : nullptr;
    }

    FakeDisassemblerState m_disassemblerState;
};

class FakeLoader : public RefObject, public ISlangSharedLibraryLoader
{
public:
    SLANG_REF_OBJECT_IUNKNOWN_ALL

    explicit FakeLoader(FakeDisassemblerState disassemblerState)
        : m_disassemblerState(disassemblerState)
    {
    }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    loadSharedLibrary(const char* path, ISlangSharedLibrary** outLibrary) SLANG_OVERRIDE
    {
        // On unix `locateGlslangSpirvDownstreamCompiler` probes pthread first; failing those keeps
        // the fake library bound to the one request we care about.
        UnownedStringSlice request(path);
        if (request.indexOf(UnownedStringSlice("slang-glslang")) < 0)
        {
            return SLANG_E_NOT_FOUND;
        }

        ComPtr<ISlangSharedLibrary> library(new FakeGlslangLibrary(m_disassemblerState));
        *outLibrary = library.detach();
        return SLANG_OK;
    }

protected:
    ISlangUnknown* getInterface(const Guid& guid)
    {
        return (guid == ISlangUnknown::getTypeGuid() ||
                guid == ISlangSharedLibraryLoader::getTypeGuid())
                   ? static_cast<ISlangSharedLibraryLoader*>(this)
                   : nullptr;
    }

    FakeDisassemblerState m_disassemblerState;
};

IDownstreamCompiler* getFakeSpirvOptCompiler(
    RefPtr<DownstreamCompilerSet>& ioSet,
    ISlangSharedLibraryLoader* loader,
    SlangResult& outLocateResult)
{
    ioSet = new DownstreamCompilerSet;
    outLocateResult = SpirvOptDownstreamCompilerUtil::locateCompilers(String(), loader, ioSet);
    if (SLANG_FAILED(outLocateResult))
    {
        return nullptr;
    }
    DownstreamCompilerDesc desc;
    desc.type = SLANG_PASS_THROUGH_SPIRV_OPT;
    return DownstreamCompilerUtil::findCompiler(
        ioSet,
        DownstreamCompilerUtil::MatchType::Newest,
        desc);
}

// A minimal, well-formed SPIR-V header. The fake disassemblers ignore the contents.
const uint32_t kSpirvHeader[] = {0x07230203, 0x00010000, 0x00080001, 1, 0};

} // namespace

// When the disassembler entry point is absent, both disassembly methods must say "not available"
// rather than "failed", so a caller can tell a missing capability from a bad module.
SLANG_UNIT_TEST(downstreamDisassembleReportsUnavailableWhenSymbolMissing)
{
    ComPtr<ISlangSharedLibraryLoader> loader(new FakeLoader(FakeDisassemblerState::Absent));
    RefPtr<DownstreamCompilerSet> set;
    SlangResult locateResult = SLANG_OK;
    IDownstreamCompiler* compiler = getFakeSpirvOptCompiler(set, loader, locateResult);
    if (locateResult == SLANG_E_NOT_AVAILABLE)
    {
        // A build with `SLANG_ENABLE_GLSLANG_SUPPORT=0` compiles the locator as a stub returning
        // exactly this code, so there is no compiler to exercise. Any other locator failure is a
        // real regression and must still abort below.
        SLANG_IGNORE_TEST;
    }
    SLANG_CHECK_ABORT(compiler != nullptr);

    gFakeDisassemblerWasCalled = false;
    const SlangResult disassembleResult =
        compiler->disassemble(kSpirvHeader, int(SLANG_COUNT_OF(kSpirvHeader)));
    SLANG_CHECK(disassembleResult == SLANG_E_NOT_AVAILABLE);

    String disassemblyText;
    const SlangResult withResultResult = compiler->disassembleWithResult(
        kSpirvHeader,
        int(SLANG_COUNT_OF(kSpirvHeader)),
        disassemblyText);
    SLANG_CHECK(withResultResult == SLANG_E_NOT_AVAILABLE);

    SLANG_CHECK(!gFakeDisassemblerWasCalled);
}

// The control for the test above: a disassembler that is present and fails must keep returning
// `SLANG_FAIL`, so an unconditional `SLANG_E_NOT_AVAILABLE` cannot satisfy both tests.
SLANG_UNIT_TEST(downstreamDisassembleReportsFailWhenDisassemblerFails)
{
    ComPtr<ISlangSharedLibraryLoader> loader(new FakeLoader(FakeDisassemblerState::PresentFailing));
    RefPtr<DownstreamCompilerSet> set;
    SlangResult locateResult = SLANG_OK;
    IDownstreamCompiler* compiler = getFakeSpirvOptCompiler(set, loader, locateResult);
    if (locateResult == SLANG_E_NOT_AVAILABLE)
    {
        SLANG_IGNORE_TEST;
    }
    SLANG_CHECK_ABORT(compiler != nullptr);

    gFakeDisassemblerWasCalled = false;
    const SlangResult disassembleResult =
        compiler->disassemble(kSpirvHeader, int(SLANG_COUNT_OF(kSpirvHeader)));
    SLANG_CHECK(disassembleResult == SLANG_FAIL);
    SLANG_CHECK(disassembleResult != SLANG_E_NOT_AVAILABLE);
    // Proves the result came from the disassembler rather than from an early-out.
    SLANG_CHECK(gFakeDisassemblerWasCalled);

    gFakeDisassemblerWasCalled = false;
    String disassemblyText;
    const SlangResult withResultResult = compiler->disassembleWithResult(
        kSpirvHeader,
        int(SLANG_COUNT_OF(kSpirvHeader)),
        disassemblyText);
    SLANG_CHECK(withResultResult == SLANG_FAIL);
    SLANG_CHECK(withResultResult != SLANG_E_NOT_AVAILABLE);
    SLANG_CHECK(gFakeDisassemblerWasCalled);
}

// A present, working disassembler must still report success and hand back its text, so neither
// failure code can be returned unconditionally.
SLANG_UNIT_TEST(downstreamDisassembleSucceedsWhenDisassemblerWorks)
{
    ComPtr<ISlangSharedLibraryLoader> loader(
        new FakeLoader(FakeDisassemblerState::PresentSucceeding));
    RefPtr<DownstreamCompilerSet> set;
    SlangResult locateResult = SLANG_OK;
    IDownstreamCompiler* compiler = getFakeSpirvOptCompiler(set, loader, locateResult);
    if (locateResult == SLANG_E_NOT_AVAILABLE)
    {
        SLANG_IGNORE_TEST;
    }
    SLANG_CHECK_ABORT(compiler != nullptr);

    const SlangResult disassembleResult =
        compiler->disassemble(kSpirvHeader, int(SLANG_COUNT_OF(kSpirvHeader)));
    SLANG_CHECK(disassembleResult == SLANG_OK);

    String disassemblyText;
    const SlangResult withResultResult = compiler->disassembleWithResult(
        kSpirvHeader,
        int(SLANG_COUNT_OF(kSpirvHeader)),
        disassemblyText);
    SLANG_CHECK(withResultResult == SLANG_OK);
    SLANG_CHECK(disassemblyText == String(gFakeDisassemblyText));
}

namespace
{

// A compiler that overrides nothing, so the calls below land on the `DownstreamCompilerBase`
// defaults. Those defaults are what an out-of-tree or partially-implemented compiler inherits, and
// no other test binds them.
class DefaultsOnlyCompiler : public DownstreamCompilerBase
{
public:
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    compile(const CompileOptions& options, IArtifact** outArtifact) SLANG_OVERRIDE
    {
        SLANG_UNUSED(options);
        SLANG_UNUSED(outArtifact);
        return SLANG_E_NOT_IMPLEMENTED;
    }

    virtual SLANG_NO_THROW bool SLANG_MCALL isFileBased() SLANG_OVERRIDE { return false; }
};

} // namespace

// The base-class defaults report an absent capability, not a failure, because a compiler that does
// not implement these cannot have examined anything.
SLANG_UNIT_TEST(downstreamCompilerBaseDefaultsReportNotAvailable)
{
    ComPtr<IDownstreamCompiler> compiler(new DefaultsOnlyCompiler());

    SLANG_CHECK(
        compiler->validate(kSpirvHeader, int(SLANG_COUNT_OF(kSpirvHeader))) ==
        SLANG_E_NOT_AVAILABLE);
    SLANG_CHECK(
        compiler->disassemble(kSpirvHeader, int(SLANG_COUNT_OF(kSpirvHeader))) ==
        SLANG_E_NOT_AVAILABLE);

    String disassemblyText;
    SLANG_CHECK(
        compiler->disassembleWithResult(
            kSpirvHeader,
            int(SLANG_COUNT_OF(kSpirvHeader)),
            disassemblyText) == SLANG_E_NOT_AVAILABLE);

    ComPtr<ISlangBlob> versionString;
    SLANG_CHECK(compiler->getVersionString(versionString.writeRef()) == SLANG_E_NOT_AVAILABLE);
    // The default must still clear the out parameter, since a caller may inspect it either way.
    SLANG_CHECK(versionString == nullptr);
}

namespace
{

// A fake `slang-tint`, which needs only the two symbols `TintDownstreamCompiler::init` requires.
// Neither is ever called -- the test only asks the resulting compiler for its version -- and `init`
// casts each looked-up address to its own function-pointer type, so only the addresses matter. That
// lets these stay untyped rather than pulling in the Tint headers, which are private to
// `compiler-core`'s include path.
void fakeTintCompile() {}

void fakeTintFreeResult() {}

class FakeTintLibrary : public RefObject, public ISlangSharedLibrary
{
public:
    SLANG_REF_OBJECT_IUNKNOWN_ALL

    virtual SLANG_NO_THROW void* SLANG_MCALL castAs(const SlangUUID& guid) SLANG_OVERRIDE
    {
        return getInterface(guid);
    }

    virtual SLANG_NO_THROW void* SLANG_MCALL findSymbolAddressByName(char const* name)
        SLANG_OVERRIDE
    {
        UnownedStringSlice symbol(name);
        if (symbol == "tint_compile")
        {
            return (void*)fakeTintCompile;
        }
        if (symbol == "tint_free_result")
        {
            return (void*)fakeTintFreeResult;
        }
        return nullptr;
    }

protected:
    void* getInterface(const Guid& guid)
    {
        return (guid == ISlangUnknown::getTypeGuid() || guid == ICastable::getTypeGuid() ||
                guid == ISlangSharedLibrary::getTypeGuid())
                   ? static_cast<ISlangSharedLibrary*>(this)
                   : nullptr;
    }
};

class FakeTintLoader : public RefObject, public ISlangSharedLibraryLoader
{
public:
    SLANG_REF_OBJECT_IUNKNOWN_ALL

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    loadSharedLibrary(const char* path, ISlangSharedLibrary** outLibrary) SLANG_OVERRIDE
    {
        UnownedStringSlice request(path);
        if (request.indexOf(UnownedStringSlice("slang-tint")) < 0)
        {
            return SLANG_E_NOT_FOUND;
        }

        ComPtr<ISlangSharedLibrary> library(new FakeTintLibrary());
        *outLibrary = library.detach();
        return SLANG_OK;
    }

protected:
    ISlangUnknown* getInterface(const Guid& guid)
    {
        return (guid == ISlangUnknown::getTypeGuid() ||
                guid == ISlangSharedLibraryLoader::getTypeGuid())
                   ? static_cast<ISlangSharedLibraryLoader*>(this)
                   : nullptr;
    }
};

} // namespace

// `TintDownstreamCompiler` overrides `getVersionString`, so the base-class default cannot stand in
// for it. Unlike the glslang locator there is no build configuration that stubs this one out, so a
// locator failure here is always a real failure.
SLANG_UNIT_TEST(tintGetVersionStringReportsNotAvailable)
{
    ComPtr<ISlangSharedLibraryLoader> loader(new FakeTintLoader());
    RefPtr<DownstreamCompilerSet> set(new DownstreamCompilerSet);
    const SlangResult locateResult =
        TintDownstreamCompilerUtil::locateCompilers(String(), loader, set);
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(locateResult));

    DownstreamCompilerDesc desc;
    desc.type = SLANG_PASS_THROUGH_TINT;
    IDownstreamCompiler* compiler =
        DownstreamCompilerUtil::findCompiler(set, DownstreamCompilerUtil::MatchType::Newest, desc);
    SLANG_CHECK_ABORT(compiler != nullptr);

    ComPtr<ISlangBlob> versionString;
    SLANG_CHECK(compiler->getVersionString(versionString.writeRef()) == SLANG_E_NOT_AVAILABLE);
}
