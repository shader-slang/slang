import { UniformController } from './playgroundInterface';

export const RUNNABLE_ENTRY_POINT_NAMES = ['imageMain', 'printMain'] as const;
export * from './playgroundInterface';

export function isControllerRendered(controller: UniformController) {
    return controller.type == "SLIDER" || controller.type == "COLOR_PICK";
}