// function-static-const.slang
//TEST_IGNORE_FILE

#pragma pack_matrix(column_major)

struct SLANG_ParameterGroup_C_0
{
    int index_0;
};

cbuffer C_0 : register(b0)
{
    SLANG_ParameterGroup_C_0 C_0;
}

static const int  kArray_0[16] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };

int test_0(int val_0)
{
    return kArray_0[val_0];
}

vector<float,4> main() : SV_TARGET
{
    int _S1 = test_0(C_0.index_0);
    return (vector<float,4>) _S1;
}
