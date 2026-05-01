// Unit test mode is used for unit testing the tiled MMA implementation.
// So we can test this single file by providing -DUNIT_TEST to the compiler.
implementing neural;

// Perf profiling toggles. Override with -DPERF_xxx=0 to disable individual
// backward components for decomposed benchmarking.
#ifndef PERF_MMA_TRANSPOSE_ACTIVE
#define PERF_MMA_TRANSPOSE_ACTIVE 1
#endif
#ifndef PERF_BIAS_REDUCE_ACTIVE
#define PERF_BIAS_REDUCE_ACTIVE 1
#endif
#ifndef PERF_OPA_ACTIVE
#define PERF_OPA_ACTIVE 1
#endif

// Cooperative matrix is only supported by CUDA and SPIR-V
// WaveTangledVector is a vector type that emulates the Cooperative Vector type by using Cooperative Matrix feature which is
// supported by CUDA and SPIR-V.
[require(cooperative_matrix, subgroup_basic, cuda_spirv)]
public struct WaveTangledVector<T, ShMemSize : ISharedMemorySize, int N, int SubgroupSize, int SubgroupCount> : IVector<T>
    where T : __BuiltinFloatingPointType
    where T.Differential == T
{
    public typealias Differential = WaveTangledVector<T, ShMemSize, N, SubgroupSize, SubgroupCount>;

    public static const int Size = N;
    public typealias ShMemPool = SharedMemoryPool<ShMemSize>;

    private typealias DTypeMatC = half;

    // Since we always use half type for A and B, we want to follow the alignment requirement of half type.
    internal static const uint ElementCountPerVector = sizeof(uint4) / sizeof(DTypeMatC);
    internal static const int Uint4AlignedInputSize = ((N + (ElementCountPerVector - 1)) / ElementCountPerVector) * ElementCountPerVector;

    internal T[Uint4AlignedInputSize] data;
    public int getCount() { return N; }

    public __init() { data = {}; }

    public __init(T value)
    {
        for (int i = 0; i < N; i++)
            this.data[i] = value;
        cleanPaddingData();
    }

    public __init(T[Size] inData)
    {
        for (int i = 0; i < N; i++)
            this.data[i] = inData[i];
        cleanPaddingData();
    }

    internal __init(T[Uint4AlignedInputSize] inData)
    {
        this.data = inData;
    }

    public __init(This other) { this.data = other.data; }

    public __init<InputArray : IArray<T>>(InputArray inData)
    {
        for (int i = 0; i < N; i++)
            this.data[i] = inData[i];
        cleanPaddingData();
    }

    [ForceInline]
    [mutating]
    internal void cleanPaddingData()
    {
        const int elementCountPerUint4 = sizeof(uint4) / sizeof(DTypeMatC);
        const int alignedSize = ((N + (elementCountPerUint4 - 1)) / elementCountPerUint4) * elementCountPerUint4;
        for (int i = N; i < alignedSize; i++)
            this.data[i] = T(0.0f);
    }

    public __subscript(int index) -> T
    {
        [ForceInline]
        get { return this.data[index]; }

        [ForceInline]
        set { this.data[index] = newValue; }
    }

    [ForceInline]
    override internal uint4 readUint4(int uint4Index)
    {
        return data.readUint4(uint4Index);
    }

    [ForceInline]
    private OutputVector linearTransformOnTarget<Address, Layout, OutputVector, TargetEnum Target, bool Bias>(
        Address weightAddress,
        Optional<Address> biasAddress)
        where Address : IPointerLikeAddress<T>
        where Address.Differential : IPointerLikeAddress<T>
        where OutputVector : IVector<T>
        where Layout : IStorageLayout
    {
        if (Layout.Layout == LayoutType.Optimal)
        {
            typealias TiledMMA = TiledMMAHelper<T, N, OutputVector.Size, SubgroupSize, Target, ShMemSize, false, SubgroupCount>;
            let resultVec = TiledMMA.forward<Address, Bias>(This(data), weightAddress, biasAddress);
            return OutputVector(resultVec.data);
        }
        else
        {
            typealias MMA = MMAHelper<T, N, OutputVector.Size, SubgroupSize, Target, ShMemSize, false, SubgroupCount>;
            uint shA = 0;
            uint shB = MMA.ShMemInfo.SharedMemSizeInVectorMatA;
            uint shC = shB;
            const int AlignedOutSize = MMA.TileInfo.Uint4AlignedM;
            let outputArray = MMA.mma<T, Address, T[Uint4AlignedInputSize], T[AlignedOutSize], T, Bias
                                >( data, shA, shB, shC, weightAddress, biasAddress);
            return OutputVector(outputArray);
        }
    }

    [ForceInline]
    private static void linearTransformBwdOnTarget<Address, Layout, OutputVector, TargetEnum Target, bool Bias>(
        inout DifferentialPair<This> dthis,
        DifferentialPtrPair<Address> dWeightAddress,
        Optional<DifferentialPtrPair<Address>> biasAddress,
        OutputVector.Differential doutput)
            where Address : IPointerLikeAddress<T>
            where Address.Differential : IPointerLikeAddress<T>
            where OutputVector : IVector<T>
            where OutputVector.Differential : IVector<T>
            where Layout : IStorageLayout
    {
        if (Layout.Layout == LayoutType.Optimal)
        {
            typealias MMA = TiledMMAHelper<T, N, OutputVector.Size, SubgroupSize, Target, ShMemSize, true, SubgroupCount>;
            let dInputVec = MMA.backward<Address, OutputVector.Differential, Bias>(
                dthis.p, doutput, dWeightAddress.p, dWeightAddress.d,
                Bias ? biasAddress.value.d : dWeightAddress.d);
            dthis = DifferentialPair<This>(dthis.p, bit_cast<This>(dInputVec));
        }
        else
        {
            typealias MMA = MMAHelper<T, N, OutputVector.Size, SubgroupSize, Target, ShMemSize, true, SubgroupCount>;

            uint shA = 0;
            uint shB = MMA.ShMemInfo.SharedMemSizeInVectorMatA;
            uint shC = shB;

            const int AlignedOutSize = MMA.TileInfo.Uint4AlignedK;

            // dIn = W^T * dOut (linear layout)
            let outArray = MMA.mma<T, Address, OutputVector.Differential, T[AlignedOutSize], T, false
                            >( doutput, shA, shB, shC, dWeightAddress.p, none);
            This.Differential dInput = This.Differential(outArray);
            dthis = DifferentialPair<This>(dthis.p, dInput);

            GroupMemoryBarrierWithGroupSync();

            static const int _outerRows = (MMA.M + MMA.CMShape.ROW_A - 1) / MMA.CMShape.ROW_A;
            static const int _outerPerWarpA = _outerRows * MMA.CMShape.ROW_A / MMA.CMShape.ElementCountPerVector * SubgroupSize;
            uint shB_outer = shA + uint(SubgroupCount * _outerPerWarpA);

            // dW = dOut * input^T, stored in linear layout
            MMA.outerProductAccumulate<T, Address.Differential,
                                       T, OutputVector.Differential,
                                       T, T[Uint4AlignedInputSize]
                                      >( shA, shB_outer, doutput, dthis.p.data, dWeightAddress.d);

            if (Bias)
            {
                const int subgroupIndex = getWaveId();
                MMA.sumReduceRows<T, Address.Differential, OutputVector.Differential
                    >(shA, doutput, subgroupIndex, biasAddress.value.d);
            }
        }
    }

    // Linear transformation without bias
    [ForceInline]
    [HasTrivialForwardDerivative]
    [BackwardDerivative(linearTransformBwd)]
    [require(cuda_spirv)]
    public OutputVector linearTransform<Address, Layout, OutputVector>(
        Address weightAddress)
            where Address : IPointerLikeAddress<T>
            where Address.Differential : IPointerLikeAddress<T>
            where Layout : IStorageLayout
            where OutputVector : IVector<T>
    {
        __target_switch
        {
        case cuda:
            return no_diff linearTransformOnTarget<Address, Layout, OutputVector, TargetEnum.CUDA, false>(weightAddress, none);
        case spirv:
            return no_diff linearTransformOnTarget<Address, Layout, OutputVector, TargetEnum.SPIR_V, false>(weightAddress, none);
        }
    }

    // Backward of linear transformation without bias
    [ForceInline]
    static void linearTransformBwd<Address, Layout, OutputVector>(
        inout DifferentialPair<This> dthis,
        DifferentialPtrPair<Address> dWeightAddress,
        OutputVector.Differential doutput)
            where Address : IPointerLikeAddress<T>
            where Address.Differential : IPointerLikeAddress<T>
            where Layout : IStorageLayout
            where OutputVector : IVector<T>
            where OutputVector.Differential : IVector<T>
    {
        Optional<DifferentialPtrPair<Address>> biasAddress = {};
        __target_switch
        {
        case cuda:
            linearTransformBwdOnTarget<Address, Layout, OutputVector, TargetEnum.CUDA, false>(dthis, dWeightAddress, biasAddress, doutput);
        case spirv:
            linearTransformBwdOnTarget<Address, Layout, OutputVector, TargetEnum.SPIR_V, false>(dthis, dWeightAddress, biasAddress, doutput);
        }
    }

    [ForceInline]
    [HasTrivialForwardDerivative]
    [BackwardDerivative(linearTransformBwd)]
    [require(cuda_spirv)]
    public OutputVector linearTransform<Address, Layout, OutputVector>(
        Address weightAddress, Address biasAddress)
            where Address : IPointerLikeAddress<T>
            where Address.Differential : IPointerLikeAddress<T>
            where Layout : IStorageLayout
            where OutputVector : IVector<T>
    {
        __target_switch
        {
        case cuda:
            return no_diff linearTransformOnTarget<Address, Layout, OutputVector, TargetEnum.CUDA, true>(weightAddress, biasAddress);
        case spirv:
            return no_diff linearTransformOnTarget<Address, Layout, OutputVector, TargetEnum.SPIR_V, true>(weightAddress, biasAddress);
        }
    }

    // Backward of linear transformation with bias
    [ForceInline]
    static void linearTransformBwd<Address, Layout, OutputVector>(
        inout DifferentialPair<This> dthis,
        DifferentialPtrPair<Address> dWeightAddress,
        DifferentialPtrPair<Address> dBiasAddress,
        OutputVector.Differential doutput)
            where Address : IPointerLikeAddress<T>
            where Address.Differential : IPointerLikeAddress<T>
            where Layout : IStorageLayout
            where OutputVector : IVector<T>
            where OutputVector.Differential : IVector<T>
    {
        __target_switch
        {
        case cuda:
            linearTransformBwdOnTarget<Address, Layout, OutputVector, TargetEnum.CUDA, true>(dthis, dWeightAddress, dBiasAddress, doutput);
        case spirv:
            linearTransformBwdOnTarget<Address, Layout, OutputVector, TargetEnum.SPIR_V, true>(dthis, dWeightAddress, dBiasAddress, doutput);
        }
    }
}
