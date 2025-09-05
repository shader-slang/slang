<script setup lang="ts">
import TabContainer from './components/ui/TabContainer.vue'
import Tab from './components/ui/Tab.vue'
import Selector from './components/ui/Selector.vue'
import Tooltip from './components/ui/Tooltip.vue'
import Help from './components/Help.vue'
import { RenderCanvas } from 'playground-render-canvas';
import 'playground-render-canvas/dist/playground-render-canvas.css';
import { UniformPanel } from 'playground-uniform-panel';
import 'playground-uniform-panel/dist/playground-uniform-panel.css';
import { compiler, checkShaderType, slangd, moduleLoadingMessage } from './try-slang'
import { computed, defineAsyncComponent, onBeforeMount, onMounted, ref, shallowRef, useTemplateRef, watch, type Ref } from 'vue'
import { isWholeProgramTarget, compilePlayground } from 'slang-compilation-engine'
import { demoList } from './demo-list'
import { compressToBase64URL, decompressFromBase64URL, isWebGPUSupported } from './util'
import type { ThreadGroupSize } from './slang-wasm'
import { Splitpanes, Pane } from 'splitpanes'
import 'splitpanes/dist/splitpanes.css'
import ReflectionView from './components/ReflectionView.vue'
import { useWindowSize } from '@vueuse/core'
import { default as spirvTools } from "./spirv-tools.js";
import { type Result, type Shader, type ReflectionJSON, type RunnableShaderType, type ShaderType, type CallCommand, type HashedStringData, type ResourceCommand, type UniformController, isControllerRendered } from 'slang-playground-shared'

// MonacoEditor is a big component, so we load it asynchronously.
const MonacoEditor = defineAsyncComponent(() => import('./components/MonacoEditor.vue'))

// target -> profile mappings
const defaultShaderURL = "circle.slang";
const compileTargets = [
    "SPIRV",
    "HLSL",
    "GLSL",
    "METAL",
    "WGSL",
    "CUDA",
] as const
const targetProfileMap: { [target in typeof compileTargets[number]]?: { default: string, options: string[] } } = {
    // TODO: uncomment when we support specifying profiles.
    // "SPIRV": {default:"1.5", options:["1.3", "1.4", "1.5", "1.6"]},
};

const targetLanguageMap: { [target in typeof compileTargets[number]]: string } = {
    "SPIRV": "spirv",
    "HLSL": "generic-shader",
    "GLSL": "generic-shader",
    "METAL": "generic-shader",
    "WGSL": "wgsl",
    "CUDA": "cuda",
};

const codeEditor = useTemplateRef("codeEditor");
const codeGenArea = useTemplateRef("codeGenArea");

const tabContainer = useTemplateRef("tabContainer");

const shareButton = useTemplateRef("shareButton");
const tooltip = useTemplateRef("tooltip");
const helpModal = useTemplateRef("helpModal");
const targetSelect = useTemplateRef("targetSelect");
const renderCanvas = useTemplateRef("renderCanvas");

const selectedDemo = ref("");
const initialized = ref(false);
const showHelp = ref(false);

const selectedProfile = ref("");
const profiles = ref<string[]>([]);
const showProfiles = ref(false);

const selectedEntrypoint = ref("");
const entrypoints = ref<string[]>([]);
const showEntrypoints = ref(false);

const printedText = ref("");
const diagnosticsText = ref("");
watch(diagnosticsText, (newText, _) => {
    if (newText != "") {
        tabContainer.value?.setActiveTab("diagnostics")
    }
})
const device = shallowRef<GPUDevice | null>(null);

const currentDisplayMode = ref<ShaderType>("imageMain");
const uniformComponents = ref<UniformController[]>([])
const areAnyUniformsRendered = computed(() => uniformComponents.value.filter(isControllerRendered).length > 0);

const { width } = useWindowSize()

const isSmallScreen = computed(() => width.value < 768)
const smallScreenEditorVisible = ref(false);

const pageLoaded = ref(false);
let reflectionJson: any = {};


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

onBeforeMount(async () => {
    device.value = await tryGetDevice();
});

function updateProfileOptions() {
    const selectedTarget = targetSelect.value!.getValue();

    // If the selected target does not have any profiles, hide the profile dropdown.
    const profileData = targetProfileMap[selectedTarget] || null;
    if (!profileData) {
        profiles.value = [];
        showProfiles.value = false;
    }
    else {
        profiles.value = profileData.options;
        showProfiles.value = true;
        selectedProfile.value = profileData.default;
    }

    // If the target can be compiled as a whole program without entrypoint selection, hide the entrypoint dropdown.
    if (isWholeProgramTarget(selectedTarget)) {
        showEntrypoints.value = false;
    } else {
        showEntrypoints.value = true;
        updateEntryPointOptions();
    }
}

