// slang-serialize-types.h
#ifndef SLANG_SERIALIZE_TYPES_H
#define SLANG_SERIALIZE_TYPES_H

#include "../core/slang-array-view.h"
#include "../core/slang-riff.h"
#include "../core/slang-string-slice-pool.h"

// #include "slang-name.h"
// #include "slang-source-loc.h"

namespace Slang
{
class Module;

// Options for IR/AST/Debug serialization

struct SerialOptionFlag
{
    typedef uint32_t Type;
    enum Enum : Type
    {
        RawSourceLocation =
            0x01, ///< If set will store directly SourceLoc - only useful if current source locs
                  ///< will be identical when read in (typically this is *NOT* the case)
        SourceLocation = 0x02, ///< If set will output SourceLoc information, that can be
                               ///< reconstructed when read after being stored.
        ASTModule = 0x04, ///< If set will output AST modules - typically required, but potentially
                          ///< not desired (for example with obsfucation)
        IRModule = 0x08,  ///< If set will output IR modules - typically required
    };
};
typedef SerialOptionFlag::Type SerialOptionFlags;

struct SerialStringData
{
    enum class StringIndex : uint32_t;

    /// enum class StringOffset : uint32_t;                     ///< Offset into the m_stringsBuffer

    typedef uint32_t SizeType;

    static const StringIndex kNullStringIndex = StringIndex(StringSlicePool::kNullHandle);
    static const StringIndex kEmptyStringIndex = StringIndex(StringSlicePool::kEmptyHandle);
};

struct SerialStringTableUtil
{
    /// Convert a pool into a string table
    static void encodeStringTable(const StringSlicePool& pool, List<char>& stringTable);
    static void encodeStringTable(
        const ConstArrayView<UnownedStringSlice>& slices,
        List<char>& stringTable);

    /// Appends the decoded strings into slicesOut
    static void appendDecodedStringTable(
        const char* table,
        size_t tableSize,
        List<UnownedStringSlice>& slicesOut);

    /// Decodes a string table (and does so such that the indices are compatible with
    /// StringSlicePool)
    static void decodeStringTable(
        const char* table,
        size_t tableSize,
        List<UnownedStringSlice>& slicesOut);

    /// Decodes a string table
    static void decodeStringTable(const char* table, size_t tableSize, StringSlicePool& outPool);

    /// Produces an index map, from slices to indices in pool
    static void calcStringSlicePoolMap(
        const List<UnownedStringSlice>& slices,
        StringSlicePool& pool,
        List<StringSlicePool::Handle>& indexMap);
};

struct SerialListUtil
{
    template<typename T>
    static size_t calcArraySize(const List<T>& list)
    {
        return list.getCount() * sizeof(T);
    }

    template<typename T>
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

template<typename T>
struct PropertyKeys
{
};

template<>
struct PropertyKeys<Module>
{
    static const FourCC::RawValue Digest = SLANG_FOUR_CC('S', 'H', 'A', '1');
    static const FourCC::RawValue ASTModule = SLANG_FOUR_CC('a', 's', 't', ' ');
    static const FourCC::RawValue IRModule = SLANG_FOUR_CC('i', 'r', ' ', ' ');
    static const FourCC::RawValue FileDependencies = SLANG_FOUR_CC('f', 'd', 'e', 'p');
};

// For types/FourCC that work for serializing in general (not just IR).
struct SerialBinary
{
    static const FourCC::RawValue kRiffFourCc = RIFF::RootChunk::kTag;

    /// Container
    static const FourCC::RawValue kContainerFourCc = SLANG_FOUR_CC('S', 'L', 'm', 'c');

    /// A string table
    static const FourCC::RawValue kStringTableFourCc = SLANG_FOUR_CC('S', 'L', 's', 't');

    /// TranslationUnitList
    static const FourCC::RawValue kModuleListFourCc = SLANG_FOUR_CC('S', 'L', 'm', 'l');

    /// An entry point
    static const FourCC::RawValue kEntryPointFourCc = SLANG_FOUR_CC('E', 'P', 'n', 't');

    static const FourCC::RawValue kEntryPointListFourCc = SLANG_FOUR_CC('e', 'p', 't', 's');

    // Module
    static const FourCC::RawValue kModuleFourCC = SLANG_FOUR_CC('s', 'm', 'o', 'd');

