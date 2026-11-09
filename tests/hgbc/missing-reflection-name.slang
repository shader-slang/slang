//TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main
//TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main -obfuscate

// Interface declarations currently have no semantic type-expression decoration. A display
// hint is not permission to invent a lookup identity for one: reject the dispatch at its source.
interface Marker {}
[shader("compute")]
[numthreads(1, 1, 1)]
void probe<T>() {}
[host]
void main()
{
    dispatch(probe<Marker>, uint3(1, 1, 1))();
}

// CHECK: error[E56006]
// CHECK: dispatch type specialization arguments require reflection names
// CHECK: missing-reflection-name.slang:13:5
