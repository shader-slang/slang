//TEST:SIMPLE(filecheck=DEFAULT): -target host-cpp -whole-program -DCTOR_KIND=0
//TEST:SIMPLE(filecheck=DEFAULT): -std 2025 -target host-cpp -whole-program -DCTOR_KIND=0
//TEST:SIMPLE(filecheck=DEFAULT): -target host-cpp -whole-program -DCTOR_KIND=1
//TEST:SIMPLE(filecheck=DEFAULT): -target host-cpp -whole-program -DCTOR_KIND=2
//TEST:SIMPLE(filecheck=CAST): -std 2026 -target host-cpp -whole-program -DCTOR_KIND=0 -DSKIP_DEFAULT
//TEST:SIMPLE(filecheck=CAST): -std 2026 -target host-cpp -whole-program -DCTOR_KIND=1 -DSKIP_DEFAULT
//TEST:SIMPLE(filecheck=CAST): -std 2026 -target host-cpp -whole-program -DCTOR_KIND=2 -DSKIP_DEFAULT
//TEST:SIMPLE(filecheck=CAST): -std 202c -target host-cpp -whole-program -DCTOR_KIND=2 -DSKIP_DEFAULT

[COM("BE18F5D2-4522-4AB0-A6EE-1D157FA2B083")]
public interface IFoo {}
typealias Alias = IFoo;
typealias Box = dyn IFoo;

// The calls to `identity(0)` also check inference for a unique generic function callee, whose
// type-level representation must not be mistaken for a constructor target.
T identity<T>(T value) { return value; }

// Both constructor forms must produce values of type `dyn IFoo`. A value incorrectly typed as
// `IFoo` would be boxed again when passed to `NativeRef`, generating an invalid `nullptr.get()` expression.
#ifndef SKIP_DEFAULT
export __extern_cpp NativeRef<Box> makeNull()
{
    int zero = identity(0);
#if CTOR_KIND == 0
    Box value = IFoo();
    return NativeRef<Box>(IFoo(value));
#elif CTOR_KIND == 1
    Box value = Alias();
    return NativeRef<Box>(Alias(value));
#else
    Box value = Box();
    return NativeRef<Box>(Box(value));
#endif
}
#endif

// Modern language versions disallow default construction even for COM-interface boxes. They
// still allow construction from an existing value and inference of a generic call's arguments.
export __extern_cpp NativeRef<Box> roundTrip(Box value)
{
    int zero = identity(0);
#if CTOR_KIND == 0
    return NativeRef<Box>(IFoo(value));
#elif CTOR_KIND == 1
    return NativeRef<Box>(Alias(value));
#else
    return NativeRef<Box>(Box(value));
#endif
}

//DEFAULT-LABEL: makeNull()
//DEFAULT: return nullptr;
//CAST-LABEL: roundTrip(
//CAST: return {{.*}}.get();
