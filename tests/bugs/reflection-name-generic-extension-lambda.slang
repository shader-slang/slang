//TEST:SIMPLE(filecheck=CHECK): -target spirv -entry main
//TEST:SIMPLE(filecheck=CHECK): -target spirv -entry main -obfuscate
//CHECK: OpEntryPoint GLCompute

// A method's generic parameters must not be confused with those of its extension when
// emitting the reflection name of the nested lambda. No host graph or autodiff is involved.
public struct Box<T> { T value; }

public extension<T> Box<T>
{
    public U map<U, F : IFunc<U, T>>(F transform)
    {
        let apply = (T input) => transform(input);
        return apply(value);
    }
}

RWStructuredBuffer<float> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    Box<float> box;
    box.value = 2.0;
    output[0] = box.map((float value) => value * 3.0);
}
