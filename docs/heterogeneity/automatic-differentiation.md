# Automatic differentiation

Most of [Slang's automatic differentiation features](https://docs.shader-slang.org/en/latest/external/slang/docs/user-guide/07-autodiff.html)
carry over: differentiable functions, `fwd_diff`, `bwd_diff`, and custom derivatives.

The core heterogeneous mechanisms (dispatch, dispatchRays, etc..) are not differentiable by default. A wrapper must define how gradients are computed for a dispatch;
`slang.tensor.differentiable` provides this for Tensor `dmap` and `dsum`.

The tensor examples require a native backend with bindless descriptors. The current
browser WebGPU backend cannot execute these tensor operations.

<!-- prettier-ignore -->
!!! note
    Auto-diff over functions that **mutate tensors in-place** is very fragile since the backward pass may rely on values that have been overwritten. Making sure the right warnings/errors are issued is work-in-progress.

## Differentiable lambdas and chained compute

`~ =>` creates a differentiable lambda. The following program chains two GPU maps
and a reduction, then differentiates the whole host function.

For `x = [3, 3, 3, 3]`, compute `sum((2x + 1)Â²) = 196`.
Each input's gradient is `4(2x + 1) = 28`.

```slang
import slang.runtime;
import slang.tensor.differentiable;

[host]
[BackwardDifferentiable]
float loss(Tensor<float, 1> x)
{
    let y = x.dmap((uint1 i, float value) ~ => 2.0 * value + 1.0);
    let squared = y.dmap((uint1 i, float value) ~ => value * value);
    let total = squared.dsum();
    return total[0];
}

[host]
void main()
{
    let x = Tensor<float, 1>(4u);
    x.fill(3.0);
    let dx = x.allocateDifferential(); // Same shape, initialized to zero.
    var pair = DifferentialPtrPair<Tensor<float, 1>>(x, dx);
    print(loss(x)); // 196.000000
    bwd_diff(loss)(pair, 1.0); // Seed dLoss/dLoss = 1.
    print(dx[0]); // 28.000000
}
```

The map callbacks are device code. Their custom dispatch wrappers retain the values
needed for backward evaluation and accumulate into the supplied gradient tensors.
Ordinary `Grid::dispatch`, `map`, and `reduce` do not acquire this behavior automatically.

## Rough edges

**Replay and aliasing.** Differentiable map callbacks can access oher buffers, but subject to the constraint that the accessed data must not be mutated between the forward and backward passes.
e.g.

```c
var y = x.dmap((uint1 i, float value) ~ => 2.0 * value + 1.0);

// This is okay provided y's data is not mutated later.
var z = y.dmap((uint1 i, float value) ~ => y[i] * 2.0);

// e.g. doing this will cause issues
Grid(size).dispatch((uint i) => y[i] = z[i] * 2.0);
```

This will become less of an issue once the custom derivatives use the autodiff 2.0 system to store forward pass context explicitly.

**Captured gradients.** Tensor backward accumulation uses floating-point atomic addition, which may not be supported on some backends (D3D12, WebGPU, etc..). CUDA is recommended for running the backward passes.

For differentiable captures and a full optimization loop, see the standalone
[affine tensor fitting example](../../examples/host-compute/tensor-affine-fit.slang).
