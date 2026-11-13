// Under `-fp-mode precise`, NVRTC receives `--fmad=false`. Slang-emitted CUDA already spells scalar
// arithmetic with the `_rn` intrinsics, so this check uses pass-through CUDA, which reaches NVRTC
// unrewritten: only the flag keeps `a * b + c` from fusing into `fma.rn`.

//TEST:SIMPLE(filecheck=PRECISE): -pass-through nvrtc -target ptx -entry multiplyAdd -fp-mode precise
//TEST:SIMPLE(filecheck=DEFAULT): -pass-through nvrtc -target ptx -entry multiplyAdd

extern "C" __global__ void multiplyAdd(float* p)
{
    p[3] = p[0] * p[1] + p[2];
}

//PRECISE-NOT: fma.rn
//PRECISE: mul.rn.f32
//PRECISE-NOT: fma.rn
//PRECISE: add.rn.f32
//PRECISE-NOT: fma.rn

//DEFAULT: fma.rn.f32
