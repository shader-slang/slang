// unit-test-vtable-stability.cpp
//
// Verifies that the vtable slot layout of every COM interface declared in
// include/slang.h has not changed.  Each test creates a concrete probe object
// that records which override was called, then dispatches through a specific
// raw vtable slot and asserts the expected method fired.
//
// If a virtual method is inserted in the middle of an interface the slot
// indices of all subsequent methods shift by one, causing a different probe
// method to fire and the corresponding SLANG_CHECK to fail.
//
// Calling convention note: callSlot() casts every vtable entry to
// void(SLANG_MCALL*)(void*) and calls it with only `this`. The actual methods
// have varying signatures; calling through a mismatched pointer is undefined
// behavior per [expr.call]. On all 64-bit ABIs Slang targets (x86_64, ARM64)
// this is benign: `this` lands in the first integer register, remaining
// argument registers hold garbage that the probe overrides never read, and the
// caller handles stack cleanup (no __stdcall callee-cleanup mismatch). On
// 32-bit x86 with __stdcall the callee would clean the stack for its declared
// parameters, corrupting the stack when called with fewer arguments, so the
// entire test is guarded by SLANG_PTR_IS_64.

#include "slang.h"
#include "unit-test/slang-unit-test.h"

using namespace slang;

#if SLANG_PTR_IS_64

// ---------------------------------------------------------------------------
// Helper: call vtable slot `slot` on `obj`, passing only `this`.
// ---------------------------------------------------------------------------
static void callSlot(void* obj, int slot)
{
    void** vtbl = *reinterpret_cast<void***>(obj);
    reinterpret_cast<void(SLANG_MCALL*)(void*)>(vtbl[slot])(obj);
}

// ---------------------------------------------------------------------------
// ISlangUnknown  (3 slots: 0-2)
// ---------------------------------------------------------------------------
struct ISlangUnknownProbe : ISlangUnknown
{
    int lastSlot = -1;
    SLANG_NO_THROW SlangResult SLANG_MCALL queryInterface(SlangUUID const&, void**) SLANG_OVERRIDE
    {
        lastSlot = 0;
        return SLANG_OK;
    }
    SLANG_NO_THROW uint32_t SLANG_MCALL addRef() SLANG_OVERRIDE
    {
        lastSlot = 1;
        return 1;
    }
    SLANG_NO_THROW uint32_t SLANG_MCALL release() SLANG_OVERRIDE
    {
        lastSlot = 2;
        return 1;
    }
};

SLANG_UNIT_TEST(vtableISlangUnknown)
{
    ISlangUnknownProbe p;
    callSlot(&p, 0);
    SLANG_CHECK(p.lastSlot == 0); // queryInterface
    callSlot(&p, 1);
    SLANG_CHECK(p.lastSlot == 1); // addRef
    callSlot(&p, 2);
    SLANG_CHECK(p.lastSlot == 2); // release
}

// ---------------------------------------------------------------------------
// ISlangCastable : ISlangUnknown  (own slot 3)
// ---------------------------------------------------------------------------
struct ISlangCastableProbe : ISlangCastable
{
    int lastSlot = -1;
    SLANG_NO_THROW SlangResult SLANG_MCALL queryInterface(SlangUUID const&, void**) SLANG_OVERRIDE
    {
        lastSlot = 0;
        return SLANG_OK;
    }
    SLANG_NO_THROW uint32_t SLANG_MCALL addRef() SLANG_OVERRIDE
    {
        lastSlot = 1;
        return 1;
    }
    SLANG_NO_THROW uint32_t SLANG_MCALL release() SLANG_OVERRIDE
    {
        lastSlot = 2;
        return 1;
    }
    SLANG_NO_THROW void* SLANG_MCALL castAs(const SlangUUID&) SLANG_OVERRIDE
    {
        lastSlot = 3;
        return nullptr;
    }
};

SLANG_UNIT_TEST(vtableISlangCastable)
{
    ISlangCastableProbe p;
    callSlot(&p, 0);
    SLANG_CHECK(p.lastSlot == 0); // queryInterface
    callSlot(&p, 3);
    SLANG_CHECK(p.lastSlot == 3); // castAs
}

// ---------------------------------------------------------------------------
// ISlangClonable : ISlangCastable  (own slot 4)
// ---------------------------------------------------------------------------
struct ISlangClonableProbe : ISlangClonable
{
    int lastSlot = -1;
    SLANG_NO_THROW SlangResult SLANG_MCALL queryInterface(SlangUUID const&, void**) SLANG_OVERRIDE
    {
        lastSlot = 0;
        return SLANG_OK;
    }
    SLANG_NO_THROW uint32_t SLANG_MCALL addRef() SLANG_OVERRIDE
    {
        lastSlot = 1;
        return 1;
    }
    SLANG_NO_THROW uint32_t SLANG_MCALL release() SLANG_OVERRIDE
    {
        lastSlot = 2;
        return 1;
    }
    SLANG_NO_THROW void* SLANG_MCALL castAs(const SlangUUID&) SLANG_OVERRIDE
    {
        lastSlot = 3;
        return nullptr;
    }
    SLANG_NO_THROW void* SLANG_MCALL clone(const SlangUUID&) SLANG_OVERRIDE
    {
        lastSlot = 4;
        return nullptr;
    }
};

SLANG_UNIT_TEST(vtableISlangClonable)
{
    ISlangClonableProbe p;
    callSlot(&p, 3);
    SLANG_CHECK(p.lastSlot == 3); // castAs
    callSlot(&p, 4);
    SLANG_CHECK(p.lastSlot == 4); // clone
}

// ---------------------------------------------------------------------------
// ISlangBlob : ISlangUnknown  (own slots 3-4)
// ---------------------------------------------------------------------------
struct ISlangBlobProbe : ISlangBlob
{
    int lastSlot = -1;
    SLANG_NO_THROW SlangResult SLANG_MCALL queryInterface(SlangUUID const&, void**) SLANG_OVERRIDE
    {
        lastSlot = 0;
        return SLANG_OK;
    }
    SLANG_NO_THROW uint32_t SLANG_MCALL addRef() SLANG_OVERRIDE
    {
        lastSlot = 1;
        return 1;
    }
    SLANG_NO_THROW uint32_t SLANG_MCALL release() SLANG_OVERRIDE
    {
        lastSlot = 2;
        return 1;
    }
    SLANG_NO_THROW void const* SLANG_MCALL getBufferPointer() SLANG_OVERRIDE
    {
        lastSlot = 3;
        return nullptr;
    }
    SLANG_NO_THROW size_t SLANG_MCALL getBufferSize() SLANG_OVERRIDE
    {
        lastSlot = 4;
        return 0;
    }
};

SLANG_UNIT_TEST(vtableISlangBlob)
{
    ISlangBlobProbe p;
    callSlot(&p, 3);
    SLANG_CHECK(p.lastSlot == 3); // getBufferPointer
    callSlot(&p, 4);
    SLANG_CHECK(p.lastSlot == 4); // getBufferSize
}

// ---------------------------------------------------------------------------
// ISlangFileSystem : ISlangCastable  (own slot 4)
// ---------------------------------------------------------------------------
struct ISlangFileSystemProbe : ISlangFileSystem
{
    int lastSlot = -1;
    SLANG_NO_THROW SlangResult SLANG_MCALL queryInterface(SlangUUID const&, void**) SLANG_OVERRIDE
    {
        lastSlot = 0;
        return SLANG_OK;
    }
    SLANG_NO_THROW uint32_t SLANG_MCALL addRef() SLANG_OVERRIDE
    {
        lastSlot = 1;
        return 1;
    }
    SLANG_NO_THROW uint32_t SLANG_MCALL release() SLANG_OVERRIDE
    {
        lastSlot = 2;
        return 1;
    }
    SLANG_NO_THROW void* SLANG_MCALL castAs(const SlangUUID&) SLANG_OVERRIDE
    {
        lastSlot = 3;
        return nullptr;
    }
    SLANG_NO_THROW SlangResult SLANG_MCALL loadFile(char const*, ISlangBlob**) SLANG_OVERRIDE
    {
        lastSlot = 4;
        return SLANG_OK;
    }
};

