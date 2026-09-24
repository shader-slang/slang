//TEST():SIMPLE(filecheck=CHECK):-entry computeMain -stage compute -line-directive-mode none -target hlsl -loop-inversion
// The same compile with debug info enabled (`-g`) must still invert the loops.
// Before the fix, the interleaved DebugLine/DebugValue insts defeated the
// structural predicates in slang-ir-loop-inversion.cpp and inversion was skipped.
//TEST():SIMPLE(filecheck=CHECKG):-entry computeMain -stage compute -line-directive-mode none -target hlsl -loop-inversion -g
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=OUT):-shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=OUT):-dx12 -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=OUT):-cpu -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=OUT):-vk -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=OUT):-cpu -shaderobj -output-using-type
// Guard runtime correctness of the debug-laden inverted IR (GPU-free): force
// loop inversion and enable debug info in the slang compile via -Xslang.
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=OUT):-cpu -shaderobj -output-using-type -Xslang -loop-inversion -Xslang -g

// Check that all the backends cope with the slightly unusual IR the loop inversion generated

// OUT: 180

// For all the below functions, verify that the body (adding to j and
// incrementing i) comes before any break. This verifies that the `break` has
// been moved to the end of the loop.

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

// A standard loop
// CHECK-LABEL: int a_{{.*}}()
// CHECK-NOT: break;
// CHECK: int {{.*}} = j_{{.*}} + [[i:i_[0-9]+]]
// CHECK: [[i]] + int(1);
// CHECK: if(
// CHECK: break;
// CHECK: return
//
// CHECKG-LABEL: int a_{{.*}}()
// CHECKG-NOT: break;
// CHECKG: = j_{{.*}} + [[ga:i_[0-9]+]]
// CHECKG: [[ga]] + int(1)
// CHECKG: if(
// CHECKG: break;
// CHECKG: return
int a()
{
    int j = 0;
    for(int i = 0; i < 10; ++i)
        j += i;
    return j;
}

// A vanilla while loop
// CHECK-LABEL: int b_{{.*}}()
// CHECK-NOT: break;
// CHECK: int {{.*}} = j_{{.*}} + [[i:i_[0-9]+]]
// CHECK: [[i]] + int(1);
// CHECK: if(
// CHECK: break;
// CHECK: return
//
// CHECKG-LABEL: int b_{{.*}}()
// CHECKG-NOT: break;
// CHECKG: = j_{{.*}} + [[gb:i_[0-9]+]]
// CHECKG: [[gb]] + int(1)
// CHECKG: if(
// CHECKG: break;
// CHECKG: return
int b()
{
    int j = 0;
    int i = 0;
    while(i < 10)
    {
        j += i;
        i++;
    }
    return j;
}

// A while loop with a break on the false branch
// CHECK-LABEL: int c_{{.*}}()
// CHECK-NOT: break;
// CHECK: int {{.*}} = j_{{.*}} + [[i:i_[0-9]+]]
// CHECK: [[i]] + int(1);
// CHECK: if(
// CHECK: break;
// CHECK: return
//
// CHECKG-LABEL: int c_{{.*}}()
// CHECKG-NOT: break;
// CHECKG: = j_{{.*}} + [[gc:i_[0-9]+]]
// CHECKG: [[gc]] + int(1)
// CHECKG: if(
// CHECKG: break;
// CHECKG: return
int c()
{
    int j = 0;
    int i = 0;
    do
    {
        if(i < 10)
            {}
        else
            break;
        j += i;
        i++;
    } while(true);
    return j;
}

// A while loop with a break on the true branch
// CHECK-LABEL: int d_{{.*}}()
// CHECK-NOT: break;
// CHECK: int {{.*}} = j_{{.*}} + [[i:i_[0-9]+]]
// CHECK: [[i]] + int(1);
// CHECK: if(
// CHECK: break;
// CHECK: return
//
// CHECKG-LABEL: int d_{{.*}}()
// CHECKG-NOT: break;
// CHECKG: = j_{{.*}} + [[gd:i_[0-9]+]]
// CHECKG: [[gd]] + int(1)
// CHECKG: if(
// CHECKG: break;
// CHECKG: return
int d()
{
    int j = 0;
    int i = 0;
    do
    {
        if(i >= 10)
            break;
        else
            {}
        j += i;
        i++;
    } while(true);
    return j;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    outputBuffer[dispatchThreadID.x] = a() + b() + c() + d();
}
