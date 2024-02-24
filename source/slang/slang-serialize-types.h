// slang-serialize-types.h
#ifndef SLANG_SERIALIZE_TYPES_H
#define SLANG_SERIALIZE_TYPES_H

#include "../core/slang-riff.h"
#include "../core/slang-string-slice-pool.h"
#include "../core/slang-array-view.h"

//#include "slang-name.h"
//#include "slang-source-loc.h"

namespace Slang {

// An enumeration of types that can be set
enum class SerialExtraType
{
    SourceLocReader,
    SourceLocWriter,
    CountOf,
};

// Options for IR/AST/Debug serialization

struct SerialOptionFlag
{
    typedef uint32_t Type;
    enum Enum : Type
    {
        RawSourceLocation   = 0x01,     ///< If set will store directly SourceLoc - only useful if current source locs will be identical when read in (typically this is *NOT* the case)
        SourceLocation      = 0x02,     ///< If set will output SourceLoc information, that can be reconstructed when read after being stored.
        ASTModule           = 0x04,     ///< If set will output AST modules - typically required, but potentially not desired (for example with obsfucation)
        IRModule            = 0x08,     ///< If set will output IR modules - typically required
        ASTFunctionBody     = 0x10,     ///< If set will serialize AST function bodies.
    };
};
typedef SerialOptionFlag::Type SerialOptionFlags;


// Compression styles

enum class SerialCompressionType : uint8_t
{
    None,
    VariableByteLite,
};


struct SerialStringData
{
    enum class StringIndex : uint32_t;
 
    ///enum class StringOffset : uint32_t;                     ///< Offset into the m_stringsBuffer

    typedef uint32_t SizeType;

    static const StringIndex kNullStringIndex = StringIndex(StringSlicePool::kNullHandle);
    static const StringIndex kEmptyStringIndex = StringIndex(StringSlicePool::kEmptyHandle);
};

struct SerialStringTableUtil
{
        /// Convert a pool into a string table
    static void encodeStringTable(const StringSlicePool& pool, List<char>& stringTable);
    static void encodeStringTable(const ConstArrayView<UnownedStringSlice>& slices, List<char>& stringTable);

        /// Appends the decoded strings into slicesOut
    static void appendDecodedStringTable(const char* table, size_t tableSize, List<UnownedStringSlice>& slicesOut);

        /// Decodes a string table (and does so such that the indices are compatible with StringSlicePool)
    static void decodeStringTable(const char* table, size_t tableSize, List<UnownedStringSlice>& slicesOut);

        /// Decodes a string table 
    static void decodeStringTable(const char* table, size_t tableSize, StringSlicePool& outPool);

        /// Produces an index map, from slices to indices in pool
    static void calcStringSlicePoolMap(const List<UnownedStringSlice>& slices, StringSlicePool& pool, List<StringSlicePool::Handle>& indexMap);
};

struct SerialParseUtil
{
    /// Given text, finds the compression type
    static SlangResult parseCompressionType(const UnownedStringSlice& text, SerialCompressionType& outType);
    /// Given a compression type, return text
    static UnownedStringSlice getText(SerialCompressionType type);
};

struct SerialListUtil
{
    template <typename T>
    static size_t calcArraySize(const List<T>& list)
    {
        return list.getCount() * sizeof(T);
    }

    template <typename T>
    static bool isEqual(const List<T>& aIn, const List<T>& bIn)
    {
        if (&aIn == &bIn)
        {
            return true;
        }
        const Index size = aIn.getCount();

        if (size != bIn.getCount())
        {
            return false;
        }

        const T* a = aIn.begin();
        const T* b = bIn.begin();

        if (a != b)
        {
            for (Index i = 0; i < size; ++i)
            {
                if (a[i] != b[i])
                {
                    return false;
                }
            }
        }

        return true;
    }
};

// For types/FourCC that work for serializing in general (not just IR). 
struct SerialBinary
{
    static const FourCC kRiffFourCc = RiffFourCC::kRiff;

