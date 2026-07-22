<script setup lang="ts">
import { computed } from 'vue';

const value = defineModel<[number, number, number]>("value");

const { name } = defineProps<{
    name: string,
}>();

const hexValue = computed(() => {
    const r = value.value![0]
    const g = value.value![1]
    const b = value.value![2]
    const toHex = (n: number) => (Math.round(255*n)).toString(16).padStart(2, '0')
    return '#' + toHex(r) + toHex(g) + toHex(b)
})

// When the user picks a new color, convert the hex string back to [r, g, b]
function updateColor(event: Event) {
    const hex = (event.target as HTMLInputElement).value
    const r = parseInt(hex.slice(1, 3), 16)/255
    const g = parseInt(hex.slice(3, 5), 16)/255
    const b = parseInt(hex.slice(5, 7), 16)/255
    value.value = [r, g, b]
}
</script>

<template>
    <div class="controllerHolder">
        <p class="controllerLabel">{{ name }}</p>
        <div class="controllerContainer">
            <input type="color" :value="hexValue" @input="updateColor">
        </div>
    </div>
</template>

<style scoped>
.controllerHolder {
    display: flex;
    flex-direction: row;
    align-items: center;
    min-width: 0;
    color: var(--vscode-editor-foreground, white);

}

.controllerLabel {
    margin-left: 20px;
    margin-right: 20px;
    flex-shrink: 1;
    min-width: 50px;
}

.controllerContainer {
    min-width: 0;
    flex-grow: 1;
    display: flex;
    flex-direction: column;
    margin-right: 20px;

}
</style>