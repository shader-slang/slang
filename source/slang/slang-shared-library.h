#ifndef SLANG_SHARED_LIBRARY_H_INCLUDED
#define SLANG_SHARED_LIBRARY_H_INCLUDED

#include "../../slang.h"
#include "../../slang-com-helper.h"
#include "../../slang-com-ptr.h"

#include "../core/platform.h"

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
        ISlangSharedLibrary** sharedLibraryOut) SLANG_OVERRIDE;

        /// Get the singleton
    static DefaultSharedLibraryLoader* getSingleton() { return &s_singleton; }

private:
        /// Make so not constructible
    DefaultSharedLibraryLoader() {}
    virtual ~DefaultSharedLibraryLoader() {}

    ISlangUnknown* getInterface(const Guid& guid);

    static DefaultSharedLibraryLoader s_singleton;
};

class DefaultSharedLibrary : public ISlangSharedLibrary
{
    public:
    // ISlangUnknown 
    SLANG_IUNKNOWN_ALL

    // ISlangSharedLibrary
    virtual SLANG_NO_THROW  bool SLANG_MCALL isLoaded() SLANG_OVERRIDE;
    virtual SLANG_NO_THROW void SLANG_MCALL unload() SLANG_OVERRIDE;
    virtual SLANG_NO_THROW SlangFuncPtr SLANG_MCALL findFuncByName(char const* name) SLANG_OVERRIDE;

        /// Ctor.
    DefaultSharedLibrary(const SharedLibrary& lib):
        m_sharedLibrary(lib)
    {}

        /// Need virtual dtor to keep delete this happy
    virtual ~DefaultSharedLibrary() {}

    protected:
    ISlangUnknown* getInterface(const Guid& guid);

    SharedLibrary m_sharedLibrary;
    int32_t m_refCount;
};

}

#endif // SLANG_SHARED_LIBRARY_H_INCLUDED