//TEST_IGNORE_FILE:
#pragma pack_matrix(column_major)

[earlydepthstencil]
vector<float,4> main() : SV_TARGET
{
    return vector<float,4>((float) 1, (float) 0, (float) 0, (float) 1);
}
