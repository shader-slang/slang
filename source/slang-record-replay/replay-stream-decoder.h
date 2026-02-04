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

private:
    ReplayStreamDecoder(ReplayStream& stream, Slang::StringBuilder& output);

    void decodeAll(size_t maxBytes);
    void decodeCall();
    void decodeValue(int indent);
    void skipValue();

    TypeId peekTypeId();
    TypeId readTypeId();
    
    void indent(int level);
    void appendHexDump(const void* data, size_t size, size_t maxBytes = 64);

    ReplayStream& m_stream;
    Slang::StringBuilder& m_output;
    size_t m_startPosition;
};

} // namespace SlangRecord
