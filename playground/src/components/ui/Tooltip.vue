<script setup lang="ts">
import { ref, useTemplateRef } from 'vue';

const tooltipText = ref("");
const show = ref(false);
const tooltipElement = useTemplateRef("tooltip");
let timeout = 0;

function showTooltip(button: HTMLElement, text: string) {
    tooltipText.value = text;
    // Position the tooltip near the button
    const rect = button.getBoundingClientRect();
    tooltipElement.value!.style.top = `${rect.bottom + window.scrollY + 15}px`;
    tooltipElement.value!.style.left = `${rect.left + window.scrollX}px`;

    // Show the tooltip
    show.value = true;

    clearTimeout(timeout);

    // Hide the tooltip after 3 seconds
    timeout = setTimeout(() => {
        show.value = false;
    }, 3000);
}

defineExpose({ showTooltip });
</script>

<template>
    <Transition>
        <div class="tooltip" v-show="show" ref="tooltip">{{ tooltipText }}</div>
    </Transition>
</template>

<style scoped>
.tooltip {
    position: absolute;
    background-color: #333;
    color: #fff;
    padding: 8px;
    border-radius: 4px;
    border: solid #808080;
    box-shadow: #111 3px 3px 10px;
    border-width: 1px;
    transition: opacity 0.3s;
    pointer-events: none;
    font-size: 14px;
    z-index: 999;
    opacity: 1;
}

.tooltip.v-leave-active {
    transition: opacity 0.3s ease;
}

.tooltip.v-leave-to {
    opacity: 0;
}
</style>