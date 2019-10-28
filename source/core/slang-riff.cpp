#include "slang-riff.h"

#include "../../slang-com-helper.h"

#include "slang-hex-dump-util.h"

namespace Slang
{

/* static */int64_t RiffUtil::calcChunkTotalSize(const RiffHeader& chunk)
{
    int64_t size = chunk.m_size + sizeof(RiffHeader);
    return (size + 3) & ~int64_t(3);
}

/* static */SlangResult RiffUtil::skip(const RiffHeader& chunk, Stream* stream, int64_t* remainingBytesInOut)
{
    int64_t chunkSize = calcChunkTotalSize(chunk);
    if (remainingBytesInOut)
    {
        *remainingBytesInOut -= chunkSize;
    }

    // Skip the payload (we don't need to skip the Chunk because that was already read
    stream->seek(SeekOrigin::Current, chunkSize - sizeof(RiffHeader));
    return SLANG_OK;
}


/* static */SlangResult RiffUtil::readChunk(Stream* stream, RiffHeader& outChunk)
{
    try
    {
        stream->read(&outChunk, sizeof(RiffHeader));
    }
    catch (IOException&)
    {
    	return SLANG_FAIL;
    }

    // TODO(JS): Could handle endianness issues here...

    return SLANG_OK;
}


/* static */SlangResult RiffUtil::writeData(const RiffHeader* header, size_t headerSize, const void* payload, size_t payloadSize, Stream* out)
{
    SLANG_ASSERT(uint64_t(payloadSize) <= uint64_t(0xfffffffff));
    SLANG_ASSERT(headerSize >= sizeof(RiffHeader));
    SLANG_ASSERT((headerSize & 3) == 0);

    // TODO(JS): Could handle endianness here

    RiffHeader chunk;
    chunk.m_type = header->m_type;
    chunk.m_size = uint32_t(headerSize - sizeof(RiffHeader) + payloadSize);

    try
    {
        // The chunk
        out->write(&chunk, sizeof(RiffHeader));

        // Remainder of header
        if (headerSize > sizeof(RiffHeader))
        {
            // The rest of the header
            out->write(header + 1, headerSize - sizeof(RiffHeader));
        }

        // Write the payload
        out->write(payload, payloadSize);

        // The riff spec requires all chunks are 4 byte aligned (even if size is not)
        size_t remaining = payloadSize & 3;
        if (remaining)
        {
            uint8_t end[4] = { 0, 0, 0, 0};
            out->write(end, 4 - remaining);
        }
    }
    catch (IOException&)
    {
        return SLANG_FAIL;
    }

    return SLANG_OK;
}

/* static */SlangResult RiffUtil::readPayload(Stream* stream, size_t size, void* outData, size_t& outReadSize)
{
    outReadSize = 0;
    try
    {
        stream->read(outData, size);
        const size_t readSize = (size + 3) & ~size_t(3);

        // Skip to the alignment
        if (readSize > size)
        {
            stream->seek(SeekOrigin::Current, readSize - size);
        }
        outReadSize = readSize;
    }
    catch (IOException&)
    {
        return SLANG_FAIL;
    }

    return SLANG_OK;
}

/* static */SlangResult RiffUtil::readData(Stream* stream, RiffHeader* outHeader, size_t headerSize, List<uint8_t>& data)
{
    RiffHeader chunk;
    SLANG_RETURN_ON_FAIL(readChunk(stream, chunk));
    if (chunk.m_size < headerSize)
    {
        return SLANG_FAIL;
    }

    *outHeader = chunk;

    try
    {
        // Read the header
        if (headerSize > sizeof(RiffHeader))
        {
            stream->read(outHeader + 1, headerSize - sizeof(RiffHeader));
        }
    }
    catch (IOException&)
    {
        return SLANG_FAIL;
    }

    const size_t payloadSize = chunk.m_size - (headerSize - sizeof(RiffHeader));
    size_t readSize;
    data.setCount(payloadSize);
    return readPayload(stream, payloadSize, data.getBuffer(), readSize);
}

/* static */SlangResult RiffUtil::writeListHeader(FourCC containerType, FourCC subType, size_t totalSize, Stream* out)
{
    RiffListHeader header;

    // Container can only be list or riff - they are the only available options
    SLANG_ASSERT(containerType == RiffFourCC::kList || containerType == RiffFourCC::kRiff);
    if (!(containerType == RiffFourCC::kList || containerType == RiffFourCC::kRiff))
    {
        return SLANG_FAIL;
    }

    // Size of header of Riff container
    totalSize += sizeof(RiffListHeader) - sizeof(RiffHeader);

    header.chunk.m_type = containerType;
    header.chunk.m_size = uint32_t(totalSize);
    header.subType = subType;
    
    try
    {
        out->write(&header, sizeof(header));
    }
    catch (IOException&)
    {
    	return SLANG_FAIL;
    }

    return SLANG_OK;
}

/* static */SlangResult RiffUtil::writeList(FourCC containerType, FourCC subType, const SubChunk* subChunks, size_t subChunkCount, Stream* out)
{
    size_t totalSize = 0;

    for (size_t i = 0; i < subChunkCount; ++i)
    {
        const auto& subChunk = subChunks[i];

        RiffHeader chunk;
        chunk.m_type = subChunk.chunkType;
        SLANG_ASSERT(subChunk.dataSize <= 0xffffffff);
        chunk.m_size = uint32_t(subChunk.dataSize);

        totalSize += calcChunkTotalSize(chunk);
    }

    // Write the container header
    SLANG_RETURN_ON_FAIL(writeListHeader(containerType, subType, totalSize, out));

    for (size_t i = 0; i < subChunkCount; ++i)
    {
        const auto& subChunk = subChunks[i];

        RiffHeader chunk;
        chunk.m_type = subChunk.chunkType;
        SLANG_ASSERT(subChunk.dataSize <= 0xffffffff);
        chunk.m_size = uint32_t(subChunk.dataSize);

        // Write the chunk
        SLANG_RETURN_ON_FAIL(writeData(&chunk, sizeof(chunk), subChunk.data, subChunk.dataSize, out));
    }

    return SLANG_OK;
}

/* static */SlangResult RiffUtil::readHeader(Stream* stream, RiffListHeader& outHeader)
{
    // Need to read the chunk header
    SLANG_RETURN_ON_FAIL(readChunk(stream, outHeader.chunk));
    outHeader.subType = 0;

    if (isListType(outHeader.chunk.m_type))
    {
        // Read the sub type
        try
        {
            stream->read(&outHeader.subType, sizeof(RiffListHeader) - sizeof(RiffHeader));
        }
        catch (const IOException&)
        {
            return SLANG_FAIL;
        }
    }

    return SLANG_OK;
}

/* static */SlangResult RiffUtil::readList(Stream* stream, RiffListHeader& outHeader, MemoryArena& ioArena, List<SubChunk>& outChunks)
{
    outChunks.clear();
    // Need to read the chunk header
    SLANG_RETURN_ON_FAIL(readHeader(stream, outHeader));

    // Must be a riff container!
    if (!isListType(outHeader.chunk.m_type))
    {
        return SLANG_FAIL;
    }

    // Okay we have the header. We now need to read all of the contained chunks. Making sure we don't read past the end
    size_t remainingSize = size_t(outHeader.chunk.m_size) - (sizeof(RiffListHeader) - sizeof(RiffHeader));
    while (remainingSize >= sizeof(RiffHeader))
    {
        // Read the contained chunk
        RiffHeader chunk;
        SLANG_RETURN_ON_FAIL(readChunk(stream, chunk));

        remainingSize -= sizeof(RiffHeader);
        if (remainingSize < 0 || chunk.m_size > remainingSize)
        {
            return SLANG_FAIL;
        }
        
        // Allocate the space for the payload
        void* data = ioArena.allocate(chunk.m_size);

        // Read the payload
        try
        {
            stream->read(data, chunk.m_size);
        }
        catch (const IOException&)
        {
            return SLANG_FAIL;
        }

        // Add to the list
        SubChunk subChunk;
        subChunk.chunkType = chunk.m_type;
        subChunk.data = data;
        subChunk.dataSize = chunk.m_size;

        // Decrease the remaining size
        remainingSize -= chunk.m_size;
    }

    // Remaining size should be 0 for well formed riff
    SLANG_ASSERT(remainingSize == 0);

    return SLANG_OK;
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!! RiffContainer::Chunk !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


SlangResult RiffContainer::Chunk::visit(Visitor* visitor)
{
    switch (m_kind)
    {
        case Kind::Data:
        {
            return visitor->handleData(static_cast<DataChunk*>(this));
        }
        case Kind::List:
        {
            auto list = static_cast<ListChunk*>(this);
            SLANG_RETURN_ON_FAIL(visitor->enterList(list));

            Chunk* chunk = list->m_containedChunks;
            while (chunk)
            {
                SLANG_RETURN_ON_FAIL(chunk->visit(visitor));

                chunk = chunk->m_next;
            }

            SLANG_RETURN_ON_FAIL(visitor->leaveList(list));
            return SLANG_OK;
        }
        default: return SLANG_FAIL;
    }
}

SlangResult RiffContainer::Chunk::visitPostOrder(VisitorCallback callback, void* data)
{
    switch (m_kind)
    {
        case Kind::Data:
        {
            return callback(this, data);
        }
        case Kind::List:
        {
            auto list = static_cast<ListChunk*>(this);

            // Do the contents first
            Chunk* chunk = list->m_containedChunks;
            while (chunk)
            {
                SLANG_RETURN_ON_FAIL(chunk->visitPostOrder(callback, data));
                chunk = chunk->m_next;
            }

            // Then the list node (so a post order)
            SLANG_RETURN_ON_FAIL(callback(this, data));
            return SLANG_OK;
        }
        default: return SLANG_FAIL;
    }
}

size_t RiffContainer::Chunk::calcPayloadSize()
{
    switch (m_kind)
    {
        case Kind::Data:        return static_cast<DataChunk*>(this)->calcPayloadSize();
        case Kind::List:        return static_cast<ListChunk*>(this)->calcPayloadSize();
        default: return 0;
    }
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! RiffContainer !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

int RiffContainer::DataChunk::calcHash() const
{
    int hash = 0;

    Data* data = m_dataList;
    while (data)
    {
        // This is a little contrived (in that we don't use the function GetHashCode), but the
        // reason to be careful is we want the same result however many Data blocks there are.
        const char* buffer = (const char*)data->getPayload();
        const size_t size = data->getSize();

        for (size_t i = 0; i < size; ++i)
        {
            hash = int(buffer[i]) + (hash << 6) + (hash << 16) - hash;
        }

        data = data->m_next;
    }

    return hash;
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! RiffContainer !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

RiffContainer::RiffContainer() :
    m_arena(4096)
{
    m_rootList = nullptr;
    m_listChunk = nullptr;
    m_dataChunk = nullptr;
}

void RiffContainer::reset()
{
    m_arena.reset();

    m_rootList = nullptr;
    m_listChunk = nullptr;
    m_dataChunk = nullptr;
}

RiffContainer::ListChunk* RiffContainer::_newListChunk(FourCC subType)
{
    SLANG_ASSERT(!RiffUtil::isListType(subType));

    ListChunk* chunk = (ListChunk*)m_arena.allocate(sizeof(ListChunk));
    chunk->init(subType);
    return chunk;
}

RiffContainer::DataChunk* RiffContainer::_newDataChunk(FourCC type)
{
    SLANG_ASSERT(!RiffUtil::isListType(type));

    DataChunk* chunk = (DataChunk*)m_arena.allocate(sizeof(DataChunk));
    chunk->init(type);
    return chunk;
}

void RiffContainer::_addChunk(Chunk* chunk)
{
    if (m_listChunk)
    {
        chunk->m_parent = m_listChunk;
        Chunk*& next = m_listChunk->m_endChunk ? m_listChunk->m_endChunk->m_next : m_listChunk->m_containedChunks;
        SLANG_ASSERT(next == nullptr);
        next = chunk;
        m_listChunk->m_endChunk = chunk;
    }
}

void RiffContainer::startChunk(Chunk::Kind kind, FourCC fourCC)
{
    SLANG_ASSERT(m_listChunk || m_rootList == nullptr);

    switch (kind)
    {
        case Chunk::Kind::Data:
        {
            // We can only start a data chunk if we are in a container, and we can't already be in data chunk
            SLANG_ASSERT(m_listChunk && m_dataChunk == nullptr);

            DataChunk* chunk = _newDataChunk(fourCC);
            _addChunk(chunk);
            m_dataChunk = chunk;
            break;
        }
        case Chunk::Kind::List:
        {
            // We can't be in a data chunk
            SLANG_ASSERT(m_dataChunk == nullptr);

            ListChunk* list = _newListChunk(fourCC);

            // If this is the first, make it the root
            if (!m_rootList)
            {
                m_rootList = list;
            }

            _addChunk(list);

            m_listChunk = list;
            break;
        }
    }
}

void RiffContainer::endChunk()
{
    size_t chunkPayloadSize;

    // The chunk we are popping 
    Chunk* chunk = nullptr;

    ListChunk* parent;
    if (m_dataChunk)
    {
        chunk = m_dataChunk;

        parent = m_dataChunk->m_parent;
        chunkPayloadSize = m_dataChunk->m_payloadSize;
        
        m_dataChunk = nullptr;
    }
    else
    {
        chunk = m_listChunk;

        SLANG_ASSERT(m_listChunk && m_dataChunk == nullptr);
        parent = m_listChunk->m_parent;
        chunkPayloadSize = m_listChunk->m_payloadSize;
    }

    m_listChunk = parent;

    if (parent)
    {
        // Fix the size taking into account the 4 bytes alignment requirement
        chunkPayloadSize = (chunkPayloadSize + 3) & ~size_t(3);
        // Update the parents size
        parent->m_payloadSize += sizeof(RiffHeader) + chunkPayloadSize;
    }

    // Check it's size seems ok
    SLANG_ASSERT(isChunkOk(chunk));
}

RiffContainer::Data* RiffContainer::addData(size_t size)
{
    // We must be in a chunk
    SLANG_ASSERT(m_dataChunk);

    // Add current chunks data
    m_dataChunk->m_payloadSize += size;

    Data* data = (Data*)m_arena.allocate(sizeof(Data) + size);

    data->m_next = nullptr;
    data->m_size = size;

    Data*& next = m_dataChunk->m_endData ? m_dataChunk->m_endData->m_next : m_dataChunk->m_dataList;
    SLANG_ASSERT(next == nullptr);

    // Add to linked list
    next = data;
    // Make this the new end
    m_dataChunk->m_endData = data;
    return data;
}

void RiffContainer::write(const void* inData, size_t size)
{
    auto data = addData(size);
    ::memcpy(data->getPayload(), inData, size);
}

static SlangResult _isChunkOk(RiffContainer::Chunk* chunk, void* data)
{
    SLANG_UNUSED(data);
    return chunk->calcPayloadSize() == chunk->m_payloadSize ? SLANG_OK : SLANG_FAIL;
}

/* static */bool RiffContainer::isChunkOk(Chunk* chunk)
{
    return SLANG_SUCCEEDED(chunk->visitPostOrder(&_isChunkOk, nullptr));
}

static SlangResult _calcAndSetSize(RiffContainer::Chunk* chunk, void* data)
{
    SLANG_UNUSED(data);
    chunk->m_payloadSize = chunk->calcPayloadSize();
    return SLANG_OK;
}

/* static */void RiffContainer::calcAndSetSize(Chunk* chunk)
{
    chunk->visitPostOrder(&_calcAndSetSize, nullptr);
}

namespace { // anonymous

struct DumpVisitor : public RiffContainer::Visitor
{
    typedef RiffContainer::Chunk Chunk;
    typedef RiffContainer::ListChunk ListChunk;
    typedef RiffContainer::DataChunk DataChunk;

  
    // Visitor
    virtual SlangResult enterList(ListChunk* list) SLANG_OVERRIDE
    {
        _dumpIndent();
        // If it's the root it's 'riff'
        _dumpRiffType(list == m_rootChunk ? RiffFourCC::kRiff : RiffFourCC::kList);
        m_writer.put(" ");
        _dumpRiffType(list->m_subType);
        m_writer.put("\n");
        m_indent++;
        return SLANG_OK;
    }
    virtual SlangResult handleData(DataChunk* data) SLANG_OVERRIDE
    {
        _dumpIndent();
        // Write out the name
        _dumpRiffType(data->m_type);
        m_writer.put(" ");


        int hash = data->calcHash();

        // We don't know in general what the contents is or means... but we can display a hash
        HexDumpUtil::dump(uint32_t(hash), m_writer.getWriter());
        m_writer.put(" ");

        m_writer.put("\n");
        return SLANG_OK;
    }
    virtual SlangResult leaveList(ListChunk* list) SLANG_OVERRIDE
    {
        SLANG_UNUSED(list);
        m_indent--;
        return SLANG_OK;
    }

    DumpVisitor(WriterHelper writer, Chunk* rootChunk):
        m_writer(writer),
        m_indent(0),
        m_rootChunk(rootChunk)
    {
    }

    void _dumpIndent()
    {
        for (int i = 0; i < m_indent; ++i)
        {
            m_writer.put("  ");
        }
    }
    void _dumpRiffType(FourCC fourCC)
    {
        char c[5];
        for (int i = 0; i < 4; ++i)
        {
            c[i] = char(fourCC);
            fourCC >>= 8;
        }
        c[4] = 0;
        m_writer.put(c);
    }

    Chunk* m_rootChunk;

    int m_indent;
    WriterHelper m_writer;
};

}

/* static */void RiffContainer::dump(Chunk* chunk, WriterHelper writer)
{
    DumpVisitor visitor(writer, chunk);
    chunk->visit(&visitor);
}

/* static */SlangResult RiffContainer::write(ListChunk* container, bool isRoot, Stream* stream)
{
    RiffListHeader containerHeader;

    containerHeader.chunk.m_type = isRoot ? RiffFourCC::kRiff : RiffFourCC::kList;
    containerHeader.chunk.m_size = uint32_t(sizeof(RiffListHeader) - sizeof(RiffHeader) + container->m_payloadSize);
    containerHeader.subType = container->m_subType;

    try
    {
        // Write the header
        stream->write(&containerHeader, sizeof(containerHeader));

        // Write the contained chunks
        Chunk* chunk = container->m_containedChunks;
        while (chunk)
        {
            if (auto containerChunk = as<ListChunk>(chunk))
            {
                // It's a container
                SLANG_RETURN_ON_FAIL(write(containerChunk, false, stream));
            }
            else if (auto dataChunk = as<DataChunk>(chunk))
            {
                // Must be a regular chunk with data

                RiffHeader chunkHeader;
                chunkHeader.m_type = dataChunk->m_type;
                chunkHeader.m_size = uint32_t(dataChunk->m_payloadSize);

                stream->write(&chunkHeader, sizeof(chunkHeader));

                Data* data = dataChunk->m_dataList;
                while (data)
                {
                    stream->write(data->getPayload(), data->getSize());

                    // Next but of data
                    data = data->m_next;
                }
            }

            // Next
            chunk = chunk->m_next;
        }
    }
    catch (const IOException&)
    {
    	return SLANG_FAIL;
    }

    return SLANG_OK;
}

/* static */SlangResult RiffContainer::read(Stream* stream, RiffContainer& outContainer)
{
    typedef RiffContainer::ScopeChunk ScopeChunk;
    typedef RiffContainer::ScopeChunk ScopeContainer;
    outContainer.reset();

    size_t remaining;
    {
        RiffListHeader header;
        SLANG_RETURN_ON_FAIL(RiffUtil::readHeader(stream, header));
        if (!RiffUtil::isListType(header.chunk.m_type))
        {
            return SLANG_FAIL;
        }

        remaining = header.chunk.m_size - (sizeof(RiffListHeader) - sizeof(RiffHeader));

        outContainer.startChunk(Chunk::Kind::List, header.subType);
    }

    List<size_t> remainingStack;
    while (true)
    {
        if (remaining == 0)
        {
            // If it's a container then we pop container
            outContainer.endChunk();
            if (remainingStack.getCount() <= 0)
            {
                break;
            }

            remaining = remainingStack.getLast();
            remainingStack.removeLast();
        }
        else
        {
            RiffListHeader header;
            SLANG_RETURN_ON_FAIL(RiffUtil::readHeader(stream, header));

            if (header.chunk.m_type == RiffFourCC::kList)
            {
                // Subtract the size of this chunk from remaining
                remaining -= sizeof(RiffListHeader);
                remaining -= (header.chunk.m_size + 3) & ~size_t(3);

                // Push it, for when we hit the end
                remainingStack.add(remaining);

                // Work out how much remains in this container
                remaining -= (header.chunk.m_size - (sizeof(RiffListHeader) - sizeof(RiffHeader)) +  3) & ~size_t(3);

                // Start a container
                outContainer.startChunk(Chunk::Kind::List, header.subType);
            }
            else
            {
                ScopeChunk scopeChunk(&outContainer, Chunk::Kind::Data, header.chunk.m_type);
                Data* data = outContainer.addData(header.chunk.m_size);

                size_t readSize;
                SLANG_RETURN_ON_FAIL(RiffUtil::readPayload(stream, header.chunk.m_size, data->getPayload(), readSize));

                // Correct remaining
                remaining -= sizeof(RiffHeader) + readSize;
            }
        }
    }

    return outContainer.isFullyConstructed() ? SLANG_OK : SLANG_FAIL;
}

}
