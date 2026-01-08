//TEST:SIMPLE(filecheck=CHECK): -target spirv -entry main -stage compute

// Test that TextureFootprint can be used as a return type without crashing

//CHECK: define void @main

[shader("compute")]
[numthreads(1,1,1)]
TextureFootprint<1> main() {
    TextureFootprint<1> result;
    return result;
}