    // The following are "generic" codes, suitable for
    // use when serializing content using JSON-like structure.
    //
    static const FourCC::RawValue kObjectFourCC = SLANG_FOUR_CC('o', 'b', 'j', ' ');
    static const FourCC::RawValue kPairFourCC = SLANG_FOUR_CC('p', 'a', 'i', 'r');
    static const FourCC::RawValue kArrayFourCC = SLANG_FOUR_CC('a', 'r', 'r', 'y');
    static const FourCC::RawValue kDictionaryFourCC = SLANG_FOUR_CC('d', 'i', 'c', 't');
    static const FourCC::RawValue kNullFourCC = SLANG_FOUR_CC('n', 'u', 'l', 'l');
    static const FourCC::RawValue kStringFourCC = SLANG_FOUR_CC('s', 't', 'r', ' ');
    static const FourCC::RawValue kTrueFourCC = SLANG_FOUR_CC('t', 'r', 'u', 'e');
    static const FourCC::RawValue kFalseFourCC = SLANG_FOUR_CC('f', 'a', 'l', 's');
    static const FourCC::RawValue kInt32FourCC = SLANG_FOUR_CC('i', '3', '2', ' ');
    static const FourCC::RawValue kUInt32FourCC = SLANG_FOUR_CC('u', '3', '2', ' ');
    static const FourCC::RawValue kFloat32FourCC = SLANG_FOUR_CC('f', '3', '2', ' ');
    static const FourCC::RawValue kInt64FourCC = SLANG_FOUR_CC('i', '6', '4', ' ');
    static const FourCC::RawValue kUInt64FourCC = SLANG_FOUR_CC('u', '6', '4', ' ');
    static const FourCC::RawValue kFloat64FourCC = SLANG_FOUR_CC('f', '6', '4', ' ');

    // The following codes are suitable for use when serializing
    // content that represents a logical file system.
    //
    static const FourCC::RawValue kDirectoryFourCC = SLANG_FOUR_CC('d', 'i', 'r', ' ');
    static const FourCC::RawValue kFileFourCC = SLANG_FOUR_CC('f', 'i', 'l', 'e');
    static const FourCC::RawValue kNameFourCC = SLANG_FOUR_CC('n', 'a', 'm', 'e');
    static const FourCC::RawValue kPathFourCC = SLANG_FOUR_CC('p', 'a', 't', 'h');
    static const FourCC::RawValue kDataFourCC = SLANG_FOUR_CC('d', 'a', 't', 'a');

    // TODO(tfoley): Figure out where to put all of these so that
    // they can be more usefully addressed.
    //
    static const FourCC::RawValue kMangledNameFourCC = SLANG_FOUR_CC('m', 'g', 'n', 'm');
    static const FourCC::RawValue kProfileFourCC = SLANG_FOUR_CC('p', 'r', 'o', 'f');


    struct ArrayHeader
    {
        uint32_t numEntries;
    };
};

struct SerialRiffUtil
{
    class ListResizer
    {
    public:
        virtual void* setSize(size_t newSize) = 0;
        SLANG_FORCE_INLINE size_t getTypeSize() const { return m_typeSize; }
        ListResizer(size_t typeSize)
            : m_typeSize(typeSize)
        {
        }

    protected:
        size_t m_typeSize;
    };

    template<typename T>
    class ListResizerForType : public ListResizer
    {
    public:
        typedef ListResizer Parent;

        SLANG_FORCE_INLINE ListResizerForType(List<T>& list)
            : Parent(sizeof(T)), m_list(list)
        {
        }

        virtual void* setSize(size_t newSize) SLANG_OVERRIDE
        {
            m_list.setCount(UInt(newSize));
            return (void*)m_list.begin();
        }

    protected:
        List<T>& m_list;
    };

    static Result writeArrayChunk(
        FourCC chunkId,
        const void* data,
        size_t numEntries,
        size_t typeSize,
        RIFF::BuildCursor& cursor);

    template<typename T>
    static Result writeArrayChunk(FourCC chunkId, const List<T>& array, RIFF::BuildCursor& cursor)
    {
        return writeArrayChunk(chunkId, array.begin(), size_t(array.getCount()), sizeof(T), cursor);
    }

    static Result readArrayChunk(RIFF::DataChunk const* dataChunk, ListResizer& listOut);

    template<typename T>
    static Result readArrayChunk(RIFF::DataChunk const* dataChunk, List<T>& arrayOut)
    {
        ListResizerForType<T> resizer(arrayOut);
        return readArrayChunk(dataChunk, resizer);
    }
};

} // namespace Slang

#endif
