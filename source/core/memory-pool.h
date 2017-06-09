#ifndef CORE_LIB_MEMORY_POOL_H
#define CORE_LIB_MEMORY_POOL_H

#include "basic.h"
#include "int-set.h"

namespace CoreLib
{
	namespace Basic
	{
		struct MemoryBlockFields
		{
			unsigned int Occupied : 1;
			unsigned int Order : 31;
		};
		struct FreeListNode
		{
			FreeListNode * PrevPtr = nullptr, *NextPtr = nullptr;
		};
		class MemoryPool
		{
		private:
			static const int MaxLevels = 32;
			int blockSize = 0, log2BlockSize = 0;
			int numLevels = 0;
			int bytesAllocated = 0;
			int bytesWasted = 0;
			unsigned char * buffer = nullptr;
			FreeListNode * freeList[MaxLevels];
			IntSet used;
			int AllocBlock(int level);
			void FreeBlock(unsigned char * ptr, int level);
		public:
			MemoryPool(unsigned char * buffer, int log2BlockSize, int numBlocks);
			MemoryPool() = default;
			void Init(unsigned char * buffer, int log2BlockSize, int numBlocks);
			unsigned char * Alloc(int size);
			void Free(unsigned char * ptr, int size);
		};

		class OutofPoolMemoryException : public Exception
		{};

		template<typename T, int PoolSize>
		class ObjectPool
		{
			static const int ObjectSize = sizeof(T) < 8 ? 8 : sizeof(T);
		private:
			struct FreeList
			{
				FreeList* Next;
			};
			FreeList * freeList = nullptr;
			int allocPtr = 0;
			int poolSize = 0;
			void * buffer = 0;
			T * GetFreeObject()
			{
				if (freeList)
				{
					auto rs = (T*)freeList;
					freeList = freeList->Next;
					return rs;
				}
				return nullptr;
			}
		public:
			ObjectPool() 
			{
				freeList = nullptr;
				allocPtr = 0;
				buffer = malloc(PoolSize * ObjectSize);
			}

			void Close()
			{
				free(buffer);
			}
			
			void Free(T * obj)
			{
				auto newList = (FreeList*)obj;
				newList->Next = freeList;
				freeList = newList;
			}

			void * Buffer()
			{
				return buffer;
			}
			
			T * Alloc()
			{
				auto rs = GetFreeObject();
				if (!rs)
				{
					if (allocPtr < PoolSize)
					{
						rs = (T*)buffer + allocPtr;
						allocPtr++;
					}
				}
				if (!rs)
				{
					throw OutofPoolMemoryException();
				}
				return rs;
			}
		};
	};

#define USE_POOL_ALLOCATOR(T, PoolSize) \
	private:\
		static CoreLib::ObjectPool<T, PoolSize> _pool;\
	public:\
		void * operator new(std::size_t) { return _pool.Alloc(); } \
		void operator delete(void * ptr) {_pool.Free((T*)ptr); }\
		int GetObjectId() { return (int)(this -  (T*)_pool.Buffer()); }\
		static void ClosePool(); 
#define IMPL_POOL_ALLOCATOR(T, PoolSize) \
	CoreLib::ObjectPool<T, PoolSize> T::_pool;\
	void T::ClosePool() { _pool.Close(); }
		
}

#endif