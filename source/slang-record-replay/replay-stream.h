#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace SlangRecord {

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
            m_buffer.resize(size);
            std::memcpy(m_buffer.data(), data, size);
        }
    }

    /// Create a reading stream by loading entire file into memory.
    /// @param path Path to the file to load.
    /// @return The stream with file contents.
    /// @throws std::runtime_error if file cannot be opened or read.
    static ReplayStream loadFromFile(const char* path)
    {
        std::ifstream file(path, std::ios::binary | std::ios::ate);
        if (!file.is_open())
            throw std::runtime_error(std::string("Failed to open file for reading: ") + path);

        size_t size = static_cast<size_t>(file.tellg());
        file.seekg(0, std::ios::beg);

        ReplayStream stream;
        stream.m_isReading = true;
        stream.m_buffer.resize(size);

        if (size > 0)
        {
            file.read(reinterpret_cast<char*>(stream.m_buffer.data()), size);
            if (!file)
                throw std::runtime_error(std::string("Failed to read file: ") + path);
        }

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
        if (m_mirrorFile.is_open())
            m_mirrorFile.close();
    }

    // =========================================================================
    // Reading/Writing
    // =========================================================================

    /// Write data to the stream.
    /// @param data Pointer to the data to write.
    /// @param size Number of bytes to write.
    /// @throws std::runtime_error if this is a reading stream.
    void write(const void* data, size_t size)
    {
        if (m_isReading)
            throw std::runtime_error("Cannot write to a reading stream");

        size_t newSize = m_position + size;
        if (newSize > m_buffer.size())
            m_buffer.resize(newSize);

        std::memcpy(m_buffer.data() + m_position, data, size);
        m_position += size;

        // Mirror to file if enabled
        if (m_mirrorFile.is_open())
        {
            m_mirrorFile.write(static_cast<const char*>(data), size);
            m_mirrorFile.flush(); // Ensure data is written immediately
        }
    }

    /// Read data from the stream.
    /// @param data Buffer to read into.
    /// @param size Number of bytes to read.
    /// @throws std::runtime_error if this is a writing stream or read past end.
    void read(void* data, size_t size)
    {
        if (!m_isReading)
            throw std::runtime_error("Cannot read from a writing stream");

        if (m_position + size > m_buffer.size())
            throw std::runtime_error("Read past end of stream");

        std::memcpy(data, m_buffer.data() + m_position, size);
        m_position += size;
    }

    // =========================================================================
    // Position/Size
    // =========================================================================

    /// Get the current position in the stream.
    size_t getPosition() const { return m_position; }

    /// Get the total size of the data in the stream.
    size_t getSize() const { return m_buffer.size(); }

    /// Seek to an absolute position.
    void seek(size_t position) { m_position = position; }

    /// Skip forward by the given number of bytes.
    void skip(size_t bytes) { m_position += bytes; }

    /// Returns true if this is a reading stream.
    bool isReading() const { return m_isReading; }

    /// Returns true if the stream has reached the end.
    bool atEnd() const { return m_position >= m_buffer.size(); }

    // =========================================================================
    // Direct Memory Access
    // =========================================================================

    /// Get a pointer to the underlying buffer data.
    /// This is always valid, even for writing streams.
    const uint8_t* getData() const { return m_buffer.data(); }

    /// Get the raw buffer (for testing/debugging).
    const std::vector<uint8_t>& getBuffer() const { return m_buffer; }

    /// Compare bytes at a given range with another buffer.
    /// @param offset Offset into this stream's buffer.
    /// @param data Data to compare against.
    /// @param size Number of bytes to compare.
    /// @return true if the bytes match, false otherwise.
    bool compareBytes(size_t offset, const void* data, size_t size) const
    {
        if (offset + size > m_buffer.size())
            return false;
        return std::memcmp(m_buffer.data() + offset, data, size) == 0;
    }

    /// Compare a range of this stream against another stream.
    /// @param offset Offset in both streams to start comparing.
    /// @param other The other stream to compare against.
    /// @param size Number of bytes to compare.
    /// @return true if the bytes match, false otherwise.
    bool compareBytes(size_t offset, const ReplayStream& other, size_t size) const
    {
        if (offset + size > m_buffer.size())
            return false;
        if (offset + size > other.m_buffer.size())
            return false;
        return std::memcmp(m_buffer.data() + offset, other.m_buffer.data() + offset, size) == 0;
    }

    /// Get a byte at a specific offset (for sync comparison).
    /// @param offset Offset into the buffer.
    /// @return The byte at that offset.
    /// @throws std::out_of_range if offset is past end.
    uint8_t getByte(size_t offset) const
    {
        if (offset >= m_buffer.size())
            throw std::out_of_range("Offset past end of stream");
        return m_buffer[offset];
    }

    // =========================================================================
    // File Operations
    // =========================================================================

    /// Set a mirror file for crash-safe capture.
    /// All subsequent writes will be immediately written to this file as well.
    /// @param path Path to the mirror file.
    /// @throws std::runtime_error if file cannot be opened.
    void setMirrorFile(const char* path)
    {
        if (m_mirrorFile.is_open())
            m_mirrorFile.close();

        m_mirrorFile.open(path, std::ios::binary | std::ios::out | std::ios::trunc);
        if (!m_mirrorFile.is_open())
            throw std::runtime_error(std::string("Failed to open mirror file: ") + path);

        // Write any existing data to the file
        if (!m_buffer.empty())
        {
            m_mirrorFile.write(reinterpret_cast<const char*>(m_buffer.data()), m_buffer.size());
            m_mirrorFile.flush();
        }
    }

    /// Save all data to a file.
    /// @param path Path to the file to write.
    /// @throws std::runtime_error if file cannot be opened or written.
    void saveToFile(const char* path) const
    {
        std::ofstream file(path, std::ios::binary | std::ios::out | std::ios::trunc);
        if (!file.is_open())
            throw std::runtime_error(std::string("Failed to open file for writing: ") + path);

        if (!m_buffer.empty())
        {
            file.write(reinterpret_cast<const char*>(m_buffer.data()), m_buffer.size());
            if (!file)
                throw std::runtime_error(std::string("Failed to write to file: ") + path);
        }
    }

    /// Check if a mirror file is currently active.
    bool hasMirrorFile() const { return m_mirrorFile.is_open(); }

    /// Close the mirror file (data remains in memory).
    void closeMirrorFile()
    {
        if (m_mirrorFile.is_open())
            m_mirrorFile.close();
    }

    // =========================================================================
    // Utility
    // =========================================================================

    /// Create a reading stream from this stream's data.
    /// Makes a copy of the current data.
    ReplayStream createReader() const { return ReplayStream(m_buffer.data(), m_buffer.size()); }

    /// Clear the stream and reset to writing mode.
    void clear()
    {
        m_buffer.clear();
        m_position = 0;
        m_isReading = false;
    }

private:
    std::vector<uint8_t> m_buffer;
    size_t m_position = 0;
    bool m_isReading = false;
    mutable std::ofstream m_mirrorFile;
};

} // namespace SlangRecord
