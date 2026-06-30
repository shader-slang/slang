<script setup lang="ts">
import { RenderCanvas } from 'playground-render-canvas';
import 'playground-render-canvas/dist/playground-render-canvas.css';
import { CompiledPlayground, PlaygroundMessage } from 'slang-playground-shared';
import { onBeforeMount, shallowRef, useTemplateRef } from 'vue';


function isWebGPUSupported() {
    return 'gpu' in navigator && navigator.gpu !== null;
}

async function tryGetDevice() {
    if (!isWebGPUSupported()) {
        console.log('WebGPU is not supported in this browser');
        return null;
    }
    const adapter = await navigator.gpu?.requestAdapter();
    if (!adapter) {
        console.log('need a browser that supports WebGPU');
        return null;
    }
    // Allow list of GPU features that can impact WGSL
    const allowedFeatures: GPUFeatureName[] = [
        "depth32float-stencil8",
        "texture-compression-bc",
        "texture-compression-bc-sliced-3d",
        "texture-compression-etc2",
        "texture-compression-astc",
        "texture-compression-astc-sliced-3d",
        "shader-f16",
        "rg11b10ufloat-renderable",
        "bgra8unorm-storage",
        "float32-filterable",
        "float32-blendable",
        "clip-distances",
        "dual-source-blending",
    ];
    const requiredFeatures: GPUFeatureName[] = allowedFeatures.filter(f => adapter.features.has(f));

    let device = await adapter.requestDevice({ requiredFeatures });
    if (!device) {
        console.log('need a browser that supports WebGPU');
        return null;
    }
    return device;
}

const device = shallowRef<GPUDevice | null>(null);
const renderCanvas = useTemplateRef("renderCanvas");

onBeforeMount(async () => {
    device.value = await tryGetDevice();
});

let hasInitialized = false;
let persistentPlaygroundRunData: CompiledPlayground | null = null;
window.addEventListener('message', event => {
    const playgroundRunData: PlaygroundMessage = event.data;
    if(playgroundRunData.type == "init") {
        persistentPlaygroundRunData = playgroundRunData.payload;
        if(renderCanvas.value != undefined) {
            renderCanvas.value.onRun(persistentPlaygroundRunData)
            hasInitialized = true;
        }
    } else if(playgroundRunData.type == "uniformUpdate") {
        persistentPlaygroundRunData.uniformComponents = playgroundRunData.payload;
    }
});

function canvasMounted() {
    if (!hasInitialized) {
        renderCanvas.value.onRun(persistentPlaygroundRunData);
        hasInitialized = true;
    }
}

declare function acquireVsCodeApi(): {
    postMessage: (msg: any) => void;
    setState: (state: any) => void;
    getState: () => any;
};
const vscode = acquireVsCodeApi();

function logOutput(text: string) {
    vscode.postMessage({
        type: 'log',
        text: text
    });
}

</script>

<template>
    <RenderCanvas v-if="device != null" :device="device" :show-fullscreen-toggle="false" @log-error="(error) => { console.error(error) }" @log-output="logOutput" @mounted="canvasMounted"
                    ref="renderCanvas"></RenderCanvas>
</template>
