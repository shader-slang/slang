import { defineConfig } from 'vite';
import dts from 'vite-plugin-dts';

export default defineConfig({
  build: {
    lib: {
      entry: 'src/index.ts',
      name: 'slang-compilation-engine',
      fileName: (format) => `index.${format}.js`
    },
    rollupOptions: {},
    sourcemap: true,
  },
  assetsInclude: ['**/*.slang'],
  plugins: [dts()],
});
