#include "slang-http.h"

#include "slang-string-util.h"

namespace Slang {

static const UnownedStringSlice g_headerEnd = UnownedStringSlice::fromLiteral("\r\n\r\n");
static const UnownedStringSlice g_contentLength = UnownedStringSlice::fromLiteral("Content-Length");
static const UnownedStringSlice g_contentType = UnownedStringSlice::fromLiteral("Content-Type");

void HttpHeader::reset()
{
    const UnownedStringSlice empty;

    m_contentLength = 0;
    m_mimeType = empty;
    m_encoding = empty;
    m_valuePairs.clear();
    m_header = empty;

    m_arena.deallocateAll();
}

/* static */SlangResult HttpHeader::readHeaderText(BufferedReadStream* stream, Index& outEndIndex)
{
    // https://microsoft.github.io/language-server-protocol/specifications/specification-current/
    while (true)
    {
        SLANG_RETURN_ON_FAIL(stream->update());

        // This could be more efficient - it just searches until there are enough bytes to have termination
        auto bytes = stream->getView();
        UnownedStringSlice input((const char*)bytes.begin(), (const char*)bytes.end());

        const Index index = input.indexOf(g_headerEnd);
        if (index >= 0)
        {
            outEndIndex = index + g_headerEnd.getLength();
            return SLANG_OK;
        }
    }
}

/* static */SlangResult HttpHeader::parse(const UnownedStringSlice& inSlice, HttpHeader& out)
{
    out.reset();

    if (!inSlice.endsWith(g_headerEnd))
    {
        return SLANG_FAIL;
    }

    {
        // Strip the end
        auto slice = inSlice.head(inSlice.getLength() - g_headerEnd.getLength());
        // Store a copy in the header
        out.m_header = UnownedStringSlice(out.m_arena.allocateString(slice.begin(), slice.getLength()), slice.getLength());
    }

    // Okay, we need to split into lines, and then examine the contents
    for (auto line : LineParser(out.m_header))
    {
        // Examine the line for :
        Index index = line.indexOf(':');
        if (index < 0)
        {
            return SLANG_FAIL;
        }

        const UnownedStringSlice key = line.head(index).trim();
        const UnownedStringSlice value = line.tail(index + 1).trim();

        // Add the pair
        Pair pair{ key, value };

        // We could check if key is already used. Some values can be repeated I believe.
        // So we just allow for now.

        out.m_valuePairs.add(pair);

        if (key == g_contentLength)
        {
            Index length;
            SLANG_RETURN_ON_FAIL(StringUtil::parseInt(value, length) || length < 0);

            out.m_contentLength = length;
        }
        else if (key == g_contentType)
        {
            List<UnownedStringSlice> slices;

            // text/html; charset=UTF-8
            StringUtil::split(value, ';', slices);

            if (slices.getCount() < 1)
            {
                return SLANG_FAIL;
            }
            // set the mime type
            out.m_mimeType = slices[0].trim();

            // Look for other parameters, in particular charset
            for (Index i = 1; i < slices.getCount(); ++i)
            {
                auto slice = slices[i];
                Index equalIndex = slice.indexOf('=');
                if (equalIndex >= 0)
                {
                    auto paramName = slice.head(equalIndex).trim();
                    auto paramValue = slice.tail(equalIndex + 1).trim();

                    if (paramName == UnownedStringSlice::fromLiteral("charset"))
                    {
                        out.m_encoding = paramValue;
                    }
                }
            }
        }
    }

    return SLANG_OK;
}

/* static */SlangResult HttpHeader::read(BufferedReadStream* stream, HttpHeader& out)
{
    Index endIndex;
    SLANG_RETURN_ON_FAIL(readHeaderText(stream, endIndex));

    // Get header into a slice
    UnownedStringSlice headerText((const char*)stream->getBuffer(), endIndex);

    // Parse the slice into the out HttpHeader
    SLANG_RETURN_ON_FAIL(parse(headerText, out));

    // Can consume these bytes from the stream.
    stream->consume(endIndex);

    return SLANG_OK;
}

void HttpHeader::append(StringBuilder& out) const
{
    // Output the content length
    out << g_contentLength << ": " << m_contentLength << "\r\n";

    // If either is set construct a content type
    if (m_mimeType.getLength() || m_encoding.getLength())
    {
        out << g_contentType << ": ";

        // https://developer.mozilla.org/en-US/docs/Web/HTTP/Basics_of_HTTP/MIME_types

        auto mimeType = m_mimeType.getLength() ? m_mimeType : UnownedStringSlice::fromLiteral("text/plain");
        auto encoding = m_encoding.getLength() ? m_encoding : UnownedStringSlice::fromLiteral("UTF-8");

        out << "mimeType" << "; ";
        out << "charset=" << encoding;

        out << "\r\n";
    }

    // Output any other data
    for (auto pair : m_valuePairs)
    {
        auto key = pair.key;
        // Ignore these types, as already output from data we already have
        if (key == g_contentType || key == g_contentLength)
        {
            continue;
        }

        out << key << ": " << pair.value << "\r\n";
    }

    // Add termination
    out << "\r\n";
}

} // namespace Slang
