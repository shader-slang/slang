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
    Encoder(Stream* stream)
        : _stream(stream)
    {
    }

    ~Encoder() { RiffUtil::write(&_riff, _stream); }

    void beginArray(FourCC typeCode)
    {
        _riff.startChunk(RiffContainer::Chunk::Kind::List, typeCode);
    }

    void beginArray() { beginArray(SerialBinary::kArrayFourCC); }

    void endArray()
    {
        _riff.endChunk();
        // TODO: maybe end key...
    }

    void beginObject(FourCC typeCode)
    {
        _riff.startChunk(RiffContainer::Chunk::Kind::List, typeCode);
    }

    void beginObject() { beginObject(SerialBinary::kObjectFourCC); }

    void endObject() { _riff.endChunk(); }

    void beginKeyValuePair()
    {
        _riff.startChunk(RiffContainer::Chunk::Kind::List, SerialBinary::kPairFourCC);
    }

    void endKeyValuePair() { _riff.endChunk(); }

    void beginKeyValuePair(FourCC keyCode)
    {
        _riff.startChunk(RiffContainer::Chunk::Kind::List, keyCode);
    }

    void encodeData(FourCC typeCode, void const* data, size_t size)
    {
        _riff.startChunk(RiffContainer::Chunk::Kind::Data, typeCode);
        _riff.write(data, size);
        _riff.endChunk();
    }

    void encodeData(void const* data, size_t size)
    {
        encodeData(SerialBinary::kDataFourCC, data, size);
    }

    void encode(nullptr_t) { encodeData(SerialBinary::kNullFourCC, nullptr, 0); }

    void encodeBool(bool value)
    {
        encodeData(value ? SerialBinary::kTrueFourCC : SerialBinary::kFalseFourCC, nullptr, 0);
    }

    void encode(Int32 value) { encodeData(SerialBinary::kInt32FourCC, &value, sizeof(value)); }

    void encode(UInt32 value) { encodeData(SerialBinary::kUInt32FourCC, &value, sizeof(value)); }

    void encode(Int64 value) { encodeData(SerialBinary::kInt64FourCC, &value, sizeof(value)); }

    void encode(UInt64 value) { encodeData(SerialBinary::kUInt64FourCC, &value, sizeof(value)); }

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
    Stream* _stream = nullptr;

    // Implementation details below...
    RiffContainer _riff;

public:
    RiffContainer* getRIFF() { return &_riff; }

    RiffContainer::Chunk* getRIFFChunk() { return _riff.getCurrentChunk(); }

    void setRIFFChunk(RiffContainer::Chunk* chunk) { _riff.setCurrentChunk(chunk); }
};

struct Decoder
{
public:
    Decoder(RiffContainer::Chunk* chunk)
        : _chunk(chunk)
    {
    }

    bool decodeBool()
    {
        switch (getTag())
        {
        case SerialBinary::kTrueFourCC:
            _chunk = _chunk->m_next;
            return true;
        case SerialBinary::kFalseFourCC:
            _chunk = _chunk->m_next;
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

        auto dataChunk = as<RiffContainer::DataChunk>(_chunk);
        if (!dataChunk)
        {
            SLANG_UNEXPECTED("invalid format in RIFF");
            UNREACHABLE_RETURN("");
        }

        auto size = dataChunk->calcPayloadSize();

        String value;
        value.appendRepeatedChar(' ', size);
        dataChunk->getPayload((char*)value.getBuffer());

        _chunk = _chunk->m_next;
        return value;
    }

    void decodeData(FourCC typeTag, void* outData, size_t dataSize)
    {
        if (getTag() == typeTag)
        {
            auto dataChunk = as<RiffContainer::DataChunk>(_chunk);
            if (dataChunk)
            {
                if (dataChunk->calcPayloadSize() >= dataSize)
                {
                    dataChunk->getPayload(outData);
                    _chunk = _chunk->m_next;
                    return;
                }
            }
        }

        SLANG_UNEXPECTED("invalid format in RIFF");
        UNREACHABLE_RETURN("");
    }

    template<typename T>
    T _decodeSimpleValue(FourCC typeTag)
    {
        T value;
        decodeData(typeTag, &value, sizeof(value));
        return value;
    }

    Int64 decodeInt64() { return _decodeSimpleValue<Int64>(SerialBinary::kInt64FourCC); }

    UInt64 decodeUInt64() { return _decodeSimpleValue<UInt64>(SerialBinary::kUInt64FourCC); }

    Int32 decodeInt32() { return _decodeSimpleValue<Int32>(SerialBinary::kInt32FourCC); }

    UInt32 decodeUInt32() { return _decodeSimpleValue<UInt32>(SerialBinary::kUInt32FourCC); }

    float decodeFloat32() { return _decodeSimpleValue<float>(SerialBinary::kFloat32FourCC); }

    double decodeFloat64() { return _decodeSimpleValue<double>(SerialBinary::kFloat64FourCC); }


    FourCC getTag() { return _chunk ? _chunk->m_fourCC : 0; }

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
        auto listChunk = as<RiffContainer::ListChunk>(_chunk);
        if (!listChunk)
        {
            SLANG_UNEXPECTED("invalid format in RIFF");
            UNREACHABLE_RETURN("");
        }

        if (listChunk->m_fourCC != typeCode)
        {
            SLANG_UNEXPECTED("invalid format in RIFF");
            UNREACHABLE_RETURN("");
        }

        _chunk = listChunk->getFirstContainedChunk();
    }

