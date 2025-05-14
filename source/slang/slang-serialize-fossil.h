// slang-serialize-fossil.h
#ifndef SLANG_SERIALIZE_FOSSIL_H
#define SLANG_SERIALIZE_FOSSIL_H

//
// This file provides implementations of `ISerializerImpl` that
// serialize hierarchical data in a "memory-mappable" binary
// format.
//
// The term "fossil" is being used here to refer to formerly
// "live" objects that have been converted into an alternative
// form that can no longer perform their original functions,
// but that can still be inspected and dug through.
//

#include "slang-serialize.h"
#include "slang-fossil.h"

#include "../core/slang-blob-builder.h"
#include "../core/slang-internally-linked-list.h"
#include "../core/slang-io.h"
#include "../core/slang-memory-arena.h"
#include "../core/slang-relative-ptr.h"

namespace Slang
{

namespace Fossil
{


/// Serializer implementation for writing objects to a fossil-format blob.
struct SerialWriter : ISerializerImpl
{
public:
    SerialWriter(ChunkBuilder* chunk);
    SerialWriter(SlabBuilder& slab);

    ~SerialWriter();

private:
    SerialWriter() = delete;

    void _initialize(ChunkBuilder* chunk);

    class LayoutObj
    {
    public:
        LayoutObj(FossilizedValKind kind, Size size = 0, Size alignment = 1)
            : kind(kind), size(size), alignment(alignment)
        {
        }

        virtual ~LayoutObj() {}

        FossilizedValKind getKind() const
        {
            return kind;
        }

        Size getSize() const { return size; }
        Size getAlignment() const { return alignment; }

        static LayoutObj* _create(FossilizedValKind kind);

        static LayoutObj* _merge(LayoutObj*& dst, FossilizedValKind kind);
        static void _merge(LayoutObj*& dst, LayoutObj* src);

        FossilizedValKind kind;
        Size size = 0;
        Size alignment = 1;

        ChunkBuilder* chunk = nullptr;
    };

    class SimpleLayoutObj : public LayoutObj
    {
    public:
        SimpleLayoutObj(FossilizedValKind kind, Size size)
            : LayoutObj(kind, size, size)
        {
        }

        SimpleLayoutObj(FossilizedValKind kind)
            : LayoutObj(kind)
        {
        }
    };

    class ContainerLayoutObj : public LayoutObj
    {
    public:
        ContainerLayoutObj(FossilizedValKind kind, LayoutObj* baseLayout, Size size = 0, Size alignment = 1)
            : LayoutObj(kind, size, alignment), baseLayout(baseLayout)
        {
        }

        LayoutObj* baseLayout = nullptr;
    };

    class PtrLayoutObj : public ContainerLayoutObj
    {
    public:
        PtrLayoutObj(LayoutObj* baseLayout)
            : ContainerLayoutObj(FossilizedValKind::Ptr, baseLayout)
        {}
    };

    class RecordLayoutObj : public LayoutObj
    {
    public:
        RecordLayoutObj(FossilizedValKind kind)
            : LayoutObj(kind)
        {
        }

        struct FieldInfo
        {
            LayoutObj* layout = nullptr;
            Size offset = 0;
        };
        FieldInfo& _getField(Index index)
        {
            SLANG_ASSERT(index >= 0);

            if (index < fields.getCount())
                return fields[index];

            SLANG_ASSERT(index == fields.getCount());
            fields.add(FieldInfo());
            return fields[index];
        }

        List<FieldInfo> fields;
    };

    class TupleLayoutObj : public RecordLayoutObj
    {};

    struct FossilizedObjectInfo
    {
        ChunkBuilder* chunk = nullptr;
        LayoutObj* ptrLayout = nullptr;

        void* liveObjectPtr = nullptr;
        Callback callback = nullptr;
        void* userData = nullptr;
    };

    List<FossilizedObjectInfo*> _fossilizedObjects;
    Dictionary<void*, FossilizedObjectInfo*> _mapLiveObjectPtrToFossilizedObject;
    Index _writtenObjectDefinitionCount = 0;

    Dictionary<String, ChunkBuilder*> _mapStringToChunk;

    struct VariantInfo
    {
        LayoutObj* layout = nullptr;
        ChunkBuilder* chunk = nullptr;
    };
    List<VariantInfo> _variants;

