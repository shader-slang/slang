1. Common Functions
GetRenderTargetSampleCount(): Returns the number of samples for a render target.
GetRenderTargetSamplePosition(): Gets the sampling position (x, y) for a given sample index.
clamp(): Clamps a value between a minimum and maximum.
isfinite(): Returns true if value is finite.
isinf(): Returns true if the value is infinite.
isnan(): Returns true if the value is NaN.
lerp(): Linear interpolation between two values.
saturate(): Clamps the input to the range [0,1].
sign(): Returns the sign of a value (-1, 0, 1).
smoothstep(): Smooth Hermite interpolation between 0 and 1.
step(): Returns 0 if less than a threshold and 1 otherwise.

1. Common Interfaces
IArray interface
IBufferDataLayout interface
IFunc interface
IMutatingFunc interface
IRWArray interface

1. Common Types
Array struct
ConstRef struct
InOut struct
InputPatch struct
Optional struct
Out struct
OutputIndices struct
OutputPatch struct
OutputPrimitives struct
OutputVertices struct
Ref struct
TensorView struct
TorchTensor struct
Tuple struct
matrix struct
vector struct

1. Texture Types
FeedbackTexture2D struct
FeedbackTexture2DArray struct
RWTexture1D struct
RWTexture1DArray struct
RWTexture2D struct
RWTexture2DArray struct
RWTexture2DMS struct
RWTexture2DMSArray struct
RWTexture3D struct
RasterizerOrderedTexture1D struct
RasterizerOrderedTexture1DArray struct
RasterizerOrderedTexture2D struct
RasterizerOrderedTexture2DArray struct
RasterizerOrderedTexture3D struct
Texture1D struct
Texture1DArray struct
Texture2D struct
Texture2DArray struct
Texture2DMS struct
Texture2DMSArray struct
Texture3D struct
TextureCube struct
TextureCubeArray struct
TextureFootprint struct

1. Sampler Types
RWSampler1D struct
RWSampler1DArray struct
RWSampler2D struct
RWSampler2DArray struct
RWSampler2DMS struct
RWSampler2DMSArray struct
RWSampler3D struct
RasterizerOrderedSampler1D struct
RasterizerOrderedSampler1DArray struct
RasterizerOrderedSampler2D struct
RasterizerOrderedSampler2DArray struct
RasterizerOrderedSampler3D struct
Sampler1D struct
Sampler1DArray struct
Sampler2D struct
Sampler2DArray struct
Sampler2DMS struct
Sampler2DMSArray struct
Sampler3D struct
SamplerComparisonState struct
SamplerCube struct
SamplerCubeArray struct
SamplerState struct

1. Uniform Buffer Types
AppendStructuredBuffer struct
ByteAddressBuffer struct
ConstantBuffer struct
ConsumeStructuredBuffer struct
ParameterBlock struct
RWByteAddressBuffer struct
RWStructuredBuffer struct
RasterizerOrderedByteAddressBuffer struct
RasterizerOrderedStructuredBuffer struct
StructuredBuffer struct
TextureBuffer struct

1. Pointer Types
ConstBufferPointer struct
NullPtr struct
Ptr struct

1. Relational Functions
all(): True if all components are non-zero.
any(): True if any component is non-zero.

1. Bitwise Functions
and(): Component-wise logical AND.
countbits(): Counts the number of bits set to 1.
firstbithigh(): Count leading zeros.
firstbitlow(): Count trailing zeros.
msad4(): Compares a 4-byte reference value and an 8-byte source value, accumulating a vector of 4 sums based on the masked sum of absolute differences.
or(): Component-wise logical OR.
reversebits(): Reverses the order of bits in an integer.