SLANG_UNIT_TEST(vtableISlangFileSystem)
{
    ISlangFileSystemProbe p;
    callSlot(&p, 3);
    SLANG_CHECK(p.lastSlot == 3); // castAs
    callSlot(&p, 4);
    SLANG_CHECK(p.lastSlot == 4); // loadFile
}

// ---------------------------------------------------------------------------
// ISlangSharedLibrary_Dep1 : ISlangUnknown  (own slot 3)
// ---------------------------------------------------------------------------
struct ISlangSharedLibraryDep1Probe : ISlangSharedLibrary_Dep1
{
    int lastSlot = -1;
    SLANG_NO_THROW SlangResult SLANG_MCALL queryInterface(SlangUUID const&, void**) SLANG_OVERRIDE
    {
        lastSlot = 0;
        return SLANG_OK;
    }
    SLANG_NO_THROW uint32_t SLANG_MCALL addRef() SLANG_OVERRIDE
    {
        lastSlot = 1;
        return 1;
    }
    SLANG_NO_THROW uint32_t SLANG_MCALL release() SLANG_OVERRIDE
    {
        lastSlot = 2;
        return 1;
    }
    SLANG_NO_THROW void* SLANG_MCALL findSymbolAddressByName(char const*) SLANG_OVERRIDE
    {
        lastSlot = 3;
        return nullptr;
    }
};

SLANG_UNIT_TEST(vtableISlangSharedLibraryDep1)
{
    ISlangSharedLibraryDep1Probe p;
    callSlot(&p, 2);
    SLANG_CHECK(p.lastSlot == 2); // release
    callSlot(&p, 3);
    SLANG_CHECK(p.lastSlot == 3); // findSymbolAddressByName
}

// ---------------------------------------------------------------------------
// ISlangSharedLibrary : ISlangCastable  (own slot 4)
// ---------------------------------------------------------------------------
struct ISlangSharedLibraryProbe : ISlangSharedLibrary
{
    int lastSlot = -1;
    SLANG_NO_THROW SlangResult SLANG_MCALL queryInterface(SlangUUID const&, void**) SLANG_OVERRIDE
    {
        lastSlot = 0;
        return SLANG_OK;
    }
    SLANG_NO_THROW uint32_t SLANG_MCALL addRef() SLANG_OVERRIDE
    {
        lastSlot = 1;
        return 1;
    }
    SLANG_NO_THROW uint32_t SLANG_MCALL release() SLANG_OVERRIDE
    {
        lastSlot = 2;
        return 1;
    }
    SLANG_NO_THROW void* SLANG_MCALL castAs(const SlangUUID&) SLANG_OVERRIDE
    {
        lastSlot = 3;
        return nullptr;
    }
    SLANG_NO_THROW void* SLANG_MCALL findSymbolAddressByName(char const*) SLANG_OVERRIDE
    {
        lastSlot = 4;
        return nullptr;
    }
};

SLANG_UNIT_TEST(vtableISlangSharedLibrary)
{
    ISlangSharedLibraryProbe p;
    callSlot(&p, 3);
    SLANG_CHECK(p.lastSlot == 3); // castAs
    callSlot(&p, 4);
    SLANG_CHECK(p.lastSlot == 4); // findSymbolAddressByName
}

// ---------------------------------------------------------------------------
// ISlangSharedLibraryLoader : ISlangUnknown  (own slot 3)
// ---------------------------------------------------------------------------
struct ISlangSharedLibraryLoaderProbe : ISlangSharedLibraryLoader
{
    int lastSlot = -1;
    SLANG_NO_THROW SlangResult SLANG_MCALL queryInterface(SlangUUID const&, void**) SLANG_OVERRIDE
    {
        lastSlot = 0;
        return SLANG_OK;
    }
    SLANG_NO_THROW uint32_t SLANG_MCALL addRef() SLANG_OVERRIDE
    {
        lastSlot = 1;
        return 1;
    }
    SLANG_NO_THROW uint32_t SLANG_MCALL release() SLANG_OVERRIDE
    {
        lastSlot = 2;
        return 1;
    }
    SLANG_NO_THROW SlangResult SLANG_MCALL loadSharedLibrary(const char*, ISlangSharedLibrary**)
        SLANG_OVERRIDE
    {
        lastSlot = 3;
        return SLANG_OK;
    }
};

SLANG_UNIT_TEST(vtableISlangSharedLibraryLoader)
{
    ISlangSharedLibraryLoaderProbe p;
    callSlot(&p, 2);
    SLANG_CHECK(p.lastSlot == 2); // release
    callSlot(&p, 3);
    SLANG_CHECK(p.lastSlot == 3); // loadSharedLibrary
}

// ---------------------------------------------------------------------------
// ISlangFileSystemExt : ISlangFileSystem  (own slots 5-11)
// ---------------------------------------------------------------------------
struct ISlangFileSystemExtProbe : ISlangFileSystemExt
{
    int lastSlot = -1;
    SLANG_NO_THROW SlangResult SLANG_MCALL queryInterface(SlangUUID const&, void**) SLANG_OVERRIDE
    {
        lastSlot = 0;
        return SLANG_OK;
    }
    SLANG_NO_THROW uint32_t SLANG_MCALL addRef() SLANG_OVERRIDE
    {
        lastSlot = 1;
        return 1;
    }
    SLANG_NO_THROW uint32_t SLANG_MCALL release() SLANG_OVERRIDE
    {
        lastSlot = 2;
        return 1;
    }
    SLANG_NO_THROW void* SLANG_MCALL castAs(const SlangUUID&) SLANG_OVERRIDE
    {
        lastSlot = 3;
        return nullptr;
    }
    SLANG_NO_THROW SlangResult SLANG_MCALL loadFile(char const*, ISlangBlob**) SLANG_OVERRIDE
    {
        lastSlot = 4;
        return SLANG_OK;
    }
    SLANG_NO_THROW SlangResult SLANG_MCALL getFileUniqueIdentity(const char*, ISlangBlob**)
        SLANG_OVERRIDE
    {
        lastSlot = 5;
        return SLANG_OK;
    }
    SLANG_NO_THROW SlangResult SLANG_MCALL
    calcCombinedPath(SlangPathType, const char*, const char*, ISlangBlob**) SLANG_OVERRIDE
    {
        lastSlot = 6;
        return SLANG_OK;
    }
    SLANG_NO_THROW SlangResult SLANG_MCALL getPathType(const char*, SlangPathType*) SLANG_OVERRIDE
    {
        lastSlot = 7;
        return SLANG_OK;
    }
    SLANG_NO_THROW SlangResult SLANG_MCALL getPath(PathKind, const char*, ISlangBlob**)
        SLANG_OVERRIDE
    {
        lastSlot = 8;
        return SLANG_OK;
    }
    SLANG_NO_THROW void SLANG_MCALL clearCache() SLANG_OVERRIDE { lastSlot = 9; }
    SLANG_NO_THROW SlangResult SLANG_MCALL
    enumeratePathContents(const char*, FileSystemContentsCallBack, void*) SLANG_OVERRIDE
    {
        lastSlot = 10;
        return SLANG_OK;
    }
    SLANG_NO_THROW OSPathKind SLANG_MCALL getOSPathKind() SLANG_OVERRIDE
    {
        lastSlot = 11;
        return OSPathKind::None;
    }
};

SLANG_UNIT_TEST(vtableISlangFileSystemExt)
{
    ISlangFileSystemExtProbe p;
    callSlot(&p, 4);
    SLANG_CHECK(p.lastSlot == 4); // loadFile
    callSlot(&p, 5);
    SLANG_CHECK(p.lastSlot == 5); // getFileUniqueIdentity
    callSlot(&p, 6);
    SLANG_CHECK(p.lastSlot == 6); // calcCombinedPath
    callSlot(&p, 7);
    SLANG_CHECK(p.lastSlot == 7); // getPathType
    callSlot(&p, 8);
    SLANG_CHECK(p.lastSlot == 8); // getPath
    callSlot(&p, 9);
    SLANG_CHECK(p.lastSlot == 9); // clearCache
    callSlot(&p, 10);
    SLANG_CHECK(p.lastSlot == 10); // enumeratePathContents
    callSlot(&p, 11);
    SLANG_CHECK(p.lastSlot == 11); // getOSPathKind
}

