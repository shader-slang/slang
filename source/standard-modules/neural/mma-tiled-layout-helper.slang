// TiledMMAHelper: dispatcher for tiled (OptimalLayout) MMA operations.
//
// Routes forward and backward to backend-specific implementations:
//   CUDA:   TiledMMACuda / MMAHelperNew (WaveMatrix register ops)
//   Vulkan: TiledMMAVulkan (WaveMatrix + shmem, CoopMat.Load)
//
// InputSize and OutputSize must be tile-aligned (multiples of 16).
// Callers with arbitrary sizes should pad via PadToTile before instantiation.
implementing neural;


// Round up a compile-time integer to the next multiple of 16 (cooperative matrix tile size).
internal struct TileAlignedSize<int Size>
{
    internal static const int Value = ((Size + 15) / 16) * 16;
}

[require(cooperative_matrix)]
internal struct TiledMMAHelper<T, int InputSize, int OutputSize, int SubgroupSize, TargetEnum Target, ShMemSize: ISharedMemorySize, bool TransposeA, int SubgroupCount>
    where T : __BuiltinFloatingPointType
    where T.Differential == T
{
    typealias BaseMMA = MMAHelper<T, InputSize, OutputSize, SubgroupSize, Target, ShMemSize, TransposeA, SubgroupCount>;

    internal typealias TileInfoT = BaseMMA.TileInfo;
    internal typealias TileInfo = TileInfoT;
    typealias CMShape = BaseMMA.CMShape;
    typealias ShMemInfo = BaseMMA.ShMemInfo;

    static const int M = BaseMMA.M;
    static const int N = BaseMMA.N;
    static const int K = BaseMMA.K;

    // =========================================================================
    // Forward MMA: dispatches to CUDA or Vulkan backend.
    //
    // CUDA:   TiledMMACuda.forward (MMAHelperNew, register-based, zero shmem)
    // Vulkan: TiledMMAVulkan.forward (WaveMatrix + shmem, CoopMat.Load)
    // =========================================================================

    [ForceInline]
    [require(cooperative_matrix, subgroup_basic, cuda_spirv)]
    internal static WaveTangledVector<T, ShMemSize, OutputSize, SubgroupSize, SubgroupCount>
    forward<Address, bool Bias = false>(
        WaveTangledVector<T, ShMemSize, InputSize, SubgroupSize, SubgroupCount> input,
        Address weightAddress,
        Optional<Address> biasAddress)
        where Address : IPointerLikeAddress<T>
    {
        __target_switch
        {
        case cuda:
            typealias Cuda = TiledMMACuda<T, InputSize, OutputSize, SubgroupSize, Target>;
            return Cuda.forward<Address, ShMemSize, SubgroupCount, Bias>(
                input, weightAddress, biasAddress);
        case spirv:
            typealias Vulkan = TiledMMAVulkan<T, InputSize, OutputSize, SubgroupSize, Target, ShMemSize, TransposeA, SubgroupCount>;
            return Vulkan.forward<Address, Bias>(
                input, weightAddress, biasAddress);
        }
    }

    // =========================================================================
    // Unified backward: dispatches to CUDA or Vulkan backend.
    //
    // CUDA:   TiledMMACuda (WaveMatrix register ops, shuffles, ChangeMajor)
    // Vulkan: TiledMMAVulkan (WaveMatrix + shmem, CoopMat.Load, no register transforms)
    // =========================================================================

    [ForceInline]
    [require(cooperative_matrix, subgroup_basic, cuda_spirv)]
    internal static WaveTangledVector<T.Differential, ShMemSize, InputSize, SubgroupSize, SubgroupCount>
    backward<Address, OutputVector, bool Bias = true>(
        WaveTangledVector<T, ShMemSize, InputSize, SubgroupSize, SubgroupCount> input,
        OutputVector dOut,
        Address weightAddress,
        Address.Differential weightGradAddress,
        Address.Differential biasGradAddress)
            where Address : IPointerLikeAddress<T>
            where Address.Differential : IPointerLikeAddress<T>
            where OutputVector : IVector<T>
    {
        // T.Differential == T, so Address.Differential == Address at the concrete level.
        typealias OutputVec = WaveTangledVector<T, ShMemSize, OutputSize, SubgroupSize, SubgroupCount>;
        let dOutConcrete = bit_cast<OutputVec>(dOut);
        let weightGradConcrete = bit_cast<Address>(weightGradAddress);
        let biasGradConcrete = bit_cast<Address>(biasGradAddress);

        __target_switch
        {
        case cuda:
            typealias Cuda = TiledMMACuda<T, InputSize, OutputSize, SubgroupSize, Target>;
            return Cuda.backward<Address, ShMemSize, SubgroupCount, Bias>(
                input, dOutConcrete, weightAddress, weightGradConcrete, biasGradConcrete);
        case spirv:
            typealias Vulkan = TiledMMAVulkan<T, InputSize, OutputSize, SubgroupSize, Target, ShMemSize, TransposeA, SubgroupCount>;
            return Vulkan.backward<Address, Bias>(
                input, dOutConcrete, weightAddress, weightGradConcrete, biasGradConcrete);
        }
    }
}
