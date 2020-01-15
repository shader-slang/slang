
// For now we'll disable any asserts in this prelude
#define SLANG_PRELUDE_ASSERT(x) 

template <typename T, size_t SIZE>
struct FixedArray
{
    __device__ const T& operator[](size_t index) const { SLANG_PRELUDE_ASSERT(index < SIZE); return m_data[index]; }
    __device__ T& operator[](size_t index) { SLANG_PRELUDE_ASSERT(index < SIZE); return m_data[index]; }
    
    T m_data[SIZE];
};

/* Type that defines the uniform entry point params. The actual content of this type is dependent on the entry point parameters, and can be
found via reflection or defined such that it matches the shader appropriately.
*/
struct UniformEntryPointParams;
struct UniformState;