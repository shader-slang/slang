implementing differentiable;

// The core-module exp10 overloads do not currently carry backward-derivative registrations. Keep
// the public adapter in a separately checked module so that its call cannot resolve back through
// the interface-generic exp10 overload declared by this module, and attach both derivatives to a
// private forwarding declaration here. Derivative registrations are resolved while checking the
// module that contains the differentiable call, so this forwarding declaration must live in the
// differentiable annex rather than in the adapter module.
[__readNone]
T __differentiableNumericsRegisteredExp10<T : __BuiltinFloatingPointType>(T value)
{
    return __differentiableNumericsExp10(value);
}

[__readNone]
vector<T, N> __differentiableNumericsRegisteredExp10<
    T : __BuiltinFloatingPointType,
    let N : int>(vector<T, N> value)
{
    return __differentiableNumericsExp10(value);
}

[PreferRecompute]
[ForwardDerivativeOf(__differentiableNumericsRegisteredExp10)]
DifferentialPair<T> __fwdDifferentiableNumericsExp10<T : __BuiltinFloatingPointType>(
    DifferentialPair<T> value)
{
    T primal = __differentiableNumericsExp10(value.p);
    T.Differential differential = __slang_noop_cast<T.Differential>(
        primal * T(2.302585092994045901) * __slang_noop_cast<T>(value.d));
    return diffPair(primal, differential);
}

[PreferRecompute]
[ForwardDerivativeOf(__differentiableNumericsRegisteredExp10)]
DifferentialPair<vector<T, N>> __fwdDifferentiableNumericsExp10<
    T : __BuiltinFloatingPointType,
    let N : int>(DifferentialPair<vector<T, N>> value)
{
    vector<T, N> primal = __differentiableNumericsExp10(value.p);
    vector<T, N> differential = primal * T(2.302585092994045901) * value.d;
    return diffPair(primal, differential);
}

[PreferRecompute]
[BackwardDerivativeOf(__differentiableNumericsRegisteredExp10)]
void __bwdDifferentiableNumericsExp10<T : __BuiltinFloatingPointType>(
    inout DifferentialPair<T> value,
    T.Differential outputDifferential)
{
    T differential =
        __differentiableNumericsExp10(value.p) * T(2.302585092994045901) *
        __slang_noop_cast<T>(outputDifferential);
    value = diffPair(value.p, __slang_noop_cast<T.Differential>(differential));
}

[PreferRecompute]
[BackwardDerivativeOf(__differentiableNumericsRegisteredExp10)]
void __bwdDifferentiableNumericsExp10<
    T : __BuiltinFloatingPointType,
    let N : int>(
        inout DifferentialPair<vector<T, N>> value,
        vector<T, N>.Differential outputDifferential)
{
    vector<T, N> differential =
        __differentiableNumericsExp10(value.p) * T(2.302585092994045901) * outputDifferential;
    value = diffPair(value.p, differential);
}
