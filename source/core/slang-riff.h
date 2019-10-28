#ifndef SLANG_RIFF_H
#define SLANG_RIFF_H

#include "slang-basic.h"
#include "slang-stream.h"
#include "slang-memory-arena.h"

namespace Slang
{

// http://fileformats.archiveteam.org/wiki/RIFF
// http://www.fileformat.info/format/riff/egff.htm

typedef uint32_t FourCC;

#define SLANG_FOUR_CC(c0, c1, c2, c3) FourCC((uint32_t(c0) << 0) | (uint32_t(c1) << 8) | (uint32_t(c2) << 16) | (uint32_t(c3) << 24)) 

struct RiffFourCC
{
        /// A 'riff' is the high level file container. It is followed by a subtype and then the contained modules
     static const FourCC kRiff = SLANG_FOUR_CC('R', 'I', 'F', 'F');
        /// A is like riff but can be contained multiple times within a file. It is also followed by a header
     static const FourCC kList = SLANG_FOUR_CC('L', 'I', 'S', 'T');


        /// The sub type of a riff container containing modules
     //static const FourCC kSlangModule = SLANG_FOUR_CC('S', 'L', 'm', 'o');
};


// Follows semantic version rules
// https://semver.org/
// 
// major.minor.patch
// Patch versions indicate a change. 
// Minor means a change that is backwards compatible with previous minor versions. A step in minor and/or major zeros patch.
// Major means a non compatible change. A step in major, zeros minor and patch. 
struct RiffSemanticVersion
{
    typedef RiffSemanticVersion ThisType;
    typedef uint32_t RawType;

        /// ==
    bool operator==(const ThisType& rhs) const { return m_raw == rhs.m_raw; }
    bool operator!=(const ThisType& rhs) const { return !(*this == rhs); }
    
        /// A patch change indices a different version but does not change the compatibility of the format
    int getPatch() const { return m_raw & 0xff; }
        /// A minor change implies a format change that is backwards compatible  
    int getMinor() const { return (m_raw >> 8) & 0xff; }
        /// A major change is binary incompatible by default
    int getMajor() const { return (m_raw >> 16); }

    static RawType makeRaw(int major, int minor, int patch)
    {
        SLANG_ASSERT((major | minor | patch) >= 0);
        SLANG_ASSERT(major < 0x10000 && minor < 0x100 && patch < 0x100);
        return (RawType(major) << 16) | (RawType(minor) << 8) | RawType(patch);
    }

    static RiffSemanticVersion makeFromRaw(RawType raw)
    {
        ThisType version;
        version.m_raw = raw;
        return version;
    }

    static RiffSemanticVersion make(int major, int minor, int patch) { return makeFromRaw(makeRaw(major, minor, patch)); }

        /// True if the read version is compatible with the current version, based on semantic rules.
    static bool areCompatible(const ThisType& currentVersion, const ThisType& readVersion)
    {
        const RawType currentRaw = currentVersion.m_raw;
        const RawType readRaw = readVersion.m_raw;

        // Must have same major version.
        // For minor version, the read version must be less than or equal. 
        return ((currentRaw & 0xffff0000) == (readRaw & 0xffff0000)) && ((currentRaw & 0xff00) >= (readRaw & 0xff00));
    }

    RawType m_raw;
};

struct RiffChunk
{
    FourCC m_type;            ///< The FourCC code that identifies this chunk
    uint32_t m_size;          ///< Size does *NOT* include the riff chunk size. The size can be byte sized, but on storage it will always be treated as aligned up by 4.
};

struct RiffListHeader
{
    RiffChunk chunk;
    FourCC subType;
    // This is then followed by the contained subchunk/s
};

struct RiffUtil
{
    struct SubChunk
    {
        FourCC chunkType;
        const void* data;
        size_t dataSize;
    };

    static int64_t calcChunkTotalSize(const RiffChunk& chunk);

    static SlangResult skip(const RiffChunk& chunk, Stream* stream, int64_t* remainingBytesInOut);

    static SlangResult readChunk(Stream* stream, RiffChunk& outChunk);

    static SlangResult writeData(const RiffChunk* header, size_t headerSize, const void* payload, size_t payloadSize, Stream* out);
    static SlangResult readData(Stream* stream, RiffChunk* outHeader, size_t headerSize, List<uint8_t>& data);


    static SlangResult readPayload(Stream* stream, size_t size, void* outData, size_t& outReadSize);


        /// Total size is the size of all the contained chunks
    static SlangResult writeContainerHeader(FourCC containerType, FourCC subType, size_t totalSize, Stream* out);

        /// Write riff with subType subtype, containing the specified chunks to stream
    static SlangResult writeContainer(FourCC containerType, FourCC subType, const SubChunk* subChunks, size_t subChunkCount, Stream* out);

        /// Read a riff container. The chunks memory is stored in the arena. 
    static SlangResult readContainer(Stream* stream, RiffListHeader& outHeader, MemoryArena& ioArena, List<SubChunk>& outChunks);

        /// Read a header. Handles special case of list/riff types
    static SlangResult readHeader(Stream* stream, RiffListHeader& outHeader);

        /// True if the type is a container type
    static bool isContainerType(FourCC type)
    {
        return type == RiffFourCC::kRiff || type == RiffFourCC::kList;
    }

};

class RiffContainer
{
public:

    struct ListChunk;
    struct DataChunk;

