//DIAGNOSTIC_TEST:SIMPLE(diag=LEGACY): -std 2026
//DIAGNOSTIC_TEST:SIMPLE(diag=SLANG202C): -std 202c
//DIAGNOSTIC_TEST:SIMPLE(filecheck=ERROR): -std 2026 -warnings-as-errors 40021
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

struct NonGenericResult
{
}

struct GenericResult
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

NonGenericResult preferNonGeneric(int value)
{
    return NonGenericResult();
}

GenericResult preferNonGeneric<T>(T value)
{
    return GenericResult();
}

NonGenericResult preferNonGenericWithDefault(int value)
{
    return NonGenericResult();
}

GenericResult preferNonGenericWithDefault<T = int>(int value)
{
    return GenericResult();
}

void equalGenericCount<T>(T first, float3 second)
{
}

void equalGenericCount<T>(float3 first, T second)
{
}

void failedInference<T>(float value)
{
}

void failedInference<T, U>(float value)
{
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
//LEGACY: overload resolution selected an otherwise ambiguous candidate by preferring the single candidate with the fewest required generic parameters; this tie-breaker is removed in Slang 202c
//ERROR: error[E40021]: deprecated generic-parameter-count overload tie-breaker
//ERROR: overload resolution selected an otherwise ambiguous candidate by preferring the single candidate with the fewest required generic parameters; this tie-breaker is removed in Slang 202c
//SLANG202C: ambiguous call to 'choose' with arguments of type (vector<float,3>)

    // The fixed-size overload accepts a strict subset of the variable-size overload's domain, but
    // Slang does not yet implement that semantic specificity comparison. The legacy fallback
    // happens to choose the fixed-size declaration; Slang 202c conservatively reports ambiguity.
    FixedShapeResult shapeResult = chooseShape(float3(0.0));
//LEGACY: deprecated generic-parameter-count overload tie-breaker
//LEGACY: overload resolution selected an otherwise ambiguous candidate by preferring the single candidate with the fewest required generic parameters; this tie-breaker is removed in Slang 202c
//ERROR: error[E40021]: deprecated generic-parameter-count overload tie-breaker
//ERROR: overload resolution selected an otherwise ambiguous candidate by preferring the single candidate with the fewest required generic parameters; this tie-breaker is removed in Slang 202c
//SLANG202C: ambiguous call to 'chooseShape' with arguments of type (vector<float,3>)

    // Equal required generic-parameter counts cannot select a unique compatibility candidate. The
    // call therefore remains ambiguous in every language and warning mode, without diagnostic
    // 40021.
    equalGenericCount(float3(0.0), float3(0.0));
//LEGACY: ambiguous call to 'equalGenericCount' with arguments of type (vector<float,3>, vector<float,3>)
//LEGACY: candidate: func equalGenericCount<vector<float,3>>(float3, float3) -> void
//LEGACY: candidate: func equalGenericCount<vector<float,3>>(float3, float3) -> void
//SLANG202C: ambiguous call to 'equalGenericCount' with arguments of type (vector<float,3>, vector<float,3>)
//SLANG202C: candidate: func equalGenericCount<vector<float,3>>(float3, float3) -> void
//SLANG202C: candidate: func equalGenericCount<vector<float,3>>(float3, float3) -> void
//ERROR: ambiguous call to 'equalGenericCount' with arguments of type (vector<float,3>, vector<float,3>)
//DISABLED: ambiguous call to 'equalGenericCount' with arguments of type (vector<float,3>, vector<float,3>)
//DISABLED: candidate: func equalGenericCount<vector<float,3>>(float3, float3) -> void
//DISABLED: candidate: func equalGenericCount<vector<float,3>>(float3, float3) -> void

    // Both candidates fail generic inference, so the tied frontier is intentionally retained for
    // the ordinary overload diagnostic. Unequal generic counts must not let the compatibility
    // fallback promote either failed candidate or emit diagnostic 40021.
    failedInference(0.0);
//LEGACY: no overload for 'failedInference' applicable to arguments of type (float)
//LEGACY: candidate: func failedInference<T> -> void
//LEGACY: candidate: func failedInference<T, U> -> void
//SLANG202C: no overload for 'failedInference' applicable to arguments of type (float)
//SLANG202C: candidate: func failedInference<T> -> void
//SLANG202C: candidate: func failedInference<T, U> -> void
//ERROR: error[E39999]: no overload for 'failedInference' applicable to arguments of type (float)
//DISABLED: no overload for 'failedInference' applicable to arguments of type (float)
//DISABLED: candidate: func failedInference<T> -> void
//DISABLED: candidate: func failedInference<T, U> -> void

    // An otherwise-equivalent non-generic declaration accepts a strict subset of the generic
    // overload's domain. This ordinary specificity rule is unchanged in Slang 202c and does not
    // use or warn about the compatibility fallback. The distinct result types make the assignment
    // verify which overload was selected.
    NonGenericResult nonGenericResult = preferNonGeneric(1);

    // The non-generic rule depends on the presence of a generic specialization, not its number of
    // required parameters. Even though `T` has a default, the generic declaration also accepts
    // explicit specialization arguments and therefore has the broader domain.
    NonGenericResult defaultedNonGenericResult = preferNonGenericWithDefault(1);

    // OverloadRank is an ordinary ranking criterion, so it resolves this call before the legacy
    // fallback is considered in both language versions. The distinct result types make the
    // assignment verify which overload was selected.
    RankedResult rankedResult = rankedChoose(float3(0.0));
}
