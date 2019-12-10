#ifndef SLANG_CORE_BLOB_H
#define SLANG_CORE_BLOB_H

#include "../../slang.h"

#include "slang-string.h"
#include "slang-list.h"

#include <stdarg.h>

#include "../../slang-com-helper.h"
#include "../../slang-com-ptr.h"

namespace Slang {

/** Base class for simple blobs.
*/
class BlobBase : public ISlangBlob, public RefObject
{
public:
    // ISlangUnknown
    SLANG_REF_OBJECT_IUNKNOWN_ALL

protected:
    ISlangUnknown* getInterface(const Guid& guid);
};

/** A blob that uses a `String` for its storage.
*/
class StringBlob : public BlobBase
{
public:
        // ISlangBlob
    SLANG_NO_THROW void const* SLANG_MCALL getBufferPointer() SLANG_OVERRIDE { return m_string.getBuffer(); }
    SLANG_NO_THROW size_t SLANG_MCALL getBufferSize() SLANG_OVERRIDE { return m_string.getLength(); }

        /// Get the contained string
    SLANG_FORCE_INLINE const String& getString() const { return m_string; }

    explicit StringBlob(String const& string)
        : m_string(string)
    {}

protected:
    String m_string;
};

class ListBlob : public BlobBase
{
public:
    typedef BlobBase Super;
    typedef ListBlob ThisType;

    // ISlangBlob
    SLANG_NO_THROW void const* SLANG_MCALL getBufferPointer() SLANG_OVERRIDE { return m_data.getBuffer(); }
    SLANG_NO_THROW size_t SLANG_MCALL getBufferSize() SLANG_OVERRIDE { return m_data.getCount(); }

    ListBlob() {}

    ListBlob(const List<uint8_t>& data): m_data(data) {}
        // Move ctor
    ListBlob(List<uint8_t>&& data): m_data(data) {}

    static RefPtr<ListBlob> moveCreate(List<uint8_t>& data) { return new ListBlob(_Move(data)); } 

    List<uint8_t> m_data;

protected:
    void operator=(const ThisType& rhs) = delete;
};

/** A blob that manages some raw data that it owns.
*/
class RawBlob : public BlobBase
{
public:
    // ISlangBlob
    SLANG_NO_THROW void const* SLANG_MCALL getBufferPointer() SLANG_OVERRIDE { return m_data; }
    SLANG_NO_THROW size_t SLANG_MCALL getBufferSize() SLANG_OVERRIDE { return m_size; }

    // Ctor
    RawBlob(const void* data, size_t size) :
        m_size(size)
    {
        m_data = malloc(size);
        memcpy(m_data, data, size);
    }
    ~RawBlob()
    {
        free(m_data);
    }

protected:
    void* m_data;
    size_t m_size;
};

/// Create a blob that will retain (a copy of) raw data.
///
inline ComPtr<ISlangBlob> createRawBlob(void const* inData, size_t size)
{
    return ComPtr<ISlangBlob>(new RawBlob(inData, size));
}

class ScopeRefObjectBlob : public BlobBase
{
public:
    // ISlangBlob
    SLANG_NO_THROW void const* SLANG_MCALL getBufferPointer() SLANG_OVERRIDE { return m_blob->getBufferPointer(); }
    SLANG_NO_THROW size_t SLANG_MCALL getBufferSize() SLANG_OVERRIDE { return m_blob->getBufferSize(); }

    // Ctor
    ScopeRefObjectBlob(ISlangBlob* blob, RefObject* scope) :
        m_blob(blob),
        m_scope(scope)
    {
    }

protected:
    RefPtr<RefObject> m_scope;
    ComPtr<ISlangBlob> m_blob;
};

} // namespace Slang

#endif // SLANG_CORE_BLOB_H
