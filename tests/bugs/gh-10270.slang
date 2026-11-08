//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry computeMain -stage compute
// CHECK-NOT: E38029
// CHECK: OpEntryPoint GLCompute

interface IDeleter<T>
{
}

struct NoDelete<T>: IDeleter<T>
{
}

// This should compile, as NoDelete<int> conforms to IDeleter<int>.
struct List<T, D : IDeleter<T> = NoDelete<T>>
{
}

[numthreads(1,1,1)]
void computeMain()
{
    List<int> l;
}
