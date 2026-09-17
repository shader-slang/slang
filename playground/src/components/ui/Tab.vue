<template>
    <div v-show="isActive" style="height: 100%;">
        <slot />
    </div>
</template>

<script setup lang="ts">
import { inject, computed, onMounted, onBeforeUnmount } from 'vue'
import { tabInjectKey } from './TabInterface'

const props = defineProps({
    name: {
        type: String,
        required: true
    },
    label: {
        type: String,
        default: null
    }
})

// Inject tab context from parent
const { activeTab, registerTab, unregisterTab } = inject(tabInjectKey)!

// Compute if this tab is active
const isActive = computed(() => activeTab.value === props.name)

// Register tab on mount
onMounted(() => {
    registerTab(props.name, props.label || props.name)
})

// Unregister tab before unmount
onBeforeUnmount(() => {
    unregisterTab(props.name)
})
</script>