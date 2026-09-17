<script setup lang="ts">
import { ref, provide } from 'vue'
import { tabInjectKey } from './TabInterface'

type TabData = {
  name: string
  label: string
}

// Active tab state
const activeTab = ref('')

// Registered tabs store
const tabs = ref<TabData[]>([])

// Register a new tab
function registerTab(name: string, label: string) {
  tabs.value.push({ name, label })
  // Set first tab as active by default
  if (tabs.value.length === 1) {
    activeTab.value = name
  }
}

// Unregister a tab
function unregisterTab(name: string) {
  const index = tabs.value.findIndex(tab => tab.name === name)
  if (index !== -1) {
    tabs.value.splice(index, 1)
    // If active tab is removed, activate first remaining tab
    if (activeTab.value === name && tabs.value.length > 0) {
      activeTab.value = tabs.value[0].name
    }
  }
}

// Method to change active tab
function setActiveTab(tabName: string) {
  activeTab.value = tabName
}

// Provide tab context to child components
provide(tabInjectKey, {
  activeTab,
  registerTab,
  unregisterTab
})

defineExpose({
  setActiveTab
})
</script>

<template>
  <div class="tabs-container">
    <!-- Tab headers -->
    <div class="tab-headers">
      <button v-for="tab in tabs" :key="tab.name" class="tab-button" :class="{ active: activeTab === tab.name }"
        @click="setActiveTab(tab.name)">
        {{ tab.label }}
      </button>
    </div>

    <!-- Tab content area -->
    <div class="tab-content">
      <slot />
    </div>
  </div>
</template>

<style scoped>
.tabs-container {
  width: 100%;
  height: 100%;
  display: flex;
  flex-direction: column;
}

.tab-headers {
  display: flex;
}

.tab-button {
  padding: 0.5rem 1rem;
  border-radius: 0px;
  border-top-left-radius: 5px;
  border-top-right-radius: 5px;
  color: var(--inactive-tab-title);
  background: none;
  border: none;
  cursor: pointer;
  transition: all 0.2s ease;
}

.tab-button:hover {
  background-color: var(--orange);
}

.tab-button.active {
  background: var(--code-editor-background);
  color: white;
}

.tab-content {
  min-height: 0;
  flex-grow: 1;
}
</style>