//TEST:SIMPLE(filecheck=CHECK):-target spirv -emit-spirv-directly

Texture2DMS<float> tex;

// CHECK-NOT: OpCapability StorageImageMultisample

[shader("fragment")]
float main() {
    return tex.Load(0.xx, 0);
}