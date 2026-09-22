# Matrix Layout Pragma Across Literate Blocks

This test checks that matrix-layout state carries across the separate source segments extracted
from a literate Slang file.

```slang
//TEST:SIMPLE: -target hlsl -profile cs_6_0 -entry main -matrix-layout-column-major -o pragma-pack-matrix-across-blocks.hlsl

typedef matrix<float, 2, 3, MatrixLayoutMode.RowMajor> RowMajorMatrix;

#pragma pack_matrix(row_major)
```

The pragma at the end of the preceding block must still apply in this block.

```slang
[numthreads(1, 1, 1)]
void main()
{
    float2x3 value;
    static_assert(value is RowMajorMatrix, "matrix layout state was not carried across blocks");
}
```