onMounted(async () => {
    pageLoaded.value = true;
    if (!device.value) {
        logError(moduleLoadingMessage + "Browser does not support WebGPU, Run shader feature is disabled.");
    }
    runIfFullyInitialized();

    window.addEventListener('slangLoaded', runIfFullyInitialized);

    document.addEventListener('keydown', function (event) {
        if (event.key === 'F5') {
            event.preventDefault();
            compileOrRun();
        }
        else if (event.ctrlKey && event.key === 'b') {
            event.preventDefault();
            // Your custom code here
            onCompile();
        }
    });
})

async function onShare() {
    if (!codeEditor.value) {
        throw new Error("Code editor not initialized");
    };
    if (!shareButton.value) {
        throw new Error("Share button not initialized");
    };
    try {
        const code = codeEditor.value.getValue();
        const compressed = await compressToBase64URL(code);
        let url = new URL(window.location.href.split('?')[0]);
        const compileTarget = targetSelect.value!.getValue();
        url.searchParams.set("target", compileTarget)
        url.searchParams.set("code", compressed);
        navigator.clipboard.writeText(url.href);
        tooltip.value!.showTooltip(shareButton.value, "Link copied to clipboard.");
    }
    catch (e) {
        tooltip.value!.showTooltip(shareButton.value, "Failed to copy link to clipboard.");
    }
}

function loadDemo(selectedDemoURL: string) {
    if (selectedDemoURL != "") {
        // Is `selectedDemoURL` a relative path?
        let finalURL;
        if (!selectedDemoURL.startsWith("http")) {
            // If so, append the current origin to it.
            // Combine the url to point to demos/${selectedDemoURL}.
            finalURL = new URL("demos/" + selectedDemoURL, window.location.href);
        }
        else {
            finalURL = new URL(selectedDemoURL);
        }
        // Retrieve text from selectedDemoURL.
        fetch(finalURL)
            .then((response) => response.text())
            .then((data) => {
                codeEditor.value?.setEditorValue(data);
                updateEntryPointOptions();
                compileOrRun();
            });
    }
}

// Function to update the profile dropdown based on the selected target
function updateEntryPointOptions() {
    if (!compiler)
        return;
    let entrypointsResult = compiler.findDefinedEntryPoints(codeEditor.value!.getValue(), window.location.href + "user.slang");
    if (!entrypointsResult.succ) {
        let errorMessage = `Failed to find entry points: ${entrypointsResult.message}`;
        if (entrypointsResult.log) {
            errorMessage += "\n" + entrypointsResult.log;
        }
        console.error(errorMessage);
        return;
    }
    entrypoints.value = entrypointsResult.result;
    if ((selectedEntrypoint.value == "" || !entrypoints.value.includes(selectedEntrypoint.value)) && entrypoints.value.length > 0)
        selectedEntrypoint.value = entrypoints.value[0];
}

function compileOrRun() {
    const userSource = codeEditor.value!.getValue();
    const shaderType = checkShaderType(userSource);

    if (shaderType == null) {
        onCompile();
    }
    else {
        if (device.value == null) {
            onCompile().then(() => {
                if (diagnosticsText.value == "")
                    diagnosticsText.value += `The shader compiled successfully, ` +
                        `but it cannot run because your browser does not support WebGPU.\n` +
                        `WebGPU is supported in Chrome, Edge, Firefox Nightly and Safari Technology Preview. ` +
                        `On iOS, WebGPU support requires Safari 16.4 or later and must be enabled in settings. ` +
                        `Please check your browser version and enable WebGPU if possible.`;
            });
        }
        else {
            doRun();
        }
    }
}

