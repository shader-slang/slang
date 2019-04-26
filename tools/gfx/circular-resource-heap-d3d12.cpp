#include "circular-resource-heap-d3d12.h"

namespace gfx {
using namespace Slang;

D3D12CircularResourceHeap::D3D12CircularResourceHeap():
	m_fence(nullptr),
	m_device(nullptr),
	m_blockFreeList(sizeof(Block), SLANG_ALIGN_OF(Block), 16),
	m_blocks(nullptr)
{
	m_back.m_block = nullptr;
	m_back.m_position = nullptr;
	m_front.m_block = nullptr;
	m_front.m_position = nullptr;
}

D3D12CircularResourceHeap::~D3D12CircularResourceHeap()
{
	_freeBlockListResources(m_blocks);
}

void D3D12CircularResourceHeap::_freeBlockListResources(const Block* start)
{
	if (start)
	{
        const Block* block = start;
        do
        {
            ID3D12Resource* resource = block->m_resource;

            resource->Unmap(0, nullptr);
            resource->Release();

            // Next in list
            block = block->m_next;

        } while (block != start);
	}
}

Result D3D12CircularResourceHeap::init(ID3D12Device* device, const Desc& desc, D3D12CounterFence* fence)
{
	assert(m_blocks == nullptr);
	assert(desc.m_blockSize > 0);

	m_fence = fence;
	m_desc = desc;
	m_device = device;

	return SLANG_OK;
}

void D3D12CircularResourceHeap::addSync(uint64_t signalValue)
{
	assert(signalValue == m_fence->getCurrentValue());
	PendingEntry entry;
	entry.m_completedValue = signalValue;
	entry.m_cursor = m_front;
	m_pendingQueue.add(entry);
}

void D3D12CircularResourceHeap::updateCompleted()
{
	const uint64_t completedValue = m_fence->getCompletedValue();

#if 0
	while (m_pendingQueue.getCount() != 0)
	{
		const PendingEntry& entry = m_pendingQueue[0];
		if (entry.m_completedValue <= completedValue)
		{
			m_back = entry.m_cursor;
			m_pendingQueue.removeAt(0);
		}
		else
		{
			break;
		}
	}
#else
	// A more efficient implementation is m_pendingQueue is implemented as a vector like type
	const Index size = m_pendingQueue.getCount();
	Index end = 0;
	while (end < size && m_pendingQueue[end].m_completedValue <= completedValue)
	{
		end++;
	}

	if (end > 0)
	{
		// Set the back position
		m_back = m_pendingQueue[end - 1].m_cursor;
		if (end == size)
		{
			m_pendingQueue.clear();
		}
		else
		{
			m_pendingQueue.removeRange(0, size);
		}
	}
#endif
}

D3D12CircularResourceHeap::Block* D3D12CircularResourceHeap::_newBlock()
{
	D3D12_RESOURCE_DESC desc;

	desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	desc.Alignment = 0;
	desc.Width = m_desc.m_blockSize;
	desc.Height = 1;
	desc.DepthOrArraySize = 1;
	desc.MipLevels = 1;
	desc.Format = DXGI_FORMAT_UNKNOWN;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	desc.Flags = D3D12_RESOURCE_FLAG_NONE;

	ComPtr<ID3D12Resource> resource;
	Result res = m_device->CreateCommittedResource(&m_desc.m_heapProperties, m_desc.m_heapFlags, &desc, m_desc.m_initialState, nullptr, IID_PPV_ARGS(resource.writeRef()));
	if (SLANG_FAILED(res))
	{
		assert(!"Resource allocation failed");
		return nullptr;
	}

	uint8_t* data = nullptr;
	if (m_desc.m_heapProperties.Type == D3D12_HEAP_TYPE_READBACK)
	{
	}
	else
	{
		// Map it, and keep it mapped
		resource->Map(0, nullptr, (void**)&data);
	}

	// We have no blocks -> so lets allocate the first
	Block* block = (Block*)m_blockFreeList.allocate();
	block->m_next = nullptr;

	block->m_resource = resource.detach();
	block->m_start = data;
	return block;
}

D3D12CircularResourceHeap::Cursor D3D12CircularResourceHeap::allocate(size_t size, size_t alignment)
{
	const size_t blockSize = getBlockSize();

	assert(size <= blockSize);

	// If nothing is allocated add the first block
	if (m_blocks == nullptr)
	{
		Block* block = _newBlock();
		if (!block)
		{
			Cursor cursor = {};
			return cursor;
		}
		m_blocks = block;
		// Make circular
		block->m_next = block;

		// Point front and back to same position, as currently it is all free
		m_back = { block, block->m_start };
		m_front = m_back;
	}

	// If front and back are in the same block then front MUST be ahead of back (as that defined as
	// an invariant and is required for block insertion to be possible
	Block* block = m_front.m_block;

	// Check the invariant
	assert(block != m_back.m_block || m_front.m_position >= m_back.m_position);

	{
		uint8_t* cur = (uint8_t*)((size_t(m_front.m_position) + alignment - 1) & ~(alignment - 1));
		// Does the the allocation fit?
		if (cur + size <= block->m_start + blockSize)
		{
			// It fits
			// Move the front forward
			m_front.m_position = cur + size;
			Cursor cursor = { block, cur };
			return cursor;
		}
	}

	// Okay I can't fit into current block...

	// If the next block contains front, we need to add a block, else we can use that block
	if (block->m_next == m_back.m_block)
	{
		Block* newBlock = _newBlock();
		// Insert into the list
		newBlock->m_next = block->m_next;
		block->m_next = newBlock;
	}

	// Use the block we are going to add to
	block = block->m_next;
	uint8_t* cur = (uint8_t*)((size_t(block->m_start) + alignment - 1) & ~(alignment - 1));
	// Does the the allocation fit?
	if (cur + size > block->m_start + blockSize)
	{
		assert(!"Couldn't fit into a free block(!) Alignment breaks it?");
		Cursor cursor = {};
		return cursor;
	}
	// It fits
	// Move the front forward
	m_front.m_block = block;
	m_front.m_position = cur + size;
	Cursor cursor = { block, cur };
	return cursor;
}

} // namespace gfx
