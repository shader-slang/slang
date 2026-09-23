// Various bad case/default statement placements
//
// While the parser allows only switch body placements, the various
// tests here intend to catch potential ICEs related to bad placement.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

RWStructuredBuffer<int> output;

void caseWithoutSwitch()
{
    case 0:
/*CHECK:
    ^^^^ 'case' is not allowed outside a 'switch' body
    ^^^^ 'case' is only allowed in a 'switch' body
*/

    default:
/*CHECK:
    ^^^^^^^ 'default' is not allowed outside a 'switch' body
    ^^^^^^^ 'default' is only allowed in a 'switch' body
*/

    return;
}

void caseInNestedBlock(uint i)
{
    switch (i)
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

void caseInIf(uint i)
{
    switch (i)
    {
    case 1:
        if (i > 0U)
        {
        case 0U:
//CHECK:^^^^ 'case' is not allowed outside a 'switch' body
//CHECK:^^^^ 'case' is only allowed in a 'switch' body
            break;

        default:
//CHECK:^^^^^^^ 'default' is not allowed outside a 'switch' body
//CHECK:^^^^^^^ 'default' is only allowed in a 'switch' body

        }
    }
}

void caseInFor(uint i)
{
    uint j = 0U;
    switch (i)
    {
    case 1:
        for (j = 0; j < 10; ++j)
        {
        case 0U:
//CHECK:^^^^ 'case' is not allowed outside a 'switch' body
//CHECK:^^^^ 'case' is only allowed in a 'switch' body
            break;

        default:
//CHECK:^^^^^^^ 'default' is not allowed outside a 'switch' body
//CHECK:^^^^^^^ 'default' is only allowed in a 'switch' body

        }
    }
}

void caseInWhile(uint i)
{
    switch (i)
    {
    case 1:
        while (true)
        {
        case 0U:
//CHECK:^^^^ 'case' is not allowed outside a 'switch' body
//CHECK:^^^^ 'case' is only allowed in a 'switch' body

        default:
//CHECK:^^^^^^^ 'default' is not allowed outside a 'switch' body
//CHECK:^^^^^^^ 'default' is only allowed in a 'switch' body

            break;
        }
    }
}

void caseInDoWhile(uint i)
{
    switch (i)
    {
    case 1:
        do
        {
        case 0U:
//CHECK:^^^^ 'case' is not allowed outside a 'switch' body
//CHECK:^^^^ 'case' is only allowed in a 'switch' body

        default:
//CHECK:^^^^^^^ 'default' is not allowed outside a 'switch' body
//CHECK:^^^^^^^ 'default' is only allowed in a 'switch' body

            break;
        }
        while (true);
    }
}

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    caseWithoutSwitch();
    caseInNestedBlock(tid.x);
    caseInIf(tid.x);
    caseInFor(tid.x);
    caseInWhile(tid.x);
    caseInDoWhile(tid.x);
}
