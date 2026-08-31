//DIAGNOSTIC_TEST:SIMPLE(diag=LEGACY): -std 2026
//DIAGNOSTIC_TEST:SIMPLE(diag=SLANG202C): -std 202c
//DIAGNOSTIC_TEST:SIMPLE(diag=ERROR): -std 2026 -warnings-as-errors 40021
//DIAGNOSTIC_TEST:SIMPLE(diag=DISABLED): -std 2026 -warnings-disable 40021

struct BroadResult
{
}

struct ShapedResult
{
}

struct RankedResult
{
}

struct FixedShapeResult
{
}

struct VariableShapeResult
{
}

BroadResult choose<T : IFloat>(T value)
//SLANG202C:^^^^^^ candidate: func choose<vector<float,3>>(float3) -> BroadResult
{
    return BroadResult();
}

__generic<T : __BuiltinFloatingPointType, let N : int>
ShapedResult choose(vector<T, N> value)
//SLANG202C: ^^^^^^ candidate: func choose<float, 3>(float3) -> ShapedResult
{
    return ShapedResult();
}

FixedShapeResult chooseShape<T>(vector<T, 3> value)
//SLANG202C:     ^^^^^^^^^^^ candidate: func chooseShape<float>(float3) -> FixedShapeResult
{
    return FixedShapeResult();
}

__generic<T, let N : int>
VariableShapeResult chooseShape(vector<T, N> value)
//SLANG202C:        ^^^^^^^^^^^ candidate: func chooseShape<float, 3>(float3) -> VariableShapeResult
{
    return VariableShapeResult();
}

BroadResult rankedChoose<T : IFloat>(T value)
{
    return BroadResult();
}

__generic<T : __BuiltinFloatingPointType, let N : int>
[OverloadRank(1)]
RankedResult rankedChoose(vector<T, N> value)
{
    return RankedResult();
}

void test()
{
    // Slang 2026 preserves the existing behavior and selects the overload with fewer required
    // generic parameters, but warns that this compatibility rule is going away. In Slang 202c,
    // the parameter count no longer breaks the tie.
    BroadResult result = choose(float3(0.0));
//LEGACY: deprecated generic-parameter-count overload tie-breaker
//LEGACY: overload resolution selected an otherwise ambiguous candidate by preferring fewer required generic parameters; this tie-breaker is removed in Slang 202c
//ERROR: deprecated generic-parameter-count overload tie-breaker
//ERROR: overload resolution selected an otherwise ambiguous candidate by preferring fewer required generic parameters; this tie-breaker is removed in Slang 202c
//SLANG202C: ambiguous call to 'choose' with arguments of type (vector<float,3>)

    // The fixed-size overload accepts a strict subset of the variable-size overload's domain, but
    // Slang does not yet implement that semantic specificity comparison. The legacy fallback
    // happens to choose the fixed-size declaration; Slang 202c conservatively reports ambiguity.
    FixedShapeResult shapeResult = chooseShape(float3(0.0));
//LEGACY: deprecated generic-parameter-count overload tie-breaker
//LEGACY: overload resolution selected an otherwise ambiguous candidate by preferring fewer required generic parameters; this tie-breaker is removed in Slang 202c
//ERROR: deprecated generic-parameter-count overload tie-breaker
//ERROR: overload resolution selected an otherwise ambiguous candidate by preferring fewer required generic parameters; this tie-breaker is removed in Slang 202c
//SLANG202C: ambiguous call to 'chooseShape' with arguments of type (vector<float,3>)

    // OverloadRank is an ordinary ranking criterion, so it resolves this call before the legacy
    // fallback is considered in both language versions.
    RankedResult rankedResult = rankedChoose(float3(0.0));
}
