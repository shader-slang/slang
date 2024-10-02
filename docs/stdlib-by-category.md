1. Common Functions

| Function name | Description |
|--|--|
| GetRenderTargetSampleCount() | Returns the number of samples for a render target. |
| GetRenderTargetSamplePosition() | Gets the sampling position (x, y) for a given sample index. |
| clamp() | Clamps a value between a minimum and maximum. |
| isfinite() | Returns true if value is finite. |
| isinf() | Returns true if the value is infinite. |
| isnan() | Returns true if the value is NaN. |
| lerp() | Linear interpolation between two values. |
| saturate() | Clamps the input to the range [0,1]. |
| sign() | Returns the sign of a value (-1, 0, 1). |
| smoothstep() | Smooth Hermite interpolation between 0 and 1. |
| step() | Returns 0 if less than a threshold and 1 otherwise. |

1. Common Interfaces

| Interface name | Description |
|--|--|
| IArray | |
| IBufferDataLayout | |
| IFunc | |
| IMutatingFunc | |
| IRWArray | |

1. Common Types

| Type name | Description |
|--|--|
| Array | |
| ConstRef | |
| InOut | |
| InputPatch | |
| Optional | |
| Out | |
| OutputIndices | |
| OutputPatch | |
| OutputPrimitives | |
| OutputVertices | |
| Ref | |
| TensorView | |
| TorchTensor | |
| Tuple | |
| matrix | |
| vector | |

1. Texture Types

| Type name | Description |
|--|--|
| FeedbackTexture2D | |
| FeedbackTexture2DArray | |
| RWTexture1D | |
| RWTexture1DArray | |
| RWTexture2D | |
| RWTexture2DArray | |
| RWTexture2DMS | |
| RWTexture2DMSArray | |
| RWTexture3D | |
| RasterizerOrderedTexture1D | |
| RasterizerOrderedTexture1DArray | |
| RasterizerOrderedTexture2D | |
| RasterizerOrderedTexture2DArray | |
| RasterizerOrderedTexture3D | |
| Texture1D | |
| Texture1DArray | |
| Texture2D | |
| Texture2DArray | |
| Texture2DMS | |
| Texture2DMSArray | |
| Texture3D | |
| TextureCube | |
| TextureCubeArray | |
| TextureFootprint | |

1. Sampler Types

| Type name | Description |
|--|--|
| RWSampler1D | |
| RWSampler1DArray | |
| RWSampler2D | |
| RWSampler2DArray | |
| RWSampler2DMS | |
| RWSampler2DMSArray | |
| RWSampler3D | |
| RasterizerOrderedSampler1D | |
| RasterizerOrderedSampler1DArray | |
| RasterizerOrderedSampler2D | |
| RasterizerOrderedSampler2DArray | |
| RasterizerOrderedSampler3D | |
| Sampler1D | |
| Sampler1DArray | |
| Sampler2D | |
| Sampler2DArray | |
| Sampler2DMS | |
| Sampler2DMSArray | |
| Sampler3D | |
| SamplerComparisonState | |
| SamplerCube | |
| SamplerCubeArray | |
| SamplerState | |

1. Uniform Buffer Types

| Type name | Description |
|--|--|
| AppendStructuredBuffer | |
| ByteAddressBuffer | |
| ConstantBuffer | |
| ConsumeStructuredBuffer | |
| ParameterBlock | |
| RWByteAddressBuffer | |
| RWStructuredBuffer | |
| RasterizerOrderedByteAddressBuffer | |
| RasterizerOrderedStructuredBuffer | |
| StructuredBuffer | |
| TextureBuffer | |

1. Pointer Types

| Type name | Description |
|--|--|
| ConstBufferPointer | |
| NullPtr | |
| Ptr | |

1. Relational Functions

| Function name | Description |
|--|--|
| all() | True if all components are non-zero. |
| any() | True if any component is non-zero. |

1. Bitwise Functions

| Function name | Description |
|--|--|
| and() | Component-wise logical AND. |
| countbits() | Counts the number of bits set to 1. |
| firstbithigh() | Count leading zeros. |
| firstbitlow() | Count trailing zeros. |
| msad4() | Compares a 4-byte reference value and an 8-byte source value, accumulating a vector of 4 sums based on the masked sum of absolute differences. |
| or() | Component-wise logical OR. |
| reversebits() | Reverses the order of bits in an integer. |

1. Trigonometry Functions