1. Trigonometry Functions
acos(): Inverse cosine.
acosh(): Inverse hyperbolic cosine.
asin(): Inverse sine.
asinh(): Inverse hyperbolic sine.
atan(): Inverse tangent.
atan2(): Inverse two parameter tangent.
atanh(): Inverse hyperbolic tangent.
cos(): Cosine.
cosh(): Hyperbolic cosine.
degrees(): Converts radians to degrees.
radians(): Converts degrees to radians.
sin(): Sine.
sincos(): Simultaneous sine and cosine.
sinh(): Hyperbolic sine.
tan(): Tangent.
tanh(): Hyperbolic tangent.

1. Exponential Functions
exp(): Exponential (e^x).
exp10(): Base 10 exponential (10^x).
exp2(): Base 2 exponential (2^x).
log(): Natural logarithm.
log10(): Base 10 logarithm.
log2(): Base 2 logarithm.
pow(): Power function.
powr(): 
rsqrt(): Reciprocal square root.
sqrt(): Square root.

1. Matrix Functions
determinant(): Returns the determinant of a matrix.
mul(): Multiplication.
transpose(): Transposes a matrix.

1. Math Functions
abs(): Absolute value.
ceil(): Round up to the nearest integer.
fabs(): Absolute value.
floor(): Round down to the nearest integer.
fma(): Fused multiply-add.
fmax(): Returns maximum of two floats.
fmin(): Returns the minimum of two floats.
fmod(): Returns the floating point remainder.
frac(): Returns the fractional part of a number.
frexp(): Splits a floating point into mantissa and exponent.
ldexp(): Combines a mantissa and exponent.
mad(): Multiply and add operation.
max(): Returns the larger of two values.
min(): Returns the smaller of two values.
modf(): Splits value into integral and fractional parts.
rcp(): Computes the reciprocal of a value.
round(): Rounds to the nearest whole number.
trunc(): Truncates the fractional part.

1. Math Interfaces
IArithmetic interface
ICompareable interface
IFloat interface
IInteger interface
ILogical interface
IRangedValue interface

1. Geometric Functions
cross(): Cross product of two 3D vectors.
distance(): Distance between two points.
dot(): Dot product of two vectors.
faceforward(): Adjusts the direction of a normal.
length(): Length of a vector.
lit(): Computes lighting coefficients.
normalize(): Normalize a vector.
reflect(): Reflects a vector around a normal.
refract(): Computes the refraction vector.

1. Conversion Functions
asdouble(): Converts two uint32_t into a double.
asfloat(): Converts int32_t or uint32_t to float.
asfloat16(): Converts uint16_t to float16_t.
asint(): Converts float or uint32_t to int32_t.
asint16(): Converts float16_t type to int16_t.
asuint(): Converts double, float or int32_t to uint32_t.
asuint16(): Converts the float16 to uint16_t.
bit_cast(): Converts a type to another type as bit data.
f16tof32(): Converts a float16 stored in the low-half of a uint to a float.
f32tof16(): Converts a float to a float16 type.

1. Atomic Functions
InterlockedAdd(): Atomic addition.
InterlockedAnd(): Atomic bitwise AND.
InterlockedCompareExchange(): Compares and swaps values atomically.
InterlockedCompareExchangeFloatBitwise(): Same as InterlockedCompareExchange() but for float value.
InterlockedCompareStore(): Atomically compares the destination to the comparison value and, if they are identical, overwrites the destination with the input value.
InterlockedCompareStoreFloatBitwise(): Same as InterlockedCompareStore() but for float value.
InterlockedExchange(): Assigns a value to a destination and returns the original value atomically.
InterlockedMax(): Atomic maximum.
InterlockedMin(): Atomic minimum.
InterlockedOr(): Atomic bitwise OR.
InterlockedXor(): Atomic bitwise XOR.

1. Atomic Interfaces
IAtomicable interface
IArithmeticAtomicable interface
IBitAtomicable interface

1. Atomic Types
Atomic struct
AtomicAdd struct

