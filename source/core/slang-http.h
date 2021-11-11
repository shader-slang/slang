#ifndef SLANG_CORE_HTTP_H
#define SLANG_CORE_HTTP_H

#include "../../slang.h"

#include "slang-string.h"
#include "slang-list.h"

#include "slang-memory-arena.h"

#include "slang-stream.h"

#include "../../slang-com-helper.h"
#include "../../slang-com-ptr.h"

namespace Slang {

/// All of the contained UnownedStringSlice are stored in the m_header 
struct HTTPHeader
{
    struct Pair
    {
        UnownedStringSlice key;
        UnownedStringSlice value;
    };

        /// Append the header (including termination) to out
    void append(StringBuilder& out) const;

        /// Reset the contents
    void reset();

    SLANG_INLINE Index indexOfKey(const UnownedStringSlice& slice) const;

        /// Ctor
    HTTPHeader() :
        m_arena(1024)
    {
    }

        /// Reads from stream until the buffer contains all of the header. The outEndIndex will point
        /// past the header termination.
    static SlangResult readHeaderText(BufferedReadStream* stream, Index& outEndIndex);

        /// Returns the index of the end of the header (index of first byte *after* the header), or < if doesn't have an end
    static Index findHeaderEnd(BufferedReadStream* stream);

        /// Parse the slice (holding a header) into out. 
    static SlangResult parse(const UnownedStringSlice& slice, HTTPHeader& out);

        /// Read from buffered stream header, and place parsed header into out
    static SlangResult read(BufferedReadStream* stream, HTTPHeader& out);

    size_t m_contentLength;             ///< Content length in bytes

    UnownedStringSlice m_mimeType;      ///< The mime type
    UnownedStringSlice m_encoding;      ///< The character encoding

    UnownedStringSlice m_header;        ///< Optionally holds the whole of the header 

    List<Pair> m_valuePairs;            /// All of the value pairs

    MemoryArena m_arena;                ///< Used to store backing memory

private:
    // Disable
    HTTPHeader(const HTTPHeader&) = delete;
    void operator=(const HTTPHeader&) = delete;
};

// -----------------------------------------------------------------
Index HTTPHeader::indexOfKey(const UnownedStringSlice& slice) const
{
    return m_valuePairs.findFirstIndex([&](const HTTPHeader::Pair& pair) -> bool { return pair.key == slice; });
}

class HTTPPacketConnection : public RefObject
{
public:

    enum class ReadState
    {
        Header,                     ///< Reading reader
        Content,                    ///< Reading content (ie header is read)
        Done,                       ///< The content is read
        Closed,                     ///< The read stream is closed - no further packets can be read
        Error,                      ///< In an error state - no further packets can be read
    };

        /// Update state
    SlangResult update();
        /// Get the current read staet
    ReadState getReadState() const { return m_readState; }
        /// Get the read header
    const HTTPHeader& getReadHeader() const { SLANG_ASSERT(hasHeader()); return m_readHeader; }
        /// Get the content 
    ConstArrayView<Byte> getContent() const { SLANG_ASSERT(m_readState == ReadState::Done); return ConstArrayView<Byte>((const Byte*)m_readStream->getBuffer(), m_readHeader.m_contentLength); }

        /// Write. Will potentially block if write stream is blocking.
    SlangResult write(const void* content, size_t sizeInBytes);

        /// Blocks until full read packet is available or the stream is appropriately closed
    SlangResult waitForContent();
        /// Consume the content - so can read next content
    void consumeContent();

        /// True if active
    bool isActive() const { return m_readState != ReadState::Error && m_readState != ReadState::Closed;  }

    bool hasHeader() const { return m_readState == ReadState::Content || m_readState == ReadState::Done;  }
        /// True if has content (implies has header)
    bool hasContent() const { return m_readState == ReadState::Done;  }

    HTTPPacketConnection(BufferedReadStream* readStream, Stream* writeStream);

protected:
    SlangResult _updateReadResult(SlangResult res)
    {
        if (SLANG_FAILED(res) && SLANG_SUCCEEDED(m_readResult))
        {
            m_readState = ReadState::Error;
            m_readResult = res;
        }
        return res;
    }

    SlangResult _handleHeader();
    SlangResult _handleContent();

    SlangResult m_readResult;
    HTTPHeader m_readHeader;

    ReadState m_readState;

    RefPtr<BufferedReadStream> m_readStream;
    RefPtr<Stream> m_writeStream;
};

} // namespace Slang

#endif // SLANG_CORE_HTTP_H
