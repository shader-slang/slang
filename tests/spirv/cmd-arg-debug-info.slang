//TEST:SIMPLE(filecheck=CHECK): -target spirv -g3 -emit-spirv-directly -profile sm_6_6+spirv_1_5
// CHECK:  OpString "-target spirv {{.*}} -capability spirv_1_5

// The `DebugEntryPoint` command line records the requested debug level and every descriptive option.
// The `OpString "-target spirv` anchor binds these checks to the reconstructed command line: at -g3
// the shader source is also embedded as an OpString, but that one begins with the file text, not
// `-target spirv`. The `-SAME` chain follows the serializer's emission order (option-set insertion
// order); if that order changes, re-pin the sequence rather than dropping the anchoring.
//TEST:SIMPLE(filecheck=CHECK_ALL): -target spirv -lang slang -profile glsl_460 -g3 -gdwarf -O0 -minimum-slang-optimization -line-directive-mode glsl -emit-spirv-directly
// CHECK_ALL:  OpString "-target spirv
// The reported level is the requested -g3. The two -NOT directives bracket the -SAME chain so a
// spurious -g2 is rejected both before the first serialized option and after the last one.
// CHECK_ALL-NOT: -g2
// CHECK_ALL-SAME: -lang slang
// CHECK_ALL-SAME: -g3
// CHECK_ALL-SAME: -gdwarf
// CHECK_ALL-SAME: -O0
// CHECK_ALL-SAME: -minimum-slang-optimization
// CHECK_ALL-SAME: -line-directive-mode glsl
// CHECK_ALL-SAME: -profile glsl_460
// CHECK_ALL-NOT: -g2

// The Vulkan binding-shift options round-trip to the flags that produced them: the "all-spaces"
// form is `-fvk-<kind>-shift <shift> all` (there is no `-fvk-all-shift` flag), and `-fvk-bind-globals`
// keeps a space before its operands.
//TEST:SIMPLE(filecheck=CHECK_VK): -target spirv -g3 -fvk-b-shift 4 all -fvk-bind-globals 2 3 -emit-spirv-directly
// CHECK_VK:  OpString "-target spirv
// CHECK_VK-SAME: -fvk-b-shift 4 all
// CHECK_VK-SAME: -fvk-bind-globals 2 3

// Downstream args round-trip with `-X` (not `-x`), and the coverage counter width round-trips to the
// CLI bit count (32/64) even though it is stored internally as a byte width.
//TEST:SIMPLE(filecheck=CHECK_XW): -target spirv -g3 -Xspirv-opt -O -trace-coverage -trace-coverage-counter-width 32 -emit-spirv-directly
// CHECK_XW:  OpString "-target spirv
// CHECK_XW-SAME: -Xspirv-opt -O
// CHECK_XW-SAME: -trace-coverage-counter-width 32

// The target's option set is re-inherited from the session several times during slangc option
// processing, so a `-X` argument must still appear exactly once in the command line reconstructed
// from the target program's options.
//TEST:SIMPLE(filecheck=CHECK_XN): -target spirv -g3 -Xnvrtc --fmad=false -emit-spirv-directly
// CHECK_XN:  OpString "-target spirv
// CHECK_XN-SAME: -Xnvrtc --fmad=false
// CHECK_XN-NOT: -Xnvrtc
// CHECK_XN-SAME: -stage compute

//TEST:SIMPLE(filecheck=CHECK_XD): -target spirv -g3 -Xnvrtc -D -Xnvrtc FOO=1 -Xnvrtc -D -Xnvrtc BAR=2 -emit-spirv-directly
// CHECK_XD:  OpString "-target spirv
// CHECK_XD-SAME: -Xnvrtc -D -Xnvrtc FOO=1 -Xnvrtc -D -Xnvrtc BAR=2
// CHECK_XD-SAME: -stage compute

[numthreads(1,1,1)]
void computeMain()
{}