async function doRun() {
    smallScreenEditorVisible.value = false;
    diagnosticsText.value = "";

    if (!renderCanvas.value) {
        throw new Error("WebGPU is not supported in this browser");
    }
    const userSource = codeEditor.value!.getValue()

    // We will have some restrictions on runnable shader, the user code has to define imageMain or printMain function.
    // We will do a pre-filter on the user input source code, if it's not runnable, we will not run it.
    const shaderType = checkShaderType(userSource);
    if (shaderType == null) {
        toggleDisplayMode(null);
        codeGenArea.value?.setEditorValue("");
        throw new Error("Error: In order to run the shader, please define either imageMain or printMain function in the shader code.");
    }

    const entryPointName = shaderType;
    const compilationResult = await compileShader(userSource, entryPointName, "WGSL", false);

    if (compilationResult.succ == false) {
        // compileShader takes care of diagnostics already, so they aren't added here
        toggleDisplayMode(null);
        return;
    }

    const compiledPlaygroundResult = compilePlayground(compilationResult.result, window.location.href + 'user.slang', shaderType);

    if (compiledPlaygroundResult.succ == false) {
        toggleDisplayMode(null);
        diagnosticsText.value += compiledPlaygroundResult.message;
        if(compiledPlaygroundResult.log) {
            diagnosticsText.value += "\n" + compiledPlaygroundResult.log;
        }
        return;
    }

    const compiledPlayground = compiledPlaygroundResult.result;

    uniformComponents.value = compiledPlayground.uniformComponents

    if (areAnyUniformsRendered.value) {
        tabContainer.value?.setActiveTab("uniforms")
    }
    toggleDisplayMode(shaderType);

    renderCanvas.value.onRun(compiledPlayground);
}

// For the compile button action, we don't have restriction on user code that it has to define imageMain or printMain function.
// But if it doesn't define any of them, then user code has to define a entry point function name. Because our built-in shader
// have no way to call the user defined function, and compile engine cannot compile the source code.
async function onCompile() {
    smallScreenEditorVisible.value = false;
    diagnosticsText.value = "";

    toggleDisplayMode(null);
    const compileTarget = targetSelect.value!.getValue();

    await updateEntryPointOptions();

    if (selectedEntrypoint.value == "" && !isWholeProgramTarget(compileTarget)) {
        diagnosticsText.value = "Please select the entry point name";
        return;
    }

    if (compiler == null) throw new Error("Compiler doesn't exist");

    if (compileTarget == "SPIRV")
        await compiler.initSpirvTools(spirvTools);

    // compile the compute shader code from input text area
    const userSource = codeEditor.value!.getValue();
    compileShader(userSource, selectedEntrypoint.value, compileTarget, true);
}

function toggleDisplayMode(displayMode: ShaderType) {
    currentDisplayMode.value = displayMode;
}

async function compileShader(userSource: string, entryPoint: string, compileTarget: typeof compileTargets[number], noWebGPU: boolean): Promise<Result<Shader>> {
    if (compiler == null) throw new Error("No compiler available");
    const compiledResult = await compiler.compile({
        target: compileTarget,
        entrypoint: entryPoint,
        sourceCode: userSource,
        shaderPath: '/user.slang',
        noWebGPU,
    }, '/user.slang', [], spirvTools);
    if (compiledResult.succ == false) {
        diagnosticsText.value += compiledResult.message;
        if(compiledResult.log) {
            diagnosticsText.value += "\n" + compiledResult.log;
        }
        codeGenArea.value?.setEditorValue('Compilation returned empty result.');
        return compiledResult;
    }

    reflectionJson = compiledResult.result.reflection;

    codeGenArea.value?.setEditorValue(compiledResult.result.code);
    codeGenArea.value?.setLanguage(targetLanguageMap[compileTarget]);

    // Update reflection info.
    window.$jsontree.setJson("reflectionDiv", reflectionJson);
    window.$jsontree.refreshAll();

    return compiledResult;
}

function restoreFromURL(): boolean {
    const urlParams = new URLSearchParams(window.location.search);
    const target = urlParams.get('target');
    if (target) {
        if (!compileTargets.includes(target as any)) {
            diagnosticsText.value = "Invalid target specified in URL: " + target;
        } else {
            targetSelect.value!.setValue(target as any);
        }
    }

    updateProfileOptions();

    let gotCodeFromUrl = false;

    let demo = urlParams.get('demo');
    if (demo) {
        if (!demo.endsWith(".slang"))
            demo += ".slang";
        selectedDemo.value = demo;
        loadDemo(demo);
        gotCodeFromUrl = true;
    }

    const code = urlParams.get('code');
    if (code) {
        decompressFromBase64URL(code).then((decompressed) => {
            codeEditor.value!.setEditorValue(decompressed);
            updateEntryPointOptions();
            compileOrRun();
        });
        gotCodeFromUrl = true;
    }

    return gotCodeFromUrl;
}

async function runIfFullyInitialized() {
    if (compiler && slangd && pageLoaded && codeEditor.value) {
        initialized.value = true;

        let gotCodeFromUrl = restoreFromURL();

        if (gotCodeFromUrl) {
            // do nothing: code already set
        } else if (codeEditor.value.getValue() == "") {
            loadDemo(defaultShaderURL);
        } else {
            compileOrRun();
        }
    }
}

