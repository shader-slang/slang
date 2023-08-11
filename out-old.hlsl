#pragma pack_matrix(column_major)
#ifdef SLANG_HLSL_ENABLE_NVAPI
#include "nvHLSLExtns.h"
#endif
#pragma warning(disable: 3557)


#line 43 "./tests/language-feature/multi-level-break.slang"
RWStructuredBuffer<int > outputBuffer_0 : register(u0);


#line 6
int test_0(int r_0)
{

#line 6
    int result_0;

#line 18
    bool _S1 = r_0 == int(0);

#line 24
    bool _S2 = r_0 == int(1);

#line 24
    int i_0 = int(0);

#line 24
    int result_1 = int(0);

#line 24
    for(;;)
    {

#line 10
        if(i_0 < int(2))
        {
        }
        else
        {

#line 10
            result_0 = result_1;

#line 10
            break;
        }

#line 10
        int _S3;

#line 10
        int _S4 = i_0 + int(1);

#line 10
        int j_0 = int(0);

#line 10
        int result_2 = result_1;

#line 10
        int result_3 = result_1;

#line 10
        for(;;)
        {

            if(j_0 < int(3))
            {
            }
            else
            {

#line 13
                _S3 = int(1);

#line 13
                result_0 = result_2;

#line 13
                break;
            }


            int _S5 = result_2 + int(1);

#line 13
            int _S6 = j_0 + int(1);

            for(;;)
            {

                if(_S1)
                {

#line 18
                    _S3 = int(0);

#line 18
                    result_0 = _S5;

#line 18
                    break;
                }
                else
                {


                    if(_S2)
                    {

#line 24
                        _S3 = int(1);

#line 24
                        result_0 = _S5;

#line 24
                        break;
                    }
                    else
                    {

#line 24
                        _S3 = int(2);

#line 24
                        result_0 = result_3;

#line 24
                        break;
                    }

#line 24
                }

#line 24
            }

#line 24
            if(_S3 != int(2))
            {

#line 24
                break;
            }

#line 13
            j_0 = _S6;

#line 13
            result_2 = _S5;

#line 13
            result_3 = result_0;

#line 13
        }

#line 13
        if(_S3 != int(1))
        {

#line 13
            break;
        }

#line 10
        i_0 = _S4;

#line 10
        result_1 = result_0;

#line 10
    }

#line 39
    return result_0;
}


#line 46
[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID_0 : SV_DISPATCHTHREADID)
{

#line 48
    int _S7 = test_0(int(0));

#line 48
    outputBuffer_0[0U] = _S7;
    int _S8 = test_0(int(1));

#line 49
    outputBuffer_0[1U] = _S8;
    int _S9 = test_0(int(2));

#line 50
    outputBuffer_0[2U] = _S9;
    outputBuffer_0[3U] = int(0);
    return;
}

