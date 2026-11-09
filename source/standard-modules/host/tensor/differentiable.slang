// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

__exported import slang.tensor;
import slang.compute.buffer_utils;
import slang.runtime.compute.grid;
import slang.runtime.core.simple_assert;

/// Value and tangent get independent allocations, with no host readback during construction.
[host]
public __func_extension<T : IDifferentiable, let Rank : int> fwd_diff(Tensor<T, Rank>::fromValue)(
    DifferentialPair<T> value)
    ->DifferentialPtrPair<Tensor<T, Rank>>
{
    return DifferentialPtrPair<Tensor<T, Rank>>(
        Tensor<T, Rank>.fromValue(value.p),
        Tensor<T.Differential, Rank>.fromValue(value.d));
}

/// Retains the exact output adjoint allocation populated by downstream backward operations.
public struct TensorFromValueContext<T : IDifferentiable, let Rank : int>
{
    public Tensor<T.Differential, Rank> gradient;

    [host]
    public void operator()(out T.Differential dValue)
    {
        // This GPU-to-host boundary waits for all contributions to the singleton adjoint.
        dValue = gradient.readBack(0);
    }
}

[host]
public __func_extension<T : IDifferentiable, let Rank : int> __apply(Tensor<T, Rank>::fromValue)(
    T value)
    ->Tuple<DifferentialPtrPair<Tensor<T, Rank>>, TensorFromValueContext<T, Rank>>
{
    let output = Tensor<T, Rank>.fromValue(value);
    let gradient = output.allocateDifferential();
    return makeTuple(
        DifferentialPtrPair<Tensor<T, Rank>>(output, gradient),
        TensorFromValueContext<T, Rank>(gradient));
}

public extension<T : IDifferentiable, let Rank : int> Tensor<T, Rank>
{
    /// Allocates and clears a gradient tensor with this tensor's current shape.
    [host]
    public Tensor<T.Differential, Rank> allocateDifferential()
    {
        let result = Tensor<T.Differential, Rank>(shape);
        result.fill(T.dzero());
        return result;
    }

    /// Maps logical elements into a distinct tensor of the same shape, passing each coordinate
    /// as vector<uint, Rank> (including uint1 for rank one), followed by the element value.
    /// The output element type may differ, for example mapping RGB pixels to scalar errors.
    ///
    /// The callback must be deterministic and have no primal side effects. Captured primal tensors
    /// stay unchanged until backward completes. Output storage must not alias this tensor or a
    /// capture; captured active inputs must not alias this tensor's gradient destination.
    /// Allocate a separate output version and clear its gradient before each forward/backward use.
    [host]
    public void dmapTo<
        TOutput : IDifferentiable,
        F : IDifferentiableFuncWithDiffThis<TOutput, vector<uint, Rank>, T>>(
        Tensor<TOutput, Rank> output,
        F transform)
    {
        assert(!any(shape != output.shape));
        let input = this;
        buffer.mapToIndexed(
            output.buffer,
            elementCount(),
            (uint flat, T value) => transform(input.indexFromFlat(flat), value));
    }

    /// Maps logical elements into a new tensor, using the same coordinate callback as dmapTo.
    [host]
    public Tensor<TOutput, Rank> dmap<
        TOutput : IDifferentiable,
        F : IDifferentiableFuncWithDiffThis<TOutput, vector<uint, Rank>, T>>(F transform)
    {
        let result = Tensor<TOutput, Rank>(shape);
        dmapTo(result, transform);
        return result;
    }
}

// Keep the dispatch lambda in a named function: HGVM cannot yet resolve callback types
// nested directly inside a function extension.
[host]
void accumulateReadBackGradient<T : IDifferentiable, let Rank : int>(
    Tensor<T.Differential, Rank> destination,
    uint index,
    T.Differential seed)
{
    let gradient = destination.buffer;
    // One lane writes the element; separate queued readback derivatives accumulate in order.
    Grid(1u).dispatch(
        (uint threadIndex) =>
        { gradient[index] = T.dadd(gradient[index], seed); });
}

/// Accumulates the host result's gradient into the element that was read.
[host]
public __func_extension<T : IDifferentiable, let Rank : int> bwd_diff(Tensor<T, Rank>::readBack)(
    DifferentialPtrPair<Tensor<T, Rank>> self,
    uint index,
    T.Differential seed)
    ->void
{
    assert(!any(self.p.shape != self.d.shape));
    assert(index < self.p.elementCount());
    accumulateReadBackGradient<T, Rank>(self.d, index, seed);
}