| Function name | Description |
|--|--|
| acos() | Inverse cosine. |
| acosh() | Inverse hyperbolic cosine. |
| asin() | Inverse sine. |
| asinh() | Inverse hyperbolic sine. |
| atan() | Inverse tangent. |
| atan2() | Inverse two parameter tangent. |
| atanh() | Inverse hyperbolic tangent. |
| cos() | Cosine. |
| cosh() | Hyperbolic cosine. |
| degrees() | Converts radians to degrees. |
| radians() | Converts degrees to radians. |
| sin() | Sine. |
| sincos() | Simultaneous sine and cosine. |
| sinh() | Hyperbolic sine. |
| tan() | Tangent. |
| tanh() | Hyperbolic tangent. |

1. Exponential Functions

| Function name | Description |
|--|--|
| exp() | Exponential (e^x). |
| exp10() | Base 10 exponential (10^x). |
| exp2() | Base 2 exponential (2^x). |
| log() | Natural logarithm. |
| log10() | Base 10 logarithm. |
| log2() | Base 2 logarithm. |
| pow() | Power function. |
| rsqrt() | Reciprocal square root. |
| sqrt() | Square root. |

1. Matrix Functions

| Function name | Description |
|--|--|
| determinant() | Returns the determinant of a matrix. |
| mul() | Multiplication. |
| transpose() | Transposes a matrix. |

1. Math Functions

| Function name | Description |
|--|--|
| abs() | Absolute value. |
| ceil() | Round up to the nearest integer. |
| fabs() | Absolute value. |
| floor() | Round down to the nearest integer. |
| fma() | Fused multiply-add. |
| fmax() | Returns maximum of two floats. |
| fmin() | Returns the minimum of two floats. |
| fmod() | Returns the floating point remainder. |
| frac() | Returns the fractional part of a number. |
| frexp() | Splits a floating point into mantissa and exponent. |
| ldexp() | Combines a mantissa and exponent. |
| mad() | Multiply and add operation. |
| max() | Returns the larger of two values. |
| min() | Returns the smaller of two values. |
| modf() | Splits value into integral and fractional parts. |
| rcp() | Computes the reciprocal of a value. |
| round() | Rounds to the nearest whole number. |
| trunc() | Truncates the fractional part. |

1. Math Interfaces

| Interface name | Description |
|--|--|
| IArithmetic | |
| ICompareable | |
| IFloat | |
| IInteger | |
| ILogical | |
| IRangedValue | |

1. Geometric Functions

| Function name | Description |
|--|--|
| cross() | Cross product of two 3D vectors. |
| distance() | Distance between two points. |
| dot() | Dot product of two vectors. |
| faceforward() | Adjusts the direction of a normal. |
| length() | Length of a vector. |
| lit() | Computes lighting coefficients. |
| normalize() | Normalize a vector. |
| reflect() | Reflects a vector around a normal. |
| refract() | Computes the refraction vector. |

1. Conversion Functions

| Function name | Description |
|--|--|
| asdouble() | Converts two uint32_t into a double. |
| asfloat() | Converts int32_t or uint32_t to float. |
| asfloat16() | Converts uint16_t to float16_t. |
| asint() | Converts float or uint32_t to int32_t. |
| asint16() | Converts float16_t type to int16_t. |
| asuint() | Converts double, float or int32_t to uint32_t. |
| asuint16() | Converts the float16 to uint16_t. |
| bit_cast() | Converts a type to another type as bit data. |
| f16tof32() | Converts a float16 stored in the low-half of a uint to a float. |
| f32tof16() | Converts a float to a float16 type. |

1. Atomic Functions

| Function name | Description |
|--|--|
| InterlockedAdd() | Atomic addition. |
| InterlockedAnd() | Atomic bitwise AND. |
| InterlockedCompareExchange() | Compares and swaps values atomically. |
| InterlockedCompareExchangeFloatBitwise() | Same as InterlockedCompareExchange() but for float value. |
| InterlockedCompareStore() | Atomically compares the destination to the comparison value and, if they are identical, overwrites the destination with the input value. |
| InterlockedCompareStoreFloatBitwise() | Same as InterlockedCompareStore() but for float value. |
| InterlockedExchange() | Assigns a value to a destination and returns the original value atomically. |
| InterlockedMax() | Atomic maximum. |
| InterlockedMin() | Atomic minimum. |
| InterlockedOr() | Atomic bitwise OR. |
| InterlockedXor() | Atomic bitwise XOR. |

1. Atomic Interfaces

