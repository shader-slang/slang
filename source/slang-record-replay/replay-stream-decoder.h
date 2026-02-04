#pragma once

#include "replay-stream.h"
#include "replay-context.h"
#include "../core/slang-string.h"

namespace SlangRecord {

/// Decodes a binary replay stream to human-readable text format.
/// Can be used to:
/// 1. Convert a captured stream.bin to text for debugging
/// 2. Print stream contents as they are recorded (live logging)
class ReplayStreamDecoder
{
public:
    // =========================================================================
    // Static API for decoding entire streams
    // =========================================================================

    /// Decode a replay stream to a string.
    /// @param stream The stream to decode (will be read from current position).
    /// @param maxBytes Maximum bytes to decode (0 = entire stream).
    /// @return Human-readable text representation.
    SLANG_API static Slang::String decode(ReplayStream& stream, size_t maxBytes = 0);

    /// Decode a file to string.
    /// @param filePath Path to the stream.bin file.
    /// @return Human-readable text representation.
    SLANG_API static Slang::String decodeFile(const char* filePath);

    /// Decode raw bytes to string.
    /// @param data Pointer to the data.
    /// @param size Size of the data in bytes.
    /// @return Human-readable text representation.
    SLANG_API static Slang::String decodeBytes(const void* data, size_t size);

    // =========================================================================
    // Static API for decoding individual values (for live logging)
    // =========================================================================

    /// Decode a single value from a stream at its current position.
    /// @param stream The stream to read from.
    /// @param output StringBuilder to append the decoded text to.
    /// @param indentLevel Number of indentation levels (4 spaces each).
    SLANG_API static void decodeValueFromStream(
        ReplayStream& stream,
        Slang::StringBuilder& output,
        int indentLevel = 0);

    /// Decode a single value from raw bytes.
    /// @param data Pointer to the data.
    /// @param size Size of the data in bytes.
    /// @return Human-readable text representation.
    SLANG_API static Slang::String decodeValueFromBytes(const void* data, size_t size);

    /// Skip a single value in the stream (advance position past it).
    /// @param stream The stream to skip in.
    SLANG_API static void skipValueInStream(ReplayStream& stream);

    /// Decode a call header (signature + this handle) from stream.
    /// @param stream The stream to read from.
    /// @param output StringBuilder to append the decoded text to.
    /// @return true if a call header was decoded, false if end of stream.
    SLANG_API static bool decodeCallHeader(
        ReplayStream& stream,
        Slang::StringBuilder& output);

    /// Get human-readable name for a TypeId.
    SLANG_API static const char* getTypeIdName(TypeId type);

private:
    ReplayStreamDecoder(ReplayStream& stream, Slang::StringBuilder& output);

    void decodeAll(size_t maxBytes);
    void decodeCall();
    
    /// Try to recover from a decoding error by finding the next valid call.
    /// @return true if recovery was successful, false if end of stream.
    bool tryRecoverToNextCall();

    static TypeId peekTypeId(ReplayStream& stream);
    static TypeId readTypeId(ReplayStream& stream);
    
    static void indent(Slang::StringBuilder& output, int level);
    static void appendHexDump(
        Slang::StringBuilder& output,
        const void* data,
        size_t size,
        size_t maxBytes = 64);

    ReplayStream& m_stream;
    Slang::StringBuilder& m_output;
    size_t m_startPosition;
};

} // namespace SlangRecord
