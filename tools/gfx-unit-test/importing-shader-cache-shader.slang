#include "imported.slang"

            uniform RWStructuredBuffer<float> buffer;
            
            [shader("compute")]
            [numthreads(4, 1, 1)]
            void computeMain(
            uint3 sv_dispatchThreadID : SV_DispatchThreadID)
            {
                TestFunction test;
                for (uint i = 0; i < 4; ++i)
                {
                    test.simpleElementAdd(buffer, i);
                }
            }