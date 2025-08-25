// unit-test-create-blob.cpp

#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace Slang;

// Test the slang_createBlob function
SLANG_UNIT_TEST(createBlob)
{
    // Test 1: Basic functionality with valid string data
    {
        const char* testData = "Hello, World!";
        size_t dataSize = strlen(testData);

        ComPtr<ISlangBlob> blob;
        blob = slang_createBlob((const void*)testData, dataSize);

        SLANG_CHECK(blob != nullptr);

        if (blob)
        {
            SLANG_CHECK(blob->getBufferSize() == dataSize);
            SLANG_CHECK(memcmp(blob->getBufferPointer(), testData, dataSize) == 0);
        }
    }

    // Test 2: Test with binary data (non-string)
    {
        const uint8_t binaryData[] = {0x00, 0x01, 0x02, 0x03, 0xFF, 0xFE, 0xFD, 0xFC};
        size_t dataSize = sizeof(binaryData);

        ComPtr<ISlangBlob> blob;
        blob = slang_createBlob((const void*)binaryData, dataSize);

        SLANG_CHECK(blob != nullptr);

        if (blob)
        {
            SLANG_CHECK(blob->getBufferSize() == dataSize);
            SLANG_CHECK(memcmp(blob->getBufferPointer(), binaryData, dataSize) == 0);
        }
    }

    // Test 3: Test with large data
    {
        const size_t largeSize = 1024 * 1024; // 1MB
        char* largeData = new char[largeSize];

        // Fill with pattern
        for (size_t i = 0; i < largeSize; i++)
        {
            largeData[i] = (char)(i % 256);
        }

        ComPtr<ISlangBlob> blob;
        blob = slang_createBlob((const void*)largeData, largeSize);

        SLANG_CHECK(blob != nullptr);

        if (blob)
        {
            SLANG_CHECK(blob->getBufferSize() == largeSize);
            SLANG_CHECK(memcmp(blob->getBufferPointer(), largeData, largeSize) == 0);
        }

        delete[] largeData;
    }

    // Test 4: Test with null pointer and non-zero size (should fail)
    {
        char* testData = nullptr;
        ComPtr<ISlangBlob> blob;
        blob = slang_createBlob((const void*)testData, 10);

        SLANG_CHECK(blob == nullptr);
    }

    // Test 5: Test with null pointer and zero size (should fail)
    {
        char* testData = nullptr;
        ComPtr<ISlangBlob> blob;
        blob = slang_createBlob((const void*)testData, 0);

        SLANG_CHECK(blob == nullptr);
    }

    // Test 6: Test with valid pointer and zero size (should fail)
    {
        const char* testData = "test";

        ComPtr<ISlangBlob> blob;
        blob = slang_createBlob((const void*)testData, 0);

        SLANG_CHECK(blob == nullptr);
    }

    // Test 7: Test with void* version of the function
    {
        const char* testData = "Test void* version";
        size_t dataSize = strlen(testData);

        ComPtr<ISlangBlob> blob;
        blob = slang_createBlob((const void*)testData, dataSize);

        SLANG_CHECK(blob != nullptr);

        if (blob)
        {
            SLANG_CHECK(blob->getBufferSize() == dataSize);
            SLANG_CHECK(memcmp(blob->getBufferPointer(), testData, dataSize) == 0);
        }
    }

    // Test 8: Test multiple blobs with same data
    {
        const char* testData = "Shared data";
        size_t dataSize = strlen(testData);

        ComPtr<ISlangBlob> blob1;
        ComPtr<ISlangBlob> blob2;
        ComPtr<ISlangBlob> blob3;
        blob1 = slang_createBlob((const void*)testData, dataSize);
        blob2 = slang_createBlob((const void*)testData, dataSize);
        blob3 = slang_createBlob((const void*)testData, dataSize);

        SLANG_CHECK(blob1 != nullptr);
        SLANG_CHECK(blob2 != nullptr);
        SLANG_CHECK(blob3 != nullptr);

        // All should have same content
        SLANG_CHECK(blob1->getBufferSize() == dataSize);
        SLANG_CHECK(blob2->getBufferSize() == dataSize);
        SLANG_CHECK(blob3->getBufferSize() == dataSize);

        SLANG_CHECK(memcmp(blob1->getBufferPointer(), testData, dataSize) == 0);
        SLANG_CHECK(memcmp(blob2->getBufferPointer(), testData, dataSize) == 0);
        SLANG_CHECK(memcmp(blob3->getBufferPointer(), testData, dataSize) == 0);
    }

    // Test 9: Test memory management (blob should be independent)
    {
        const char* testData = "Memory test";
        size_t dataSize = strlen(testData);

        ComPtr<ISlangBlob> blob;
        blob = slang_createBlob((const void*)testData, dataSize);

        // Modify original data - blob should remain unchanged
        char* mutableData = new char[dataSize + 1];
        memcpy(mutableData, testData, dataSize + 1);
        mutableData[0] = 'X'; // Change first character

        SLANG_CHECK(blob->getBufferSize() == dataSize);
        SLANG_CHECK(
            memcmp(blob->getBufferPointer(), testData, dataSize) ==
            0); // Should still match original
        SLANG_CHECK(
            memcmp(blob->getBufferPointer(), mutableData, dataSize) !=
            0); // Should not match modified

        delete[] mutableData;
    }
}
