//TEST:SIMPLE(filecheck=CHECK): -target spirv -entry main -stage vertex

// Test simple struct with empty base
struct Base {}
struct Derive : Base { int y; }

//CHECK: define void @main

[shader("vertex")]
void main(Derive a) {
    unused(a.y);
}