/// Owns the saved primal contents and the original gradient destinations until queued work ends.
/// A context is single-use; separate forward operations may retain independent contexts.
public struct DMapContext<
    T : IDifferentiable,
    TOutput : IDifferentiable,
    let Rank : int,
    F : IDifferentiableFuncWithDiffThis<TOutput, vector<uint, Rank>, T>>
{
    public Tensor<T, Rank> savedInput;
    public Tensor<T.Differential, Rank> inputGradient;
    public Tensor<TOutput.Differential, Rank> outputGradient;
    public DifferentialPtrPair<F> transform;

    [host]
    public void operator()(out __none_t dInput, out __none_t dOutput, out __none_t dTransform)
    {
        dInput = none;
        dOutput = none;
        dTransform = none;
        let saved = savedInput;
        let inputAdjoint = inputGradient.buffer;
        let outputAdjoint = outputGradient.buffer;
        let function = transform;
        Grid(saved.elementCount())
            .dispatch(
                (uint flat) =>
                {
                    // Use the forward operation's shape even if the original tensor was reshaped
                    // later.
                    let index = saved.indexFromFlat(flat);
                    T value = saved.buffer[flat];
                    let applied = __apply(function.p.operator())(function, index, value);
                    let backward = function.p.operator().remat(applied._1, function, index, value);
                    __none_t receiverGradient;
                    __none_t indexGradient;
                    T.Differential elementGradient;
                    backward(receiverGradient, indexGradient, elementGradient, outputAdjoint[flat]);
                    // Each explicit input element has one writer in this dispatch. Separate queued
                    // contexts accumulate in order; shared captured parameters use atomic tensor
                    // reads.
                    inputAdjoint[flat] = T.dadd(inputAdjoint[flat], elementGradient);
                });
    }
}

[host]
public __func_extension<
    T : IDifferentiable,
    let Rank : int,
    TOutput : IDifferentiable,
    F : IDifferentiableFuncWithDiffThis<TOutput, vector<uint, Rank>, T>> __apply(Tensor<T, Rank>::
                                                                                     dmapTo)(
    DifferentialPtrPair<Tensor<T, Rank>> input,
    DifferentialPtrPair<Tensor<TOutput, Rank>> output,
    DifferentialPtrPair<F> transform)
    ->DMapContext<T, TOutput, Rank, F>
{
    assert(!any(input.p.shape != input.d.shape));
    assert(!any(output.p.shape != output.d.shape));
    let saved = input.p.copy();
    input.p.dmapTo(output.p, transform.p);
    return DMapContext<T, TOutput, Rank, F>(saved, input.d, output.d, transform);
}

/// Retains the allocating map's output gradient for the same backward dispatch as dmapTo.
public struct DMapResultContext<
    T : IDifferentiable,
    TOutput : IDifferentiable,
    let Rank : int,
    F : IDifferentiableFuncWithDiffThis<TOutput, vector<uint, Rank>, T>>
{
    public DMapContext<T, TOutput, Rank, F> map;

    [host]
    public void operator()(out __none_t dInput, out __none_t dTransform)
    {
        __none_t dOutput;
        map(dInput, dOutput, dTransform);
    }
}

[host]
public __func_extension<
    T : IDifferentiable,
    let Rank : int,
    TOutput : IDifferentiable,
    F : IDifferentiableFuncWithDiffThis<TOutput, vector<uint, Rank>, T>> __apply(Tensor<T, Rank>::
                                                                                     dmap)(
    DifferentialPtrPair<Tensor<T, Rank>> input,
    DifferentialPtrPair<F> transform)
    ->Tuple<DifferentialPtrPair<Tensor<TOutput, Rank>>, DMapResultContext<T, TOutput, Rank, F>>
{
    let output = Tensor<TOutput, Rank>(input.p.shape);
    let result = DifferentialPtrPair<Tensor<TOutput, Rank>>(output, output.allocateDifferential());
    let context = __apply(input.p.dmapTo)(input, result, transform);
    return makeTuple(result, DMapResultContext<T, TOutput, Rank, F>(context));
}