    void beginObject(FourCC typeCode = SerialBinary::kObjectFourCC)
    {
        auto listChunk = as<RiffContainer::ListChunk>(_chunk);
        if (!listChunk)
        {
            SLANG_UNEXPECTED("invalid format in RIFF");
            UNREACHABLE_RETURN("");
        }

        if (listChunk->m_fourCC != typeCode)
        {
            SLANG_UNEXPECTED("invalid format in RIFF");
            UNREACHABLE_RETURN("");
        }

        _chunk = listChunk->getFirstContainedChunk();
    }

    void beginKeyValuePair(FourCC typeCode = SerialBinary::kPairFourCC)
    {
        auto listChunk = as<RiffContainer::ListChunk>(_chunk);
        if (!listChunk)
        {
            SLANG_UNEXPECTED("invalid format in RIFF");
            UNREACHABLE_RETURN("");
        }

        if (listChunk->m_fourCC != typeCode)
        {
            SLANG_UNEXPECTED("invalid format in RIFF");
            UNREACHABLE_RETURN("");
        }

        _chunk = listChunk->getFirstContainedChunk();
    }

    void beginProperty(FourCC propertyCode)
    {
        auto listChunk = as<RiffContainer::ListChunk>(_chunk);
        if (!listChunk)
        {
            SLANG_UNEXPECTED("invalid format in RIFF");
            UNREACHABLE_RETURN("");
        }

        auto found = listChunk->findContainedList(propertyCode);
        if (!found)
        {
            SLANG_UNEXPECTED("invalid format in RIFF");
            UNREACHABLE_RETURN("");
        }

        _chunk = found->getFirstContainedChunk();
    }

    bool hasElements() { return _chunk != nullptr; }

    bool isNull()
    {
        if (_chunk == nullptr)
            return true;
        if (getTag() == SerialBinary::kNullFourCC)
            return true;
        return false;
    }

    bool decodeNull()
    {
        if (!isNull())
            return false;

        if (_chunk != nullptr)
        {
            _chunk = _chunk->m_next;
        }
        return true;
    }

    struct WithArray
    {
    public:
        WithArray(Decoder& decoder)
            : _decoder(decoder)
        {
            _saved = decoder._chunk;
            decoder.beginArray();
        }

        WithArray(Decoder& decoder, FourCC typeCode)
            : _decoder(decoder)
        {
            _saved = decoder._chunk;
            decoder.beginArray(typeCode);
        }

        ~WithArray() { _decoder._chunk = _saved->m_next; }

    private:
        RiffContainer::Chunk* _saved;
        Decoder& _decoder;
    };

    struct WithObject
    {
    public:
        WithObject(Decoder& decoder)
            : _decoder(decoder)
        {
            _saved = decoder._chunk;
            decoder.beginObject();
        }

        WithObject(Decoder& decoder, FourCC typeCode)
            : _decoder(decoder)
        {
            _saved = decoder._chunk;
            decoder.beginObject(typeCode);
        }

        ~WithObject() { _decoder._chunk = _saved->m_next; }

    private:
        RiffContainer::Chunk* _saved;
        Decoder& _decoder;
    };

    struct WithKeyValuePair
    {
    public:
        WithKeyValuePair(Decoder& decoder)
            : _decoder(decoder)
        {
            _saved = decoder._chunk;
            decoder.beginKeyValuePair();
        }

        WithKeyValuePair(Decoder& decoder, FourCC typeCode)
            : _decoder(decoder)
        {
            _saved = decoder._chunk;
            _decoder.beginKeyValuePair(typeCode);
        }

        ~WithKeyValuePair() { _decoder._chunk = _saved->m_next; }

    private:
        RiffContainer::Chunk* _saved;
        Decoder& _decoder;
    };

    struct WithProperty
    {
    public:
        WithProperty(Decoder& decoder, FourCC typeCode)
            : _decoder(decoder)
        {
            _saved = decoder._chunk;
            _decoder.beginProperty(typeCode);
        }

        ~WithProperty() { _decoder._chunk = _saved->m_next; }

    private:
        RiffContainer::Chunk* _saved;
        Decoder& _decoder;
    };


    RiffContainer::Chunk* getCursor() { return _chunk; }
    void setCursor(RiffContainer::Chunk* chunk) { _chunk = chunk; }

private:
    RiffContainer::Chunk* _chunk = nullptr;
};


} // namespace Slang

#endif
