// The includer's row-major default must apply before this file changes it.
cbuffer InInclude : register(b1)
{
    float2x3 rowMatrix;
    float rowTail;
};

// This default must persist after returning to the includer.
#pragma pack_matrix(column_major)