        /// Container 
    static const FourCC kContainerFourCc = SLANG_FOUR_CC('S', 'L', 'm', 'c');

        /// A string table
    static const FourCC kStringTableFourCc = SLANG_FOUR_CC('S', 'L', 's', 't');

        /// TranslationUnitList
    static const FourCC kModuleListFourCc = SLANG_FOUR_CC('S', 'L', 'm', 'l');

        /// An entry point
    static const FourCC kEntryPointFourCc = SLANG_FOUR_CC('E', 'P', 'n', 't');

        /// Container
    static const FourCC kContainerHeaderFourCc = SLANG_FOUR_CC('S', 'c', 'h', 'd');

        // Module header
    static const FourCC kModuleHeaderFourCc = SLANG_FOUR_CC('S', 'm', 'h', 'd');

    struct ContainerHeader
    {
        uint32_t compressionType;         ///< Holds the compression type used (if used at all)
    };

    struct ArrayHeader
    {
        uint32_t numEntries;
    };
    struct CompressedArrayHeader
    {
        uint32_t numEntries;              ///< The number of entries
        uint32_t numCompressedEntries;    ///< The amount of compressed entries
    };
};

// Replace first char with 's'
#define SLANG_MAKE_COMPRESSED_FOUR_CC(fourCc) SLANG_FOUR_CC_REPLACE_FIRST_CHAR(fourCc, 's')

struct SerialRiffUtil
{
    class ListResizer
    {
    public:
        virtual void* setSize(size_t newSize) = 0;
        SLANG_FORCE_INLINE size_t getTypeSize() const { return m_typeSize; }
        ListResizer(size_t typeSize) :m_typeSize(typeSize) {}

    protected:
        size_t m_typeSize;
    };

    template <typename T>
    class ListResizerForType : public ListResizer
    {
    public:
        typedef ListResizer Parent;

        SLANG_FORCE_INLINE ListResizerForType(List<T>& list) :
            Parent(sizeof(T)),
            m_list(list)
        {}

        virtual void* setSize(size_t newSize) SLANG_OVERRIDE
        {
            m_list.setCount(UInt(newSize));
            return (void*)m_list.begin();
        }

    protected:
        List<T>& m_list;
    };

    static Result writeArrayChunk(SerialCompressionType compressionType, FourCC chunkId, const void* data, size_t numEntries, size_t typeSize, RiffContainer* container);
    
    template <typename T>
    static Result writeArrayChunk(SerialCompressionType compressionType, FourCC chunkId, const List<T>& array, RiffContainer* container)
    {
        return writeArrayChunk(compressionType, chunkId, array.begin(), size_t(array.getCount()), sizeof(T), container);
    }

    template <typename T>
    static Result writeArrayUncompressedChunk(FourCC chunkId, const List<T>& array, RiffContainer* container)
    {
        return writeArrayChunk(SerialCompressionType::None, chunkId, array.begin(), size_t(array.getCount()), sizeof(T), container);
    }

    static Result readArrayChunk(SerialCompressionType compressionType, RiffContainer::DataChunk* dataChunk, ListResizer& listOut);

    template <typename T>
    static Result readArrayChunk(SerialCompressionType moduleCompressionType, RiffContainer::DataChunk* dataChunk, List<T>& arrayOut)
    {
        SerialCompressionType compressionType = SerialCompressionType::None;
        if (dataChunk->m_fourCC == SLANG_MAKE_COMPRESSED_FOUR_CC(dataChunk->m_fourCC))
        {
            // If it has compression, use the compression type set in the header
            compressionType = moduleCompressionType;
        }
        ListResizerForType<T> resizer(arrayOut);
        return readArrayChunk(compressionType, dataChunk, resizer);
    }

    template <typename T>
    static Result readArrayUncompressedChunk(RiffContainer::DataChunk* chunk, List<T>& arrayOut)
    {
        ListResizerForType<T> resizer(arrayOut);
        return readArrayChunk(SerialCompressionType::None, chunk, resizer);
    }


};

} // namespace Slang

#endif
