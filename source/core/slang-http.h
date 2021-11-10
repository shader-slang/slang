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
struct HttpHeader
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
    HttpHeader() :
        m_arena(1024)
    {
    }

        /// Reads from stream until the buffer contains all of the header. The outEndIndex will point
        /// past the header termination.
    static SlangResult read(BufferedReadStream* stream, Index& outEndIndex);

    static SlangResult parse(const UnownedStringSlice& slice, HttpHeader& out);
    
    static SlangResult read(BufferedReadStream* stream, HttpHeader& out);

    size_t m_contentLength;             ///< Content length in bytes

    UnownedStringSlice m_mimeType;      ///< The mime type
    UnownedStringSlice m_encoding;      ///< The character encoding

    UnownedStringSlice m_header;        ///< Optionally holds the whole of the header 

    List<Pair> m_valuePairs;            /// All of the value pairs

    MemoryArena m_arena;                ///< Used to store backing memory
};

// -----------------------------------------------------------------
Index HttpHeader::indexOfKey(const UnownedStringSlice& slice) const
{
    return m_valuePairs.findFirstIndex([&](const HttpHeader::Pair& pair) -> bool { return pair.key == slice; });
}

} // namespace Slang

#endif // SLANG_CORE_HTTP_H
