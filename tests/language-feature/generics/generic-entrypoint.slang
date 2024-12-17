//TEST:SIMPLE(filecheck=CHECK): -target glsl -entry "vertMain<X>" -stage vertex

// CHECK: gl_Position = vec4(100.0, 0.0, 0.0, 1.0);

interface I { int getValue(); }
struct X : I { [ForceInline] int getValue() { return 100; } }
float4 vertMain<T:I>(uniform T o) {
    return float4(o.getValue(), 0, 0, 1);
}

