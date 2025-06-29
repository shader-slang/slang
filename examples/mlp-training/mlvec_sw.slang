implementing mlp_sw;

public struct MLVec<int N> : IDifferentiable
{
    public NFloat data[N];
    
    [Differentiable]
    public NFloat[N] toArray()
    {
        return data;
    }

    [Differentiable]
    public static MLVec<N> fromArray(NFloat[N] values)
    {
        MLVec<N> result;
        [ForceUnroll]
        for (int i = 0; i < N; i++)
            result.data[i] = values[i];
        return result;
    }
}

MLVec<OutputSize> matMulAdd<int OutputSize, int InputSize>(MLVec<InputSize> input, NFloat* matrix, NFloat* bias)
{
    let getMatElem = (int row, int col) => matrix[row*InputSize + col];
    let getBias = (int idx) => bias[idx];
    MLVec<OutputSize> result = {};
    for (int i = 0; i < OutputSize; i++)
    {
        NFloat r = getBias(i);
        for (int j = 0; j < InputSize; j++)
            r += getMatElem(i, j) * input.data[j];
        result.data[i] = r;
    }
    return result;
}

MLVec<OutputSize> matMulTransposed<int OutputSize, int InputSize>(MLVec<InputSize> input, NFloat* matrix)
{
    let getMatElem = (int row, int col) => matrix[col*OutputSize + row];
    MLVec<OutputSize> result = {};
    for (int i = 0; i < OutputSize; i++)
    {
        NFloat r = {};
        for (int j = 0; j < InputSize; j++)
            r += getMatElem(i, j) * input.data[j];
        result.data[i] = r;
    }
    return result;
}

void outerProductAccumulate<int M, int N>(MLVec<M> v0, MLVec<N> v1, NFloat* matrix)
{
    for (int i = 0; i < M; i++)
    {
        for (int j = 0; j < N; j++)
        {
            let elem = v0.data[i] * v1.data[j];
            half original;
            InterlockedAddF16Emulated(matrix + (i*N + j), elem, original);
        }
    }
}
