<script setup lang="ts">
const value = defineModel<number>("value");
const { name, min, max } = defineProps<{
    name: string,
    min: number,
    max: number,
}>();
</script>

<template>
    <div class="controllerHolder">
        <p class="controllerLabel">{{ name }}</p>
        <div class="controllerContainer">
            <div class="labels">
                <span class="value">{{ min }}</span>
                <input type="number" :min="min" :max="max" v-model.number="value" :step="(max - min) / 200.0"
                    class="numberInput"></input>
                <span class="value">{{ max }}</span>
            </div>
            <input type="range" :min="min" :max="max" v-model.number="value" :step="(max - min) / 200.0"
                class="sliderInput">
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

.labels {
    display: flex;
    justify-content: space-between;
}

.value {
    font-size: 14px;
}

.numberInput {
    background-color: transparent;
    color: var(--vscode-editor-foreground, white);
    border: none;
    outline: none;
    width: 60px;
    text-align: center;
}

.controllerContainer {
    min-width: 0;
    flex-grow: 1;
    display: flex;
    flex-direction: column;
    margin-right: 20px;

}

.sliderInput {
    min-width: 0;
    flex-grow: 1;
    z-index: 1;
}

.sliderInput {
    height: 6px;
    appearance: none;
    background: #ddd;
    outline: none;
    border-radius: 8px;
}

.sliderInput::-webkit-slider-thumb {
    -webkit-appearance: none;
    width: 15px;
    height: 15px;
    background: #4299e1;
    border-radius: 50%;
    cursor: pointer;
}

.sliderInput::-moz-range-thumb {
    width: 15px;
    height: 15px;
    background: #4299e1;
    border-radius: 50%;
    cursor: pointer;
    border: none;
}
</style>