public extension<let Rank : int> Tensor<float, Rank>
{
    /// Sums logical elements into a distinct one-element tensor. The empty sum is zero.
    [host]
    public void dsumTo(Tensor<float, 1> output)
    {
        assert(output.elementCount() == 1);
        if (elementCount() == 0)
            output.fill(0.0f);
        else
            buffer.reduceTo(output.buffer, elementCount(), (float a, float b) => a + b);
    }

    /// Sums logical elements into a new one-element tensor. The empty sum is zero.
    [host]
    public Tensor<float, 1> dsum()
    {
        let result = Tensor<float, 1>(1u);
        dsumTo(result);
        return result;
    }
}

public struct DSumContext<let Rank : int>
{
    public Tensor<float, Rank> inputGradient;
    public Tensor<float, 1> outputGradient;
    public uint count;

    [host]
    public void operator()(out __none_t dInput, out __none_t dOutput)
    {
        dInput = none;
        dOutput = none;
        let inputAdjoint = inputGradient.buffer;
        let outputAdjoint = outputGradient.buffer;
        Grid(count).dispatch(
            (uint index) =>
            { inputAdjoint[index] += outputAdjoint[0]; });
    }
}

[host]
public __func_extension<let Rank : int> __apply(Tensor<float, Rank>::dsumTo)(
    DifferentialPtrPair<Tensor<float, Rank>> input,
    DifferentialPtrPair<Tensor<float, 1>> output)
    ->DSumContext<Rank>
{
    assert(!any(input.p.shape != input.d.shape));
    assert(!any(output.p.shape != output.d.shape));
    input.p.dsumTo(output.p);
    return DSumContext<Rank>(input.d, output.d, input.p.elementCount());
}

/// Retains the allocating sum's output gradient for the same backward broadcast as dsumTo.
public struct DSumResultContext<let Rank : int>
{
    public DSumContext<Rank> sum;

    [host]
    public void operator()(out __none_t dInput)
    {
        __none_t dOutput;
        sum(dInput, dOutput);
    }
}

[host]
public __func_extension<let Rank : int> __apply(Tensor<float, Rank>::dsum)(
    DifferentialPtrPair<Tensor<float, Rank>> input)
    ->Tuple<DifferentialPtrPair<Tensor<float, 1>>, DSumResultContext<Rank>>
{
    let output = Tensor<float, 1>(1u);
    let result = DifferentialPtrPair<Tensor<float, 1>>(output, output.allocateDifferential());
    let context = __apply(input.p.dsumTo)(input, result);
    return makeTuple(result, DSumResultContext<Rank>(context));
}

// Reads through captured float tensors accumulate into the supplied gradient allocation.
// Different lanes may read the same parameter, so the backward write must be atomic.
public __func_extension<let Rank : int> fwd_diff(Tensor<float, Rank>::__subscript::get)(
    DifferentialPtrPair<Tensor<float, Rank>> self,
    vector<uint, Rank> index)
    ->DifferentialPair<float>
{
    return diffPair(self.p[index], self.d[index]);
}

public __func_extension<let Rank : int> bwd_diff(Tensor<float, Rank>::__subscript::get)(
    DifferentialPtrPair<Tensor<float, Rank>> self,
    vector<uint, Rank> index,
    float seed)
    ->void
{
    uint flat = self.p.flatIndex(index);
    __target_switch
    {
    case hgbc: self.d.buffer[flat] += seed; break;
    default: InterlockedAdd(self.d.buffer[flat], seed); break;
    }
}

public __func_extension<let Width : int, let Rank : int> fwd_diff(
    Tensor<vector<float, Width>, Rank>::__subscript::get)(
    DifferentialPtrPair<Tensor<vector<float, Width>, Rank>> self,
    vector<uint, Rank> index)
    ->DifferentialPair<vector<float, Width>>
{
    return diffPair(self.p[index], self.d[index]);
}

public __func_extension<let Width : int, let Rank : int> bwd_diff(
    Tensor<vector<float, Width>, Rank>::__subscript::get)(
    DifferentialPtrPair<Tensor<vector<float, Width>, Rank>> self,
    vector<uint, Rank> index,
    vector<float, Width> seed)
    ->void
{
    uint flat = self.p.flatIndex(index);
    __target_switch
    {
    case hgbc: self.d.buffer[flat] += seed; break;
    default:
        [ForceUnroll]
        for (uint component = 0; component < uint(Width); ++component)
            InterlockedAdd(self.d.buffer[flat][component], seed[component]);
        break;
    }
}
