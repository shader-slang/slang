#pragma once

#include "../core/slang-exception.h"
#include "../core/slang-io.h"
#include "../core/slang-list.h"
#include "../core/slang-stream.h"
#include "../core/slang-string.h"

#include <cstddef>
#include <cstdint>
#include <cstring>

namespace SlangRecord {

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
    ReplayStream()
        : m_isReading(false)
        , m_position(0)
    {
    }

    /// Create a reading stream from existing data.
    /// Makes a copy of the data.
    /// @param data Pointer to the data.
    /// @param size Size of the data in bytes.
    ReplayStream(const void* data, size_t size)
        : m_isReading(true)
        , m_position(0)
    {
        if (data && size > 0)
        {
            m_buffer.setCount(size);
            std::memcpy(m_buffer.getBuffer(), data, size);
        }
    }

    /// Create a reading stream by loading entire file into memory.
    /// @param path Path to the file to load.
    /// @return The stream with file contents.
    /// @throws Slang::Exception if file cannot be opened or read.
    static ReplayStream loadFromFile(const char* path)
    {
        List<unsigned char> contents;
        SlangResult result = File::readAllBytes(String(path), contents);
        if (SLANG_FAILED(result))
            throw Slang::Exception(String("Failed to open file for reading: ") + path);

        ReplayStream stream;
        stream.m_isReading = true;
        stream.m_buffer = Slang::_Move(contents);

        return stream;
    }

    // Move-only (no copying)
    ReplayStream(ReplayStream&&) = default;
    ReplayStream& operator=(ReplayStream&&) = default;
    ReplayStream(const ReplayStream&) = delete;
    ReplayStream& operator=(const ReplayStream&) = delete;

    ~ReplayStream()
    {
        // Close mirror file if open
        closeMirrorFile();
    }

    // =========================================================================
    // Reading/Writing
    // =========================================================================

    /// Write data to the stream.
    /// @param data Pointer to the data to write.
    /// @param size Number of bytes to write.
    /// @throws Slang::Exception if this is a reading stream.
    void write(const void* data, size_t size)
    {
        if (m_isReading)
            throw Slang::Exception("Cannot write to a reading stream");

        size_t newSize = m_position + size;
        if (newSize > size_t(m_buffer.getCount()))
        {
            m_buffer.reserve(Slang::Index(newSize)*2);
            m_buffer.setCount(Slang::Index(newSize));
        }

        std::memcpy(m_buffer.getBuffer() + m_position, data, size);
        m_position += size;

        // Mirror to file if enabled
        if (m_mirrorFile)
        {
            m_mirrorFile->write(data, size);
            m_mirrorFile->flush(); // Ensure data is written immediately
        }
    }

    /// Read data from the stream.
    /// @param data Buffer to read into.
    /// @param size Number of bytes to read.
    /// @throws Slang::Exception if this is a writing stream or read past end.
    void read(void* data, size_t size)
    {
        if (!m_isReading)
            throw Slang::Exception("Cannot read from a writing stream");

        if (m_position + size > size_t(m_buffer.getCount()))
            throw Slang::Exception("Read past end of stream");

        std::memcpy(data, m_buffer.getBuffer() + m_position, size);
        m_position += size;
    }

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
    void reset()
    {
        m_buffer.clear();
        m_position = 0;
        m_isReading = false;
    }

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
        return std::memcmp(m_buffer.getBuffer() + offset, other.m_buffer.getBuffer() + offset, size) == 0;
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
    void setMirrorFile(const char* path)
    {
        closeMirrorFile();

        m_mirrorFile = new FileStream();
        SlangResult result = m_mirrorFile->init(
            String(path),
            FileMode::Create,
            FileAccess::Write,
            FileShare::ReadWrite);
        if (SLANG_FAILED(result))
        {
            m_mirrorFile = nullptr;
            throw Slang::Exception(String("Failed to open mirror file: ") + path);
        }

        // Write any existing data to the file
        if (m_buffer.getCount() > 0)
        {
            m_mirrorFile->write(m_buffer.getBuffer(), m_buffer.getCount());
            m_mirrorFile->flush();
        }
    }

    /// Save all data to a file.
    /// @param path Path to the file to write.
    /// @throws Slang::Exception if file cannot be opened or written.
    void saveToFile(const char* path) const
    {
        SlangResult result = File::writeAllBytes(
            String(path),
            m_buffer.getBuffer(),
            m_buffer.getCount());
        if (SLANG_FAILED(result))
            throw Slang::Exception(String("Failed to write to file: ") + path);
    }

    /// Check if a mirror file is currently active.
    bool hasMirrorFile() const { return m_mirrorFile != nullptr; }

    /// Close the mirror file (data remains in memory).
    void closeMirrorFile()
    {
        if (m_mirrorFile)
        {
            m_mirrorFile->close();
            m_mirrorFile = nullptr;
        }
    }

    // =========================================================================
    // Utility
    // =========================================================================

    /// Create a reading stream from this stream's data.
    /// Makes a copy of the current data.
    ReplayStream createReader() const { return ReplayStream(m_buffer.getBuffer(), m_buffer.getCount()); }

    /// Clear the stream and reset to writing mode.
    void clear()
    {
        m_buffer.clear();
        m_position = 0;
        m_isReading = false;
    }

private:
    List<uint8_t> m_buffer;
    size_t m_position = 0;
    bool m_isReading = false;
    mutable RefPtr<FileStream> m_mirrorFile;
};

} // namespace SlangRecord
