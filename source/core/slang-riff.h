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

struct RiffContainerHeader
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
    static SlangResult readContainer(Stream* stream, RiffContainerHeader& outHeader, MemoryArena& ioArena, List<SubChunk>& outChunks);

        /// Read a header. Handles special case of list/riff types
    static SlangResult readHeader(Stream* stream, RiffContainerHeader& outHeader);

        /// True if the type is a container type
    static bool isContainerType(FourCC type)
    {
        return type == RiffFourCC::kRiff || type == RiffFourCC::kList;
    }

};

class RiffContainer
{
public:

    struct Data
    {
            /// Get the payload
        void* getPayload() { return (void*)(this + 1); }
        size_t getSize() { return size; }

        size_t size;
        Data* next;
        // Followed by the payload
    };

    struct Chunk
    {
        FourCC type;                    ///< Type of chunk
        size_t size;                    ///< After closed 
        Chunk* next;                    ///< Next chunk in this list
        Data* dataList;                 ///< List of 0 or more data items
    };

    struct Container : public Chunk
    {
        // the type can only be list or riff
        FourCC subType;                       ///< The subtype of this contained
        Chunk* containedChunks;               ///< The contained chunks
        Chunk* endChunk;                      ///< The last chunk (only set when pushed, and used when popped)
    };

    class ScopeChunk
    {
    public:
        ScopeChunk(RiffContainer* container, FourCC type):
            m_container(container)
        {
            container->startChunk(type);
        }
        ~ScopeChunk()
        {
            m_container->endChunk();
        }
    private:
        RiffContainer* m_container;
    };

    class ScopeContainer
    {
    public:
        ScopeContainer(RiffContainer* container, FourCC subType):
            m_container(container)
        {
            container->startContainer(subType);
        }
        ~ScopeContainer()
        {
            m_container->endContainer();
        }
    private:
        RiffContainer* m_container;
    };

        /// Start a chunk
    void startChunk(FourCC chunkType);
        /// Write data into a chunk
    void write(const void* data, size_t size);
    Data* addData(size_t size);
        /// End a chunk
    void endChunk();

        /// Start a container
    void startContainer(FourCC subType);
        /// End a container
    void endContainer();

        /// Get the root
    Container* getRoot() const { return m_rootContainer; }

        /// Reset the container
    void reset();

        /// true if has a root container, and nothing remains open
    bool isFullyConstructed() { return m_rootContainer && m_container == nullptr; }

        /// Write a container and contents to a stream
    static SlangResult write(Container* container, bool isRoot, Stream* stream);

        /// Read the stream into the container
    static SlangResult read(Stream* stream, RiffContainer& outContainer);

    static bool isContainerOk(Container* container);

        /// Ctor
    RiffContainer();

protected:
    void _initAndAddChunk(FourCC chunkType, Chunk* chunk);
    
    Container* m_rootContainer;
    Container* m_container;
    Chunk* m_endChunk;
    Data* m_endData;

    // The last member is the top of the stack and the current container
    List<Container*> m_containerStack;

    MemoryArena m_arena;
};

}

#endif
