//TEST:SIMPLE(filecheck=CHECK): -target spirv -entry main -stage vertex

// Test that a struct containing TextureFootprint as a field can be used as parameter

//CHECK: define void @main

struct MyStruct {
    TextureFootprint<1> footprint;
    int someValue;
}

[shader("vertex")]
void main(MyStruct s) {
    var dummy = s.someValue;
}
