#ifndef SLANG_CORE_SHARED_LIBRARY_H
#define SLANG_CORE_SHARED_LIBRARY_H

#include "../../slang.h"
#include "../../slang-com-helper.h"
#include "../../slang-com-ptr.h"

#include "../core/slang-platform.h"
#include "../core/slang-common.h"
#include "../core/slang-dictionary.h"

namespace Slang
{

/* NOTE! Do not change this enum without making the appropriate changes to DefaultSharedLibraryLoader::s_libraryNames */
enum class SharedLibraryType
{
    Unknown,            ///< Unknown compiler
    Dxc,                ///< Dxc compiler
    Fxc,                ///< Fxc compiler
    Glslang,            ///< Slang specific glslang compiler
    Dxil,               ///< Dxil is used with dxc
    CountOf,
};

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

        /// Get the type from the name
    static SharedLibraryType getSharedLibraryTypeFromName(const UnownedStringSlice& name);

        /// Get the name from the type, or nullptr if not known
    static const char* getSharedLibraryNameFromType(SharedLibraryType type) { return s_libraryNames[int(type)]; }

        /// Make a shared library to it's name
    static const char* s_libraryNames[int(SharedLibraryType::CountOf)];

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
    // ISlangUnknown 
    SLANG_REF_OBJECT_IUNKNOWN_ALL

    // ISlangSharedLibrary
    virtual SLANG_NO_THROW SlangFuncPtr SLANG_MCALL findFuncByName(char const* name) SLANG_OVERRIDE;

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

class ConfigurableSharedLibraryLoader: public ISlangSharedLibraryLoader, public RefObject
{
public:
    typedef Result (*Func)(const char* pathIn, const String& entryString, SharedLibrary::Handle& handleOut);

    // IUnknown
    SLANG_REF_OBJECT_IUNKNOWN_ALL

    // ISlangSharedLibraryLoader
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL loadSharedLibrary(const char* path, ISlangSharedLibrary** sharedLibraryOut) SLANG_OVERRIDE;

        /// Function to replace the the path with entryString
    static Result replace(const char* pathIn, const String& entryString, SharedLibrary::Handle& handleOut);
        /// Function to change the path using the entryStrinct
    static Result changePath(const char* pathIn, const String& entryString, SharedLibrary::Handle& handleOut);

    void addEntry(const String& libName, Func func, const String& entryString) { m_entryMap.Add(libName, Entry{ func, entryString} ); }
    void addEntry(SharedLibraryType libType, Func func, const String& entryString) { m_entryMap.Add(DefaultSharedLibraryLoader::getSharedLibraryNameFromType(libType), Entry { func, entryString} ); }

    virtual ~ConfigurableSharedLibraryLoader() {}
    protected:

    struct Entry
    {
        Func func;
        String entryString;
    };

    ISlangUnknown* getInterface(const Guid& guid);

    Dictionary<String, Entry> m_entryMap;
};

}

#endif // SLANG_SHARED_LIBRARY_H_INCLUDED
