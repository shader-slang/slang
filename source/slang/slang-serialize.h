// slang-serialize.h
#ifndef SLANG_SERIALIZE_H
#define SLANG_SERIALIZE_H

// #include <type_traits>

#include "../compiler-core/slang-name.h"
#include "../core/slang-byte-encode-util.h"
#include "../core/slang-riff.h"
#include "../core/slang-stream.h"
#include "slang-serialize-types.h"

namespace Slang
{

class Linkage;

/*
A discussion of the serialization system design can be found in

docs/design/serialization.md
*/

// Predeclare
typedef uint32_t SerialSourceLoc;
class NodeBase;
class Val;
struct ValNodeDesc;

struct Encoder
{
public:
    Encoder() {}

    Encoder(RIFF::Builder& riff)
        : _cursor(riff)
    {
    }

    Encoder(RIFF::ListChunkBuilder* chunk)
        : _cursor(chunk)
    {
    }

    void beginArray(FourCC typeCode) { _cursor.beginListChunk(typeCode); }

    void beginArray() { beginArray(SerialBinary::kArrayFourCC); }

    void endArray() { _cursor.endChunk(); }

    void beginObject(FourCC typeCode) { _cursor.beginListChunk(typeCode); }

    void beginObject() { beginObject(SerialBinary::kObjectFourCC); }

    void endObject() { _cursor.endChunk(); }

    void beginKeyValuePair(FourCC keyCode) { _cursor.beginListChunk(keyCode); }

    void beginKeyValuePair() { beginKeyValuePair(SerialBinary::kPairFourCC); }

    void endKeyValuePair() { _cursor.endChunk(); }

    void encodeData(FourCC typeCode, void const* data, size_t size)
    {
        _cursor.addDataChunk(typeCode, data, size);
    }

    void encodeData(void const* data, size_t size)
    {
        encodeData(SerialBinary::kDataFourCC, data, size);
    }

    void encode(std::nullptr_t) { encodeData(SerialBinary::kNullFourCC, nullptr, 0); }

    void encodeBool(bool value)
    {
        encodeData(value ? SerialBinary::kTrueFourCC : SerialBinary::kFalseFourCC, nullptr, 0);
    }

    void encodeInt(Int64 value)
    {
        if (Int32(value) == value)
        {
            auto v = Int32(value);
            encodeData(SerialBinary::kInt32FourCC, &v, sizeof(v));
        }
        else
        {
            encodeData(SerialBinary::kInt64FourCC, &value, sizeof(value));
        }
    }


    void encodeUInt(UInt64 value)
    {
        if (UInt32(value) == value)
        {
            auto v = UInt32(value);
            encodeData(SerialBinary::kUInt32FourCC, &v, sizeof(v));
        }
        else
        {
            encodeData(SerialBinary::kUInt64FourCC, &value, sizeof(value));
        }
    }

    void encode(Int32 value) { encodeInt(value); }
    void encode(Int64 value) { encodeInt(value); }

    void encode(UInt32 value) { encodeUInt(value); }
    void encode(UInt64 value) { encodeUInt(value); }

    void encode(float value) { encodeData(SerialBinary::kFloat32FourCC, &value, sizeof(value)); }

    void encode(double value) { encodeData(SerialBinary::kFloat64FourCC, &value, sizeof(value)); }

    void encodeString(String const& value)
    {
        Int size = value.getLength();
        encodeData(SerialBinary::kStringFourCC, value.getBuffer(), size);
    }


    void encode(String const& value) { encodeString(value); }

    struct WithArray
    {
    public:
        WithArray(Encoder* encoder)
            : _encoder(encoder)
        {
            encoder->beginArray();
        }

        WithArray(Encoder* encoder, FourCC typeCode)
            : _encoder(encoder)
        {
            encoder->beginArray(typeCode);
        }

        ~WithArray() { _encoder->endArray(); }

    private:
        Encoder* _encoder;
    };

    struct WithObject
    {
    public:
        WithObject(Encoder* encoder)
            : _encoder(encoder)
        {
            encoder->beginObject();
        }

        WithObject(Encoder* encoder, FourCC typeCode)
            : _encoder(encoder)
        {
            encoder->beginObject(typeCode);
        }

        ~WithObject() { _encoder->endObject(); }

    private:
        Encoder* _encoder;
    };

    struct WithKeyValuePair
    {
    public:
        WithKeyValuePair(Encoder* encoder)
            : _encoder(encoder)
        {
            encoder->beginKeyValuePair();
        }

        WithKeyValuePair(Encoder* encoder, FourCC typeCode)
            : _encoder(encoder)
        {
            encoder->beginKeyValuePair(typeCode);
        }

        ~WithKeyValuePair() { _encoder->endKeyValuePair(); }

    private:
        Encoder* _encoder;
    };

private:
    RIFF::BuildCursor _cursor;

public:
    operator RIFF::BuildCursor&() { return _cursor; }