// ---------------------------------------------------------------------------
// ISlangMutableFileSystem : ISlangFileSystemExt  (own slots 12-15)
// ---------------------------------------------------------------------------
struct ISlangMutableFileSystemProbe : ISlangMutableFileSystem
{
    int lastSlot = -1;
    SLANG_NO_THROW SlangResult SLANG_MCALL queryInterface(SlangUUID const&, void**) SLANG_OVERRIDE
    {
        lastSlot = 0;
        return SLANG_OK;
    }
    SLANG_NO_THROW uint32_t SLANG_MCALL addRef() SLANG_OVERRIDE
    {
        lastSlot = 1;
        return 1;
    }
    SLANG_NO_THROW uint32_t SLANG_MCALL release() SLANG_OVERRIDE
    {
        lastSlot = 2;
        return 1;
    }
    SLANG_NO_THROW void* SLANG_MCALL castAs(const SlangUUID&) SLANG_OVERRIDE
    {
        lastSlot = 3;
        return nullptr;
    }
    SLANG_NO_THROW SlangResult SLANG_MCALL loadFile(char const*, ISlangBlob**) SLANG_OVERRIDE
    {
        lastSlot = 4;
        return SLANG_OK;
    }
    SLANG_NO_THROW SlangResult SLANG_MCALL getFileUniqueIdentity(const char*, ISlangBlob**)
        SLANG_OVERRIDE
    {
        lastSlot = 5;
        return SLANG_OK;
    }
    SLANG_NO_THROW SlangResult SLANG_MCALL
    calcCombinedPath(SlangPathType, const char*, const char*, ISlangBlob**) SLANG_OVERRIDE
    {
        lastSlot = 6;
        return SLANG_OK;
    }
    SLANG_NO_THROW SlangResult SLANG_MCALL getPathType(const char*, SlangPathType*) SLANG_OVERRIDE
    {
        lastSlot = 7;
        return SLANG_OK;
    }
    SLANG_NO_THROW SlangResult SLANG_MCALL getPath(PathKind, const char*, ISlangBlob**)
        SLANG_OVERRIDE
    {
        lastSlot = 8;
        return SLANG_OK;
    }
    SLANG_NO_THROW void SLANG_MCALL clearCache() SLANG_OVERRIDE { lastSlot = 9; }
    SLANG_NO_THROW SlangResult SLANG_MCALL
    enumeratePathContents(const char*, FileSystemContentsCallBack, void*) SLANG_OVERRIDE
    {
        lastSlot = 10;
        return SLANG_OK;
    }
    SLANG_NO_THROW OSPathKind SLANG_MCALL getOSPathKind() SLANG_OVERRIDE
    {
        lastSlot = 11;
        return OSPathKind::None;
    }
    SLANG_NO_THROW SlangResult SLANG_MCALL saveFile(const char*, const void*, size_t) SLANG_OVERRIDE
    {
        lastSlot = 12;
        return SLANG_OK;
    }
    SLANG_NO_THROW SlangResult SLANG_MCALL saveFileBlob(const char*, ISlangBlob*) SLANG_OVERRIDE
    {
        lastSlot = 13;
        return SLANG_OK;
    }
    SLANG_NO_THROW SlangResult SLANG_MCALL remove(const char*) SLANG_OVERRIDE
    {
        lastSlot = 14;
        return SLANG_OK;
    }
    SLANG_NO_THROW SlangResult SLANG_MCALL createDirectory(const char*) SLANG_OVERRIDE
    {
        lastSlot = 15;
        return SLANG_OK;
    }
};

SLANG_UNIT_TEST(vtableISlangMutableFileSystem)
{
    ISlangMutableFileSystemProbe p;
    callSlot(&p, 11);
    SLANG_CHECK(p.lastSlot == 11); // getOSPathKind
    callSlot(&p, 12);
    SLANG_CHECK(p.lastSlot == 12); // saveFile
    callSlot(&p, 13);
    SLANG_CHECK(p.lastSlot == 13); // saveFileBlob
    callSlot(&p, 14);
    SLANG_CHECK(p.lastSlot == 14); // remove
    callSlot(&p, 15);
    SLANG_CHECK(p.lastSlot == 15); // createDirectory
}

// ---------------------------------------------------------------------------
// ISlangWriter : ISlangUnknown  (own slots 3-8)
// ---------------------------------------------------------------------------
struct ISlangWriterProbe : ISlangWriter
{
    int lastSlot = -1;
    SLANG_NO_THROW SlangResult SLANG_MCALL queryInterface(SlangUUID const&, void**) SLANG_OVERRIDE
    {
        lastSlot = 0;
        return SLANG_OK;
    }
    SLANG_NO_THROW uint32_t SLANG_MCALL addRef() SLANG_OVERRIDE
    {
        lastSlot = 1;
        return 1;
    }
    SLANG_NO_THROW uint32_t SLANG_MCALL release() SLANG_OVERRIDE
    {
        lastSlot = 2;
        return 1;
    }
    SLANG_NO_THROW char* SLANG_MCALL beginAppendBuffer(size_t) SLANG_OVERRIDE
    {
        lastSlot = 3;
        return nullptr;
    }
    SLANG_NO_THROW SlangResult SLANG_MCALL endAppendBuffer(char*, size_t) SLANG_OVERRIDE
    {
        lastSlot = 4;
        return SLANG_OK;
    }
    SLANG_NO_THROW SlangResult SLANG_MCALL write(const char*, size_t) SLANG_OVERRIDE
    {
        lastSlot = 5;
        return SLANG_OK;
    }
    SLANG_NO_THROW void SLANG_MCALL flush() SLANG_OVERRIDE { lastSlot = 6; }
    SLANG_NO_THROW SlangBool SLANG_MCALL isConsole() SLANG_OVERRIDE
    {
        lastSlot = 7;
        return 0;
    }
    SLANG_NO_THROW SlangResult SLANG_MCALL setMode(SlangWriterMode) SLANG_OVERRIDE
    {
        lastSlot = 8;
        return SLANG_OK;
    }
};

SLANG_UNIT_TEST(vtableISlangWriter)
{
    ISlangWriterProbe p;
    callSlot(&p, 3);
    SLANG_CHECK(p.lastSlot == 3); // beginAppendBuffer
    callSlot(&p, 4);
    SLANG_CHECK(p.lastSlot == 4); // endAppendBuffer
    callSlot(&p, 5);
    SLANG_CHECK(p.lastSlot == 5); // write
    callSlot(&p, 6);
    SLANG_CHECK(p.lastSlot == 6); // flush
    callSlot(&p, 7);
    SLANG_CHECK(p.lastSlot == 7); // isConsole
    callSlot(&p, 8);
    SLANG_CHECK(p.lastSlot == 8); // setMode
}

// ---------------------------------------------------------------------------
// ISlangProfiler : ISlangUnknown  (own slots 3-6)
// ---------------------------------------------------------------------------
struct ISlangProfilerProbe : ISlangProfiler
{
    int lastSlot = -1;
    SLANG_NO_THROW SlangResult SLANG_MCALL queryInterface(SlangUUID const&, void**) SLANG_OVERRIDE
    {
        lastSlot = 0;
        return SLANG_OK;
    }
    SLANG_NO_THROW uint32_t SLANG_MCALL addRef() SLANG_OVERRIDE
    {
        lastSlot = 1;
        return 1;
    }
    SLANG_NO_THROW uint32_t SLANG_MCALL release() SLANG_OVERRIDE
    {
        lastSlot = 2;
        return 1;
    }
    SLANG_NO_THROW size_t SLANG_MCALL getEntryCount() SLANG_OVERRIDE
    {
        lastSlot = 3;
        return 0;
    }
    SLANG_NO_THROW const char* SLANG_MCALL getEntryName(uint32_t) SLANG_OVERRIDE
    {
        lastSlot = 4;
        return nullptr;
    }
    SLANG_NO_THROW long SLANG_MCALL getEntryTimeMS(uint32_t) SLANG_OVERRIDE
    {
        lastSlot = 5;
        return 0;
    }
    SLANG_NO_THROW uint32_t SLANG_MCALL getEntryInvocationTimes(uint32_t) SLANG_OVERRIDE
    {
        lastSlot = 6;
        return 0;
    }
};

