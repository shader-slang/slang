// An explicit language selection that agrees with the file extension should preserve the stage
// implied by that extension. Omit both `-entry` and `-stage` so `.vert` must select `main` as a
// vertex entry point.

//TEST:SIMPLE_EX(filecheck=CHECK): -lang glsl tests/glsl/explicit-language-preserves-extension-stage.vert -target spirv -emit-spirv-directly

#version 450

void main()
{
    gl_Position = vec4(0.0);
}

// CHECK: OpEntryPoint Vertex
