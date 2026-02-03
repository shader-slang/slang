#pragma once

#include "slang-com-ptr.h"
#include "slang.h"

namespace SlangRecord
{

/// Base class for all proxy types that wrap Slang COM interfaces.
/// Holds a ref-counted pointer to the underlying object.
class ProxyBase
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

} // namespace SlangRecord