    RIFF::ChunkBuilder* getRIFFChunk() { return _cursor.getCurrentChunk(); }

    void setRIFFChunk(RIFF::ChunkBuilder* chunk) { _cursor.setCurrentChunk(chunk); }
};

struct Decoder
{
public:
    Decoder(RIFF::Chunk const* chunk)
        : _cursor(chunk)
    {
    }

    bool decodeBool()
    {
        switch (getTag())
        {
        case SerialBinary::kTrueFourCC:
            _advanceCursor();
            return true;
        case SerialBinary::kFalseFourCC:
            _advanceCursor();
            return false;

        default:
            SLANG_UNEXPECTED("invalid format in RIFF");
            UNREACHABLE_RETURN(false);
        }
    }

    String decodeString()
    {
        if (getTag() != SerialBinary::kStringFourCC)
        {
            SLANG_UNEXPECTED("invalid format in RIFF");
            UNREACHABLE_RETURN("");
        }

        auto dataChunk = as<RIFF::DataChunk>(_cursor);
        if (!dataChunk)
        {
            SLANG_UNEXPECTED("invalid format in RIFF");
            UNREACHABLE_RETURN("");
        }

        auto size = dataChunk->getPayloadSize();

        String value;
        value.appendRepeatedChar(' ', size);
        dataChunk->writePayloadInto((char*)value.getBuffer(), size);

        _advanceCursor();
        return value;
    }

    void decodeData(FourCC typeTag, void* outData, size_t dataSize)
    {
        if (getTag() == typeTag)
        {
            auto dataChunk = as<RIFF::DataChunk>(_cursor);
            if (dataChunk)
            {
                auto payloadSize = dataChunk->getPayloadSize();
                if (payloadSize >= dataSize)
                {
                    dataChunk->writePayloadInto(outData, dataSize);
                    _advanceCursor();
                    return;
                }
            }
        }

        SLANG_UNEXPECTED("invalid format in RIFF");
    }

    template<typename T>
    T _decodeSimpleValue(FourCC typeTag)
    {
        T value;
        decodeData(typeTag, &value, sizeof(value));
        return value;
    }

    Int64 decodeInt()
    {
        switch (getTag())
        {
        case SerialBinary::kInt64FourCC:
            return _decodeSimpleValue<Int64>(getTag());
        case SerialBinary::kInt32FourCC:
            return _decodeSimpleValue<Int32>(getTag());

        case SerialBinary::kUInt32FourCC:
            return _decodeSimpleValue<UInt32>(getTag());

        case SerialBinary::kUInt64FourCC:
            {
                auto uintValue = _decodeSimpleValue<UInt64>(getTag());
                if (Int64(uintValue) < 0)
                {
                    SLANG_UNEXPECTED("signed/unsigned mismatch in RIFF");
                }
                return Int64(uintValue);
            }

        default:
            SLANG_UNEXPECTED("invalid format in RIFF");
            UNREACHABLE_RETURN(0);
        }
    }

    UInt64 decodeUInt()
    {
        switch (getTag())
        {
        case SerialBinary::kUInt64FourCC:
            return _decodeSimpleValue<UInt64>(getTag());
        case SerialBinary::kUInt32FourCC:
            return _decodeSimpleValue<UInt32>(getTag());

        case SerialBinary::kInt32FourCC:
        case SerialBinary::kInt64FourCC:
            {
                auto intValue = decodeInt();
                if (intValue < 0)
                {
                    SLANG_UNEXPECTED("signed/unsigned mismatch in RIFF");
                }
                return UInt64(intValue);
            }

        default:
            SLANG_UNEXPECTED("invalid format in RIFF");
            UNREACHABLE_RETURN(0);
        }
    }

    double decodeFloat()
    {
        switch (getTag())
        {
        case SerialBinary::kFloat32FourCC:
            return _decodeSimpleValue<float>(getTag());
        case SerialBinary::kFloat64FourCC:
            return _decodeSimpleValue<double>(getTag());

        default:
            SLANG_UNEXPECTED("invalid format in RIFF");
            UNREACHABLE_RETURN(0);
        }
    }

    Int32 decodeInt32() { return Int32(decodeInt()); }
    Int64 decodeInt64() { return decodeInt(); }

    UInt32 decodeUInt32() { return UInt32(decodeUInt()); }
    UInt64 decodeUInt64() { return decodeUInt(); }

    float decodeFloat32() { return float(decodeFloat()); }
    double decodeFloat64() { return decodeFloat(); }

    FourCC getTag() { return _cursor ? _cursor->getType() : FourCC(0); }

    Int32 _decodeImpl(Int32*) { return decodeInt32(); }
    UInt32 _decodeImpl(UInt32*) { return decodeUInt32(); }

    Int64 _decodeImpl(Int64*) { return decodeInt64(); }
    UInt64 _decodeImpl(UInt64*) { return decodeUInt64(); }

