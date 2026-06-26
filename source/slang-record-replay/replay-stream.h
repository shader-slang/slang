#pragma once

#include "../core/slang-exception.h"
#include "../core/slang-io.h"
#include "../core/slang-list.h"
#include "../core/slang-stream.h"
#include "../core/slang-string.h"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <slang.h>

namespace SlangRecord
{

using Slang::File;
using Slang::FileAccess;
using Slang::FileMode;
using Slang::FileShare;
using Slang::FileStream;
using Slang::List;
using Slang::RefPtr;
using Slang::SeekOrigin;
using Slang::String;

/// A simple memory-backed stream for replay data.
///
/// All replay data is stored in memory for fast access. Optionally, data can be
/// mirrored to disk as it's written (for crash safety during capture), or
/// loaded from disk into memory (for playback).
///
/// This replaces the previous IStream/MemoryStream/FileStream hierarchy with
/// a single, simpler class.
class ReplayStream
{
public:
    // =========================================================================
    // Construction
    // =========================================================================

    /// Create an empty stream for writing (capture mode).
    ReplayStream() = default;

    /// Create a reading stream from existing data.
    /// Makes a copy of the data.
    /// @param data Pointer to the data.
    /// @param size Size of the data in bytes.
    /// data must be non-null when size is nonzero.
    SLANG_API ReplayStream(const void* data, size_t size);

    /// Create a reading stream by loading entire file into memory.
    /// @param path Path to the file to load.
    /// @return The stream with file contents.
    /// @throws Slang::Exception if file cannot be opened or read.
    SLANG_API static ReplayStream loadFromFile(const char* path);

    // Move-only (no copying)
    SLANG_API ReplayStream(ReplayStream&& other);
    SLANG_API ReplayStream& operator=(ReplayStream&& other);
    ReplayStream(const ReplayStream&) = delete;
    ReplayStream& operator=(const ReplayStream&) = delete;

    SLANG_API ~ReplayStream();

    // =========================================================================
    // Reading/Writing
    // =========================================================================

    /// Write data to the stream.
    /// @param data Pointer to the data to write.
    /// @param size Number of bytes to write.
    /// @throws Slang::Exception if this is a reading stream.
    SLANG_API void write(const void* data, size_t size);

    /// Read data from the stream.
    /// @param data Buffer to read into.
    /// @param size Number of bytes to read.
    /// @throws Slang::Exception if this is a writing stream or read past end.
    SLANG_API void read(void* data, size_t size);

    // =========================================================================
    // Position/Size
    // =========================================================================

    /// Get the current position in the stream.
    size_t getPosition() const { return m_position; }

    /// Get the total size of the data in the stream.
    size_t getSize() const { return size_t(m_buffer.getCount()); }

    /// Seek to an absolute position.
    void seek(size_t position) { m_position = position; }

    /// Skip forward by the given number of bytes.
    void skip(size_t bytes) { m_position += bytes; }

    /// Returns true if this is a reading stream.
    bool isReading() const { return m_isReading; }

    /// Set the stream to reading or writing mode.
    /// This allows reusing a buffer for both writing and reading.
    void setReading(bool reading) { m_isReading = reading; }

    /// Returns true if the stream has reached the end.
    bool atEnd() const { return m_position >= size_t(m_buffer.getCount()); }

    /// Reset the stream to initial empty writing state.
    SLANG_API void reset();

    // =========================================================================
    // Direct Memory Access
    // =========================================================================

    /// Get a pointer to the underlying buffer data.
    /// This is always valid, even for writing streams.
    const uint8_t* getData() const { return m_buffer.getBuffer(); }

    /// Get the raw buffer (for testing/debugging).
    const List<uint8_t>& getBuffer() const { return m_buffer; }

    /// Compare bytes at a given range with another buffer.
    /// @param offset Offset into this stream's buffer.
    /// @param data Data to compare against.
    /// @param size Number of bytes to compare.
    /// @return true if the bytes match, false otherwise.
    bool compareBytes(size_t offset, const void* data, size_t size) const
    {
        if (offset + size > size_t(m_buffer.getCount()))
            return false;
        return std::memcmp(m_buffer.getBuffer() + offset, data, size) == 0;
    }

    /// Compare a range of this stream against another stream.
    /// @param offset Offset in both streams to start comparing.
    /// @param other The other stream to compare against.
    /// @param size Number of bytes to compare.
    /// @return true if the bytes match, false otherwise.
    bool compareBytes(size_t offset, const ReplayStream& other, size_t size) const
    {
        if (offset + size > size_t(m_buffer.getCount()))
            return false;
        if (offset + size > size_t(other.m_buffer.getCount()))
            return false;
        return std::memcmp(
                   m_buffer.getBuffer() + offset,
                   other.m_buffer.getBuffer() + offset,
                   size) == 0;
    }

    /// Get a byte at a specific offset (for sync comparison).
    /// @param offset Offset into the buffer.
    /// @return The byte at that offset.
    /// @throws Slang::Exception if offset is past end.
    uint8_t getByte(size_t offset) const
    {
        if (offset >= size_t(m_buffer.getCount()))
            throw Slang::Exception("Offset past end of stream");
        return m_buffer[Slang::Index(offset)];
    }

    // =========================================================================
    // File Operations
    // =========================================================================

    /// Set a mirror file for crash-safe capture.
    /// All subsequent writes will be immediately written to this file as well.
    /// @param path Path to the mirror file.
    /// @throws Slang::Exception if file cannot be opened.
    SLANG_API void setMirrorFile(const char* path);

    /// Save all data to a file.
    /// @param path Path to the file to write.
    /// @throws Slang::Exception if file cannot be opened or written.
    SLANG_API void saveToFile(const char* path) const;

    /// Check if a mirror file is currently active.
    bool hasMirrorFile() const { return m_mirrorFile != nullptr; }

    /// Close the mirror file (data remains in memory).
    SLANG_API void closeMirrorFile();

    // =========================================================================
    // Utility
    // =========================================================================

    /// Create a reading stream from this stream's data.
    /// Makes a copy of the current data.
    SLANG_API ReplayStream createReader() const;

    /// Clear the stream and reset to writing mode.
    SLANG_API void clear();

private:
    List<uint8_t> m_buffer;
    size_t m_position = 0;
    bool m_isReading = false;
    mutable RefPtr<FileStream> m_mirrorFile;
};

} // namespace SlangRecord
