//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -spirv
// CHECK-NOT: error 38029

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