1. Synchronization Functions
AllMemoryBarrier(): Ensures all threads in a group are synchronized.
AllMemoryBarrierWithGroupSync(): Ensures all threads in a group are synchronized and all memory accesses are completed.
DeviceMemoryBarrier(): Ensures all device memory accesses are completed.
DeviceMemoryBarrierWithGroupSync(): Ensures all device memory accesses are completed and all threads in the group have reached this call.
GroupMemoryBarrier(): Ensures all group shared accesses are completed.
GroupMemoryBarrierWithGroupSync(): Ensures all group shared accesses are completed and all threads in the group have reached this call.
beginInvocationInterlock(): Mark beginning of "interlocked" operations.
endInvocationInterlock(): Mark end of "interlocked" operations.

1. Wave Functions
AllMemoryBarrierWithWaveMaskSync(): Same to AllMemoryBarrierWithWaveSync() but limits to the threads marked in the mask.
AllMemoryBarrierWithWaveSync(): Similar to AllMemoryBarrierWithGroupSync() but limits to the current wave.
GroupMemoryBarrierWithWaveMaskSync(): Same to GroupMemoryBarrierWithWaveSync() but limits to the threads marked in the mask.
GroupMemoryBarrierWithWaveSync(): SImilar to GroupMemoryBarrierWithGroupSync() but limits to the current wave.
QuadReadAcrossDiagonal(): Returns the specified local value read from the diagonally opposite lane in the quad.
QuadReadAcrossX(): Returns the specified local value read from the other lane in the quad in the X direction.
QuadReadAcrossY(): Returns the specified local value read from the other lane in the quad in the Y direction.
QuadReadLaneAt(): Returns the specified source value from the lane identified by the lane ID within the current quad.
WaveActiveAllEqual(): Returns true if the expression is the same for every active lane in the current wave.
WaveActiveAllTrue(): Returns true if all lanes in the wave evaluate the expression to true.
WaveActiveAnyTrue(): Returns true if any lane in the wave evaluates the expression to true.
WaveActiveBallot(): Returns a bitmask where each bit represents whether the corresponding lane evaluates the expression to true.
WaveActiveBitAnd(): Computes the bitwise AND of the expression across all active lanes in the wave.
WaveActiveBitOr(): Computes the bitwise OR of the expression across all active lanes in the wave.
WaveActiveBitXor(): Computes the bitwise XOR of the expression across all active lanes in the wave.
WaveActiveCountBits(): Returns the number of bits set to 1 in the bitmask returned by WaveActiveBallot.
WaveActiveMax(): Computes the maximum value of the expression across all active lanes in the wave.
WaveActiveMin(): Computes the minimum value of the expression across all active lanes in the wave.
WaveActiveProduct(): Computes the product of the expression across all active lanes in the wave.
WaveActiveSum(): Computes the sum of the expression across all active lanes in the wave.
WaveBroadcastLaneAt():
WaveGetActiveMask(): 
WaveGetActiveMulti():
WaveGetConvergedMask():
WaveGetConvergedMulti():
WaveGetLaneCount(): Returns the number of lanes in a wave on the current architecture.
WaveGetLaneIndex(): Returns the index of the current lane within the current wave.
WaveIsFirstLane(): Returns true if the current lane is the first active lane in the wave.
WaveMaskBroadcastLaneAt():
WaveMaskIsFirstLane(): 
WaveMaskAllEqual():
WaveMaskAllTrue(): 
WaveMaskAnyTrue(): 
WaveMaskBallot():
WaveMaskBitAnd(): 
WaveMaskBitOr():
WaveMaskBitXor():
WaveMaskCountBits():
WaveMaskMatch():
WaveMaskMax():
WaveMaskMin():
WaveMaskPrefixBitAnd():
WaveMaskPrefixBitOr():
WaveMaskPrefixBitXor():
WaveMaskPrefixCountBits():
WaveMaskPrefixProduct():
WaveMaskPrefixSum():
WaveMaskProduct():
WaveMaskReadLaneAt(): 
WaveMaskReadLaneFirst():
WaveMaskShuffle():
WaveMaskSum():
WaveMatch():
WaveMultiPrefixBitAnd():
WaveMultiPrefixBitOr():
WaveMultiPrefixBitXor():
WaveMultiPrefixCountBits():
WaveMultiPrefixProduct():
WaveMultiPrefixSum():
WavePrefixCountBits(): Computes the prefix sum of the number of bits set to 1 across all active lanes in the wave.
WavePrefixProduct(): Computes the prefix product of the expression across all active lanes in the wave.
WavePrefixSum(): Computes the prefix sum of the expression across all active lanes in the wave.
WaveReadLaneAt(): Returns the value of the expression for the specified lane index within the wave1.
WaveReadLaneFirst(): Returns the value of the expression for the active lane with the smallest index in the wave.
WaveShuffle():