| Interface name | Description |
|--|--|
| IAtomicable | |
| IArithmeticAtomicable | |
| IBitAtomicable | |

1. Atomic Types

| Type name | Description |
|--|--|
| Atomic | |
| AtomicAdd | |

1. Synchronization Functions

| Function name | Description |
|--|--|
| AllMemoryBarrier() | Ensures all threads in a group are synchronized. |
| AllMemoryBarrierWithGroupSync() | Ensures all threads in a group are synchronized and all memory accesses are completed. |
| DeviceMemoryBarrier() | Ensures all device memory accesses are completed. |
| DeviceMemoryBarrierWithGroupSync() | Ensures all device memory accesses are completed and all threads in the group have reached this call. |
| GroupMemoryBarrier() | Ensures all group shared accesses are completed. |
| GroupMemoryBarrierWithGroupSync() | Ensures all group shared accesses are completed and all threads in the group have reached this call. |
| beginInvocationInterlock() | Mark beginning of "interlocked" operations. |
| endInvocationInterlock() | Mark end of "interlocked" operations. |

1. Wave Functions

| Function name | Description |
|--|--|
| AllMemoryBarrierWithWaveMaskSync() | Same to AllMemoryBarrierWithWaveSync() but limits to the threads marked in the mask. |
| AllMemoryBarrierWithWaveSync() | Similar to AllMemoryBarrierWithGroupSync() but limits to the current wave. |
| GroupMemoryBarrierWithWaveMaskSync() | Same to GroupMemoryBarrierWithWaveSync() but limits to the threads marked in the mask. |
| GroupMemoryBarrierWithWaveSync() | SImilar to GroupMemoryBarrierWithGroupSync() but limits to the current wave. |
| QuadReadAcrossDiagonal() | Returns the specified local value read from the diagonally opposite lane in the quad. |
| QuadReadAcrossX() | Returns the specified local value read from the other lane in the quad in the X direction. |
| QuadReadAcrossY() | Returns the specified local value read from the other lane in the quad in the Y direction. |
| QuadReadLaneAt() | Returns the specified source value from the lane identified by the lane ID within the current quad. |
| WaveActiveAllEqual() | Returns true if the expression is the same for every active lane in the current wave. |
| WaveActiveAllTrue() | Returns true if all lanes in the wave evaluate the expression to true. |
| WaveActiveAnyTrue() | Returns true if any lane in the wave evaluates the expression to true. |
| WaveActiveBallot() | Returns a bitmask where each bit represents whether the corresponding lane evaluates the expression to true. |
| WaveActiveBitAnd() | Computes the bitwise AND of the expression across all active lanes in the wave. |
| WaveActiveBitOr() | Computes the bitwise OR of the expression across all active lanes in the wave. |
| WaveActiveBitXor() | Computes the bitwise XOR of the expression across all active lanes in the wave. |
| WaveActiveCountBits() | Returns the number of bits set to 1 in the bitmask returned by WaveActiveBallot. |
| WaveActiveMax() | Computes the maximum value of the expression across all active lanes in the wave. |
| WaveActiveMin() | Computes the minimum value of the expression across all active lanes in the wave. |
| WaveActiveProduct() | Computes the product of the expression across all active lanes in the wave. |
| WaveActiveSum() | Computes the sum of the expression across all active lanes in the wave. |
| WaveBroadcastLaneAt() | |
| WaveGetActiveMask() | |
| WaveGetActiveMulti() | |
| WaveGetConvergedMask() | |
| WaveGetConvergedMulti() | |
| WaveGetLaneCount() | Returns the number of lanes in a wave on the current architecture. |
| WaveGetLaneIndex() | Returns the index of the current lane within the current wave. |
| WaveIsFirstLane() | Returns true if the current lane is the first active lane in the wave. |
| WaveMaskBroadcastLaneAt() | |
| WaveMaskIsFirstLane() | |
| WaveMaskAllEqual() | |
| WaveMaskAllTrue() | |
| WaveMaskAnyTrue() | |
| WaveMaskBallot() | |
| WaveMaskBitAnd() | |
| WaveMaskBitOr() | |
| WaveMaskBitXor() | |
| WaveMaskCountBits() | |
| WaveMaskMatch() | |
| WaveMaskMax() | |
| WaveMaskMin() | |
| WaveMaskPrefixBitAnd() | |
| WaveMaskPrefixBitOr() | |
| WaveMaskPrefixBitXor() | |
| WaveMaskPrefixCountBits() | |
| WaveMaskPrefixProduct() | |
| WaveMaskPrefixSum() | |
| WaveMaskProduct() | |
| WaveMaskReadLaneAt() | | 
| WaveMaskReadLaneFirst() | |
| WaveMaskShuffle() | |
| WaveMaskSum() | |
| WaveMatch() | |
| WaveMultiPrefixBitAnd() | |
| WaveMultiPrefixBitOr() | |
| WaveMultiPrefixBitXor() | |
| WaveMultiPrefixCountBits() | |
| WaveMultiPrefixProduct() | |
| WaveMultiPrefixSum() | |
| WavePrefixCountBits() | Computes the prefix sum of the number of bits set to 1 across all active lanes in the wave. |
| WavePrefixProduct() | Computes the prefix product of the expression across all active lanes in the wave. |
| WavePrefixSum() | Computes the prefix sum of the expression across all active lanes in the wave. |
| WaveReadLaneAt() | Returns the value of the expression for the specified lane index within the wave1. |
| WaveReadLaneFirst() | Returns the value of the expression for the active lane with the smallest index in the wave. |
| WaveShuffle() | |

