//DIAGNOSTIC_TEST:SIMPLE(diag=diag): -stage compute -entry computeMain -std 2018
//DIAGNOSTIC_TEST:SIMPLE(diag=diag): -stage compute -entry computeMain -std 2025
//DIAGNOSTIC_TEST:SIMPLE(diag=diag): -stage compute -entry computeMain -std 2026
//DIAGNOSTIC_TEST:SIMPLE(diag=diag): -stage compute -entry computeMain -std 202c
//
// Negative test for substatements. Diagnose places where only substatements are allowed.

void ifLetFn<T>(Optional<T> p)
{
    if (let i = p)
        int v0;
//diag:     ^^ declaration not allowed here
//diag:     ^^ variable definition is not allowed here.
    else
        int v1;
//diag:     ^^ declaration not allowed here
//diag:     ^^ variable definition is not allowed here.
}


[numthreads(1,1,1)]
void computeMain()
{
    if (true)
        int v0;
//diag:     ^^ declaration not allowed here
//diag:     ^^ variable definition is not allowed here.

    if (true)
        int v1;
//diag:     ^^ declaration not allowed here
//diag:     ^^ variable definition is not allowed here.
    else
        int v2;
//diag:     ^^ declaration not allowed here
//diag:     ^^ variable definition is not allowed here.

    while (false)
        int v3;
//diag:     ^^ declaration not allowed here
//diag:     ^^ variable definition is not allowed here.

    do
        int v4;
//diag:     ^^ declaration not allowed here
//diag:     ^^ variable definition is not allowed here.
    while (false);

    for (;;)
        int v5;
//diag:     ^^ declaration not allowed here
//diag:     ^^ variable definition is not allowed here.

    $for (i in Range(0, 10))
        int v6;
//diag:     ^^ declaration not allowed here
//diag:     ^^ variable definition is not allowed here.

    do
        int v7;
//diag:     ^^ declaration not allowed here
//diag:     ^^ variable definition is not allowed here.
    catch
        int v8;
//diag:     ^^ declaration not allowed here
//diag:     ^^ variable definition is not allowed here.
}
