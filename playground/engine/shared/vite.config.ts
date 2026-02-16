import { defineConfig } from 'vite';
import dts from 'vite-plugin-dts';

export default defineConfig({
  build: {
    lib: {
      entry: 'src/index.ts',
      name: 'slang-playground-shared',
      fileName: (format) => `index.${format}.js`
    },
    rollupOptions: {},
    sourcemap: true,
  },
  plugins: [dts()],
});