1. Auto-diff Functions

| Function name | Description |
|--|--|
| isDifferentialNull() | existential check for null differential type. |
| detach() | Detach and set derivatives to zero. |
| diffPair() | Constructs a `DifferentialPair` value from a primal and differential values. |
| updatePrimal() | Changes the primal value in a DifferentialPair type. |
| updateDiff() | Changes the diff value in a DifferentialPair type. |
| updatePair() | Changes both primal and diff values in a DifferentialPair type. |

1. Auto-diff Interfaces

| Interface name | Description |
|--|--|
| IDifferentiable | |
| IDifferentiablePtrType | |
| IDifferentiableMutatingFunc
| IDifferentiableFunc
| IDiffTensorWrapper

1. Auto-diff Types

| Type name | Description |
|--|--|
| DifferentialPair | |
| DifferentialPtrPair | |
| DiffTensorView | |

1. Misc Functions

| Function name | Description |
|--|--|
| CheckAccessFullyMapped() | Check access status to tiled resource. |
| NonUniformResourceIndex() | Indicate if the resource index is divergent. |
| asDynamicUniform() | | 
| clock2x32ARB() | |
| clockARB() | |
| concat() | Concatnate Tuple types. |
| createDynamicObject() | Create Existential object. |
| getRealtimeClockLow(): | |
| getRealtimeClock() | |
| makeArrayFromElement() | |
| makeTuple() | |
| reinterpret() | Changes the type from one to another. |
| static_assert() | Error out when a compile-time value evaluates to false. |
| unmodified() | Silence the warning message about not writing to an inout parameter. |

1. Stage specific - Fragment Shader Functions

| Function name | Description |
|--|--|
| EvaluateAttributeAtCentroid() | Evaluates an attribute at the pixel centroid. |
| EvaluateAttributeAtSample() | Evaluates an attribute at the indexed sample location. |
| EvaluateAttributeSnapped() | Evaluates an attribute at the pixel centroid with an offset. |
| GetAttributeAtVertex() | Returns the vertex attribute for the given vertexIndex. |
| IsHelperLane() | Returns true if a given lane in a pixel shader is a helper lane, which are nonvisible pixels that are executing due to gradient operations or discarded pixels. |
| clip() | Discards the current pixel if any component of the input is less than zero. |
| ddx() | Compute the derivative in the x direction. |
| ddx_coarse() | Computes a low precision partial derivative with respect to the screen-space x-coordinate. |
| ddx_fine() | Computes a high precision partial derivative with respect to the screen-space x-coordinate. |
| ddy() | Same as ddx() but for y direction. |
| ddy_coarse() | Same as ddx_coarse() but for y direction. |
| ddy_fine() | Same as ddx_fine() but for y direction. |
| discard() | Discards a pixel in a fragment shader. |
| fwidth() | Absolute sum of derivatives in x and y. |

1. Stage specific - Compute Shader Functions

| Function name | Description |
|--|--|
| WorkgroupSize() | Returns the workgroup size of the calling entry point. |

1. Stage specific - Hull Shader Functions

