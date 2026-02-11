//TEST:SIMPLE(filecheck=CHECK): -target spirv-assembly -entry main -stage vertex

// Test that TextureFootprint can be used as a parameter without crashing

//CHECK: OpEntryPoint Vertex {{%\w+}} "main"

[shader("vertex")]
void main(TextureFootprint<1> a) {
    // Just reference the parameter to avoid DCE
    let dummy = a.isSingleLevel;
}
