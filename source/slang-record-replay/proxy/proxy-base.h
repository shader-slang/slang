#pragma once

#include "../../core/slang-smart-pointer.h"
#include "slang-com-ptr.h"
#include "slang.h"
#include "../replay-context.h"

namespace SlangRecord
{
using namespace Slang;

/// Variadic template base class for proxy types that wrap Slang COM interfaces.
/// Handles ref-counting and queryInterface automatically.
///
/// Usage:
///   class BlobProxy : public ProxyBase<ISlangBlob>
///   class ModuleProxy : public ProxyBase<slang::IModule, slang::IComponentType2, ...>
///
template<typename TFirstInterface, typename... TRestInterfaces>
class ProxyBase : public TFirstInterface, public TRestInterfaces..., public RefObject
{
public:
    explicit ProxyBase(ISlangUnknown* actual)
        : m_actual(actual)
    {
    }

    virtual ~ProxyBase()
    {
        // The proxy is being destroyed - unregister it from the context
        ReplayContext::get().unregisterProxy(toSlangUnknown());
    }

    /// Get the canonical ISlangUnknown* identity for this proxy.
    /// Always casts through TFirstInterface to ensure a consistent pointer
    /// value regardless of which base class 'this' is currently typed as.
    ISlangUnknown* toSlangUnknown()
    {
        return static_cast<ISlangUnknown*>(static_cast<TFirstInterface*>(this));
    }

    // ISlangUnknown implementation - these are virtual so derived classes can override
    // to record the calls for replay
    SLANG_NO_THROW uint32_t SLANG_MCALL addRef() SLANG_OVERRIDE
    {
        return addRefImpl();
    }

    SLANG_NO_THROW uint32_t SLANG_MCALL release() SLANG_OVERRIDE
    {
        return releaseImpl();
    }

    /// Internal addRef - used by the replay system to hold references without recording
    uint32_t addRefImpl()
    {
        return (uint32_t)RefObject::addReference();
    }

    /// Internal release - used by the replay system to release references without recording
    uint32_t releaseImpl()
    {
        return (uint32_t)RefObject::releaseReference();
    }

    SLANG_NO_THROW SlangResult SLANG_MCALL
    queryInterface(SlangUUID const& uuid, void** outObject) SLANG_OVERRIDE
    {
        // Delegate to the underlying object to check if it supports the interface
        ComPtr<ISlangUnknown> ptr;
        if (m_actual->queryInterface(uuid, (void**)ptr.writeRef()) == SLANG_OK)
        {
            addRef();
            // Cast through TFirstInterface to avoid ambiguity with multiple inheritance
            *outObject = static_cast<TFirstInterface*>(this);
            return SLANG_OK;
        }
        if (outObject)
            *outObject = nullptr;
        return SLANG_E_NO_INTERFACE;
    }

    /// Get the underlying actual object, cast to the requested interface type.
    template<typename T>
    T* getActual() const
    {
        return static_cast<T*>(m_actual.get());
    }

protected:
    Slang::ComPtr<ISlangUnknown> m_actual;
};

/// Wrap a Slang COM interface pointer in its corresponding proxy type.
/// Returns nullptr if the object cannot be wrapped (unknown type).
/// The returned pointer is ref-counted and must be released by the caller.
SLANG_API ISlangUnknown* wrapObject(ISlangUnknown* obj);
SLANG_API ISlangUnknown* unwrapObject(ISlangUnknown* proxy);

} // namespace SlangRecord

/// Macro to mark unimplemented methods in proxy classes.
/// Records "UNIMPLEMENTED: <message>" to the replay stream, then calls SLANG_UNIMPLEMENTED_X.
#define REPLAY_UNIMPLEMENTED_X(message) \
    do { \
        auto& ctx = SlangRecord::ReplayContext::get(); \
        if (ctx.isActive()) \
        { \
            const char* unimplementedMsg = "UNIMPLEMENTED: " message; \
            ctx.record(SlangRecord::RecordFlag::None, unimplementedMsg); \
        } \
        SLANG_UNIMPLEMENTED_X(message); \
    } while (0)

