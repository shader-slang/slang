#line 1 "slang-cuda-prelude.h"

#define SLANG_PRELUDE_ASSERT(x) 

template <typename T, size_t SIZE>
struct FixedArray
{
    __device__ const T& operator[](size_t index) const { SLANG_PRELUDE_ASSERT(index < SIZE); return m_data[index]; }
    __device__ T& operator[](size_t index) { SLANG_PRELUDE_ASSERT(index < SIZE); return m_data[index]; }
    
    T m_data[SIZE];
};