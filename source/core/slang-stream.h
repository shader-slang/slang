#ifndef SLANG_CORE_STREAM_H
#define SLANG_CORE_STREAM_H

#include "slang-basic.h"

namespace Slang
{

enum class SeekOrigin
{
	Start,      ///< Seek from the start of the stream
    End,        ///< Seek from the end of the stream
    Current,    ///< Seek from the current cursor position
};

class Stream : public RefObject
{
public:
    virtual ~Stream() {}
        /// Get the current 'cursor' position in the stream
	virtual Int64 getPosition()=0;
        /// Seek the cursor to a position. How the seek is performed is dependent on the 'origin' and the offset required.
        /// NOTE that *any* seek will reset the 'end of stream' status. See 'read' for requirements for 'isEnd' to be reached.
	virtual SlangResult seek(SeekOrigin origin, Int64 offset)=0;
        /// Read from the current position into buffer.
        /// If there are less bytes available than requested only the amount available will be read. outReadBytes holds the actual amount of bytes read.
        /// It is valid (and not an error) for read to return 0 bytes read - even if the end of the stream.
        /// 
        /// 'isEnd' only becomes true when a read is performed *past* the end of a stream.
        /// If a non zero read is performed from the end then isEnd must be true.
        /// 
        /// Will return an error if there is a reading failure.
	virtual SlangResult read(void* buffer, size_t length, size_t& outReadBytes) = 0;
        /// Write to the stream from current position
	virtual SlangResult write(const void* buffer, size_t length) = 0;
        /// True if the of the stream has been hit. The 'read' method has more discussion as to when this can occur.
	virtual bool isEnd() = 0;
        /// Returns true if it's possible to read from the stream. 
	virtual bool canRead() = 0;
        /// Returns true when it's possible to write to the stream. 
	virtual bool canWrite() = 0;
        /// Close the stream. Once closed no more operations can be performed on the stream.
        /// Implies any pending data is flushed.
	virtual void close() = 0;

        /// Helper function that will also *fail* if the specified amount of bytes aren't read.
    SlangResult readExactly(void* buffer, size_t length);
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
    virtual SlangResult seek(SeekOrigin origin, Int64 offset) SLANG_OVERRIDE;
    virtual SlangResult read(void * buffer, size_t length, size_t& outReadByts) SLANG_OVERRIDE;
    virtual SlangResult write(const void * buffer, size_t length) SLANG_OVERRIDE { SLANG_UNUSED(buffer); SLANG_UNUSED(length); return SLANG_E_NOT_IMPLEMENTED; }
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

    virtual SlangResult write(const void* buffer, size_t length) SLANG_OVERRIDE;

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
	virtual Int64 getPosition() SLANG_OVERRIDE;
	virtual SlangResult seek(SeekOrigin origin, Int64 offset) SLANG_OVERRIDE;
	virtual SlangResult read(void* buffer, size_t length, size_t& outReadBytes) SLANG_OVERRIDE;
	virtual SlangResult write(const void* buffer, size_t length) SLANG_OVERRIDE;
	virtual bool canRead() SLANG_OVERRIDE;
	virtual bool canWrite() SLANG_OVERRIDE;
	virtual void close() SLANG_OVERRIDE;
	virtual bool isEnd() SLANG_OVERRIDE;

    FileStream();
    
    SlangResult init(const String& fileName, FileMode fileMode, FileAccess access, FileShare share);
    SlangResult init(const String& fileName, FileMode fileMode = FileMode::Open);

    ~FileStream();

private:
    SlangResult _init(const String& fileName, FileMode fileMode, FileAccess access, FileShare share);

    FILE* m_handle;
    FileAccess m_fileAccess;            
    bool m_endReached = false;
};

} // namespace Slang

#endif