    struct State
    {
        /// The layout for the value being composed.
        LayoutObj* layout = nullptr;

        /// The number of elements/fields or other sub-values written so var.
        Count elementCount = 0;

        /// The chunk that holds the data for the value.

        ChunkBuilder* chunk = nullptr;

        State() {}

        State(LayoutObj* layout, ChunkBuilder* chunk = nullptr)
            : layout(layout), chunk(chunk)
        {
        }
    };

    State _state;
    List<State> _stack;

    SlabBuilder* _slab = nullptr;

    struct ValInfo
    {
    public:
        enum class Kind
        {
            RawData,
            RelativePtr,
            ContentsOfChunk,
        };

        static ValInfo rawData(void const* data, Size size, Size alignment);
        static ValInfo relativePtrTo(ChunkBuilder* targetChunk);
        static ValInfo contentsOf(ChunkBuilder* chunk);

        Size getAlignment() const;

        Kind kind;
        union
        {
            struct
            {
                void const* ptr;
                Size size;
                Size alignment;
            } data;
            ChunkBuilder* chunk;
        };

    private:
        ValInfo() = default;
        ValInfo(const ValInfo&) = default;
        ValInfo(ValInfo&&) = default;
        ValInfo(Kind kind)
            : kind(kind)
        {
        }
    };


    /// Information on the objects that have been referenced,
    /// and which need their definitions to be serialized into
    /// the object definition list chunk.
    ///

    #if 0
    LayoutObj* _createUnknownLayout();
    LayoutObj* _getContainerLayout(FossilizedValKind kind, LayoutObj* elementLayout);
    LayoutObj* _getPtrLayout(LayoutObj* valLayout);
    LayoutObj* _getLayout(FossilizedValKind kind, Size size, Size alignment);
    #endif

    // An inline value will never be indirected.
    //
    void _pushInlineValueScope(FossilizedValKind kind);
    void _popInlineValueScope();

    void _pushIndirectValueScope(FossilizedValKind kind);
    void _popIndirectValueScope();

    void _pushPotentiallyIndirectValueScope(FossilizedValKind kind);
    ChunkBuilder* _popPotentiallyIndirectValueScope();

    void _pushContainerScope(FossilizedValKind kind);
    void _popContainerScope();

    void _pushVariantScope();
    void _popVariantScope();

    void _pushState(LayoutObj* layout);
    void _popState();

    bool _shouldEmitWithPointerIndirection(FossilizedValKind kind);
    ChunkBuilder* _writeKnownIndirectValueSharedLogic(ChunkBuilder* valueChunk);


    void _writeSimpleValue(FossilizedValKind kind, void const* data, size_t size, size_t alignment);

    template<typename T>
    void _writeSimpleValue(FossilizedValKind kind, T const& value)
    {
        _writeSimpleValue(kind, &value, sizeof(value), sizeof(value));
    }

//    bool _trySkipValue();
    void _writeNull();


    LayoutObj*& _reserveDestinationForWrite();
    LayoutObj* _reserveDestinationForWrite(FossilizedValKind srcKind);
    LayoutObj* _reserveDestinationForWrite(LayoutObj* srcLayout);

    void _commitWrite(ValInfo const& val);

    void _writeValueRaw(ValInfo const& val);

    void _ensureChunkExists();

    void _flush();
    ChunkBuilder* _getOrCreateChunkForLayout(LayoutObj* layout);
//    ChunkBuilder* _createChunkForLayout(LayoutObj* layout);

    struct LayoutObjKey
    {
        LayoutObjKey() {}

        LayoutObjKey(LayoutObj* obj)
            : obj(obj)
        {}

        LayoutObj* obj = nullptr;

        bool operator==(LayoutObjKey const& that) const;
        bool operator!=(LayoutObjKey const& that) const;

        HashCode64 getHashCode() const;
        void hashInto(Hasher& hasher) const;
    };
    Dictionary<LayoutObjKey, ChunkBuilder*> _mapLayoutObjToChunk;

private:
    //
    // The following declarations are the requirements
    // of the `ISerializerImpl` interface:
    //

    virtual SerializationMode getMode() override;

    virtual void handleBool(bool& value) override;

