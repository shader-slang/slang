#include "proxy-base.h"

#include "proxy-blob.h"
#include "proxy-compile-request.h"
#include "proxy-compile-result.h"
#include "proxy-component-type.h"
#include "proxy-entry-point.h"
#include "proxy-global-session.h"
#include "proxy-metadata.h"
#include "proxy-module.h"
#include "proxy-mutable-file-system.h"
#include "proxy-session.h"
#include "proxy-shared-library.h"
#include "proxy-type-conformance.h"

namespace SlangRecord
{

// Helper template that attempts to query for interface T and wrap in ProxyT
template<typename T, typename ProxyT>
ISlangUnknown* tryWrap(ISlangUnknown* obj)
{
    Slang::ComPtr<T> queried;
    if (SLANG_SUCCEEDED(obj->queryInterface(T::getTypeGuid(), (void**)queried.writeRef())))
    {
        // Use static_cast to T* first (the interface), then to ISlangUnknown*
        // to avoid ambiguity from multiple inheritance
        ProxyT* proxy = new ProxyT(queried.get());
        obj->release();
        proxy->addRef();
        return static_cast<ISlangUnknown*>(static_cast<T*>(proxy));
    }
    return nullptr;
}

// Macro to make the try-wrap pattern cleaner
#define TRY_WRAP(InterfaceType, ProxyType)          \
    if (auto* wrapped = tryWrap<InterfaceType, ProxyType>(obj)) \
        return wrapped;

ISlangUnknown* wrapObject(ISlangUnknown* obj)
{
    if (!obj)
        return nullptr;

    // Order matters due to inheritance!
    // Check more derived types before base types.

    // IModule derives from IComponentType
    TRY_WRAP(slang::IModule, ModuleProxy)

    // IEntryPoint derives from IComponentType
    TRY_WRAP(slang::IEntryPoint, EntryPointProxy)

    // ITypeConformance derives from IComponentType
    TRY_WRAP(slang::ITypeConformance, TypeConformanceProxy)

    // IComponentType (base for Module, EntryPoint, TypeConformance)
    TRY_WRAP(slang::IComponentType, ComponentTypeProxy)

    // Session types
    TRY_WRAP(slang::IGlobalSession, GlobalSessionProxy)
    TRY_WRAP(slang::ISession, SessionProxy)

    // Compile-related
    TRY_WRAP(slang::ICompileRequest, CompileRequestProxy)
    TRY_WRAP(slang::ICompileResult, CompileResultProxy)
    TRY_WRAP(slang::IMetadata, MetadataProxy)

    // File system - ISlangMutableFileSystem derives from ISlangFileSystemExt
    TRY_WRAP(ISlangMutableFileSystem, MutableFileSystemProxy)

    // Other types
    TRY_WRAP(ISlangSharedLibrary, SharedLibraryProxy)
    TRY_WRAP(ISlangBlob, BlobProxy)

    // Unknown type - return nullptr
    return nullptr;
}

#undef TRY_WRAP

} // namespace SlangRecord
