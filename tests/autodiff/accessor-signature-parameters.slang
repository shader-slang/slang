//TEST:SIMPLE: -no-codegen -Wno-38109
//TEST:SIMPLE(filecheck=CHECK): -no-codegen -dump-ir -Wno-38109

// A subscript accessor's function signature contains the subscript parameters before the
// accessor-owned parameters. Keep that ordering consistent wherever differentiability information
// is produced or consumed.

struct NoDiffValueStorage
{
    __subscript(float index) -> no_diff float
    {
        [Differentiable]
        set(no_diff float newValue)
        {}
    }
}

// `T` is not known to be differentiable when this declaration is checked, so the differentiable
// getter freezes its parent subscript parameter as `no_diff`. Specializing `T` to `float` must not
// revisit that declaration-time decision.
struct GenericIndexStorage<T>
{
    __subscript(T index) -> float
    {
        [Differentiable]
        get
        {
            return 1.0;
        }
    }
}

void exerciseSetterDerivative(
    inout NoDiffValueStorage storage,
    DifferentialPair<float> index,
    float newValue)
{
    // The parent subscript index is differentiable, while the accessor-owned new value is no_diff.
    fwd_diff(NoDiffValueStorage::__subscript::set)(storage, index, newValue);
}

DifferentialPair<float> exerciseSpecializedGetterDerivative(
    GenericIndexStorage<float> storage,
    float index)
{
    // The unspecialized declaration classified `index` as no_diff. Its specialization therefore
    // accepts an ordinary `float`, rather than revisiting the mode and requiring a pair.
    return fwd_diff(GenericIndexStorage<float>::__subscript::get)(storage, index);
}

// CHECK-LABEL: [nameHint("NoDiffValueStorage.operator[].set.fwd_diff")]
// CHECK: let {{.*}} : Func(Void,
// CHECK-SAME: BorrowInOutParam(Attributed(%NoDiffValueStorage,
// CHECK-SAME: DiffPair(Float,
// CHECK-SAME: Attributed(Float,
// CHECK-SAME: = ForwardDifferentiate(

// CHECK-LABEL: [nameHint("GenericIndexStorage.operator[].get.fwd_diff")]
// CHECK: let {{.*}} : Func(DiffPair(Float,
// CHECK-SAME: Attributed(specialize(
// CHECK-SAME: Attributed(%
// CHECK-SAME: = ForwardDifferentiate(
