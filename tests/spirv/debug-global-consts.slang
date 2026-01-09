//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry main -stage fragment -O0 -g2 -emit-spirv-directly -fvk-b-shift 0 0
struct PS_OUTPUT
{
    float Color : SV_Target0;
};

static const double cf1 = 3.0;

float main() : SV_TARGET{
   PS_OUTPUT o;
   o.Color = float(cf1);
   return o.Color;
}
// CHECK: %cf1 = OpConstant %double 3