1. Auto-diff Functions
isDifferentialNull(): existential check for null differential type
detach(): Detach and set derivatives to zero.
diffPair(): Constructs a `DifferentialPair` value from a primal and differential values.
updatePrimal(): Changes the primal value in a DifferentialPair type.
updateDiff(): Changes the diff value in a DifferentialPair type.
updatePair(): Changes both primal and diff values in a DifferentialPair type.

1. Auto-diff Interfaces
IDifferentiable interface
IDifferentiablePtrType interface
IDifferentiableMutatingFunc
IDifferentiableFunc
IDiffTensorWrapper

1. Auto-diff Types
DifferentialPair struct
DifferentialPtrPair struct
DiffTensorView struct

1. Misc Functions
CheckAccessFullyMapped(): Check access status to tiled resource.
NonUniformResourceIndex(): Indicate if the resource index is divergent.
asDynamicUniform(): 
clock2x32ARB():
clockARB():
concat(): Concatnate Tuple types.
createDynamicObject(): Create Existential object.
getRealtimeClockLow(): 
getRealtimeClock():
makeArrayFromElement():
makeTuple():
reinterpret(): Changes the type from one to another.
static_assert(): Error out when a compile-time value evaluates to false.
unmodified(): Silence the warning message about not writing to an inout parameter.

1. Stage specific - Fragment Shader Functions
EvaluateAttributeAtCentroid(): Evaluates an attribute at the pixel centroid.
EvaluateAttributeAtSample(): Evaluates an attribute at the indexed sample location.
EvaluateAttributeSnapped(): Evaluates an attribute at the pixel centroid with an offset.
GetAttributeAtVertex(): Returns the vertex attribute for the given vertexIndex.
IsHelperLane(): Returns true if a given lane in a pixel shader is a helper lane, which are nonvisible pixels that are executing due to gradient operations or discarded pixels.
clip(): Discards the current pixel if any component of the input is less than zero.
ddx(): Compute the derivative in the x direction.
ddx_coarse(): Computes a low precision partial derivative with respect to the screen-space x-coordinate.
ddx_fine(): Computes a high precision partial derivative with respect to the screen-space x-coordinate.
ddy(): Same as ddx() but for y direction.
ddy_coarse(): Same as ddx_coarse() but for y direction.
ddy_fine(): Same as ddx_fine() but for y direction.
discard(): Discards a pixel in a fragment shader.
fwidth(): Absolute sum of derivatives in x and y.

1. Stage specific - Compute Shader Functions
WorkgroupSize(): Returns the workgroup size of the calling entry point.

