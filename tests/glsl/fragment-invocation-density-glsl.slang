//TEST:SIMPLE(filecheck=CHECK-GLSL): -target glsl -stage fragment -entry main

// Test for EXT_fragment_invocation_density support (gl_FragSizeEXT and gl_FragInvocationCountEXT )
import glsl;

layout(location = 0) out highp vec4 FragColor;

void main()
{
    // CHECK-GLSL: #extension GL_EXT_fragment_invocation_density : require
    // CHECK-GLSL-DAG: gl_FragSizeEXT
    // CHECK-GLSL-DAG: gl_FragInvocationCountEXT

    FragColor = vec4(gl_FragSizeEXT.x, gl_FragSizeEXT.y, gl_FragInvocationCountEXT, 1.0);
}