    virtual void handleInt8(int8_t& value) override;
    virtual void handleInt16(int16_t& value) override;
    virtual void handleInt32(Int32& value) override;
    virtual void handleInt64(Int64& value) override;

    virtual void handleUInt8(uint8_t& value) override;
    virtual void handleUInt16(uint16_t& value) override;
    virtual void handleUInt32(UInt32& value) override;
    virtual void handleUInt64(UInt64& value) override;

    virtual void handleFloat32(float& value) override;
    virtual void handleFloat64(double& value) override;

    virtual void handleString(String& value) override;

    virtual void beginArray() override;
    virtual void endArray() override;

    virtual void beginOptional() override;
    virtual void endOptional() override;

    virtual void beginDictionary() override;
    virtual void endDictionary() override;

    virtual bool hasElements() override;

    virtual void beginTuple() override;
    virtual void endTuple() override;

    virtual void beginStruct() override;
    virtual void endStruct() override;

    virtual void beginVariant() override;
    virtual void endVariant() override;

    virtual void handleFieldKey(char const* name, Int index) override;

    virtual void handleSharedPtr(void*& value, Callback callback, void* userData) override;
    virtual void handleUniquePtr(void*& value, Callback callback, void* userData) override;

    virtual void handleDeferredObjectContents(void* valuePtr, Callback callback, void* userData)
        override;
};

/// Serializer implementation for reading objects from a fossil-format blob.
struct SerialReader : ISerializerImpl
{
public:
    SerialReader(FossilizedValRef valRef);
    ~SerialReader();

private:
    struct State
    {
        enum class Type
        {
            Root,
            Array,
            Dictionary,
            Optional,
            Tuple,
            Struct,
            Object,
        };

        Type type = Type::Root;
        FossilizedValRef baseValue;
        Index elementIndex = 0;
        Count elementCount = 0;
    };
    State _state;
    List<State> _stack;

    enum class ObjectState
    {
        Unread,
        ReadingInProgress,
        ReadingComplete,
    };

    struct ObjectInfo
    {
        ObjectState state = ObjectState::Unread;

        void* resurrectedObjectPtr = nullptr;
        FossilizedValRef fossilizedObjectRef;
    };
    Dictionary<void*, ObjectInfo*> _mapFossilizedObjectPtrToObjectInfo;

    struct DeferredAction
    {
        void* resurrectedObjectPtr;

        State savedState;

        Callback callback;
        void* userData;
    };

    List<DeferredAction> _deferredActions;

    void _flush();

    FossilizedValRef _readValRef();
    FossilizedValRef _readIndirectValRef();
    FossilizedValRef _readPotentiallyIndirectValRef();
        //    void _advanceCursor();

    void _pushState();
    void _popState();

private:
    //
    // The following declarations are the requirements
    // of the `ISerializerImpl` interface:
    //

    virtual SerializationMode getMode() override;

    virtual void handleBool(bool& value) override;

    virtual void handleInt8(int8_t& value) override;
    virtual void handleInt16(int16_t& value) override;
    virtual void handleInt32(Int32& value) override;
    virtual void handleInt64(Int64& value) override;

    virtual void handleUInt8(uint8_t& value) override;
    virtual void handleUInt16(uint16_t& value) override;
    virtual void handleUInt32(UInt32& value) override;
    virtual void handleUInt64(UInt64& value) override;

    virtual void handleFloat32(float& value) override;
    virtual void handleFloat64(double& value) override;

    virtual void handleString(String& value) override;

    virtual void beginArray() override;
    virtual void endArray() override;

    virtual void beginDictionary() override;
    virtual void endDictionary() override;

    virtual bool hasElements() override;

    virtual void beginStruct() override;
    virtual void endStruct() override;

    virtual void beginVariant() override;
    virtual void endVariant() override;

    virtual void handleFieldKey(char const* name, Int index) override;

    virtual void beginTuple() override;
    virtual void endTuple() override;

    virtual void beginOptional() override;
    virtual void endOptional() override;

    virtual void handleSharedPtr(void*& value, Callback callback, void* userData) override;
    virtual void handleUniquePtr(void*& value, Callback callback, void* userData) override;

    virtual void handleDeferredObjectContents(void* valuePtr, Callback callback, void* userData)
        override;
};
} // namespace Fossil
} // namespace Slang

#endif
