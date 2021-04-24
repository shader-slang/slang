#ifndef SLANG_CORE_SHARED_LIBRARY_H
#define SLANG_CORE_SHARED_LIBRARY_H

#include "../../slang.h"
#include "../../slang-com-helper.h"
#include "../../slang-com-ptr.h"

#include "../core/slang-io.h"
#include "../core/slang-platform.h"
#include "../core/slang-common.h"
#include "../core/slang-dictionary.h"

namespace Slang
{

class DefaultSharedLibraryLoader : public ISlangSharedLibraryLoader
{
public:

    // ISlangUnknown 
    // override ref counting, as DefaultSharedLibraryLoader is singleton
    SLANG_IUNKNOWN_QUERY_INTERFACE 
    SLANG_NO_THROW uint32_t SLANG_MCALL addRef() SLANG_OVERRIDE { return 1; }
    SLANG_NO_THROW uint32_t SLANG_MCALL release() SLANG_OVERRIDE { return 1; } 

    // ISlangSharedLibraryLoader
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL loadSharedLibrary(const char* path, 
        ISlangSharedLibrary** outSharedLibrary) SLANG_OVERRIDE;

    SlangResult loadPlatformSharedLibrary(const char* path, ISlangSharedLibrary** outSharedLibrary);

        /// Get the singleton
    static DefaultSharedLibraryLoader* getSingleton() { return &s_singleton; }


    static SlangResult load(ISlangSharedLibraryLoader* loader, const String& path, const String& name, ISlangSharedLibrary** outLibrary);

private:
        /// Make so not constructible
    DefaultSharedLibraryLoader() {}
    virtual ~DefaultSharedLibraryLoader() {}

    ISlangUnknown* getInterface(const Guid& guid);

    static DefaultSharedLibraryLoader s_singleton;
};

class DefaultSharedLibrary : public ISlangSharedLibrary, public RefObject
{
    public:
    SLANG_CLASS_GUID(0xe7f2597b, 0xf803, 0x4b6e, { 0xaf, 0x8b, 0xcb, 0xe3, 0xa2, 0x21, 0xfd, 0x5a })

    // ISlangUnknown
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL queryInterface(SlangUUID const& uuid, void** outObject) SLANG_OVERRIDE;
    SLANG_REF_OBJECT_IUNKNOWN_ADD_REF         
    SLANG_REF_OBJECT_IUNKNOWN_RELEASE
    
    // ISlangSharedLibrary
    virtual SLANG_NO_THROW void* SLANG_MCALL findSymbolAddressByName(char const* name) SLANG_OVERRIDE;

        /// Ctor.
    DefaultSharedLibrary(const SharedLibrary::Handle sharedLibraryHandle):
        m_sharedLibraryHandle(sharedLibraryHandle)
    {
        SLANG_ASSERT(sharedLibraryHandle);
    }

        /// Need virtual dtor to keep delete this happy
    virtual ~DefaultSharedLibrary();

    protected:
    ISlangUnknown* getInterface(const Guid& guid);

    SharedLibrary::Handle m_sharedLibraryHandle = nullptr;
};

class TemporarySharedLibrary : public DefaultSharedLibrary
{
public:
    typedef DefaultSharedLibrary Super;

        /// Get the path to the shared library
    const String& getPath() const { return m_path; }

        /// Ctor
    TemporarySharedLibrary(const SharedLibrary::Handle sharedLibraryHandle, const String& path):
        Super(sharedLibraryHandle),
        m_path(path)
    {
    }

    virtual ~TemporarySharedLibrary();

        /// Any files specified in this set will be deleted on exit
    RefPtr<TemporaryFileSet> m_temporaryFileSet;

protected:
    String m_path;
};

class SharedLibraryUtils
{
public:
    static String getSharedLibraryFileName(void* symbolInLib);
    static uint64_t getSharedLibraryTimestamp(void* symbolInLib);
};

}

#endif // SLANG_SHARED_LIBRARY_H_INCLUDED
