// unit-test-free-list.cpp

#include "../../source/core/slang-free-list.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "test-context.h"

#include "../../source/core/slang-random-generator.h"
#include "../../source/core/list.h"

using namespace Slang;

static void freeListUnitTest()
{
    FreeList freeList;
    freeList.init(sizeof(int), sizeof(void*), 10);

    DefaultRandomGenerator randGen(0x24343);

    List<int*> allocs;

    for (int i = 0; i < 1000; i++)
    {
        const int numAlloc = randGen.nextInt32UpTo(20);
        
        for (int j = 0; j < numAlloc; j++)
        {
            int* ptr = (int*)freeList.allocate();
            *ptr = i;
            allocs.Add(ptr);
        }

        int numDealloc = randGen.nextInt32UpTo(19);
        numDealloc = int(allocs.Count()) < numDealloc ? int(allocs.Count()) : numDealloc;

        for (int j = 0; j < numDealloc; j++)
        {
            const int index = randGen.nextInt32UpTo(int(allocs.Count()));

            int* alloc = allocs[index];

            SLANG_CHECK(*alloc <= i);
            SLANG_CHECK(*alloc >= 0);

            freeList.deallocate(alloc);

            allocs.FastRemoveAt(index);
        }
    }
}

SLANG_UNIT_TEST("FreeList", freeListUnitTest);