| Function name | Description |
|--|--|
| Process2DQuadTessFactorsAvg() | Processes 2D quad tessellation factors using the average method. |
| Process2DQuadTessFactorsMax() | Processes 2D quad tessellation factors using the maximum method. |
| Process2DQuadTessFactorsMin() | Processes 2D quad tessellation factors using the minimum method. |
| ProcessIsolineTessFactors() | Processes isoline tessellation factors. |
| ProcessQuadTessFactorsAvg() | Processes quad tessellation factors using the average method. |
| ProcessQuadTessFactorsMax() | Processes quad tessellation factors using the maximum method. |
| ProcessQuadTessFactorsMin() | Processes quad tessellation factors using the minimum method. |
| ProcessTriTessFactorsAvg() | Processes triangle tessellation factors using the average method. |
| ProcessTriTessFactorsMax() | Processes triangle tessellation factors using the maximum method. |
| ProcessTriTessFactorsMin() | Processes triangle tessellation factors using the minimum method. |

1. Stage specific - Geometry Shader Functions

| Function name | Description |
|--|--|
| LineStream::Append() | Appends a vertex to the current line primitive. |
| LineStream::RestartStrip() | Completes the current line strip and starts a new one. |
| PointStream::Append() | Appends a vertex to the current point primitive. |
| PointStream::RestartStrip() | Completes the current point strip and starts a new one. |
| TriangleStream::Append() | Appends a vertex to the current triangle primitive. |
| TriangleStream::RestartStrip() | Completes the current triangle strip and starts a new one. |

1. Stage specific - Geometry Shader Types

| Type name | Description |
|--|--|
| LineStream | |
| PointStream | |
| TriangleStream | |

1. Stage specific - Mesh Shader Functions

| Function name | Description |
|--|--|
| DispatchMesh() | Dispatches work for mesh shaders. |
| SetMeshOutputCounts() | Sets the number of vertices and primitives to emit. |

1. Stage specific - Ray-Tracing Functions

| Function name | Description |
|--|--|
| AcceptHitAndEndSearch() | Stops ray traversal upon a hit. |
| CallShader() | Invokes another shader from within a shader. |
| DispatchRaysIndex() | Gets the current location within the width, height, and depth obtained with the DispatchRaysDimensions system value intrinsic. |
| DispatchRaysDimensions() | The width, height and depth values from the originating DispatchRays call. |
| GeometryIndex() | The autogenerated index of the current geometry in the bottom-level acceleration structure. |
| HitKind() | Returns the value passed as the HitKind parameter to ReportHit. |
| HitTriangleVertexPosition() | Get the vertex positions of the currently hit triangle in any-hit or closest-hit shader. |
| IgnoreHit() | Ignores a hit during ray traversal. |
| InstanceID() | The user-provided identifier for the instance on the bottom-level acceleration structure instance within the top-level structure. |
| InstanceIndex() | The autogenerated index of the current instance in the top-level raytracing acceleration structure. |
| ObjectRayDirection() | The object-space direction for the current ray. Object-space refers to the space of the current bottom-level acceleration structure. |
| ObjectRayOrigin() | The object-space origin for the current ray. Object-space refers to the space of the current bottom-level acceleration structure. |
| ObjectToWorld3x4() | A matrix for transforming from object-space to world-space. Object-space refers to the space of the current bottom-level acceleration structure. |
| ObjectToWorld4x3() | A matrix for transforming from object-space to world-space. Object-space refers to the space of the current bottom-level acceleration structure. |
| PrimitiveIndex() | Retrieves the autogenerated index of the primitive within the geometry inside the bottom-level acceleration structure instance. |
| RayCurrentTime() | |
| RayFlags() | Returns the current ray flag values. |
| RayTCurrent() | The current parametric ending point for the ray. |
| RayTMin() | The current parametric starting point for the ray. |
| ReorderThread() | Reorders threads based on a coherence hint value. |
| ReportHit() | Reports a hit during ray traversal. |
| TraceMotionRay() | |
| TraceRay() | Traces a ray and returns the hit information. |
| WorldRayDirection() | The world-space direction for the current ray. |
| WorldRayOrigin() | The world-space origin of the current ray. |
| WorldToObject3x4() | A matrix for transforming from world-space to object-space. Object-space refers to the space of the current bottom-level acceleration structure. |
| WorldToObject4x3() | A matrix for transforming from world-space to object-space. Object-space refers to the space of the current bottom-level acceleration structure. |

1. Stage specific - Ray-tracing Types

| Type name | Description |
|--|--|
| RayDesc | |
| RaytracingAccelerationStructure | |
| BuiltInTriangleIntersectionAttributes
| RayQuery | |
| HitObject | |