SLANG_UNIT_TEST(vtableISlangProfiler)
{
    ISlangProfilerProbe p;
    callSlot(&p, 3);
    SLANG_CHECK(p.lastSlot == 3); // getEntryCount
    callSlot(&p, 4);
    SLANG_CHECK(p.lastSlot == 4); // getEntryName
    callSlot(&p, 5);
    SLANG_CHECK(p.lastSlot == 5); // getEntryTimeMS
    callSlot(&p, 6);
    SLANG_CHECK(p.lastSlot == 6); // getEntryInvocationTimes
}

// ---------------------------------------------------------------------------
// IGlobalSession : ISlangUnknown  (own slots 3-31)
// ---------------------------------------------------------------------------
struct IGlobalSessionProbe : IGlobalSession
{
    int lastSlot = -1;
    SLANG_NO_THROW SlangResult SLANG_MCALL queryInterface(SlangUUID const&, void**) SLANG_OVERRIDE
    {
        lastSlot = 0;
        return SLANG_OK;
    }
    SLANG_NO_THROW uint32_t SLANG_MCALL addRef() SLANG_OVERRIDE
    {
        lastSlot = 1;
        return 1;
    }
    SLANG_NO_THROW uint32_t SLANG_MCALL release() SLANG_OVERRIDE
    {
        lastSlot = 2;
        return 1;
    }
    SLANG_NO_THROW SlangResult SLANG_MCALL createSession(SessionDesc const&, ISession**)
        SLANG_OVERRIDE
    {
        lastSlot = 3;
        return SLANG_OK;
    }
    SLANG_NO_THROW SlangProfileID SLANG_MCALL findProfile(char const*) SLANG_OVERRIDE
    {
        lastSlot = 4;
        return SLANG_PROFILE_UNKNOWN;
    }
    SLANG_NO_THROW void SLANG_MCALL setDownstreamCompilerPath(SlangPassThrough, char const*)
        SLANG_OVERRIDE
    {
        lastSlot = 5;
    }
    SLANG_NO_THROW void SLANG_MCALL setDownstreamCompilerPrelude(SlangPassThrough, const char*)
        SLANG_OVERRIDE
    {
        lastSlot = 6;
    }
    SLANG_NO_THROW void SLANG_MCALL getDownstreamCompilerPrelude(SlangPassThrough, ISlangBlob**)
        SLANG_OVERRIDE
    {
        lastSlot = 7;
    }
    SLANG_NO_THROW const char* SLANG_MCALL getBuildTagString() SLANG_OVERRIDE
    {
        lastSlot = 8;
        return nullptr;
    }
    SLANG_NO_THROW SlangResult SLANG_MCALL
    setDefaultDownstreamCompiler(SlangSourceLanguage, SlangPassThrough) SLANG_OVERRIDE
    {
        lastSlot = 9;
        return SLANG_OK;
    }
    SLANG_NO_THROW SlangPassThrough SLANG_MCALL getDefaultDownstreamCompiler(SlangSourceLanguage)
        SLANG_OVERRIDE
    {
        lastSlot = 10;
        return SLANG_PASS_THROUGH_NONE;
    }
    SLANG_NO_THROW void SLANG_MCALL setLanguagePrelude(SlangSourceLanguage, const char*)
        SLANG_OVERRIDE
    {
        lastSlot = 11;
    }
    SLANG_NO_THROW void SLANG_MCALL getLanguagePrelude(SlangSourceLanguage, ISlangBlob**)
        SLANG_OVERRIDE
    {
        lastSlot = 12;
    }
    SLANG_NO_THROW SlangResult SLANG_MCALL createCompileRequest(ICompileRequest**) SLANG_OVERRIDE
    {
        lastSlot = 13;
        return SLANG_OK;
    }
    SLANG_NO_THROW void SLANG_MCALL addBuiltins(char const*, char const*) SLANG_OVERRIDE
    {
        lastSlot = 14;
    }
    SLANG_NO_THROW void SLANG_MCALL setSharedLibraryLoader(ISlangSharedLibraryLoader*)
        SLANG_OVERRIDE
    {
        lastSlot = 15;
    }
    SLANG_NO_THROW ISlangSharedLibraryLoader* SLANG_MCALL getSharedLibraryLoader() SLANG_OVERRIDE
    {
        lastSlot = 16;
        return nullptr;
    }
    SLANG_NO_THROW SlangResult SLANG_MCALL checkCompileTargetSupport(SlangCompileTarget)
        SLANG_OVERRIDE
    {
        lastSlot = 17;
        return SLANG_OK;
    }
    SLANG_NO_THROW SlangResult SLANG_MCALL checkPassThroughSupport(SlangPassThrough) SLANG_OVERRIDE
    {
        lastSlot = 18;
        return SLANG_OK;
    }
    SLANG_NO_THROW SlangResult SLANG_MCALL compileCoreModule(CompileCoreModuleFlags) SLANG_OVERRIDE
    {
        lastSlot = 19;
        return SLANG_OK;
    }
    SLANG_NO_THROW SlangResult SLANG_MCALL loadCoreModule(const void*, size_t) SLANG_OVERRIDE
    {
        lastSlot = 20;
        return SLANG_OK;
    }
    SLANG_NO_THROW SlangResult SLANG_MCALL saveCoreModule(SlangArchiveType, ISlangBlob**)
        SLANG_OVERRIDE
    {
        lastSlot = 21;
        return SLANG_OK;
    }
    SLANG_NO_THROW SlangCapabilityID SLANG_MCALL findCapability(char const*) SLANG_OVERRIDE
    {
        lastSlot = 22;
        return SLANG_CAPABILITY_UNKNOWN;
    }
    SLANG_NO_THROW void SLANG_MCALL setDownstreamCompilerForTransition(
        SlangCompileTarget,
        SlangCompileTarget,
        SlangPassThrough) SLANG_OVERRIDE
    {
        lastSlot = 23;
    }
    SLANG_NO_THROW SlangPassThrough SLANG_MCALL
    getDownstreamCompilerForTransition(SlangCompileTarget, SlangCompileTarget) SLANG_OVERRIDE
    {
        lastSlot = 24;
        return SLANG_PASS_THROUGH_NONE;
    }
    SLANG_NO_THROW void SLANG_MCALL getCompilerElapsedTime(double*, double*) SLANG_OVERRIDE
    {
        lastSlot = 25;
    }
    SLANG_NO_THROW SlangResult SLANG_MCALL setSPIRVCoreGrammar(char const*) SLANG_OVERRIDE
    {
        lastSlot = 26;
        return SLANG_OK;
    }
    SLANG_NO_THROW SlangResult SLANG_MCALL
    parseCommandLineArguments(int, const char* const*, SessionDesc*, ISlangUnknown**) SLANG_OVERRIDE
    {
        lastSlot = 27;
        return SLANG_OK;
    }
    SLANG_NO_THROW SlangResult SLANG_MCALL getSessionDescDigest(SessionDesc*, ISlangBlob**)
        SLANG_OVERRIDE
    {
        lastSlot = 28;
        return SLANG_OK;
    }
    SLANG_NO_THROW SlangResult SLANG_MCALL
    compileBuiltinModule(BuiltinModuleName, CompileCoreModuleFlags) SLANG_OVERRIDE
    {
        lastSlot = 29;
        return SLANG_OK;
    }
    SLANG_NO_THROW SlangResult SLANG_MCALL loadBuiltinModule(BuiltinModuleName, const void*, size_t)
        SLANG_OVERRIDE
    {
        lastSlot = 30;
        return SLANG_OK;
    }
    SLANG_NO_THROW SlangResult SLANG_MCALL
    saveBuiltinModule(BuiltinModuleName, SlangArchiveType, ISlangBlob**) SLANG_OVERRIDE
    {
        lastSlot = 31;
        return SLANG_OK;
    }
};

SLANG_UNIT_TEST(vtableIGlobalSession)
{
    IGlobalSessionProbe p;
    callSlot(&p, 3);
    SLANG_CHECK(p.lastSlot == 3); // createSession
    callSlot(&p, 4);
    SLANG_CHECK(p.lastSlot == 4); // findProfile
    callSlot(&p, 8);
    SLANG_CHECK(p.lastSlot == 8); // getBuildTagString
    callSlot(&p, 13);
    SLANG_CHECK(p.lastSlot == 13); // createCompileRequest (deprecated)
    callSlot(&p, 16);
    SLANG_CHECK(p.lastSlot == 16); // getSharedLibraryLoader
    callSlot(&p, 22);
    SLANG_CHECK(p.lastSlot == 22); // findCapability
    callSlot(&p, 26);
    SLANG_CHECK(p.lastSlot == 26); // setSPIRVCoreGrammar
    callSlot(&p, 31);
    SLANG_CHECK(p.lastSlot == 31); // saveBuiltinModule
}

