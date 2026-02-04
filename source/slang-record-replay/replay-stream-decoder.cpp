#include "replay-stream-decoder.h"

#include <cstdio>

namespace SlangRecord {

using Slang::String;
using Slang::StringBuilder;

// =============================================================================
// Public Static API - Full Stream Decoding
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
// Public Static API - Individual Value Decoding (for live logging)
// =============================================================================

void ReplayStreamDecoder::decodeValueFromStream(
    ReplayStream& stream,
    StringBuilder& output,
    int indentLevel)
{
    TypeId type = readTypeId(stream);
    
    switch (type)
    {
    case TypeId::Int8:
        {
            int8_t v;
            stream.read(&v, sizeof(v));
            output << "Int8: " << (int)v;
        }
        break;

    case TypeId::Int16:
        {
            int16_t v;
            stream.read(&v, sizeof(v));
            output << "Int16: " << v;
        }
        break;

    case TypeId::Int32:
        {
            int32_t v;
            stream.read(&v, sizeof(v));
            output << "Int32: " << v;
        }
        break;

    case TypeId::Int64:
        {
            int64_t v;
            stream.read(&v, sizeof(v));
            output << "Int64: " << v;
        }
        break;

    case TypeId::UInt8:
        {
            uint8_t v;
            stream.read(&v, sizeof(v));
            output << "UInt8: " << (unsigned)v;
        }
        break;

    case TypeId::UInt16:
        {
            uint16_t v;
            stream.read(&v, sizeof(v));
            output << "UInt16: " << v;
        }
        break;

    case TypeId::UInt32:
        {
            uint32_t v;
            stream.read(&v, sizeof(v));
            output << "UInt32: " << v;
        }
        break;

    case TypeId::UInt64:
        {
            uint64_t v;
            stream.read(&v, sizeof(v));
            output << "UInt64: " << v;
        }
        break;

    case TypeId::Float32:
        {
            float v;
            stream.read(&v, sizeof(v));
            output << "Float32: " << v;
        }
        break;

    case TypeId::Float64:
        {
            double v;
            stream.read(&v, sizeof(v));
            output << "Float64: " << v;
        }
        break;

    case TypeId::Bool:
        {
            uint8_t v;
            stream.read(&v, sizeof(v));
            output << "Bool: " << (v ? "true" : "false");
        }
        break;

    case TypeId::String:
        {
            uint32_t len;
            stream.read(&len, sizeof(len));
            
            if (len == 0)
            {
                output << "String: \"\"";
            }
            else if (len < 256)
            {
                char buffer[256];
                stream.read(buffer, len);
                buffer[len] = '\0';
                output << "String: \"" << buffer << "\"";
            }
            else
            {
                // Long string - show truncated
                char buffer[128];
                stream.read(buffer, 127);
                buffer[127] = '\0';
                stream.skip(len - 127);
                output << "String(" << len << "): \"" << buffer << "...\"";
            }
        }
        break;

    case TypeId::Blob:
        {
            uint64_t size;
            stream.read(&size, sizeof(size));
            output << "Blob(" << size << " bytes)";
            
            if (size > 0)
            {
                // Show hex dump of first few bytes
                size_t showBytes = (size < 32) ? (size_t)size : 32;
                uint8_t buffer[32];
                stream.read(buffer, showBytes);
                output << ": ";
                appendHexDump(output, buffer, showBytes, 32);
                
                // Skip remaining bytes
                if (size > showBytes)
                    stream.skip((size_t)size - showBytes);
            }
        }
        break;

    case TypeId::ObjectHandle:
        {
            uint64_t handle;
            stream.read(&handle, sizeof(handle));
            if (handle == kNullHandle)
                output << "Handle: null";
            else if (handle == kInlineBlobHandle)
                output << "Handle: inline-blob";
            else
                output << "Handle: #" << handle;
        }
        break;

    case TypeId::Null:
        output << "Null";
        break;

    case TypeId::Array:
        {
            uint32_t count;
            stream.read(&count, sizeof(count));
            output << "Array[" << count << "]:";
            for (uint32_t i = 0; i < count && i < 100; i++)
            {
                output << "\n";
                indent(output, indentLevel + 1);
                output << "[" << i << "] ";
                decodeValueFromStream(stream, output, indentLevel + 1);
            }
            if (count > 100)
            {
                output << "\n";
                indent(output, indentLevel + 1);
                output << "... (" << (count - 100) << " more elements)";
                // Skip remaining elements
                for (uint32_t i = 100; i < count; i++)
                    skipValueInStream(stream);
            }
        }
        break;

    case TypeId::Error:
        {
            // Error marker - read the error message
            uint32_t len;
            stream.read(&len, sizeof(len));
            if (len > 0 && len < 4096)
            {
                Slang::List<char> buffer;
                buffer.setCount(len + 1);
                stream.read(buffer.getBuffer(), len);
                buffer[len] = '\0';
                output << "ERROR: \"" << buffer.getBuffer() << "\"";
            }
            else
            {
                output << "ERROR (invalid length: " << len << ")";
            }
        }
        break;

    default:
        {
            char hex[32];
            snprintf(hex, sizeof(hex), "Unknown type: 0x%02X", static_cast<uint8_t>(type));
            output << hex;
        }
        break;
    }
}

String ReplayStreamDecoder::decodeValueFromBytes(const void* data, size_t size)
{
    ReplayStream stream(data, size);
    StringBuilder output;
    decodeValueFromStream(stream, output, 0);
    return output.produceString();
}

void ReplayStreamDecoder::skipValueInStream(ReplayStream& stream)
{
    TypeId type = readTypeId(stream);
    
    switch (type)
    {
    case TypeId::Int8:
    case TypeId::UInt8:
    case TypeId::Bool:
        stream.skip(1);
        break;

    case TypeId::Int16:
    case TypeId::UInt16:
        stream.skip(2);
        break;

    case TypeId::Int32:
    case TypeId::UInt32:
    case TypeId::Float32:
        stream.skip(4);
        break;

    case TypeId::Int64:
    case TypeId::UInt64:
    case TypeId::Float64:
    case TypeId::ObjectHandle:
        stream.skip(8);
        break;

    case TypeId::String:
        {
            uint32_t len;
            stream.read(&len, sizeof(len));
            stream.skip(len);
        }
        break;

    case TypeId::Blob:
        {
            uint64_t size;
            stream.read(&size, sizeof(size));
            stream.skip((size_t)size);
        }
        break;

    case TypeId::Null:
        // Nothing to skip
        break;

    case TypeId::Array:
        {
            uint32_t count;
            stream.read(&count, sizeof(count));
            for (uint32_t i = 0; i < count; i++)
                skipValueInStream(stream);
        }
        break;

    case TypeId::Error:
        {
            uint32_t len;
            stream.read(&len, sizeof(len));
            stream.skip(len);
        }
        break;

    default:
        // Unknown type - can't skip safely
        throw Slang::Exception("Cannot skip unknown type");
    }
}

bool ReplayStreamDecoder::decodeCallHeader(
    ReplayStream& stream,
    StringBuilder& output)
{
    if (stream.getPosition() >= stream.getSize())
        return false;

    // Read function signature
    TypeId sigType = readTypeId(stream);
    if (sigType != TypeId::String)
    {
        output << "Unexpected type for signature: " << getTypeIdName(sigType);
        return false;
    }

    uint32_t sigLen;
    stream.read(&sigLen, sizeof(sigLen));
    
    char sigBuffer[512];
    size_t readLen = (sigLen < sizeof(sigBuffer) - 1) ? sigLen : sizeof(sigBuffer) - 1;
    if (readLen > 0)
        stream.read(sigBuffer, readLen);
    sigBuffer[readLen] = '\0';
    
    // Skip remaining signature bytes if truncated
    if (sigLen > readLen)
        stream.skip(sigLen - readLen);

    output << "Function: " << sigBuffer;

    // Read 'this' pointer handle
    TypeId thisType = readTypeId(stream);
    if (thisType != TypeId::ObjectHandle)
    {
        output << " (unexpected 'this' type: " << getTypeIdName(thisType) << ")";
        return true;
    }

    uint64_t thisHandle;
    stream.read(&thisHandle, sizeof(thisHandle));
    if (thisHandle == kNullHandle)
        output << " [static]";
    else
        output << " [this=#" << thisHandle << "]";

    return true;
}

const char* ReplayStreamDecoder::getTypeIdName(TypeId type)
{
    switch (type)
    {
    case TypeId::Int8:          return "Int8";
    case TypeId::Int16:         return "Int16";
    case TypeId::Int32:         return "Int32";
    case TypeId::Int64:         return "Int64";
    case TypeId::UInt8:         return "UInt8";
    case TypeId::UInt16:        return "UInt16";
    case TypeId::UInt32:        return "UInt32";
    case TypeId::UInt64:        return "UInt64";
    case TypeId::Float32:       return "Float32";
    case TypeId::Float64:       return "Float64";
    case TypeId::Bool:          return "Bool";
    case TypeId::String:        return "String";
    case TypeId::Blob:          return "Blob";
    case TypeId::ObjectHandle:  return "ObjectHandle";
    case TypeId::Null:          return "Null";
    case TypeId::Array:         return "Array";
    case TypeId::Error:         return "Error";
    default:                    return "Unknown";
    }
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
// Private Instance Methods
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
    // Use the public static function to decode the header
    StringBuilder header;
    if (!decodeCallHeader(m_stream, header))
    {
        m_output << "  " << header << "\n";
        return;
    }
    m_output << "  " << header << "\n";

    // Read remaining arguments until we hit another String (next call) or end
    m_output << "  Arguments:\n";
    int argNum = 0;
    while (m_stream.getPosition() < m_stream.getSize())
    {
        // Peek at next type - if it's a String, it might be the start of the next call
        TypeId nextType = peekTypeId(m_stream);
        if (nextType == TypeId::String)
        {
            // This is likely the next call's signature - stop here
            break;
        }

        m_output << "    [" << argNum++ << "] ";
        decodeValueFromStream(m_stream, m_output, 2);
        m_output << "\n";
    }
}

// =============================================================================
// Static Helpers
// =============================================================================

TypeId ReplayStreamDecoder::peekTypeId(ReplayStream& stream)
{
    size_t pos = stream.getPosition();
    TypeId type = readTypeId(stream);
    stream.seek(pos);
    return type;
}

TypeId ReplayStreamDecoder::readTypeId(ReplayStream& stream)
{
    uint8_t v;
    stream.read(&v, sizeof(v));
    return static_cast<TypeId>(v);
}

void ReplayStreamDecoder::indent(StringBuilder& output, int level)
{
    for (int i = 0; i < level; i++)
        output << "    ";
}

void ReplayStreamDecoder::appendHexDump(
    StringBuilder& output,
    const void* data,
    size_t size,
    size_t maxBytes)
{
    const uint8_t* bytes = static_cast<const uint8_t*>(data);
    size_t showBytes = (size < maxBytes) ? size : maxBytes;
    
    for (size_t i = 0; i < showBytes; i++)
    {
        char hex[4];
        snprintf(hex, sizeof(hex), "%02X ", bytes[i]);
        output << hex;
    }
    
    if (size > maxBytes)
        output << "...";
}

} // namespace SlangRecord
