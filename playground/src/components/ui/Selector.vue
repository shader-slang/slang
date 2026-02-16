<script setup lang="ts" generic="T extends readonly string[]">
import { ref } from 'vue'

const props = defineProps<{
    options: T
    modelValue: T[number]
}>()

const emit = defineEmits<{
    (e: 'update:modelValue', value: T[number]): void
    (e: 'change', value: T[number]): void
}>()

const selectedValue = ref<T[number]>(props.modelValue)

const handleChange = () => {
    emit('update:modelValue', selectedValue.value)
    emit('change', selectedValue.value)
}

function getValue(): T[number] {
    return selectedValue.value as T[number]
}

function setValue(value: T[number]) {
    selectedValue.value = value
}

defineExpose({
    getValue,
    setValue,
})
</script>

<template>
    <select v-model="selectedValue" @change="handleChange" class="dropdown-select">
        <option v-for="option in options" :key="option" :value="option">
            {{ option }}
        </option>
    </select>
</template>