
#include <inttypes.h>
#include <math.h>
#include <inttypes.h>
#include <math.h>
template <typename T>
struct RWStructuredBuffer
{
    T& operator[](size_t index) const { return data[index]; }
    
    T* data;
    size_t count;
};




