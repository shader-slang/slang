#pragma once

#include "../../core/slang-smart-pointer.h"
#include "slang-com-ptr.h"
#include "slang.h"

namespace SlangRecord
{
using namespace Slang;

/// Base class for all proxy types that wrap Slang COM interfaces.
/// Holds a ref-counted pointer to the underlying object.
///
/// Derived classes must inherit from their interface FIRST, then from ProxyBase.
/// Example: class BlobProxy : public ISlangBlob, public ProxyBase
///
/// This allows getInterface() to cast 'this' to ISlangUnknown* via the interface.
class ProxyBase : public RefObject
{
public:
    explicit ProxyBase(ISlangUnknown* actual)
        : m_actual(actual)
    {
    }

    template<typename T>
    T* getActual() const
    {
        return static_cast<T*>(m_actual.get());
    }

protected:
    Slang::ComPtr<ISlangUnknown> m_actual;
};

/// Macro to implement getInterface for proxy classes.
/// Must be used in classes that inherit from an interface first, then ProxyBase.
/// The InterfaceType should be the primary interface the proxy implements.
#define SLANG_PROXY_GET_INTERFACE(InterfaceType)                                     \
    ISlangUnknown* getInterface(const Guid& guid)                                    \
    {                                                                                \
        if (m_actual->queryInterface(guid, nullptr) == SLANG_OK)                     \
            return static_cast<InterfaceType*>(this);                                \
        return nullptr;                                                              \
    }

/// Wrap a Slang COM interface pointer in its corresponding proxy type.
/// Returns nullptr if the object cannot be wrapped (unknown type).
/// The returned pointer is ref-counted and must be released by the caller.
ISlangUnknown* wrapObject(ISlangUnknown* obj);

/// Helper template for wrapping with automatic type casting.
template<typename T>
T* wrap(T* obj)
{
    return static_cast<T*>(wrapObject(obj));
}

} // namespace SlangRecord
