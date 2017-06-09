#include "memory-pool.h"
#include <cassert>

namespace CoreLib
{
	namespace Basic
	{
		MemoryPool::MemoryPool(unsigned char * pBuffer, int pLog2BlockSize, int numBlocks)
		{
			Init(pBuffer, pLog2BlockSize, numBlocks);
		}
		void MemoryPool::Init(unsigned char * pBuffer, int pLog2BlockSize, int numBlocks)
		{
			assert(pLog2BlockSize >= 1 && pLog2BlockSize <= 30);
			assert(numBlocks >= 4);
			buffer = pBuffer;
			blockSize = 1 << pLog2BlockSize;
			log2BlockSize = pLog2BlockSize;
			numLevels = Math::Log2Floor(numBlocks);
			freeList[0] = (FreeListNode*)buffer;
			freeList[0]->NextPtr = nullptr;
			freeList[0]->PrevPtr = nullptr;
			used.SetMax(1 << (numLevels));
			for (int i = 1; i < MaxLevels; i++)
			{
				freeList[i] = nullptr;
			}
		}
		int MemoryPool::AllocBlock(int level)
		{
			if (level < 0)
				return -1;
			if (freeList[level] == nullptr)
			{
				auto largeBlockAddr = AllocBlock(level - 1);
				if (largeBlockAddr != -1)
				{
					auto block1 = (FreeListNode*)(buffer + ((largeBlockAddr ^ (1 << (numLevels - level))) << log2BlockSize));
					block1->NextPtr = nullptr;
					block1->PrevPtr = nullptr;
					freeList[level] = block1;

					int blockIndex = (1 << level) + (largeBlockAddr >> (numLevels-level)) - 1;
					used.Add(blockIndex);
					return largeBlockAddr;
				}
				else
					return -1;
			}
			else
			{
				auto node = freeList[level];
				if (node->NextPtr)
				{
					node->NextPtr->PrevPtr = node->PrevPtr;
				}
				freeList[level] = freeList[level]->NextPtr;
				int rs = (int)((unsigned char *)node - buffer) >> log2BlockSize;
				int blockIndex = (1 << level) + (rs >> (numLevels - level)) - 1;
				used.Add(blockIndex);
				return rs;
			}
		}
		unsigned char * MemoryPool::Alloc(int size)
		{
			if (size == 0)
				return nullptr;
			int originalSize = size;
			if (size < blockSize)
				size = blockSize;
			int order = numLevels - (Math::Log2Ceil(size) - log2BlockSize);
			assert(order >= 0 && order < MaxLevels);

			bytesAllocated += (1 << ((numLevels-order) + log2BlockSize));
			bytesWasted += (1 << ((numLevels - order) + log2BlockSize)) - originalSize;

			int blockId = AllocBlock(order);
			if (blockId != -1)
				return buffer + (blockId << log2BlockSize);
			else
				return nullptr;
		}
		void MemoryPool::FreeBlock(unsigned char * ptr, int level)
		{
			int indexInLevel = (int)(ptr - buffer) >> (numLevels - level + log2BlockSize);
			int blockIndex = (1 << level) + indexInLevel - 1;
			assert(used.Contains(blockIndex));
			int buddyIndex = (blockIndex & 1) ? blockIndex + 1 : blockIndex - 1;
			used.Remove(blockIndex);
			if (level > 0 && !used.Contains(buddyIndex))
			{
				auto buddyPtr = (FreeListNode *)(buffer + ((((int)(ptr - buffer) >> log2BlockSize) ^ (1 << (numLevels - level))) << log2BlockSize));
				if (buddyPtr->PrevPtr)
				{
					buddyPtr->PrevPtr->NextPtr = buddyPtr->NextPtr;
				}
				if (buddyPtr->NextPtr)
				{
					buddyPtr->NextPtr->PrevPtr = buddyPtr->PrevPtr;
				}
				if (freeList[level] == buddyPtr)
				{
					freeList[level] = buddyPtr->NextPtr;
				}
				// recursively free parent blocks
				auto parentPtr = Math::Min(buddyPtr, (FreeListNode*)ptr);
				if (level > 0)
					FreeBlock((unsigned char*)parentPtr, level - 1);
			}
			else
			{
				// insert to freelist
				auto freeNode = (FreeListNode *)ptr;
				freeNode->NextPtr = freeList[level];
				freeNode->PrevPtr = nullptr;
				if (freeList[level])
					freeList[level]->PrevPtr = freeNode;
				freeList[level] = freeNode;
			}
		}
		void MemoryPool::Free(unsigned char * ptr, int size)
		{
			if (size == 0)
				return;
			int originalSize = size;
			if (size < blockSize)
				size = blockSize;
			int level = numLevels - (Math::Log2Ceil(size) - log2BlockSize);
			bytesAllocated -= (1 << ((numLevels-level) + log2BlockSize));
			bytesWasted -= (1 << ((numLevels - level) + log2BlockSize)) - originalSize;
			FreeBlock(ptr, level);
		}
	}
}

