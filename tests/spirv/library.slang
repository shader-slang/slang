//TEST:SIMPLE(filecheck=CHECK):-target spirv -emit-spirv-directly -incomplete-library -O0 -dump-ir

// CHECK: OpDecorate %f LinkageAttributes "_S1fp2pi_hi_hh" Export
export float16_t f(float16_t x, float16_t y) {
  return x + y;
}
