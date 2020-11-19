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


class ScopedAllocation
{
public:
    typedef ScopedAllocation ThisType;
    // Returns the allocation if successful.
    void* allocate(size_t size)
    {
        deallocate();
        m_data = ::malloc(size);
        m_sizeInBytes = size;
        return m_data;
    }
    /// Deallocates if holds an allocation
    void deallocate()
    {
        if (m_data)
        {
            ::free(m_data);
            m_data = nullptr;
        }
        m_sizeInBytes = 0;
    }
    /// Makes this no longer own the allocation. Returns the allocated data (or nullptr if no allocation)
    void* detach()
    {
        void* data = m_data;
        m_data = nullptr;
        m_sizeInBytes = 0;
        return data;
    }
    /// Attach some data.
    /// NOTE! data must be a pointer that was returned from malloc, otherwise will incorrectly free.
    void attach(void* data, size_t size)
    {
        deallocate();
        m_data = data;
        m_sizeInBytes = size;
    }

    /// Get the allocated data. Returns nullptr if there is no allocated data
    void* getData() const { return m_data; }
    /// Get the size of the allocated data.
    size_t getSizeInBytes() const { return m_sizeInBytes; }

    ScopedAllocation() :
        m_data(nullptr),
        m_sizeInBytes(0)
    {
    }

private:
    // disable
    ScopedAllocation(const ThisType& rhs) = delete;
    void operator=(const ThisType& rhs) = delete;

    void* m_data;
    size_t m_sizeInBytes;
};

/** A blob that manages some raw data that it owns.
*/
class RawBlob : public BlobBase
{
public:
    // ISlangBlob
    SLANG_NO_THROW void const* SLANG_MCALL getBufferPointer() SLANG_OVERRIDE { return m_data.getData(); }
    SLANG_NO_THROW size_t SLANG_MCALL getBufferSize() SLANG_OVERRIDE { return m_data.getSizeInBytes(); }

        // Ctor
        // NOTE! Takes a copy of the input data
    RawBlob(const void* data, size_t size) 
    {
        memcpy(m_data.allocate(size), data, size);
    }

        /// Moves ownership of data and dataCount to the blob
        /// data must be a pointer returned by ::malloc.
    static RefPtr<RawBlob> moveCreate(uint8_t* data, size_t dataCount)
    {
        RawBlob* blob = new RawBlob;
        blob->m_data.attach(data, dataCount);
        return blob;
    }

protected:
    RawBlob() = default;

    ScopedAllocation m_data;

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
