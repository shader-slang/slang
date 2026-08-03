// ir-type-alignment-attr.slang

// Verify that lowering an AST type layout to IR emits `IRTypeAlignmentAttr`
// (mnemonic `TypeAlignment`) carrying the byte alignment alongside the
// existing `size` attribute, and that the dump for a `float[N]` array in a
// constant buffer contains the expected array size and alignment values. The alignment operand
// comes first and the layout-unit operand is omitted for the `Uniform` (bytes)
// default, so the attribute prints with a single operand.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute -dump-ir -o -

// slang-test puts the compiler's exit status on the first line of the buffer FileCheck reads,
// ahead of the captured output — so this must stay the first directive of the prefix. The checks
// below all match `-dump-ir` output, which is complete before the output artifact is written, so
// a failure in that write (or any later emit-time diagnostic) would leave every pattern matched
// and the test green. The exit status has to be pinned separately.
// No `-g` here, so the dump carries no echoed copy of this file and the plain `0` is safe; see
// subscript-accessor.slang for the spelling required when source is embedded.
//CHECK: result code = 0

struct S
{
    float a;
    float data[4];
    float4 b;
}

ConstantBuffer<S> cb;
RWStructuredBuffer<float> outp;

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    outp[0] = cb.a + cb.data[0] + cb.b.x;
}

// A byte-unit (`Uniform`, kind 8) type layout carries a `TypeAlignment` alongside its `size`
// when its alignment is greater than 1 (alignment 1 is the absent-attribute
// default and is not emitted). The single-operand form confirms the layout-unit
// operand is omitted for the `Uniform` default. `TypeAlignment` attributes are
// hoistable and deduplicated, so each distinct value is one shared inst; these
// checks assert the expected values are present in the dump, not which layout
// references them.

// Alignment 16 (the `float4` field and the array both force 16) and alignment 4
// (the `float` field) are both generated.
//CHECK-DAG: TypeAlignment(16 : Int)
//CHECK-DAG: TypeAlignment(4 : Int)
// The `float` field is 4 bytes.
//CHECK-DAG: size(8 : Int, 4 : Int)

// The array `float data[4]` layout: under constant-buffer rules the array is
// aligned to 16 while its element type layout stays 4 bytes / aligned 4. The
// array's own alignment (16) is the element's in-array alignment, so the element
// stride is align(4, 16) = 16 and the whole array occupies 4 * 16 = 64 bytes.
// `getElementStrideInBytes()` combines the element size (4) and the array
// alignment (16) to yield that 16-byte stride.
//CHECK-DAG: arrayTypeLayout({{.*}})
//CHECK-DAG: size(8 : Int, 64 : Int)

// The identity alignment of 1 is never emitted — it is exactly what an absent
// attribute encodes, so a resource-only layout (e.g. the `RWStructuredBuffer`,
// `size(9 : Int, ...)`) carries no `TypeAlignment` at all.
//CHECK-NOT: TypeAlignment(1 : Int)
