#ifndef SLANG_CORE_STREAM_H
#define SLANG_CORE_STREAM_H

#include "slang-basic.h"

namespace Slang
{

class IOException : public Exception
{
public:
	IOException()
	{}
	IOException(const String & message)
		: Slang::Exception(message)
	{
	}
};

class EndOfStreamException : public IOException
{
public:
	EndOfStreamException()
	{}
	EndOfStreamException(const String & message)
		: IOException(message)
	{
	}
};

enum class SeekOrigin
{
	Start, End, Current
};

class Stream : public RefObject
{
public:
    virtual ~Stream() {}
	virtual Int64 getPosition()=0;
	virtual void seek(SeekOrigin origin, Int64 offset)=0;
	virtual size_t read(void * buffer, size_t length) = 0;
	virtual size_t write(const void * buffer, size_t length) = 0;
	virtual bool isEnd() = 0;
	virtual bool canRead() = 0;
	virtual bool canWrite() = 0;
	virtual void close() = 0;
};

enum class FileMode
{
	Create, Open, CreateNew, Append
};

enum class FileAccess
{
	None = 0, Read = 1, Write = 2, ReadWrite = 3
};

enum class FileShare
{
	None, ReadOnly, WriteOnly, ReadWrite
};

/// Base class for memory streams. Only supports reading and does NOT own contained data.
class MemoryStreamBase : public Stream
{
public:
    typedef Stream Super;

    virtual Int64 getPosition() SLANG_OVERRIDE { return m_position; }
    virtual void seek(SeekOrigin origin, Int64 offset) SLANG_OVERRIDE;
    virtual size_t read(void * buffer, size_t length) SLANG_OVERRIDE;
    virtual size_t write(const void * buffer, size_t length) SLANG_OVERRIDE { SLANG_UNUSED(buffer); SLANG_UNUSED(length); return 0; }
    virtual bool isEnd() SLANG_OVERRIDE { return m_atEnd; }
    virtual bool canRead() SLANG_OVERRIDE { return (int(m_access) & int(FileAccess::Read)) != 0; }
    virtual bool canWrite() SLANG_OVERRIDE { return (int(m_access) & int(FileAccess::Write)) != 0; }
    virtual void close() SLANG_OVERRIDE { m_access = FileAccess::None; }

        /// Get the contents
    ConstArrayView<uint8_t> getContents() const { return ConstArrayView<uint8_t>(m_contents, m_contentsSize); }

    MemoryStreamBase(FileAccess access = FileAccess::Read, const void* contents = nullptr, size_t contentsSize = 0):
        m_access(access)
    {
        _setContents(contents, contentsSize);
    }

protected:
        /// Set to replace wholly current content with specified content
    void _setContents(const void* contents, size_t contentsSize)
    {
        m_contents = (const uint8_t*)contents;
        m_contentsSize = ptrdiff_t(contentsSize);
        m_position = 0;
        m_atEnd = false;    
    }
        /// Update means that the content has changed, but position should be maintained
    void _updateContents(const void* contents, size_t contentsSize)
    {
        const ptrdiff_t newPosition = (m_position > ptrdiff_t(contentsSize)) ? ptrdiff_t(contentsSize) : m_position;
        _setContents(contents, contentsSize);
        m_position = newPosition;
    }

    const uint8_t* m_contents;   ///< The content held in the stream

    // Using ptrdiff_t (as opposed to size_t) as makes maths simpler
    ptrdiff_t m_contentsSize;    ///< Total size of the content in bytes
    ptrdiff_t m_position;       ///< The current position within content (valid values can only be between 0 and m_contentSize)

    bool m_atEnd;               ///< Happens when a read is done and nothing can be returned because already at end

    FileAccess m_access;
};

/// Memory stream that owns it's contents
class OwnedMemoryStream : public MemoryStreamBase
{
public:
    typedef MemoryStreamBase Super;

    virtual size_t write(const void* buffer, size_t length) SLANG_OVERRIDE;

        /// Set the contents
    void setContent(const void* contents, size_t contentsSize)
    {
        m_ownedContents.setCount(contentsSize);
        ::memcpy(m_ownedContents.getBuffer(), contents, contentsSize);
        _setContents(m_ownedContents.getBuffer(), m_ownedContents.getCount());
    }

    void swapContents(List<uint8_t>& rhs)
    {
        rhs.swapWith(m_ownedContents);
        _setContents(m_ownedContents.getBuffer(), m_ownedContents.getCount());
    }

    OwnedMemoryStream(FileAccess access) :
        Super(access)
    {}

protected:
     
    List<uint8_t> m_ownedContents;
};

class FileStream : public Stream
{
public:
    typedef Stream Super;

    // Stream interface
	virtual Int64 getPosition();
	virtual void seek(SeekOrigin origin, Int64 offset);
	virtual size_t read(void* buffer, size_t length);
	virtual size_t write(const void* buffer, size_t length);
	virtual bool canRead();
	virtual bool canWrite();
	virtual void close();
	virtual bool isEnd();

    FileStream(const String& fileName, FileMode fileMode = FileMode::Open);
    FileStream(const String& fileName, FileMode fileMode, FileAccess access, FileShare share);
    ~FileStream();

private:
    void _init(const String& fileName, FileMode fileMode, FileAccess access, FileShare share);

    FILE* m_handle;
    FileAccess m_fileAccess;            
    bool m_endReached = false;
};

} // namespace Slang

#endif
