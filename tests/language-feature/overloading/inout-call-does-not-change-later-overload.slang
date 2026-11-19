// An earlier call that converts an argument for an `inout` parameter does not
// change which overload a later by-value call with the same types selects.
//
// Consider the call `g<N>(a)` below. Converting the `vector<int, N>` variable
// `a` to the `inout vector<uint, N>` parameter costs extra, because the
// conversion goes through a temporary that is written back after the call.
// The later call `f<N>(make<N>())` converts a `vector<int, N>` value without
// binding its storage, so it must not see that extra cost when it compares
// the `uint` and `float` overloads (GitHub issue #13394).

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

float f<let N : int>(vector<uint, N> v) { return 1.0; }
float f<let N : int>(vector<float, N> v) { return 2.0; }
void g<let N : int>(inout vector<uint, N> v) {}
vector<int, N> make<let N : int>() { return vector<int, N>(0); }

float test<let N : int>()
{
    vector<int, N> a = vector<int, N>(0);
    g<N>(a);
    return f<N>(make<N>());
}

[numthreads(1, 1, 1)]
void computeMain()
{
    // CHECK: 1
    outputBuffer[0] = test<3>();
}