// ---------------------------------------------------------------------------
// IMetadata : ISlangCastable  (own slots 4-5)
// ---------------------------------------------------------------------------
struct IMetadataProbe : IMetadata
{
    int lastSlot = -1;
    SLANG_NO_THROW SlangResult SLANG_MCALL queryInterface(SlangUUID const&, void**) SLANG_OVERRIDE
    {
        lastSlot = 0;
        return SLANG_OK;
    }
    SLANG_NO_THROW uint32_t SLANG_MCALL addRef() SLANG_OVERRIDE
    {
        lastSlot = 1;
        return 1;
    }
    SLANG_NO_THROW uint32_t SLANG_MCALL release() SLANG_OVERRIDE
    {
        lastSlot = 2;
        return 1;
    }
    SLANG_NO_THROW void* SLANG_MCALL castAs(const SlangUUID&) SLANG_OVERRIDE
    {
        lastSlot = 3;
        return nullptr;
    }
    SlangResult SLANG_MCALL
    isParameterLocationUsed(SlangParameterCategory, SlangUInt, SlangUInt, bool&) SLANG_OVERRIDE
    {
        lastSlot = 4;
        return SLANG_OK;
    }
    SLANG_NO_THROW const char* SLANG_MCALL getDebugBuildIdentifier() SLANG_OVERRIDE
    {
        lastSlot = 5;
        return nullptr;
    }
};

SLANG_UNIT_TEST(vtableIMetadata)
{
    IMetadataProbe p;
    callSlot(&p, 3);
    SLANG_CHECK(p.lastSlot == 3); // castAs
    callSlot(&p, 4);
    SLANG_CHECK(p.lastSlot == 4); // isParameterLocationUsed
    callSlot(&p, 5);
    SLANG_CHECK(p.lastSlot == 5); // getDebugBuildIdentifier
}

// ---------------------------------------------------------------------------
// ICompileResult : ISlangCastable  (own slots 4-6)
// ---------------------------------------------------------------------------
struct ICompileResultProbe : ICompileResult
{
    int lastSlot = -1;
    SLANG_NO_THROW SlangResult SLANG_MCALL queryInterface(SlangUUID const&, void**) SLANG_OVERRIDE
    {
        lastSlot = 0;
        return SLANG_OK;
    }
    SLANG_NO_THROW uint32_t SLANG_MCALL addRef() SLANG_OVERRIDE
    {
        lastSlot = 1;
        return 1;
    }
    SLANG_NO_THROW uint32_t SLANG_MCALL release() SLANG_OVERRIDE
    {
        lastSlot = 2;
        return 1;
    }
    SLANG_NO_THROW void* SLANG_MCALL castAs(const SlangUUID&) SLANG_OVERRIDE
    {
        lastSlot = 3;
        return nullptr;
    }
    SLANG_NO_THROW uint32_t SLANG_MCALL getItemCount() SLANG_OVERRIDE
    {
        lastSlot = 4;
        return 0;
    }
    SLANG_NO_THROW SlangResult SLANG_MCALL getItemData(uint32_t, IBlob**) SLANG_OVERRIDE
    {
        lastSlot = 5;
        return SLANG_OK;
    }
    SLANG_NO_THROW SlangResult SLANG_MCALL getMetadata(IMetadata**) SLANG_OVERRIDE
    {
        lastSlot = 6;
        return SLANG_OK;
    }
};

SLANG_UNIT_TEST(vtableICompileResult)
{
    ICompileResultProbe p;
    callSlot(&p, 3);
    SLANG_CHECK(p.lastSlot == 3); // castAs
    callSlot(&p, 4);
    SLANG_CHECK(p.lastSlot == 4); // getItemCount
    callSlot(&p, 5);
    SLANG_CHECK(p.lastSlot == 5); // getItemData
    callSlot(&p, 6);
    SLANG_CHECK(p.lastSlot == 6); // getMetadata
}

// ---------------------------------------------------------------------------
// IComponentType : ISlangUnknown  (own slots 3-16)
// ---------------------------------------------------------------------------
struct IComponentTypeProbe : IComponentType
{
    int lastSlot = -1;
    SLANG_NO_THROW SlangResult SLANG_MCALL queryInterface(SlangUUID const&, void**) SLANG_OVERRIDE
    {
        lastSlot = 0;
        return SLANG_OK;
    }
    SLANG_NO_THROW uint32_t SLANG_MCALL addRef() SLANG_OVERRIDE
    {
        lastSlot = 1;
        return 1;
    }
    SLANG_NO_THROW uint32_t SLANG_MCALL release() SLANG_OVERRIDE
    {
        lastSlot = 2;
        return 1;
    }
    SLANG_NO_THROW ISession* SLANG_MCALL getSession() SLANG_OVERRIDE
    {
        lastSlot = 3;
        return nullptr;
    }
    SLANG_NO_THROW ProgramLayout* SLANG_MCALL getLayout(SlangInt, IBlob**) SLANG_OVERRIDE
    {
        lastSlot = 4;
        return nullptr;
    }
    SLANG_NO_THROW SlangInt SLANG_MCALL getSpecializationParamCount() SLANG_OVERRIDE
    {
        lastSlot = 5;
        return 0;
    }
    SLANG_NO_THROW SlangResult SLANG_MCALL getEntryPointCode(SlangInt, SlangInt, IBlob**, IBlob**)
        SLANG_OVERRIDE
    {
        lastSlot = 6;
        return SLANG_OK;
    }
    SLANG_NO_THROW SlangResult SLANG_MCALL
    getResultAsFileSystem(SlangInt, SlangInt, ISlangMutableFileSystem**) SLANG_OVERRIDE
    {
        lastSlot = 7;
        return SLANG_OK;
    }
    SLANG_NO_THROW void SLANG_MCALL getEntryPointHash(SlangInt, SlangInt, IBlob**) SLANG_OVERRIDE
    {
        lastSlot = 8;
    }
    SLANG_NO_THROW SlangResult SLANG_MCALL
    specialize(SpecializationArg const*, SlangInt, IComponentType**, ISlangBlob**) SLANG_OVERRIDE
    {
        lastSlot = 9;
        return SLANG_OK;
    }
    SLANG_NO_THROW SlangResult SLANG_MCALL link(IComponentType**, ISlangBlob**) SLANG_OVERRIDE
    {
        lastSlot = 10;
        return SLANG_OK;
    }
    SLANG_NO_THROW SlangResult SLANG_MCALL
    getEntryPointHostCallable(int, int, ISlangSharedLibrary**, ISlangBlob**) SLANG_OVERRIDE
    {
        lastSlot = 11;
        return SLANG_OK;
    }
    SLANG_NO_THROW SlangResult SLANG_MCALL renameEntryPoint(const char*, IComponentType**)
        SLANG_OVERRIDE
    {
        lastSlot = 12;
        return SLANG_OK;
    }
    SLANG_NO_THROW SlangResult SLANG_MCALL
    linkWithOptions(IComponentType**, uint32_t, CompilerOptionEntry const*, ISlangBlob**)
        SLANG_OVERRIDE
    {
        lastSlot = 13;
        return SLANG_OK;
    }
    SLANG_NO_THROW SlangResult SLANG_MCALL getTargetCode(SlangInt, IBlob**, IBlob**) SLANG_OVERRIDE
    {
        lastSlot = 14;
        return SLANG_OK;
    }
    SLANG_NO_THROW SlangResult SLANG_MCALL getTargetMetadata(SlangInt, IMetadata**, IBlob**)
        SLANG_OVERRIDE
    {
        lastSlot = 15;
        return SLANG_OK;
    }
    SLANG_NO_THROW SlangResult SLANG_MCALL
    getEntryPointMetadata(SlangInt, SlangInt, IMetadata**, IBlob**) SLANG_OVERRIDE
    {
        lastSlot = 16;
        return SLANG_OK;
    }
};