    struct Data
    {
            /// Get the payload
        void* getPayload() { return (void*)(this + 1); }
        size_t getSize() { return m_size; }

        void init()
        {
            m_size = 0;
            m_next = nullptr;
        }

        size_t m_size;
        Data* m_next;
        // Followed by the payload
    };

    struct Chunk;
    typedef SlangResult(*VisitorCallback)(Chunk* chunk, void* data);

    class Visitor;
    struct Chunk
    {
        enum class Kind
        {
            List,           ///< Strictly speaking this can be a 'LIST' or a 'RIFF' as they have the same structure
            Data,
        };

        void init(Kind kind)
        {
            m_kind = kind;
            m_payloadSize = 0;
            m_next = nullptr;
            m_parent = nullptr;
        }

        SlangResult visit(Visitor* visitor);
        SlangResult visitPostOrder(VisitorCallback callback, void* data);

        size_t calcPayloadSize();

        Kind m_kind;                      ///< Kind of chunk
        size_t m_payloadSize;             ///< The payload size (ie does NOT include RiffChunk header). 
        Chunk* m_next;                    ///< Next chunk in this list
        ListChunk* m_parent;         ///< The chunk this belongs to
    };

    struct ListChunk : public Chunk
    {
        typedef Chunk Super;
        SLANG_FORCE_INLINE static bool isType(const Chunk* chunk) { return chunk->m_kind == Kind::List; }

        void init(FourCC subType)
        {
            Super::init(Kind::List);
            m_subType = subType;
            m_containedChunks = nullptr;
            m_endChunk = nullptr;

            m_payloadSize = uint32_t(sizeof(RiffListHeader) - sizeof(RiffChunk));
        }

            /// NOTE! Assumes all contained chunks have correct payload sizes
        size_t calcPayloadSize()
        {
            // Have to include the part of the header not taken up by the RiffHeader
            size_t size = sizeof(RiffListHeader) - sizeof(RiffChunk);
            Chunk* chunk = m_containedChunks;
            while (chunk)
            {
                size_t chunkSize = chunk->m_payloadSize + sizeof(RiffChunk);
                // Align the contained chunk size
                chunkSize = (chunkSize + 3) & ~size_t(3);

                size += chunkSize;

                chunk = chunk->m_next;
            }
            return size;
        }

        // the type can only be list or riff
        FourCC m_subType;                       ///< The subtype of this contained
        Chunk* m_containedChunks;               ///< The contained chunks
        Chunk* m_endChunk;                      ///< The last chunk (only set when pushed, and used when popped)
    };

    struct DataChunk : public Chunk
    {
        typedef Chunk Super;

        SLANG_FORCE_INLINE static bool isType(const Chunk* chunk) { return chunk->m_kind == Kind::Data; }

        size_t calcPayloadSize() const
        {
            size_t size = 0;
            Data* data = m_dataList;
            while (data)
            {
                size += data->getSize();
                data = data->m_next;
            }
            return size;
        }

        void init(FourCC type)
        {
            Super::init(Kind::Data);
            m_type = type;
            m_dataList = nullptr;
            m_endData = nullptr;
        }

        FourCC m_type;                    ///< Type of chunk
        Data* m_dataList;                 ///< List of 0 or more data items
        Data* m_endData;                  ///< The last data point
    };

    class ScopeChunk
    {
    public:
        ScopeChunk(RiffContainer* container, Chunk::Kind kind, FourCC fourCC):
            m_container(container)
        {
            container->startChunk(kind, fourCC);
        }
        ~ScopeChunk()
        {
            m_container->endChunk();
        }
    private:
        RiffContainer* m_container;
    };

    class Visitor
    {
    public:
        virtual SlangResult enterList(ListChunk* list) = 0;
        virtual SlangResult handleData(DataChunk* data) = 0;
        virtual SlangResult leaveList(ListChunk* list) = 0; 
    };

        /// Start a chunk
    void startChunk(Chunk::Kind kind, FourCC type);

        /// Write data into a chunk (can only be inside a Kind::Data)
    void write(const void* data, size_t size);
    Data* addData(size_t size);
    
        /// End a chunk
    void endChunk();

        /// Get the root
    ListChunk* getRoot() const { return m_rootList; }

        /// Reset the container
    void reset();

        /// true if has a root container, and nothing remains open
    bool isFullyConstructed() { return m_rootList && m_listChunk == nullptr && m_dataChunk == nullptr; }

        /// Write a container and contents to a stream
    static SlangResult write(ListChunk* container, bool isRoot, Stream* stream);

        /// Read the stream into the container
    static SlangResult read(Stream* stream, RiffContainer& outContainer);

        /// The if the list and sublists appear correct
    static bool isChunkOk(Chunk* chunk);

        /// Traverses over chunk hierarchy and sets the sizes
    static void calcAndSetSize(Chunk* chunk);

        /// Ctor
    RiffContainer();

protected:
    void _addChunk(Chunk* chunk);
    ListChunk* _newListChunk(FourCC subType);
    DataChunk* _newDataChunk(FourCC type);
    
    ListChunk* m_rootList;          ///< Root list

    ListChunk* m_listChunk;            
    DataChunk* m_dataChunk;                 

    MemoryArena m_arena;
};

template <typename T>
T* as(RiffContainer::Chunk* chunk)
{
    return T::isType(chunk) ? static_cast<T*>(chunk) : nullptr;
}

}

#endif
