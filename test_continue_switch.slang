[shader("fragment")]
float main(): SV_Target0 {
    float sum = 0;

    for (int i = 0; i < 1; ++i) {
        sum += 1;

        switch (i) {
            case 0: {
                continue;
            }
        }
    }

    return sum;
}
