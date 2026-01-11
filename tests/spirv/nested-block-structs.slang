// The compiler should notice that View is already decorated as `Block` or
// `BufferBlock` due to main_view; shadow_views would have an array of View,
// which would count as nesting `Block`s and is not valid SPIR-V.
//TEST:SIMPLE(filecheck=CHECK): -target spirv -emit-spirv-directly
struct View {
    float foo;
}

ParameterBlock<View> main_view;
// The compiler needs to duplicate the View_std140 type for main_view and not
// annotate the original one with `Block`.
//CHECK-NOT: OpDecorate %View_std140 Block
ParameterBlock<View[3]> shadow_views;

[shader("fragment")]
float main() {
    return main_view.foo + shadow_views[0].foo;
}