SLANG_UNIT_TEST(vtableIComponentType)
{
    IComponentTypeProbe p;
    callSlot(&p, 3);
    SLANG_CHECK(p.lastSlot == 3); // getSession
    callSlot(&p, 5);
    SLANG_CHECK(p.lastSlot == 5); // getSpecializationParamCount
    callSlot(&p, 8);
    SLANG_CHECK(p.lastSlot == 8); // getEntryPointHash
    callSlot(&p, 10);
    SLANG_CHECK(p.lastSlot == 10); // link
    callSlot(&p, 13);
    SLANG_CHECK(p.lastSlot == 13); // linkWithOptions
    callSlot(&p, 16);
    SLANG_CHECK(p.lastSlot == 16); // getEntryPointMetadata
}

// ---------------------------------------------------------------------------
// IEntryPoint : IComponentType  (own slot 17)
// ---------------------------------------------------------------------------
struct IEntryPointProbe : IEntryPoint
{
    int lastSlot = -1;
    SLANG_NO_THROW SlangResult SLANG_MCALL queryInterface(SlangUUID const&, void**) SLANG_OVERRIDE
    {
        lastSlot = 0;
        return SLANG_OK;
    }
    SLANG_NO_THROW uint32_t SLANG_MCALL addRef() SLANG_OVERRIDE
    {
        lastSlot = 1;
        return 1;
    }
    SLANG_NO_THROW uint32_t SLANG_MCALL release() SLANG_OVERRIDE
    {
        lastSlot = 2;
        return 1;
    }
    SLANG_NO_THROW ISession* SLANG_MCALL getSession() SLANG_OVERRIDE
    {
        lastSlot = 3;
        return nullptr;
    }
    SLANG_NO_THROW ProgramLayout* SLANG_MCALL getLayout(SlangInt, IBlob**) SLANG_OVERRIDE
    {
        lastSlot = 4;
        return nullptr;
    }
    SLANG_NO_THROW SlangInt SLANG_MCALL getSpecializationParamCount() SLANG_OVERRIDE
    {
        lastSlot = 5;
        return 0;
    }
    SLANG_NO_THROW SlangResult SLANG_MCALL getEntryPointCode(SlangInt, SlangInt, IBlob**, IBlob**)
        SLANG_OVERRIDE
    {
        lastSlot = 6;
        return SLANG_OK;
    }
    SLANG_NO_THROW SlangResult SLANG_MCALL
    getResultAsFileSystem(SlangInt, SlangInt, ISlangMutableFileSystem**) SLANG_OVERRIDE
    {
        lastSlot = 7;
        return SLANG_OK;
    }
    SLANG_NO_THROW void SLANG_MCALL getEntryPointHash(SlangInt, SlangInt, IBlob**) SLANG_OVERRIDE
    {
        lastSlot = 8;
    }
    SLANG_NO_THROW SlangResult SLANG_MCALL
    specialize(SpecializationArg const*, SlangInt, IComponentType**, ISlangBlob**) SLANG_OVERRIDE
    {
        lastSlot = 9;
        return SLANG_OK;
    }
    SLANG_NO_THROW SlangResult SLANG_MCALL link(IComponentType**, ISlangBlob**) SLANG_OVERRIDE
    {
        lastSlot = 10;
        return SLANG_OK;
    }
    SLANG_NO_THROW SlangResult SLANG_MCALL
    getEntryPointHostCallable(int, int, ISlangSharedLibrary**, ISlangBlob**) SLANG_OVERRIDE
    {
        lastSlot = 11;
        return SLANG_OK;
    }
    SLANG_NO_THROW SlangResult SLANG_MCALL renameEntryPoint(const char*, IComponentType**)
        SLANG_OVERRIDE
    {
        lastSlot = 12;
        return SLANG_OK;
    }
    SLANG_NO_THROW SlangResult SLANG_MCALL
    linkWithOptions(IComponentType**, uint32_t, CompilerOptionEntry const*, ISlangBlob**)
        SLANG_OVERRIDE
    {
        lastSlot = 13;
        return SLANG_OK;
    }
    SLANG_NO_THROW SlangResult SLANG_MCALL getTargetCode(SlangInt, IBlob**, IBlob**) SLANG_OVERRIDE
    {
        lastSlot = 14;
        return SLANG_OK;
    }
    SLANG_NO_THROW SlangResult SLANG_MCALL getTargetMetadata(SlangInt, IMetadata**, IBlob**)
        SLANG_OVERRIDE
    {
        lastSlot = 15;
        return SLANG_OK;
    }
    SLANG_NO_THROW SlangResult SLANG_MCALL
    getEntryPointMetadata(SlangInt, SlangInt, IMetadata**, IBlob**) SLANG_OVERRIDE
    {
        lastSlot = 16;
        return SLANG_OK;
    }
    SLANG_NO_THROW FunctionReflection* SLANG_MCALL getFunctionReflection() SLANG_OVERRIDE
    {
        lastSlot = 17;
        return nullptr;
    }
};

SLANG_UNIT_TEST(vtableIEntryPoint)
{
    IEntryPointProbe p;
    callSlot(&p, 16);
    SLANG_CHECK(p.lastSlot == 16); // getEntryPointMetadata
    callSlot(&p, 17);
    SLANG_CHECK(p.lastSlot == 17); // getFunctionReflection
}

// ---------------------------------------------------------------------------
// IComponentType2 : ISlangUnknown  (own slots 3-5)
// ---------------------------------------------------------------------------
struct IComponentType2Probe : IComponentType2
{
    int lastSlot = -1;
    SLANG_NO_THROW SlangResult SLANG_MCALL queryInterface(SlangUUID const&, void**) SLANG_OVERRIDE
    {
        lastSlot = 0;
        return SLANG_OK;
    }
    SLANG_NO_THROW uint32_t SLANG_MCALL addRef() SLANG_OVERRIDE
    {
        lastSlot = 1;
        return 1;
    }
    SLANG_NO_THROW uint32_t SLANG_MCALL release() SLANG_OVERRIDE
    {
        lastSlot = 2;
        return 1;
    }
    SLANG_NO_THROW SlangResult SLANG_MCALL
    getTargetCompileResult(SlangInt, ICompileResult**, IBlob**) SLANG_OVERRIDE
    {
        lastSlot = 3;
        return SLANG_OK;
    }
    SLANG_NO_THROW SlangResult SLANG_MCALL
    getEntryPointCompileResult(SlangInt, SlangInt, ICompileResult**, IBlob**) SLANG_OVERRIDE
    {
        lastSlot = 4;
        return SLANG_OK;
    }
    SLANG_NO_THROW SlangResult SLANG_MCALL
    getTargetHostCallable(int, ISlangSharedLibrary**, ISlangBlob**) SLANG_OVERRIDE
    {
        lastSlot = 5;
        return SLANG_OK;
    }
};

SLANG_UNIT_TEST(vtableIComponentType2)
{
    IComponentType2Probe p;
    callSlot(&p, 3);
    SLANG_CHECK(p.lastSlot == 3); // getTargetCompileResult
    callSlot(&p, 4);
    SLANG_CHECK(p.lastSlot == 4); // getEntryPointCompileResult
    callSlot(&p, 5);
    SLANG_CHECK(p.lastSlot == 5); // getTargetHostCallable
}

