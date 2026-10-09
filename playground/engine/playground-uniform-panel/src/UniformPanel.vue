<script setup lang="ts">
import { ref, watch } from 'vue';
import { UniformController } from 'slang-playground-shared';
import Slider from './ui/Slider.vue'
import Colorpick from './ui/Colorpick.vue'

const { uniformComponents } = defineProps<{
	uniformComponents: UniformController[]
}>();

</script>
<template>
	<div class="uniformPanel">
		<div v-for="uniformComponent in uniformComponents">
			<Slider v-if="uniformComponent.type == 'SLIDER'" :name="uniformComponent.name"
				v-model:value="uniformComponent.value" :min="uniformComponent.min" :max="uniformComponent.max" />
			<Colorpick v-if="uniformComponent.type == 'COLOR_PICK'" :name="uniformComponent.name"
				v-model:value="uniformComponent.value" />
		</div>
	</div>
</template>

<style scoped>
.uniformPanel {
	background-color: var(--vscode-editor-background, var(--code-editor-background));
	height: 100%;
	overflow-y: scroll;
}
</style>