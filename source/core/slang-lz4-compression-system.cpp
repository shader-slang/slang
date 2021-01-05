#include "slang-lz4-compression-system.h"

#include "../../slang-com-helper.h"
#include "../../slang-com-ptr.h"

#include "slang-blob.h"

#include "../../external/lz4/lib/lz4.h"

namespace Slang
{

// Allocate static const storage for the various interface IDs that the Slang API needs to expose
static const Guid IID_ISlangUnknown = SLANG_UUID_ISlangUnknown;
static const Guid IID_ICompressionSystem = SLANG_UUID_ICompressionSystem;

class LZ4CompressionSystemImpl : public RefObject, public ICompressionSystem 
{
public:
    // ISlangUnknown 
    // override ref counting, as singleton
    SLANG_IUNKNOWN_QUERY_INTERFACE
    SLANG_NO_THROW uint32_t SLANG_MCALL addRef() SLANG_OVERRIDE { return 1; }
    SLANG_NO_THROW uint32_t SLANG_MCALL release() SLANG_OVERRIDE { return 1; }

    // ICompressionSystem
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL compress(const CompressionStyle* style, const void* src, size_t srcSizeInBytes, size_t compressedCapacity, void* outCompressed, size_t* outCompressedSizeInBytes) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL compressToBlob(const CompressionStyle* style, const void* src, size_t srcSizeInBytes, ISlangBlob** outBlob) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW size_t SLANG_MCALL calcCompressedBound(size_t srcSizeInBytes) SLANG_OVERRIDE;
    virtual SLANG_NO_THROW SlangResult SLANG_MCALL decompress(const void* compressed, size_t compressedSizeInBytes, size_t dstCapacity, void* outDecompressed, size_t* outDecompressedSize) SLANG_OVERRIDE;

protected:

    ICompressionSystem* getInterface(const Guid& guid);
};

ICompressionSystem* LZ4CompressionSystemImpl::getInterface(const Guid& guid)
{
    return (guid == IID_ISlangUnknown || guid == IID_ICompressionSystem) ? static_cast<ICompressionSystem*>(this) : nullptr;
}

SlangResult LZ4CompressionSystemImpl::compress(const CompressionStyle* style, const void* src, size_t srcSizeInBytes, size_t compressedCapacity, void* outCompressed, size_t* outCompressedSizeInBytes)
{
    SLANG_UNUSED(style);

    const int compressedSize = LZ4_compress_default((const char*)src, (char*)outCompressed, int(srcSizeInBytes), int(compressedCapacity));
    *outCompressedSizeInBytes = size_t(compressedSize);
    return SLANG_OK;
}

SlangResult LZ4CompressionSystemImpl::compressToBlob(const CompressionStyle* style, const void* src, size_t srcSizeInBytes, ISlangBlob** outBlob)
{
    SLANG_UNUSED(style);
    const size_t compressedBound = calcCompressedBound(srcSizeInBytes);

    ScopedAllocation alloc;
    void* compressedData = alloc.allocate(compressedBound);

    size_t compressedSize;
    SLANG_RETURN_ON_FAIL(compress(style, src, srcSizeInBytes, compressedBound, compressedData, &compressedSize));
    alloc.reallocate(compressedSize);

    auto blob = RawBlob::moveCreate(alloc);

    *outBlob = blob.detach();
    return SLANG_OK;
}

size_t LZ4CompressionSystemImpl::calcCompressedBound(size_t srcSizeInBytes)
{
    return LZ4_compressBound(int(srcSizeInBytes));
}

SlangResult LZ4CompressionSystemImpl::decompress(const void* compressed, size_t compressedSizeInBytes, size_t dstCapacity, void* outDecompressed, size_t* outDecompressedSize)
{
    const int decompressedSize = LZ4_decompress_safe((const char*)compressed, (char*)outDecompressed, int(compressedSizeInBytes), int(dstCapacity));

    *outDecompressedSize = decompressedSize;
    return SLANG_OK;
}

/* static */ICompressionSystem* LZ4CompressionSystem::getSingleton()
{
    static LZ4CompressionSystemImpl impl;
    return &impl;
}

} // namespace Slang
