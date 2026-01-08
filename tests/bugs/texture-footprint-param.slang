//TEST:SIMPLE(filecheck=CHECK): -target spirv -entry main -stage vertex

// Test that TextureFootprint can be used as a parameter without crashing

//CHECK: define void @main

[shader("vertex")]
void main(TextureFootprint<1> a) {
    // Just reference the parameter to avoid DCE
    var dummy = a.isSingleLevel;
}
