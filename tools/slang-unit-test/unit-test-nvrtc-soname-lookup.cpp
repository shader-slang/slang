// unit-test-nvrtc-soname-lookup.cpp

#include "compiler-core/slang-downstream-compiler-set.h"
#include "compiler-core/slang-nvrtc-compiler.h"
#include "core/slang-list.h"
#include "core/slang-smart-pointer.h"
#include "core/slang-string.h"
#include "slang-com-helper.h"
#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <string.h>

using namespace Slang;

// On Linux the unversioned `libnvrtc.so` exists only where the CUDA development packages are
// installed; runtime-only installs (the CUDA runtime container images, the `cuda-nvrtc-X-Y`
// packages, distro packages such as `libnvrtc12`, the `nvidia-cuda-nvrtc` pip wheel) provide only
// the versioned sonames, such as `libnvrtc.so.12`. Whether any given soname is loadable is a
// property of the machine, so no `.slang` test can reach this lookup: these tests pass a fake
// loader to `NVRTCDownstreamCompilerUtil::locateCompilers` directly, deciding per name whether a
// "library" exists, so one body can simulate a dev install, a runtime-only install, or a machine
// with no CUDA at all. The first two scenarios run on every platform; the soname scenarios run
// only on Linux, where the fallback they pin is compiled.

namespace
{

// `init` stores each of these entry points but only invokes `nvrtcVersion`, so every other symbol
// is only ever stored, never called, and a do-nothing stand-in suffices for it. `nvrtcVersion` gets
// a real function because it is called through the pointer; it answers NVRTC_SUCCESS (0) with a
// plausible version so `init` completes and the compiler is added to the set.
int fakeNvrtcVersion(int* major, int* minor)
{
    *major = 12;
    *minor = 8;
    return 0; // NVRTC_SUCCESS
}

void fakeNvrtcUnusedEntry() {}

// A shared library that exists only as a symbol table: every name the NVRTC downstream compiler
// requires resolves, which is all `init` asks of it.
class FakeNvrtcLibrary : public RefObject, public ISlangSharedLibrary
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
        return strcmp(name, "nvrtcVersion") == 0 ? (void*)fakeNvrtcVersion
                                                 : (void*)fakeNvrtcUnusedEntry;
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

// Records every name `locateCompilers` requests, in order, and hands out the fake library only
// for names the test marked as installed. `locateCompilers` probes the bare logical name first
// and only reaches a fallback after that fails, so the request order is itself the behaviour under
// test.
class RecordingLoader : public RefObject, public ISlangSharedLibraryLoader
{
public:
    SLANG_REF_OBJECT_IUNKNOWN_ALL

