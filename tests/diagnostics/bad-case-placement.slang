//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

RWStructuredBuffer<int> output;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    switch (int(tid.x))
    {
    case 0:

        {
        case 1:
//CHECK:^^^^ 'case' is not allowed outside a 'switch' body
//CHECK:^^^^ 'case' is only allowed in a 'switch' body

        default:
//CHECK:^^^^^^^ 'default' is not allowed outside a 'switch' body
//CHECK:^^^^^^^ 'default' is only allowed in a 'switch' body
            output[0] = 1;
        }
        break;

    }
}
