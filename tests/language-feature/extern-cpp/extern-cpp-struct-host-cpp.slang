//TEST:SIMPLE(filecheck=CHECK): -target host-cpp -entry main -stage compute -line-directive-mode none

// The fields of an `__extern_cpp` struct keep their source names in host C++
// output too (https://github.com/shader-slang/slang/issues/14028), which is
// also the intermediate source of executable and host shared-library builds.

//CHECK: struct Result
//CHECK-NEXT: {
//CHECK-NEXT: float value;
//CHECK-NEXT: int32_t count;
//CHECK-NEXT: };

public __extern_cpp struct Result
{
    public float value;
    public int count;
}

public export __extern_cpp Result makeResult(float value)
{
    Result result;
    result.value = value;
    result.count = 2;
    return result;
}

[numthreads(1, 1, 1)]
void main() {}
