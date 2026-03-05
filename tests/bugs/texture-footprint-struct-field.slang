//TEST:SIMPLE(filecheck=CHECK): -target spirv-assembly -entry main -stage vertex

// Test that a struct containing TextureFootprint as a field can be used as parameter

//CHECK: OpEntryPoint Vertex {{%\w+}} "main"

struct MyStruct {
    TextureFootprint<1> footprint;
    int someValue;
}

[shader("vertex")]
void main(MyStruct s) {
    let dummy = s.someValue;
}
