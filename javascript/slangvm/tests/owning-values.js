import { HTTPSlangFileSystem, standardLibraryURL } from "slangvm/http-file-system";
import { MemorySlangFileSystem, OverlaySlangFileSystem } from "slangvm/file-system";
/** Exercise the shared owning-value path without running unrelated browser workloads. */
export async function testOwningValues(slangvm, assert, backend = "none") {
    const types = ["Timer", "RWTexture2D<float4>", "RaytracingAccelerationStructure", "Window", "Box",
        "DescriptorHandle<RWTexture2D<float4>>", "Defer<uint>"];
    const sources = {
        "types.slang": `
            __target_intrinsic(hgbc, "OpTimerType") struct Timer {}
            __target_intrinsic(hgbc, "OpWindowType") struct Window {}
            __target_intrinsic(hgbc, "OpBoxType") struct Box {}
            struct Holder<T> { T first; T second; }
        `,
    };
    let producer = "import types;\n";
    let consumer = "import types;\n";
    for (const [i, type] of types.entries()) {
        const op = type === "Timer" ? "OpCreateTimer" : "OpNullLit";
        producer += `
            [host] ${type} make${i}() { return hgbc_asm { result : $$${type} = ${op}; }; }
            [host] Holder<${type}> wrap${i}(${type} value) { return Holder<${type}>(value, value); }
        `;
        consumer += `
            [host] ${type} copy${i}(${type} value) { return value; }
            [host] ${type} unwrap${i}(Holder<${type}> value) { return value.second; }
        `;
    }
    if (backend === "webgpu") {
        producer += `
            [shader("compute")] [numthreads(1, 1, 1)]
            void writePixel(RWTexture2D<float4> image) { image[uint2(0, 0)] = float4(23, 0, 0, 1); }
            [host] RWTexture2D<float4> makeImage() {
                uint size = 2; uint one = 1;
                let image = hgbc_asm {
                    result : $$RWTexture2D<float4> = OpAllocateResource
                        Texture Texture2D RGBA32Float DeviceLocal UnorderedAccess $$float4
                        $size $size $one 1 1 0
                        ShaderResource UnorderedAccess CopySource CopyDestination RenderTarget;
                };
                dispatch(writePixel, uint3(1, 1, 1))(image);
                return image;
            }
        `;
        consumer += `
            [shader("compute")] [numthreads(1, 1, 1)]
            void readPixel(RWTexture2D<float4> image, RWStructuredBuffer<uint> output) {
                output[0] = uint(image[uint2(0, 0)].x);
            }
            [host] uint readImage(RWTexture2D<float4> image) {
                let output = RWStructuredBuffer<uint>::alloc(1);
                dispatch(readPixel, uint3(1, 1, 1))(image, output);
                return output.readBack(0);
            }
        `;
    }
    sources["producer.slang"] = producer;
    sources["consumer.slang"] = consumer;
    const vm = await slangvm.init({ backend, fileSystem: new OverlaySlangFileSystem([
        new MemorySlangFileSystem(sources), new HTTPSlangFileSystem(standardLibraryURL),
    ]) });
    const survivors = [];
    try {
        const p = await vm.load("producer.slang");
        const c = await vm.load("consumer.slang");
        for (const [i, type] of types.entries()) {
            const original = await p[`make${i}`]();
            const nested = await p[`wrap${i}`](original);
            const copy = await c[`copy${i}`](original);
            original.dispose();
            const unpacked = await c[`unwrap${i}`](nested);
            assert(copy.typeName === unpacked.typeName, `${type}: direct and nested cross-module round trips`);
            copy.dispose();
            nested.dispose();
            survivors.push(unpacked);
        }
        if (backend === "webgpu") {
            const image = await p.makeImage();
            const nested = await p.wrap1(image);
            image.dispose();
            const alias = await c.unwrap1(nested);
            nested.dispose();
            assert(await c.readImage(alias) === 23, "texture survives parent disposal and retains GPU contents");
            survivors.push(alias);
        }
    } finally {
        await vm.close();
    }
    for (const value of survivors) {
        assert(!value.isValid, "VM close invalidates surviving owning value");
        value.dispose();
    }
}
