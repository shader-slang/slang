#ifndef SLANG_STRING_UTIL_H
#define SLANG_STRING_UTIL_H

#include "slang-string.h"
#include "list.h"

#include <stdarg.h>

#include "../../slang-com-helper.h"
#include "../../slang-com-ptr.h"

namespace Slang {

/** A blob that uses a `String` for its storage.
*/
class StringBlob : public ISlangBlob, public RefObject
{
public:
    // ISlangUnknown
    SLANG_REF_OBJECT_IUNKNOWN_ALL

        // ISlangBlob
    SLANG_NO_THROW void const* SLANG_MCALL getBufferPointer() SLANG_OVERRIDE { return m_string.Buffer(); }
    SLANG_NO_THROW size_t SLANG_MCALL getBufferSize() SLANG_OVERRIDE { return m_string.Length(); }

        /// Get the contained string
    SLANG_FORCE_INLINE const String& getString() const { return m_string; }

    explicit StringBlob(String const& string)
        : m_string(string)
    {}

protected:
    ISlangUnknown* getInterface(const Guid& guid);
    String m_string;
};

struct StringUtil
{
        /// Split in, by specified splitChar into slices out
        /// Slices contents will directly address into in, so contents will only stay valid as long as in does.
    static void split(const UnownedStringSlice& in, char splitChar, List<UnownedStringSlice>& slicesOut);

        /// Returns the size in bytes needed to hold the formatted string using the specified args, NOT including a terminating 0
        /// NOTE! The caller *should* assume this will consume the va_list (use va_copy to make a copy to be consumed)
    static size_t calcFormattedSize(const char* format, va_list args);

        /// Calculate the formatted string using the specified args.
        /// NOTE! The caller *should* assume this will consume the va_list
        /// The buffer should be at least calcFormattedSize + 1 bytes. The +1 is needed because a terminating 0 is written. 
    static void calcFormatted(const char* format, va_list args, size_t numChars, char* dst);

        /// Appends formatted string with args into buf
    static void append(const char* format, va_list args, StringBuilder& buf);

        /// Appends the formatted string with specified trailing args
    static void appendFormat(StringBuilder& buf, const char* format, ...);

        /// Create a string from the format string applying args (like sprintf)
    static String makeStringWithFormat(const char* format, ...);

        /// Given a string held in a blob, returns as a String
        /// Returns an empty string if blob is nullptr 
    static String getString(ISlangBlob* blob);

        /// Create a blob from a string
    static ComPtr<ISlangBlob> createStringBlob(const String& string);
};

} // namespace Slang

#endif // SLANG_STRING_UTIL_H
