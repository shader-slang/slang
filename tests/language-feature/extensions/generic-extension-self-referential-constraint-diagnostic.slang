//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -entry computeMain -stage compute

// A blanket extension whose only route to its own `where` clause is the
// conformance it provides cannot be applied, so the call is rejected with a
// diagnostic instead of recursing until the stack overflows (gh-13461).

interface IRec<X>
{
}

extension<T> T : IRec<T>
    where T : IRec<T>
{
}

interface IA<X>
{
}

interface IB<X>
{
}

extension<T> T : IA<T>
    where T : IB<T>
{
}

extension<T> T : IB<T>
    where T : IA<T>
{
}

// The equality spelling of the same shape is rejected where it is declared.
interface IOther<X>
{
}

extension<T> T : IRec<T>
    where T == IOther<T>
//CHECK:       ^^^^^^ type 'IOther<T>' is not a proper type to use in a generic equality constraint
{
}

struct HasOther : IOther<HasOther>
{
}

int useRec<T>(T t)
/*CHECK:
    ^^^^^^ see declaration of func useRec<T> -> int
*/
    where T : IRec<T>
{
    return 0;
}

int useA<T>(T t)
/*CHECK:
    ^^^^ see declaration of func useA<T> -> int
*/
    where T : IA<T>
{
    return 0;
}

struct Z
{
}

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    Z z;
    outputBuffer[0] = useRec(z);
//CHECK:                    ^ type argument doesn't conform to interface
//CHECK:                    ^ type argument 'Z' does not conform to the required interface 'IRec<Z>'
    outputBuffer[1] = useA(z);
//CHECK:                  ^ type argument doesn't conform to interface
//CHECK:                  ^ type argument 'Z' does not conform to the required interface 'IA<Z>'
}
