#include "replay-stream-decoder.h"

#include <cstdio>

namespace SlangRecord {

using Slang::String;
using Slang::StringBuilder;

// =============================================================================
// Public API
// =============================================================================

String ReplayStreamDecoder::decode(ReplayStream& stream, size_t maxBytes)
{
    StringBuilder output;
    ReplayStreamDecoder decoder(stream, output);
    decoder.decodeAll(maxBytes);
    return output.produceString();
}

String ReplayStreamDecoder::decodeFile(const char* filePath)
{
    ReplayStream stream = ReplayStream::loadFromFile(filePath);
    return decode(stream, 0);
}

String ReplayStreamDecoder::decodeBytes(const void* data, size_t size)
{
    ReplayStream stream(data, size);
    return decode(stream, 0);
}

// =============================================================================
// Constructor
// =============================================================================

ReplayStreamDecoder::ReplayStreamDecoder(ReplayStream& stream, StringBuilder& output)
    : m_stream(stream)
    , m_output(output)
    , m_startPosition(stream.getPosition())
{
}

// =============================================================================
// Decoding
// =============================================================================

void ReplayStreamDecoder::decodeAll(size_t maxBytes)
{
    size_t endPosition = (maxBytes > 0) 
        ? m_startPosition + maxBytes 
        : m_stream.getSize();

    m_output << "=== Replay Stream Dump ===\n";
    m_output << "Total size: " << m_stream.getSize() << " bytes\n";
    m_output << "Start position: " << m_startPosition << "\n\n";

    int callNumber = 0;
    while (m_stream.getPosition() < endPosition && m_stream.getPosition() < m_stream.getSize())
    {
        size_t callStart = m_stream.getPosition();
        m_output << "--- Call #" << callNumber++ << " (offset " << callStart << ") ---\n";
        
        try
        {
            decodeCall();
        }
        catch (const Slang::Exception& e)
        {
            m_output << "  ERROR: " << e.Message.getBuffer() << "\n";
            m_output << "  (stopped decoding at offset " << m_stream.getPosition() << ")\n";
            break;
        }
        m_output << "\n";
    }

    m_output << "=== End of Stream ===\n";
}

void ReplayStreamDecoder::decodeCall()
{
    // Each call starts with:
    // 1. Function signature (String)
    // 2. 'this' pointer handle (ObjectHandle)
    // 3. Arguments (various types until next call or end)

    // Read function signature
    TypeId sigType = readTypeId();
    if (sigType != TypeId::String)
    {
        m_output << "  Unexpected type for signature: " << getTypeIdName(sigType) << "\n";
        return;
    }

    uint32_t sigLen;
    m_stream.read(&sigLen, sizeof(sigLen));
    
    char sigBuffer[512];
    size_t readLen = (sigLen < sizeof(sigBuffer) - 1) ? sigLen : sizeof(sigBuffer) - 1;
    if (readLen > 0)
        m_stream.read(sigBuffer, readLen);
    sigBuffer[readLen] = '\0';
    
    // Skip remaining signature bytes if truncated
    if (sigLen > readLen)
        m_stream.skip(sigLen - readLen);

    m_output << "  Function: " << sigBuffer << "\n";

    // Read 'this' pointer handle
    TypeId thisType = readTypeId();
    if (thisType != TypeId::ObjectHandle)
    {
        m_output << "  Unexpected type for 'this': " << getTypeIdName(thisType) << "\n";
        return;
    }

    uint64_t thisHandle;
    m_stream.read(&thisHandle, sizeof(thisHandle));
    if (thisHandle == kNullHandle)
        m_output << "  this: null (static function)\n";
    else
        m_output << "  this: handle #" << thisHandle << "\n";

    // Read remaining arguments until we hit another String (next call) or end
    m_output << "  Arguments:\n";
    int argNum = 0;
    while (m_stream.getPosition() < m_stream.getSize())
    {
        // Peek at next type - if it's a String, it might be the start of the next call
        TypeId nextType = peekTypeId();
        if (nextType == TypeId::String)
        {
            // This is likely the next call's signature - stop here
            break;
        }

        m_output << "    [" << argNum++ << "] ";
        decodeValue(3);
    }
}

void ReplayStreamDecoder::decodeValue(int indentLevel)
{
    TypeId type = readTypeId();
    
    switch (type)
    {
    case TypeId::Int8:
        {
            int8_t v;
            m_stream.read(&v, sizeof(v));
            m_output << "Int8: " << (int)v << "\n";
        }
        break;

    case TypeId::Int16:
        {
            int16_t v;
            m_stream.read(&v, sizeof(v));
            m_output << "Int16: " << v << "\n";
        }
        break;

    case TypeId::Int32:
        {
            int32_t v;
            m_stream.read(&v, sizeof(v));
            m_output << "Int32: " << v << "\n";
        }
        break;

    case TypeId::Int64:
        {
            int64_t v;
            m_stream.read(&v, sizeof(v));
            m_output << "Int64: " << v << "\n";
        }
        break;

    case TypeId::UInt8:
        {
            uint8_t v;
            m_stream.read(&v, sizeof(v));
            m_output << "UInt8: " << (unsigned)v << "\n";
        }
        break;

    case TypeId::UInt16:
        {
            uint16_t v;
            m_stream.read(&v, sizeof(v));
            m_output << "UInt16: " << v << "\n";
        }
        break;

    case TypeId::UInt32:
        {
            uint32_t v;
            m_stream.read(&v, sizeof(v));
            m_output << "UInt32: " << v << "\n";
        }
        break;

    case TypeId::UInt64:
        {
            uint64_t v;
            m_stream.read(&v, sizeof(v));
            m_output << "UInt64: " << v << "\n";
        }
        break;

    case TypeId::Float32:
        {
            float v;
            m_stream.read(&v, sizeof(v));
            m_output << "Float32: " << v << "\n";
        }
        break;

    case TypeId::Float64:
        {
            double v;
            m_stream.read(&v, sizeof(v));
            m_output << "Float64: " << v << "\n";
        }
        break;

    case TypeId::Bool:
        {
            uint8_t v;
            m_stream.read(&v, sizeof(v));
            m_output << "Bool: " << (v ? "true" : "false") << "\n";
        }
        break;

    case TypeId::String:
        {
            uint32_t len;
            m_stream.read(&len, sizeof(len));
            
            if (len == 0)
            {
                m_output << "String: \"\"\n";
            }
            else if (len < 256)
            {
                char buffer[256];
                m_stream.read(buffer, len);
                buffer[len] = '\0';
                m_output << "String: \"" << buffer << "\"\n";
            }
            else
            {
                // Long string - show truncated
                char buffer[128];
                m_stream.read(buffer, 127);
                buffer[127] = '\0';
                m_stream.skip(len - 127);
                m_output << "String(" << len << "): \"" << buffer << "...\" (truncated)\n";
            }
        }
        break;

    case TypeId::Blob:
        {
            uint64_t size;
            m_stream.read(&size, sizeof(size));
            m_output << "Blob(" << size << " bytes)";
            
            if (size > 0)
            {
                // Show hex dump of first few bytes
                size_t showBytes = (size < 32) ? (size_t)size : 32;
                uint8_t buffer[32];
                m_stream.read(buffer, showBytes);
                m_output << ": ";
                appendHexDump(buffer, showBytes, 32);
                
                // Skip remaining bytes
                if (size > showBytes)
                    m_stream.skip((size_t)size - showBytes);
            }
            m_output << "\n";
        }
        break;

    case TypeId::ObjectHandle:
        {
            uint64_t handle;
            m_stream.read(&handle, sizeof(handle));
            if (handle == kNullHandle)
                m_output << "Handle: null\n";
            else if (handle == kInlineBlobHandle)
                m_output << "Handle: inline-blob\n";
            else
                m_output << "Handle: #" << handle << "\n";
        }
        break;

    case TypeId::Null:
        m_output << "Null\n";
        break;

    case TypeId::Array:
        {
            uint32_t count;
            m_stream.read(&count, sizeof(count));
            m_output << "Array[" << count << "]:\n";
            for (uint32_t i = 0; i < count && i < 100; i++)  // Limit to 100 elements
            {
                indent(indentLevel + 1);
                m_output << "[" << i << "] ";
                decodeValue(indentLevel + 1);
            }
            if (count > 100)
            {
                indent(indentLevel + 1);
                m_output << "... (" << (count - 100) << " more elements)\n";
                // Skip remaining elements
                for (uint32_t i = 100; i < count; i++)
                    skipValue();
            }
        }
        break;

    default:
        {
            char hex[16];
            snprintf(hex, sizeof(hex), "Unknown type: 0x%02X", static_cast<uint8_t>(type));
            m_output << hex << "\n";
        }
        break;
    }
}

void ReplayStreamDecoder::skipValue()
{
    TypeId type = readTypeId();
    
    switch (type)
    {
    case TypeId::Int8:
    case TypeId::UInt8:
    case TypeId::Bool:
        m_stream.skip(1);
        break;

    case TypeId::Int16:
    case TypeId::UInt16:
        m_stream.skip(2);
        break;

    case TypeId::Int32:
    case TypeId::UInt32:
    case TypeId::Float32:
        m_stream.skip(4);
        break;

    case TypeId::Int64:
    case TypeId::UInt64:
    case TypeId::Float64:
    case TypeId::ObjectHandle:
        m_stream.skip(8);
        break;

    case TypeId::String:
        {
            uint32_t len;
            m_stream.read(&len, sizeof(len));
            m_stream.skip(len);
        }
        break;

    case TypeId::Blob:
        {
            uint64_t size;
            m_stream.read(&size, sizeof(size));
            m_stream.skip((size_t)size);
        }
        break;

    case TypeId::Null:
        // Nothing to skip
        break;

    case TypeId::Array:
        {
            uint32_t count;
            m_stream.read(&count, sizeof(count));
            for (uint32_t i = 0; i < count; i++)
                skipValue();
        }
        break;

    default:
        // Unknown type - can't skip safely
        throw Slang::Exception("Cannot skip unknown type");
    }
}

TypeId ReplayStreamDecoder::peekTypeId()
{
    size_t pos = m_stream.getPosition();
    TypeId type = readTypeId();
    m_stream.seek(pos);
    return type;
}

TypeId ReplayStreamDecoder::readTypeId()
{
    uint8_t v;
    m_stream.read(&v, sizeof(v));
    return static_cast<TypeId>(v);
}

void ReplayStreamDecoder::indent(int level)
{
    for (int i = 0; i < level; i++)
        m_output << "    ";
}

void ReplayStreamDecoder::appendHexDump(const void* data, size_t size, size_t maxBytes)
{
    const uint8_t* bytes = static_cast<const uint8_t*>(data);
    size_t showBytes = (size < maxBytes) ? size : maxBytes;
    
    for (size_t i = 0; i < showBytes; i++)
    {
        char hex[4];
        snprintf(hex, sizeof(hex), "%02X ", bytes[i]);
        m_output << hex;
    }
    
    if (size > maxBytes)
        m_output << "...";
}

} // namespace SlangRecord