1. Stage specific - Hull Shader Functions
Process2DQuadTessFactorsAvg(): Processes 2D quad tessellation factors using the average method.
Process2DQuadTessFactorsMax(): Processes 2D quad tessellation factors using the maximum method.
Process2DQuadTessFactorsMin(): Processes 2D quad tessellation factors using the minimum method.
ProcessIsolineTessFactors(): Processes isoline tessellation factors.
ProcessQuadTessFactorsAvg(): Processes quad tessellation factors using the average method.
ProcessQuadTessFactorsMax(): Processes quad tessellation factors using the maximum method.
ProcessQuadTessFactorsMin(): Processes quad tessellation factors using the minimum method.
ProcessTriTessFactorsAvg(): Processes triangle tessellation factors using the average method.
ProcessTriTessFactorsMax(): Processes triangle tessellation factors using the maximum method.
ProcessTriTessFactorsMin(): Processes triangle tessellation factors using the minimum method.

1. Stage specific - Geometry Shader Functions
LineStream::Append(): Appends a vertex to the current line primitive.
LineStream::RestartStrip(): Completes the current line strip and starts a new one.
PointStream::Append(): Appends a vertex to the current point primitive.
PointStream::RestartStrip(): Completes the current point strip and starts a new one.
TriangleStream::Append(): Appends a vertex to the current triangle primitive.
TriangleStream::RestartStrip(): Completes the current triangle strip and starts a new one.

1. Stage specific - Geometry Shader Types
LineStream struct
PointStream struct
TriangleStream struct

1. Stage specific - Mesh Shader Functions
DispatchMesh(): Dispatches work for mesh shaders.
SetMeshOutputCounts(): Sets the number of vertices and primitives to emit.

1. Stage specific - Ray-Tracing Functions
AcceptHitAndEndSearch(): Stops ray traversal upon a hit.
CallShader(): Invokes another shader from within a shader.
DispatchRaysIndex(): Gets the current location within the width, height, and depth obtained with the DispatchRaysDimensions system value intrinsic.
DispatchRaysDimensions(): The width, height and depth values from the originating DispatchRays call.
GeometryIndex(): The autogenerated index of the current geometry in the bottom-level acceleration structure.
HitKind(): Returns the value passed as the HitKind parameter to ReportHit.
HitTriangleVertexPosition(): Get the vertex positions of the currently hit triangle in any-hit or closest-hit shader.
IgnoreHit(): Ignores a hit during ray traversal.
InstanceID(): The user-provided identifier for the instance on the bottom-level acceleration structure instance within the top-level structure.
InstanceIndex(): The autogenerated index of the current instance in the top-level raytracing acceleration structure.
ObjectRayDirection(): The object-space direction for the current ray. Object-space refers to the space of the current bottom-level acceleration structure.
ObjectRayOrigin(): The object-space origin for the current ray. Object-space refers to the space of the current bottom-level acceleration structure.
ObjectToWorld3x4(): A matrix for transforming from object-space to world-space. Object-space refers to the space of the current bottom-level acceleration structure.
ObjectToWorld4x3(): A matrix for transforming from object-space to world-space. Object-space refers to the space of the current bottom-level acceleration structure.
PrimitiveIndex(): Retrieves the autogenerated index of the primitive within the geometry inside the bottom-level acceleration structure instance.
RayCurrentTime(): 
RayFlags(): Returns the current ray flag values.
RayTCurrent(): The current parametric ending point for the ray.
RayTMin(): The current parametric starting point for the ray.
ReorderThread(): Reorders threads based on a coherence hint value.
ReportHit(): Reports a hit during ray traversal.
TraceMotionRay(): 
TraceRay(): Traces a ray and returns the hit information.
WorldRayDirection(): The world-space direction for the current ray.
WorldRayOrigin(): The world-space origin of the current ray.
WorldToObject3x4(): A matrix for transforming from world-space to object-space. Object-space refers to the space of the current bottom-level acceleration structure.
WorldToObject4x3(): A matrix for transforming from world-space to object-space. Object-space refers to the space of the current bottom-level acceleration structure.

1. Stage specific - Ray-tracing Types
RayDesc struct
RaytracingAccelerationStructure struct
BuiltInTriangleIntersectionAttributes
RayQuery struct
HitObject struct

