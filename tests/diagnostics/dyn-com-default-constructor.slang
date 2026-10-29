//TEST:SIMPLE(filecheck=CHECK): -std 2026 -target host-cpp -whole-program -DCTOR_KIND=0
//TEST:SIMPLE(filecheck=CHECK): -std 2026 -target host-cpp -whole-program -DCTOR_KIND=1
//TEST:SIMPLE(filecheck=CHECK): -std 202c -target host-cpp -whole-program -DCTOR_KIND=1

[COM("BE18F5D2-4522-4AB0-A6EE-1D157FA2B083")]
public interface IFoo {}
typealias Box = dyn IFoo;

// Proper-type conversion must preserve the existing prohibition on default interface values.
export __extern_cpp NativeRef<Box> makeNull()
{
#if CTOR_KIND == 0
    return NativeRef<Box>(IFoo());
#else
    return NativeRef<Box>(Box());
#endif
}

//CHECK-NOT: internal error
//CHECK: error[E30524]
//CHECK-NOT: internal error
