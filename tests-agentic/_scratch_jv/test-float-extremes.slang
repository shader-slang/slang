//TEST:INTERPRET(filecheck=CHECK):
void main()
{
    float pz = 0.0f;
    float nz = -0.0f;
    float inf = 1.0f / 0.0f;
    float ninf = -1.0f / 0.0f;
    float nan = inf - inf;
    // CHECK: pz_eq_nz=1
    printf("pz_eq_nz=%d\n", (pz == nz) ? 1 : 0);
    // CHECK: inf_gt=1
    printf("inf_gt=%d\n", (inf > 1.0e30f) ? 1 : 0);
    // CHECK: ninf_lt=1
    printf("ninf_lt=%d\n", (ninf < -1.0e30f) ? 1 : 0);
    // CHECK: nan_ne_self=1
    printf("nan_ne_self=%d\n", (nan != nan) ? 1 : 0);
}
