import type { InjectionKey, Ref } from 'vue'

type TabInterface = {
    activeTab: Ref<string, string>,
    registerTab: (name: string, label: string) => void,
    unregisterTab: (name: string) => void,
};
export const tabInjectKey = Symbol() as InjectionKey<TabInterface>