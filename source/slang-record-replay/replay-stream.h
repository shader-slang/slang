#pragma once

#include "core/slang-exception.h"
#include "core/slang-io.h"
#include "core/slang-list.h"
#include "core/slang-stream.h"
#include "core/slang-string.h"

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

    ReplayStream(const ReplayStream&) = delete;
    ReplayStream& operator=(const ReplayStream&) = delete;

    // =========================================================================
    // Module-Boundary Operations
    // =========================================================================
    //
    // These methods are intentionally SLANG_API and defined out of line in
    // replay-stream.cpp. They copy, move, grow, clear, close, or release
    // heap-backed state in m_buffer or m_mirrorFile, so calls from another DLL
    // execute in the compiler module that owns the replay stream state.

    /// Create a reading stream from existing data.
    /// Makes a copy of the data.
    /// @param data Pointer to the data.
    /// @param size Size of the data in bytes.
    /// data must be non-null when size is nonzero.
    SLANG_API ReplayStream(const void* data, size_t size);

    /// Create a reading stream by loading entire file into memory.
    /// @param path Path to the file to load.
    /// @return The stream with the file contents; on an IO failure the returned stream is in the
    /// failed state (isFailed() is true, getErrorMessage() describes it) rather than throwing.
    SLANG_API static ReplayStream loadFromFile(const char* path);

    /// Move-construct from another stream.
    SLANG_API ReplayStream(ReplayStream&& other);

    /// Move-assign from another stream.
    SLANG_API ReplayStream& operator=(ReplayStream&& other);

    SLANG_API ~ReplayStream();

    /// Write data to the stream. On a wrong-mode or out-of-bounds write the stream is put into the
    /// failed state (see setError) and the write is dropped; check isFailed() at the operation
    /// boundary rather than after every write.
    SLANG_API void write(const void* data, size_t size);

    /// Read data from the stream into `data`. On a read past the end (or a wrong-mode read) the
    /// stream is put into the failed state and `data` is left untouched, so callers must
    /// value-initialize their destination before reading; a skipped read then leaves a defined
    /// value. Reads on an already-failed stream are no-ops. Check isFailed() at the boundary.
    SLANG_API void read(void* data, size_t size);

    /// Reset the stream to initial empty writing state (also clears any failed state).
    SLANG_API void reset();

    /// Set a mirror file for crash-safe capture.
    /// All subsequent writes will be immediately written to this file as well.
    /// @param path Path to the mirror file.
    /// @return SLANG_OK, or a failure code if the file cannot be opened. Does not affect the
    /// stream's read/write failed state (mirroring is an optional write-side feature).
    SLANG_API SlangResult setMirrorFile(const char* path);

    /// Save all data to a file.
    /// @param path Path to the file to write.
    /// @return SLANG_OK, or a failure code if the file cannot be opened or written.
    SLANG_API SlangResult saveToFile(const char* path) const;

    /// Close the mirror file (data remains in memory).
    SLANG_API void closeMirrorFile();

    /// Create a reading stream from this stream's data.
    /// Makes a copy of the current data.
    SLANG_API ReplayStream createReader() const;

    /// Clear the stream and reset to writing mode.
    SLANG_API void clear();

    // =========================================================================
    // Inline State Accessors
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

    /// Check if a mirror file is currently active.
    bool hasMirrorFile() const { return m_mirrorFile != nullptr; }

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

    /// Get a byte at a specific offset (for sync comparison). Returns 0 and puts the stream into
    /// the failed state if the offset is past the end; callers detect this via isFailed().
    uint8_t getByte(size_t offset) const
    {
        if (offset >= size_t(m_buffer.getCount()))
        {
            setError("Offset past end of stream");
            return 0;
        }
        return m_buffer[Slang::Index(offset)];
    }

    // =========================================================================
    // Failure state (exception-free error reporting)
    // =========================================================================
    //
    // Low-level reads/writes latch a sticky failure here instead of throwing, so the many small
    // deserialization reads stay branch-free at the call site and the operation boundary checks
    // once. First error wins so the root-cause message is preserved.

    /// Latch a failure with a diagnostic message. No-op if already failed (first error wins). const
    /// so the const read accessors (getByte) can report a past-end access.
    void setError(String message) const
    {
        if (!m_failed)
        {
            m_failed = true;
            m_errorMessage = message;
        }
    }

    /// True if a read/write has failed since the last clearError()/reset().
    bool isFailed() const { return m_failed; }

    /// The message from the first failure (empty if not failed).
    const String& getErrorMessage() const { return m_errorMessage; }

    /// Clear the failed state so decoding can resume (used at per-call recovery boundaries).
    void clearError()
    {
        m_failed = false;
        m_errorMessage = String();
    }

private:
    List<uint8_t> m_buffer;
    size_t m_position = 0;
    bool m_isReading = false;
    mutable RefPtr<FileStream> m_mirrorFile;
    mutable bool m_failed = false;
    mutable String m_errorMessage;
};

} // namespace SlangRecord
