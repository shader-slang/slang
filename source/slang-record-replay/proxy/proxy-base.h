#pragma once

#include "../../core/slang-smart-pointer.h"
#include "slang-com-ptr.h"
#include "slang.h"

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

    // ISlangUnknown implementation
    SLANG_NO_THROW uint32_t SLANG_MCALL addRef() SLANG_OVERRIDE
    {
        return (uint32_t)RefObject::addReference();
    }

    SLANG_NO_THROW uint32_t SLANG_MCALL release() SLANG_OVERRIDE
    {
        return (uint32_t)RefObject::releaseReference();
    }

    SLANG_NO_THROW SlangResult SLANG_MCALL
    queryInterface(SlangUUID const& uuid, void** outObject) SLANG_OVERRIDE
    {
        // Delegate to the underlying object to check if it supports the interface
        if (m_actual->queryInterface(uuid, nullptr) == SLANG_OK)
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
        return dynamic_cast<T*>(m_actual.get());
    }

protected:
    Slang::ComPtr<ISlangUnknown> m_actual;
};

/// Wrap a Slang COM interface pointer in its corresponding proxy type.
/// Returns nullptr if the object cannot be wrapped (unknown type).
/// The returned pointer is ref-counted and must be released by the caller.
ISlangUnknown* wrapObject(ISlangUnknown* obj);


} // namespace SlangRecord
