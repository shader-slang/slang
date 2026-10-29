//TEST:SIMPLE(filecheck=HPP): -target hpp -line-directive-mode none
//TEST:SIMPLE(filecheck=CPP): -target cpp -line-directive-mode none
//TEST:SIMPLE(filecheck=CUH): -target cuh -line-directive-mode none
//TEST:SIMPLE(filecheck=CUDA): -target cuda -line-directive-mode none

// Regression test for https://github.com/shader-slang/slang/issues/14028 :
// host C++ code accesses the fields of an `__extern_cpp` struct by their
// source names, and reads the header prototype's parameter names as API docs.

//HPP: struct ClosestResult
//HPP-NEXT: {
//HPP-NEXT: Vector<float, 3> point;
//HPP-NEXT: Vector<float, 2> bary;
//HPP-NEXT: float dist2;
//HPP-NEXT: };

//HPP: ClosestResult closestPointOnSegment(Vector<float, 3> p_{{[0-9]+}}, Vector<float, 3> a_{{[0-9]+}}, Vector<float, 3> b_{{[0-9]+}});

//CPP: struct ClosestResult
//CPP-NEXT: {
//CPP-NEXT: Vector<float, 3> point;
//CPP-NEXT: Vector<float, 2> bary;
//CPP-NEXT: float dist2;
//CPP-NEXT: };

//CPP: ClosestResult closestPointOnSegment(Vector<float, 3> p_{{[0-9]+}}, Vector<float, 3> a_{{[0-9]+}}, Vector<float, 3> b_{{[0-9]+}})
//CPP: ->point =
//CPP: ->bary =
//CPP: ->dist2 =

//CUH: struct ClosestResult
//CUH-NEXT: {
//CUH-NEXT: float3 point;
//CUH-NEXT: float2 bary;
//CUH-NEXT: float dist2;
//CUH-NEXT: };
//CUH: ClosestResult closestPointOnSegment(float3 p_{{[0-9]+}}, float3 a_{{[0-9]+}}, float3 b_{{[0-9]+}});

//CUDA: struct ClosestResult
//CUDA-NEXT: {
//CUDA-NEXT: float3 point;
//CUDA-NEXT: float2 bary;
//CUDA-NEXT: float dist2;
//CUDA-NEXT: };
//CUDA: ->dist2 =

public __extern_cpp struct ClosestResult
{
    public float3 point;
    public float2 bary;
    public float dist2;
}

public export __extern_cpp ClosestResult closestPointOnSegment(float3 p, float3 a, float3 b)
{
    float3 ab = b - a;
    float t = clamp(dot(p - a, ab) / max(dot(ab, ab), 1e-20f), 0.0f, 1.0f);
    ClosestResult r;
    r.point = a + t * ab;
    r.bary = float2(1.0f - t, t);
    float3 d = p - r.point;
    r.dist2 = dot(d, d);
    return r;
}

// Without an entry point no output is written to stdout.
[shader("compute")]
[numthreads(1, 1, 1)]
void dummyEntryPoint() {}
