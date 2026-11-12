<script setup lang="ts">
import { ref, watch } from 'vue';
import { UniformController } from 'slang-playground-shared';
import { UniformPanel } from 'playground-uniform-panel';
import 'playground-uniform-panel/dist/playground-uniform-panel.css';

declare function acquireVsCodeApi(): {
    postMessage: (msg: any) => void;
    setState: (state: any) => void;
    getState: () => any;
};
const vscode = acquireVsCodeApi();

const initialized = ref(false);
const uniformComponents = ref<UniformController[]>([])

watch(uniformComponents, (newValues, _) => {
    if (!initialized.value) return;
	vscode.postMessage({
		type: 'update',
		data: JSON.parse(JSON.stringify(newValues))
	});
}, {deep: true})

window.addEventListener('message', event => {
    if(event.data.type == "init") {
		uniformComponents.value = event.data.uniformComponents;
        initialized.value = true;
	}
});
</script>
<template>
	<UniformPanel :uniformComponents="uniformComponents"/>
</template>