function logError(message: string) {
    diagnosticsText.value += message + "\n";
}
</script>

<template>
    <Tooltip ref="tooltip"></Tooltip>
    <Transition>
        <div id="loading-screen" v-if="!initialized">
            <img height="80px" src="./assets/slang-logo.svg" alt="Slang Logo" />
            <div id="loading-status-line">
                <div class="spinner"></div>
                <p class="loading-status" id="loadingStatusLabel">Loading Playground...</p>
            </div>
            <div id="progress-bar-container">
                <div class="progress-bar" id="progress-bar"></div>
            </div>
        </div>
    </Transition>
    <div class="mainContainer" v-show="initialized">
        <Splitpanes class="slang-theme" v-show="!isSmallScreen">
            <Pane class="leftContainer" size="62">
                <div id="big-screen-navbar"></div>
                <div class="workSpace" id="big-screen-editor">
                </div>
            </Pane>
            <Pane class="rightContainer">
                <Splitpanes horizontal class="resultSpace">
                    <Pane class="outputSpace" size="69" v-if="device != null" v-show="currentDisplayMode != null">
                    </Pane>
                    <Pane class="codeGenSpace">
                    </Pane>
                </Splitpanes>
            </Pane>
        </Splitpanes>
        <div id="small-screen-container" v-show="isSmallScreen">
            <div id="small-screen-navbar"></div>
            <Splitpanes horizontal class="resultSpace slang-theme" v-show="!smallScreenEditorVisible">
                <Pane id="small-screen-display" size="69" v-if="device != null" v-show="currentDisplayMode != null">
                </Pane>
                <Pane id="small-screen-code-gen"></Pane>
            </Splitpanes>
            <div id="small-screen-editor" v-show="smallScreenEditorVisible"></div>
        </div>
        <Teleport v-if="pageLoaded" defer :to="isSmallScreen ? '#small-screen-navbar' : '#big-screen-navbar'">
            <div class="navbar">
                <!-- Logo section -->
                <div class="navbar-logo">
                    <a href="/" title="Return to home page."><img src="./assets/slang-logo.svg" alt="Logo"
                            class="logo-img" /></a>
                </div>

                <!-- Load Demo section -->
                <div class="navbar-actions navbar-item">
                    <div class="navbar-group">
                        <select class="dropdown-select" id="demo-select" name="demo" aria-label="Load Demo"
                            v-model="selectedDemo" @change="loadDemo(selectedDemo)">
                            <option value="" disabled selected>Load Demo</option>
                            <option v-for="demo in demoList" :value="demo.url" :key="demo.url"
                                :disabled="demo.url == ''">{{ demo.name }}
                            </option>
                        </select>
                    </div>
                </div>

                <!-- Run section -->
                <div class="navbar-run navbar-item">
                    <button :disabled="device == null"
                        :title="device == null ? `Run shader feature is disabled because the current browser does not support WebGPU.` : `(F5) Compile and run the shader that provides either 'printMain' or 'imageMain'.);`"
                        @click="doRun()">&#9658;
                        Run</button>
                </div>

                <!-- Entry/Compile section -->
                <div class="navbar-compile navbar-item">
                    <Selector :options="compileTargets" modelValue="SPIRV" name="target" aria-label="target"
                        ref="targetSelect" @change="updateProfileOptions"></Selector>

                    <select class="dropdown-select" name="profile" aria-label="profile" v-model="selectedProfile"
                        v-show="showProfiles">
                        <option v-for="profile in profiles" :value="profile" :key="profile">{{
                            profile }}</option>
                    </select>

                    <select class="dropdown-select" name="entrypoint" aria-label="Entrypoint"
                        :onfocus="updateEntryPointOptions" @onmousedown="updateEntryPointOptions"
                        v-model="selectedEntrypoint" v-show="showEntrypoints">
                        <option value="" disabled selected>Entrypoint</option>
                        <option v-for="entrypoint in entrypoints" :value="entrypoint" :key="entrypoint">{{
                            entrypoint }}</option>
                    </select>

                    <button id="compile-btn"
                        title='(Ctrl+B) Compile the shader to the selected target. Entrypoints needs to be marked with the `[shader("stage")]` attribute.'
                        @click="onCompile()">
                        Compile
                    </button>
                </div>
                <!-- Share button section -->
                <div class="navbar-standalone-button-item">
                    <button class="svg-btn" title="Create sharable link" ref="shareButton" @click="onShare">
                        <svg xmlns="http://www.w3.org/2000/svg" width="28px" height="28px" viewBox="0 0 64 64"
                            fill="currentColor">
                            <path
                                d="M 9 9 L 9 14 L 9 54 L 51 54 L 56 54 L 55 42 L 51 42 L 51 49.095703 L 13 50 L 13.900391 14 L 21 14 L 21 10 L 9 9 z M 44 9 L 44 17.072266 C 29.919275 17.731863 19 23.439669 19 44 L 23 44 C 23 32.732824 29.174448 25.875825 44 25.080078 L 44 33 L 56 20.5 L 44 9 z">
                            </path>
                        </svg>
                    </button>
                </div>

                <!-- Help button section -->
                <div class="navbar-standalone-button-item">
                    <button class="svg-btn" title="Show Help" @click="helpModal!.openHelp()">
                        <svg xmlns="http://www.w3.org/2000/svg" width="28px" height="28px" viewBox="0 0 24 24"
                            fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round"
                            stroke-linejoin="round" class="feather feather-help-circle">
                            <circle cx="12" cy="12" r="10" />
                            <path d="M9.09 9a3 3 0 0 1 5.83 1c0 2-3 3-3 3" />
                            <line x1="12" y1="17" x2="12" y2="17" />
                        </svg>
                    </button>
                </div>
                <div class="navbar-standalone-button-item" v-show="isSmallScreen">
                    <button title="Toggle editor" @click="smallScreenEditorVisible = !smallScreenEditorVisible">
                        {{ smallScreenEditorVisible ? "View Results" : "View Source" }}
                    </button>
                </div>
            </div>
        </Teleport>
        <Teleport defer :to="isSmallScreen ? '#small-screen-display' : '.outputSpace'" v-if="device != null">
            <div id="renderOutput" v-show="currentDisplayMode == 'imageMain'">
                <RenderCanvas :device="device" :show-fullscreen-toggle="true" @log-error="logError" @log-output="(log) => { printedText = log }"
                    ref="renderCanvas"></RenderCanvas>
            </div>
            <textarea readonly class="printSpace outputSpace"
                v-show="currentDisplayMode == 'printMain'">{{ printedText }}</textarea>
        </Teleport>
        <Teleport v-if="pageLoaded" defer :to="isSmallScreen ? '#small-screen-code-gen' : '.codeGenSpace'">
            <TabContainer ref="tabContainer">
                <Tab name="code" label="Target Code">
                    <MonacoEditor ref="codeGenArea" readOnlyMode />
                </Tab>

                <Tab name="reflection" label="Reflection">
                    <ReflectionView />
                </Tab>

                <Tab name="uniforms" label="Uniforms"
                    v-if="currentDisplayMode == 'imageMain' && areAnyUniformsRendered">
                    <UniformPanel :uniformComponents="uniformComponents"/>
                </Tab>

                <Tab name="diagnostics" label="Diagnostics" v-if="diagnosticsText != ''">
                    <textarea readonly class="diagnosticSpace outputSpace">{{ diagnosticsText }}</textarea>
                </Tab>
            </TabContainer>
        </Teleport>
        <Teleport v-if="pageLoaded" defer :to="isSmallScreen ? '#small-screen-editor' : '#big-screen-editor'">
            <MonacoEditor class="codingSpace" ref="codeEditor" @vue:mounted="runIfFullyInitialized()" />
        </Teleport>
    </div>
    <Help v-show="showHelp" ref="helpModal"></Help>
</template>

<style scoped>
#small-screen-container {
    height: 100vh;
    display: flex;
    flex-direction: column;
}

#small-screen-editor {
    flex-grow: 1;
}

#small-screen-diagnostic {
    height: 250px;
}

#renderOutput {
    display: block;
    width: 100%;
    height: 100%;
    position: relative;
    overflow: hidden;
}

#loading-screen.v-leave-active {
    transition: opacity 0.5s ease;
}

#loading-screen.v-leave-to {
    opacity: 0;
}

.printSpace {
    margin-top: 10px;
}

.diagnosticSpace {
    padding-left: 10px;
}

.outputSpace {
    background-color: var(--code-editor-background);
    border: none;
    color: white;
    width: 100%;
    height: 100%;
}

.outputSpace:focus {
    outline: none;
}

.resultSpace.splitpanes .splitpanes__pane {
    transition: none !important;
    overflow: hidden;
}

.resultSpace [style*="display: none"]~.splitpanes__splitter~div {
    height: 100% !important;
}
</style>