// ---------------------------------------------------------------------------
// ISession : ISlangUnknown  (own slots 3-23)
// ---------------------------------------------------------------------------
struct ISessionProbe : ISession
{
    int lastSlot = -1;
    SLANG_NO_THROW SlangResult SLANG_MCALL queryInterface(SlangUUID const&, void**) SLANG_OVERRIDE
    {
        lastSlot = 0;
        return SLANG_OK;
    }
    SLANG_NO_THROW uint32_t SLANG_MCALL addRef() SLANG_OVERRIDE
    {
        lastSlot = 1;
        return 1;
    }
    SLANG_NO_THROW uint32_t SLANG_MCALL release() SLANG_OVERRIDE
    {
        lastSlot = 2;
        return 1;
    }
    SLANG_NO_THROW IGlobalSession* SLANG_MCALL getGlobalSession() SLANG_OVERRIDE
    {
        lastSlot = 3;
        return nullptr;
    }
    SLANG_NO_THROW IModule* SLANG_MCALL loadModule(const char*, IBlob**) SLANG_OVERRIDE
    {
        lastSlot = 4;
        return nullptr;
    }
    SLANG_NO_THROW IModule* SLANG_MCALL
    loadModuleFromSource(const char*, const char*, IBlob*, IBlob**) SLANG_OVERRIDE
    {
        lastSlot = 5;
        return nullptr;
    }
    SLANG_NO_THROW SlangResult SLANG_MCALL
    createCompositeComponentType(IComponentType* const*, SlangInt, IComponentType**, ISlangBlob**)
        SLANG_OVERRIDE
    {
        lastSlot = 6;
        return SLANG_OK;
    }
    SLANG_NO_THROW TypeReflection* SLANG_MCALL
    specializeType(TypeReflection*, SpecializationArg const*, SlangInt, ISlangBlob**) SLANG_OVERRIDE
    {
        lastSlot = 7;
        return nullptr;
    }
    SLANG_NO_THROW TypeLayoutReflection* SLANG_MCALL
    getTypeLayout(TypeReflection*, SlangInt, LayoutRules, ISlangBlob**) SLANG_OVERRIDE
    {
        lastSlot = 8;
        return nullptr;
    }
    SLANG_NO_THROW TypeReflection* SLANG_MCALL
    getContainerType(TypeReflection*, ContainerType, ISlangBlob**) SLANG_OVERRIDE
    {
        lastSlot = 9;
        return nullptr;
    }
    SLANG_NO_THROW TypeReflection* SLANG_MCALL getDynamicType() SLANG_OVERRIDE
    {
        lastSlot = 10;
        return nullptr;
    }
    SLANG_NO_THROW SlangResult SLANG_MCALL getTypeRTTIMangledName(TypeReflection*, ISlangBlob**)
        SLANG_OVERRIDE
    {
        lastSlot = 11;
        return SLANG_OK;
    }
    SLANG_NO_THROW SlangResult SLANG_MCALL getTypeConformanceWitnessMangledName(
        TypeReflection*,
        TypeReflection*,
        ISlangBlob**) SLANG_OVERRIDE
    {
        lastSlot = 12;
        return SLANG_OK;
    }
    SLANG_NO_THROW SlangResult SLANG_MCALL getTypeConformanceWitnessSequentialID(
        TypeReflection*,
        TypeReflection*,
        uint32_t*) SLANG_OVERRIDE
    {
        lastSlot = 13;
        return SLANG_OK;
    }
    SLANG_NO_THROW SlangResult SLANG_MCALL createCompileRequest(SlangCompileRequest**)
        SLANG_OVERRIDE
    {
        lastSlot = 14;
        return SLANG_OK;
    }
    SLANG_NO_THROW SlangResult SLANG_MCALL createTypeConformanceComponentType(
        TypeReflection*,
        TypeReflection*,
        ITypeConformance**,
        SlangInt,
        ISlangBlob**) SLANG_OVERRIDE
    {
        lastSlot = 15;
        return SLANG_OK;
    }
    SLANG_NO_THROW IModule* SLANG_MCALL
    loadModuleFromIRBlob(const char*, const char*, IBlob*, IBlob**) SLANG_OVERRIDE
    {
        lastSlot = 16;
        return nullptr;
    }
    SLANG_NO_THROW SlangInt SLANG_MCALL getLoadedModuleCount() SLANG_OVERRIDE
    {
        lastSlot = 17;
        return 0;
    }
    SLANG_NO_THROW IModule* SLANG_MCALL getLoadedModule(SlangInt) SLANG_OVERRIDE
    {
        lastSlot = 18;
        return nullptr;
    }
    SLANG_NO_THROW bool SLANG_MCALL isBinaryModuleUpToDate(const char*, IBlob*) SLANG_OVERRIDE
    {
        lastSlot = 19;
        return false;
    }
    SLANG_NO_THROW IModule* SLANG_MCALL
    loadModuleFromSourceString(const char*, const char*, const char*, IBlob**) SLANG_OVERRIDE
    {
        lastSlot = 20;
        return nullptr;
    }
    SLANG_NO_THROW SlangResult SLANG_MCALL
    getDynamicObjectRTTIBytes(TypeReflection*, TypeReflection*, uint32_t*, uint32_t) SLANG_OVERRIDE
    {
        lastSlot = 21;
        return SLANG_OK;
    }
    SLANG_NO_THROW SlangResult SLANG_MCALL
    loadModuleInfoFromIRBlob(IBlob*, SlangInt&, const char*&, const char*&) SLANG_OVERRIDE
    {
        lastSlot = 22;
        return SLANG_OK;
    }
    SLANG_NO_THROW SlangResult SLANG_MCALL getDeclSourceLocation(DeclReflection*, SourceLocation*)
        SLANG_OVERRIDE
    {
        lastSlot = 23;
        return SLANG_OK;
    }
};

SLANG_UNIT_TEST(vtableISession)
{
    ISessionProbe p;
    callSlot(&p, 3);
    SLANG_CHECK(p.lastSlot == 3); // getGlobalSession
    callSlot(&p, 10);
    SLANG_CHECK(p.lastSlot == 10); // getDynamicType
    callSlot(&p, 14);
    SLANG_CHECK(p.lastSlot == 14); // createCompileRequest
    callSlot(&p, 17);
    SLANG_CHECK(p.lastSlot == 17); // getLoadedModuleCount
    callSlot(&p, 23);
    SLANG_CHECK(p.lastSlot == 23); // getDeclSourceLocation
}

