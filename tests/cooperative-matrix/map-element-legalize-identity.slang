//TEST:SIMPLE(filecheck=CHECK): -target spirv -dump-ir-after legalizeResourceTypes

// A cooperative-matrix MapElement functor flows through type legalization as
// an `IRCoopMatMapElementIFunc`: the functor's capture struct is legal input
// that legalization must carry through unchanged, and this compile-only test
// pins that shape — after `legalizeResourceTypes` the element-functor
// instruction is still present with its capture-struct and lambda operands
// intact, so the deferred-removal scheme around the rebuild paths cannot
// silently drop or degrade the identity case that user shaders actually
// reach. (The rebuild branches themselves are exercised elsewhere: the
// none-flavor rebuild by tests/cooperative-matrix/flash-attention.slang,
// and the pair/tuple diagnostic by map-element-resource-capture.slang.)

StructuredBuffer<int> input1;

RWStructuredBuffer<int32_t> outputBuffer;

using namespace linalg;

typealias CoopMatType = CoopMat<int, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse.MatrixAccumulator>;

[shader("compute")]
[numthreads(32, 1, 1)]
void computeMain()
{
    let stride = 16;
    CoopMatType mat1 = CoopMatType.Load<CoopMatMatrixLayout.RowMajor>(input1, 0, stride);
    CoopMatType result = mat1.MapElement((uint r, uint c, int x)=>x*2+1);
    result.Store<CoopMatMatrixLayout.RowMajor>(outputBuffer, 0, stride);
}

//CHECK: ### AFTER legalizeResourceTypes:
//CHECK: CoopMatMapElementIFunc
