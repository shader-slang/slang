//DIAGNOSTIC_TEST:SIMPLE:
float doSomething(int x)
{
    // This represents the exact issue from bug #6784
    float PI = 3.14159f;
    float sample_delta = 0.1f;
    float val = 0.0f;
    
    for (float sample_phi = 0.0; sample_phi < 2.0 * PI; sample_phi += sample_delta) {
            for (float sample_theta = 0.0; sample_theta < 0.5 * PI; sample_theta + sample_delta) { //
            val += sample_theta;
        }
    }
    
    return val;
}
