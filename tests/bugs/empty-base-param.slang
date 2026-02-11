//TEST:SIMPLE(filecheck=CHECK): -target spirv-assembly -entry main -stage vertex

// Test simple struct with empty base
struct Base {}
struct Derive : Base { int y; }

//CHECK: OpEntryPoint Vertex {{%\w+}} "main"

[shader("vertex")]
void main(Derive a) {
    unused(a.y);
}
