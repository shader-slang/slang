//TEST:SIMPLE(filecheck=CHECK_1):-stage compute -entry computeMain -target spirv -DT1
//TEST:SIMPLE(filecheck=CHECK_2):-stage compute -entry computeMain -target spirv -DT2
//TEST:SIMPLE(filecheck=CHECK_3):-stage compute -entry computeMain -target spirv -DT3
//TEST:SIMPLE(filecheck=CHECK_4):-stage compute -entry computeMain -target spirv -DT4

// Tests for invalid use of `const` with Ptr/T*
// Due to bad syntax breaking the parser, it is more robust to use disjoint tests with
// #define's.
cbuffer Globals
{
    int* ptr;
}

[numthreads(1, 1, 1)]
void computeMain(int id : SV_DispatchThreadID)
{
    // disallowed syntax with modifier `const`
#ifdef T1
    // CHECK_1: ([[# @LINE+1]]): error
    int const* ptr1 = ptr;
#endif

#ifdef T2
    // CHECK_2: ([[# @LINE+1]]): error
    int* const ptr2 = ptr;
#endif

#ifdef T3
    // CHECK_3: ([[# @LINE+1]]): error 20017
    const int* ptr3 = ptr;
    // CHECK_3: ([[# @LINE+1]]): error 20018
    Ptr<const int> ptr4 = ptr;
#endif

#ifdef T4
    // CHECK_4: OpEntryPoint
    // CHECK_4-NOT: error
    const Ptr<int> ptr5 = ptr;
#endif
}