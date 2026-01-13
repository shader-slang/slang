//TEST:SIMPLE(filecheck=CHECK):-target spirv -emit-spirv-directly

RWTexture2DMS<float> tex;

// CHECK: OpCapability StorageImageMultisample

[shader("fragment")]
float main() {
    return tex.Load(0.xx, 0);
}