    float _decodeImpl(float*) { return decodeFloat32(); }
    double _decodeImpl(double*) { return decodeFloat64(); }

    template<typename T>
    T decode()
    {
        return _decodeImpl((T*)nullptr);
    }

    template<typename T>
    void decode(T& outValue)
    {
        outValue = _decodeImpl((T*)nullptr);
    }

    void beginArray(FourCC typeCode = SerialBinary::kArrayFourCC)
    {
        auto listChunk = as<RIFF::ListChunk>(_cursor);
        if (!listChunk)
        {
            SLANG_UNEXPECTED("invalid format in RIFF");
        }

        if (listChunk->getType() != typeCode)
        {
            SLANG_UNEXPECTED("invalid format in RIFF");
        }

        _cursor = listChunk->getFirstChild();
    }

    void beginObject(FourCC typeCode = SerialBinary::kObjectFourCC)
    {
        auto listChunk = as<RIFF::ListChunk>(_cursor);
        if (!listChunk)
        {
            SLANG_UNEXPECTED("invalid format in RIFF");
        }

        if (listChunk->getType() != typeCode)
        {
            SLANG_UNEXPECTED("invalid format in RIFF");
        }

        _cursor = listChunk->getFirstChild();
    }

    void beginKeyValuePair(FourCC typeCode = SerialBinary::kPairFourCC)
    {
        auto listChunk = as<RIFF::ListChunk>(_cursor);
        if (!listChunk)
        {
            SLANG_UNEXPECTED("invalid format in RIFF");
        }

        if (listChunk->getType() != typeCode)
        {
            SLANG_UNEXPECTED("invalid format in RIFF");
        }

        _cursor = listChunk->getFirstChild();
    }

    void beginProperty(FourCC propertyCode)
    {
        auto listChunk = as<RIFF::ListChunk>(_cursor);
        if (!listChunk)
        {
            SLANG_UNEXPECTED("invalid format in RIFF");
        }

        auto found = listChunk->findListChunk(propertyCode);
        if (!found)
        {
            SLANG_UNEXPECTED("invalid format in RIFF");
        }

        _cursor = found->getFirstChild();
    }

    bool hasElements() { return _cursor != nullptr; }

    bool isNull()
    {
        if (_cursor == nullptr)
            return true;
        if (getTag() == SerialBinary::kNullFourCC)
            return true;
        return false;
    }

    bool decodeNull()
    {
        if (!isNull())
            return false;

        if (_cursor != nullptr)
        {
            _advanceCursor();
        }
        return true;
    }

    using Cursor = RIFF::BoundsCheckedChunkPtr;

    struct WithArray
    {
    public:
        WithArray(Decoder& decoder)
            : _decoder(decoder)
        {
            _saved = decoder._cursor;
            decoder.beginArray();
        }

        WithArray(Decoder& decoder, FourCC typeCode)
            : _decoder(decoder)
        {
            _saved = decoder._cursor;
            decoder.beginArray(typeCode);
        }

        ~WithArray() { _decoder._cursor = _saved.getNextSibling(); }

    private:
        Cursor _saved;
        Decoder& _decoder;
    };

    struct WithObject
    {
    public:
        WithObject(Decoder& decoder)
            : _decoder(decoder)
        {
            _saved = decoder._cursor;
            decoder.beginObject();
        }

        WithObject(Decoder& decoder, FourCC typeCode)
            : _decoder(decoder)
        {
            _saved = decoder._cursor;
            decoder.beginObject(typeCode);
        }

        ~WithObject() { _decoder._cursor = _saved.getNextSibling(); }

    private:
        Cursor _saved;
        Decoder& _decoder;
    };

    struct WithKeyValuePair
    {
    public:
        WithKeyValuePair(Decoder& decoder)
            : _decoder(decoder)
        {
            _saved = decoder._cursor;
            decoder.beginKeyValuePair();
        }

        WithKeyValuePair(Decoder& decoder, FourCC typeCode)
            : _decoder(decoder)
        {
            _saved = decoder._cursor;
            _decoder.beginKeyValuePair(typeCode);
        }

        ~WithKeyValuePair() { _decoder._cursor = _saved.getNextSibling(); }

    private:
        Cursor _saved;
        Decoder& _decoder;
    };

    struct WithProperty
    {
    public:
        WithProperty(Decoder& decoder, FourCC typeCode)
            : _decoder(decoder)
        {
            _saved = decoder._cursor;
            _decoder.beginProperty(typeCode);
        }

        ~WithProperty() { _decoder._cursor = _saved.getNextSibling(); }

    private:
        Cursor _saved;
        Decoder& _decoder;
    };

    Cursor getCursor() const { return _cursor; }
    void setCursor(Cursor const& cursor) { _cursor = cursor; }

    RIFF::Chunk const* getCurrentChunk() const { return getCursor(); }

private:
    void _advanceCursor() { _cursor = _cursor.getNextSibling(); }

    Cursor _cursor;
};


} // namespace Slang

#endif
