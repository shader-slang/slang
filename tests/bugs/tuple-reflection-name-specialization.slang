//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain
//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain -obfuscate
//TEST:SIMPLE(filecheck=SPIRV): -target spirv -entry computeMain

// CUDA: __global__
// SPIRV: OpEntryPoint GLCompute

// A tuple is one generic type argument, not an expanded pack. Its reflection name must
// fold before tuple lowering replaces its source type with a generated struct. Otherwise
// FormatGenericArgReflectionNameList survives into CUDA emission and triggers E99999.
struct Values<T>
{
    T value;
    __init(T value) { this.value = value; }
}

extension<each T> Values<Tuple<T>>
{
    Tuple<expand Values<each T>> unzip()
    {
        let elements = value;
        return makeTuple(expand Values<each T>(each elements));
    }
}

RWStructuredBuffer<uint> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    let pair = Values<Tuple<uint, uint>>(makeTuple(tid.x + 1u, tid.x * 2u));
    let parts = pair.unzip();
    output[tid.x] = parts._0.value + parts._1.value;
}
