// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

import slang.compute.texture_utils;
import slang.runtime;


public struct MSELoss<T, int N> : IFunc<T, vector<T, N>, vector<T, N>>
    where T : __BuiltinFloatingPointType
    where vector<T, N>
{
    private vector<T, N> channelWeights;

    __init()
    {
        this.channelWeights = vector<T, N>(1.0);
        // this.sumWeights = T(N);

        // if (!(this.sumWeights > T(0)))
        //     fail("Image comparison channel weights must have a positive sum");
    }

    __init(vector<T, N> wts)
    {
        this.channelWeights = wts;
        // this.sumWeights = T(0);

        // if (!(this.sumWeights > T(0)))
        //     fail("Image comparison channel weights must have a positive sum");
    }

    public T operator()(vector<T, N> actualValue, vector<T, N> expectedValue)
    {
        T sumWeights = T(0);

        // TODO: Once interpreters can index vectors, we should move this into
        // the CPU side.
        [ForceUnroll]
        for (uint i = 0; i < N; ++i)
            sumWeights += channelWeights[i];

        vector<T, N> difference = actualValue - expectedValue;
        return dot(difference * difference, channelWeights) / sumWeights;
    }
}

/// Computes the channel-weighted mean squared error between equally sized images.
[host]
public T computePixelwiseAggregateError<
    T,
    int N,
    let ActualFormat : int,
    let ExpectedFormat : int,
    LossFunc : IFunc<T, vector<T, N>, vector<T, N>>>(
    RWTexture2D<vector<T, N>, 0, ActualFormat> actual,
    RWTexture2D<vector<T, N>, 0, ExpectedFormat> expected,
    LossFunc lossFunc)
    where T : __BuiltinFloatingPointType, ITexelElement
    where vector<T, N> : ITexelElement
{
    uint actualWidth;
    uint actualHeight;
    uint expectedWidth;
    uint expectedHeight;
    actual.GetDimensions(actualWidth, actualHeight);
    expected.GetDimensions(expectedWidth, expectedHeight);
    if (actualWidth != expectedWidth || actualHeight != expectedHeight)
        fail("Image comparison dimensions do not match");

    let loss = allocateTexture2D<T>(actualWidth, actualHeight);
    actual.mapToIndexed(
        loss,
        (uint2 location, vector<T, N> actualValue) =>
        {
            vector<T, N> expectedValue = expected[location];
            return lossFunc(actualValue, expectedValue);
        });

    let total = RWStructuredBuffer<T>::alloc(1);
    loss.reduceTo(total, (T a, T b) => a + b);
    return readBack(total, 0) / T(actualWidth * actualHeight);
}

public T computeImageMSE<T, int N, let ActualFormat : int, let ExpectedFormat : int>(
    RWTexture2D<vector<T, N>, 0, ActualFormat> actual,
    RWTexture2D<vector<T, N>, 0, ExpectedFormat> expected,
    vector<T, N> channelWeights = vector<T, N>(1.0))
    where T : __BuiltinFloatingPointType, ITexelElement
    where vector<T, N> : ITexelElement
{
    return computePixelwiseAggregateError(actual, expected, MSELoss<T, N>(channelWeights));
}
