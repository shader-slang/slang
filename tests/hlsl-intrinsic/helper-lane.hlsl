float4 main() : SV_Target {
    float ret = 1.0;

    if (IsHelperLane()) {
        ret = 2.0;
    }

    return (float4) ret;
}
