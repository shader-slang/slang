//DISABLE_TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -shaderobj

/* Test here is to try and associate a value with a type

This doesn't work - it produces dynamic dispatch code, that is uncompilable because cases 
are missing.

.slang(24): error :  control reaches end of non-void function [-Wreturn-type]
dxc: note : }
dxc: note : ^

*/

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

enum class Enum
{
    A, B
};

interface IGetE
{
	static Enum getE();    
};

struct A : IGetE
{
    static Enum getE() { return Enum::A; }
};

struct B : IGetE
{
    static Enum getE() { return Enum::B; }
};

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{    
    int index = dispatchThreadID.x;
   
    // Err.. even though IGetE doesn't require an instanciation, not clear how to set it. So lets try with instanciation
    B b;
    IGetE g = b;

    let e = g.getE();
    
    outputBuffer[dispatchThreadID.x] = int(e);
}