// ---------------------------------------------------------------------------
// IModule : IComponentType  (own slots 17-29)
// ---------------------------------------------------------------------------
struct IModuleProbe : IModule
{
    int lastSlot = -1;
    SLANG_NO_THROW SlangResult SLANG_MCALL queryInterface(SlangUUID const&, void**) SLANG_OVERRIDE
    {
        lastSlot = 0;
        return SLANG_OK;
    }
    SLANG_NO_THROW uint32_t SLANG_MCALL addRef() SLANG_OVERRIDE
    {
        lastSlot = 1;
        return 1;
    }
    SLANG_NO_THROW uint32_t SLANG_MCALL release() SLANG_OVERRIDE
    {
        lastSlot = 2;
        return 1;
    }
    SLANG_NO_THROW ISession* SLANG_MCALL getSession() SLANG_OVERRIDE
    {
        lastSlot = 3;
        return nullptr;
    }
    SLANG_NO_THROW ProgramLayout* SLANG_MCALL getLayout(SlangInt, IBlob**) SLANG_OVERRIDE
    {
        lastSlot = 4;
        return nullptr;
    }
    SLANG_NO_THROW SlangInt SLANG_MCALL getSpecializationParamCount() SLANG_OVERRIDE
    {
        lastSlot = 5;
        return 0;
    }
    SLANG_NO_THROW SlangResult SLANG_MCALL getEntryPointCode(SlangInt, SlangInt, IBlob**, IBlob**)
        SLANG_OVERRIDE
    {
        lastSlot = 6;
        return SLANG_OK;
    }
    SLANG_NO_THROW SlangResult SLANG_MCALL
    getResultAsFileSystem(SlangInt, SlangInt, ISlangMutableFileSystem**) SLANG_OVERRIDE
    {
        lastSlot = 7;
        return SLANG_OK;
    }
    SLANG_NO_THROW void SLANG_MCALL getEntryPointHash(SlangInt, SlangInt, IBlob**) SLANG_OVERRIDE
    {
        lastSlot = 8;
    }
    SLANG_NO_THROW SlangResult SLANG_MCALL
    specialize(SpecializationArg const*, SlangInt, IComponentType**, ISlangBlob**) SLANG_OVERRIDE
    {
        lastSlot = 9;
        return SLANG_OK;
    }
    SLANG_NO_THROW SlangResult SLANG_MCALL link(IComponentType**, ISlangBlob**) SLANG_OVERRIDE
    {
        lastSlot = 10;
        return SLANG_OK;
    }
    SLANG_NO_THROW SlangResult SLANG_MCALL
    getEntryPointHostCallable(int, int, ISlangSharedLibrary**, ISlangBlob**) SLANG_OVERRIDE
    {
        lastSlot = 11;
        return SLANG_OK;
    }
    SLANG_NO_THROW SlangResult SLANG_MCALL renameEntryPoint(const char*, IComponentType**)
        SLANG_OVERRIDE
    {
        lastSlot = 12;
        return SLANG_OK;
    }
    SLANG_NO_THROW SlangResult SLANG_MCALL
    linkWithOptions(IComponentType**, uint32_t, CompilerOptionEntry const*, ISlangBlob**)
        SLANG_OVERRIDE
    {
        lastSlot = 13;
        return SLANG_OK;
    }
    SLANG_NO_THROW SlangResult SLANG_MCALL getTargetCode(SlangInt, IBlob**, IBlob**) SLANG_OVERRIDE
    {
        lastSlot = 14;
        return SLANG_OK;
    }
    SLANG_NO_THROW SlangResult SLANG_MCALL getTargetMetadata(SlangInt, IMetadata**, IBlob**)
        SLANG_OVERRIDE
    {
        lastSlot = 15;
        return SLANG_OK;
    }
    SLANG_NO_THROW SlangResult SLANG_MCALL
    getEntryPointMetadata(SlangInt, SlangInt, IMetadata**, IBlob**) SLANG_OVERRIDE
    {
        lastSlot = 16;
        return SLANG_OK;
    }
    SLANG_NO_THROW SlangResult SLANG_MCALL findEntryPointByName(char const*, IEntryPoint**)
        SLANG_OVERRIDE
    {
        lastSlot = 17;
        return SLANG_OK;
    }
    SLANG_NO_THROW SlangInt32 SLANG_MCALL getDefinedEntryPointCount() SLANG_OVERRIDE
    {
        lastSlot = 18;
        return 0;
    }
    SLANG_NO_THROW SlangResult SLANG_MCALL getDefinedEntryPoint(SlangInt32, IEntryPoint**)
        SLANG_OVERRIDE
    {
        lastSlot = 19;
        return SLANG_OK;
    }
    SLANG_NO_THROW SlangResult SLANG_MCALL serialize(ISlangBlob**) SLANG_OVERRIDE
    {
        lastSlot = 20;
        return SLANG_OK;
    }
    SLANG_NO_THROW SlangResult SLANG_MCALL writeToFile(char const*) SLANG_OVERRIDE
    {
        lastSlot = 21;
        return SLANG_OK;
    }
    SLANG_NO_THROW const char* SLANG_MCALL getName() SLANG_OVERRIDE
    {
        lastSlot = 22;
        return nullptr;
    }
    SLANG_NO_THROW const char* SLANG_MCALL getFilePath() SLANG_OVERRIDE
    {
        lastSlot = 23;
        return nullptr;
    }
    SLANG_NO_THROW const char* SLANG_MCALL getUniqueIdentity() SLANG_OVERRIDE
    {
        lastSlot = 24;
        return nullptr;
    }
    SLANG_NO_THROW SlangResult SLANG_MCALL
    findAndCheckEntryPoint(char const*, SlangStage, IEntryPoint**, ISlangBlob**) SLANG_OVERRIDE
    {
        lastSlot = 25;
        return SLANG_OK;
    }
    SLANG_NO_THROW SlangInt32 SLANG_MCALL getDependencyFileCount() SLANG_OVERRIDE
    {
        lastSlot = 26;
        return 0;
    }
    SLANG_NO_THROW char const* SLANG_MCALL getDependencyFilePath(SlangInt32) SLANG_OVERRIDE
    {
        lastSlot = 27;
        return nullptr;
    }
    SLANG_NO_THROW DeclReflection* SLANG_MCALL getModuleReflection() SLANG_OVERRIDE
    {
        lastSlot = 28;
        return nullptr;
    }
    SLANG_NO_THROW SlangResult SLANG_MCALL disassemble(IBlob**) SLANG_OVERRIDE
    {
        lastSlot = 29;
        return SLANG_OK;
    }
};

SLANG_UNIT_TEST(vtableIModule)
{
    IModuleProbe p;
    callSlot(&p, 3);
    SLANG_CHECK(p.lastSlot == 3); // getSession (inherited from IComponentType)
    callSlot(&p, 16);
    SLANG_CHECK(p.lastSlot == 16); // getEntryPointMetadata (last IComponentType slot)
    callSlot(&p, 17);
    SLANG_CHECK(p.lastSlot == 17); // findEntryPointByName
    callSlot(&p, 22);
    SLANG_CHECK(p.lastSlot == 22); // getName
    callSlot(&p, 23);
    SLANG_CHECK(p.lastSlot == 23); // getFilePath
    callSlot(&p, 24);
    SLANG_CHECK(p.lastSlot == 24); // getUniqueIdentity
    callSlot(&p, 29);
    SLANG_CHECK(p.lastSlot == 29); // disassemble
}

// ---------------------------------------------------------------------------
// IByteCodeRunner : ISlangUnknown  (own slots 3-13)
// ---------------------------------------------------------------------------
struct IByteCodeRunnerProbe : IByteCodeRunner
{
    int lastSlot = -1;
    SLANG_NO_THROW SlangResult SLANG_MCALL queryInterface(SlangUUID const&, void**) SLANG_OVERRIDE
    {
        lastSlot = 0;
        return SLANG_OK;
    }
    SLANG_NO_THROW uint32_t SLANG_MCALL addRef() SLANG_OVERRIDE
    {
        lastSlot = 1;
        return 1;
    }
    SLANG_NO_THROW uint32_t SLANG_MCALL release() SLANG_OVERRIDE
    {
        lastSlot = 2;
        return 1;
    }
    SLANG_NO_THROW SlangResult SLANG_MCALL loadModule(IBlob*) SLANG_OVERRIDE
    {
        lastSlot = 3;
        return SLANG_OK;
    }
    SLANG_NO_THROW SlangResult SLANG_MCALL selectFunctionByIndex(uint32_t) SLANG_OVERRIDE
    {
        lastSlot = 4;
        return SLANG_OK;
    }
    SLANG_NO_THROW int SLANG_MCALL findFunctionByName(const char*) SLANG_OVERRIDE
    {
        lastSlot = 5;
        return -1;
    }
    SLANG_NO_THROW SlangResult SLANG_MCALL getFunctionInfo(uint32_t, ByteCodeFuncInfo*) SLANG_OVERRIDE
    {
        lastSlot = 6;
        return SLANG_OK;
    }
    SLANG_NO_THROW void* SLANG_MCALL getCurrentWorkingSet() SLANG_OVERRIDE
    {
        lastSlot = 7;
        return nullptr;
    }
    SLANG_NO_THROW SlangResult SLANG_MCALL execute(void*, size_t) SLANG_OVERRIDE
    {
        lastSlot = 8;
        return SLANG_OK;
    }
    SLANG_NO_THROW void SLANG_MCALL getErrorString(IBlob**) SLANG_OVERRIDE { lastSlot = 9; }
    SLANG_NO_THROW void* SLANG_MCALL getReturnValue(size_t*) SLANG_OVERRIDE
    {
        lastSlot = 10;
        return nullptr;
    }
    SLANG_NO_THROW void SLANG_MCALL setExtInstHandlerUserData(void*) SLANG_OVERRIDE
    {
        lastSlot = 11;
    }
    SLANG_NO_THROW SlangResult SLANG_MCALL registerExtCall(const char*, VMExtFunction) SLANG_OVERRIDE
    {
        lastSlot = 12;
        return SLANG_OK;
    }
    SLANG_NO_THROW SlangResult SLANG_MCALL setPrintCallback(VMPrintFunc, void*) SLANG_OVERRIDE
    {
        lastSlot = 13;
        return SLANG_OK;
    }
};

SLANG_UNIT_TEST(vtableIByteCodeRunner)
{
    IByteCodeRunnerProbe p;
    callSlot(&p, 3);
    SLANG_CHECK(p.lastSlot == 3); // loadModule
    callSlot(&p, 5);
    SLANG_CHECK(p.lastSlot == 5); // findFunctionByName
    callSlot(&p, 7);
    SLANG_CHECK(p.lastSlot == 7); // getCurrentWorkingSet
    callSlot(&p, 10);
    SLANG_CHECK(p.lastSlot == 10); // getReturnValue
    callSlot(&p, 13);
    SLANG_CHECK(p.lastSlot == 13); // setPrintCallback
}

#endif // SLANG_PTR_IS_64