    List<String> m_requestedNames;
    List<String> m_installedNames;

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    loadSharedLibrary(const char* path, ISlangSharedLibrary** outLibrary) SLANG_OVERRIDE
    {
        const String name(path);
        m_requestedNames.add(name);
        if (m_installedNames.indexOf(name) < 0)
        {
            return SLANG_E_NOT_FOUND;
        }
        ComPtr<ISlangSharedLibrary> library(new FakeNvrtcLibrary);
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

SLANG_UNIT_TEST(nvrtcSonameLookup)
{
    // An explicit path (the `-nvrtc-path` command-line option, or
    // `IGlobalSession::setDownstreamCompilerPath(SLANG_PASS_THROUGH_NVRTC, ...)`) is the whole
    // search: nothing else is probed, so an unrelated install cannot be picked up over the user's
    // choice.
    {
        RefPtr<RecordingLoader> loader = new RecordingLoader;
        loader->m_installedNames.add("vendor/nvrtc/libnvrtc.so");

        RefPtr<DownstreamCompilerSet> set = new DownstreamCompilerSet;
        SLANG_CHECK(
            NVRTCDownstreamCompilerUtil::locateCompilers(
                String("vendor/nvrtc/libnvrtc.so"),
                loader,
                set) == SLANG_OK);
        SLANG_CHECK(set->hasCompiler(SLANG_PASS_THROUGH_NVRTC));
        SLANG_CHECK_ABORT(loader->m_requestedNames.getCount() == 1);
        SLANG_CHECK(loader->m_requestedNames[0] == "vendor/nvrtc/libnvrtc.so");
    }

    // An explicit path that fails to load must not fall back to auto-discovery: the user asked for
    // that library, so its absence is the answer.
    {
        RefPtr<RecordingLoader> loader = new RecordingLoader;

        RefPtr<DownstreamCompilerSet> set = new DownstreamCompilerSet;
        SLANG_CHECK(
            NVRTCDownstreamCompilerUtil::locateCompilers(
                String("vendor/nvrtc/libnvrtc.so"),
                loader,
                set) == SLANG_E_NOT_FOUND);
        SLANG_CHECK(!set->hasCompiler(SLANG_PASS_THROUGH_NVRTC));
        SLANG_CHECK_ABORT(loader->m_requestedNames.getCount() == 1);
        SLANG_CHECK(loader->m_requestedNames[0] == "vendor/nvrtc/libnvrtc.so");
    }

    // A dev install provides the unversioned file, so the first probe — the bare logical name
    // `"nvrtc"`, which the platform loader maps to that file — succeeds and no fallback name is
    // ever requested.
    {
        RefPtr<RecordingLoader> loader = new RecordingLoader;
        loader->m_installedNames.add("nvrtc");

        RefPtr<DownstreamCompilerSet> set = new DownstreamCompilerSet;
        SLANG_CHECK(
            NVRTCDownstreamCompilerUtil::locateCompilers(String(), loader, set) == SLANG_OK);
        SLANG_CHECK(set->hasCompiler(SLANG_PASS_THROUGH_NVRTC));
        SLANG_CHECK_ABORT(loader->m_requestedNames.getCount() == 1);
        SLANG_CHECK(loader->m_requestedNames[0] == "nvrtc");
    }

#if SLANG_LINUX_FAMILY
    // The scenarios below are Linux-only because the soname fallback they pin is compiled only in
    // the `SLANG_LINUX_FAMILY` branch of `_findAndLoadNVRTC`; on other platforms those names are
    // never probed, so the count assertions could not hold there.
    //
    // A runtime-only install has just a versioned soname. The lookup must fall back from the
    // unversioned name to the versioned sonames, newest first, and succeed. The expected probe
    // sequences here (with counts 3 and 4) are a golden copy of the soname list hardcoded in
    // `_findAndLoadNVRTC` (`kNVRTCSonames`, source/compiler-core/slang-nvrtc-compiler.cpp): when
    // that list deliberately changes, these expectations must be updated in lockstep; when it
    // has not, a failure here is a probe-order regression.
    {
        RefPtr<RecordingLoader> loader = new RecordingLoader;
        loader->m_installedNames.add("libnvrtc.so.12");

        RefPtr<DownstreamCompilerSet> set = new DownstreamCompilerSet;
        SLANG_CHECK(
            NVRTCDownstreamCompilerUtil::locateCompilers(String(), loader, set) == SLANG_OK);
        SLANG_CHECK(set->hasCompiler(SLANG_PASS_THROUGH_NVRTC));
        SLANG_CHECK_ABORT(loader->m_requestedNames.getCount() == 3);
        SLANG_CHECK(loader->m_requestedNames[0] == "nvrtc");
        SLANG_CHECK(loader->m_requestedNames[1] == "libnvrtc.so.13");
        SLANG_CHECK(loader->m_requestedNames[2] == "libnvrtc.so.12");
    }

    // With no NVRTC anywhere, every candidate is probed in order, the aggregate result is
    // SLANG_E_NOT_FOUND, and no compiler is added to the set.
    {
        RefPtr<RecordingLoader> loader = new RecordingLoader;

        RefPtr<DownstreamCompilerSet> set = new DownstreamCompilerSet;
        SLANG_CHECK(
            NVRTCDownstreamCompilerUtil::locateCompilers(String(), loader, set) ==
            SLANG_E_NOT_FOUND);
        SLANG_CHECK(!set->hasCompiler(SLANG_PASS_THROUGH_NVRTC));
        SLANG_CHECK_ABORT(loader->m_requestedNames.getCount() == 4);
        SLANG_CHECK(loader->m_requestedNames[0] == "nvrtc");
        SLANG_CHECK(loader->m_requestedNames[1] == "libnvrtc.so.13");
        SLANG_CHECK(loader->m_requestedNames[2] == "libnvrtc.so.12");
        SLANG_CHECK(loader->m_requestedNames[3] == "libnvrtc.so.11.2");
    }
#endif
}
