#pragma pack_matrix(column_major)

#line 6 "tests/cross-compile/loop-attribs.slang"
vector<float,4> main() : SV_TARGET
{
    int i_0;
    float sum_0;
    int j_0;
    float sum_1;
    i_0 = int(0);
    sum_0 = 0.00000000000000000000;
    [loop]
    for(;;)
    {

#line 11
        if(i_0 < int(100))
        {
        }
        else
        {
            break;
        }
        float _S1 = sum_0 + (float) i_0;

#line 11
        int _S2 = i_0 + (int) int(1);
        i_0 = _S2;
        sum_0 = _S1;
    }
    j_0 = int(0);
    sum_1 = sum_0;
    [unroll]
    for(;;)
    {

#line 15
        if(j_0 < int(100))
        {
        }
        else
        {
            break;
        }
        float _S3 = sum_1 + (float) j_0;

#line 15
        int _S4 = j_0 + (int) int(1);
        j_0 = _S4;
        sum_1 = _S3;
    }

#line 18
    return vector<float,4>(sum_1, (float) int(0), (float) int(0), (float) int(0));
}