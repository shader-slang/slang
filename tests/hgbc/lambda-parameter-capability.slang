//TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main

void acceptCallback<TCallback : IFunc<uint, BuiltInTriangleIntersectionAttributes>>(
    uint value,
    TCallback callback)
{
    // Referencing an ordinary parameter must not recursively acquire capabilities from the
    // generic constraint witness attached to the enclosing specialization.
    uint unused = value + 1;
}

[host]
void main()
{
    // Constructing and forwarding the closure does not execute its call operator, so the
    // closest-hit-only parameter type must not become a requirement of this host entry point.
    acceptCallback(1, (BuiltInTriangleIntersectionAttributes attributes) => 0u);
}

// CHECK: OpExport Function {{%[A-Za-z0-9_]+}} %main
