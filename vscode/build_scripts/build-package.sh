npm install
npm run compile
npm install -g @vscode/vsce

target_build() {
  TEMP_DIR="$(mktemp -d)"
  ZIP="$1"
  TARGET="$2"
  TEMP_LIBRARY="$3"
  TEMP_EXECUTABLE="$4"
  TEMP_LIBRARY_2="$5"

  echo "extracting $ZIP"
  unzip -n "$ZIP" -d "$TEMP_DIR"

  echo "installing binaries for $TARGET"
  mkdir -p "./server/bin/$TARGET"
  cp "$TEMP_DIR/$TEMP_LIBRARY" ./server/bin/"$TARGET"/
  cp "$TEMP_DIR/$TEMP_LIBRARY_2" ./server/bin/"$TARGET"/
  cp "$TEMP_DIR/$TEMP_EXECUTABLE" ./server/bin/"$TARGET"/
  chmod +x ./server/bin/"$TARGET"/*

  echo "building for $TARGET"
  vsce package --target "$TARGET"

  echo "cleanup for $TARGET"
  rm -rf $TEMP_DIR
  rm -rf ./server/bin/
}

echo "updating .vscodeignore for native builds"
cp .vscodeignore .vscodeignore.bak
# Exclude web files for native builds
echo '' >> .vscodeignore
echo 'media/*.worker.js' >> .vscodeignore
echo 'server/dist/browserServerMain.js' >> .vscodeignore

target_build "$WIN32_X64_ZIP" win32-x64 bin/slang.dll bin/slang-glsl-module.dll bin/slangd.exe
target_build "$WIN32_ARM64_ZIP" win32-arm64 bin/slang.dll bin/slang-glsl-module.dll bin/slangd.exe
target_build "$LINUX_X64_ZIP" linux-x64 lib/libslang.so lib/libslang-glsl-module.so bin/slangd
target_build "$LINUX_ARM64_ZIP" linux-arm64 lib/libslang.so lib/libslang-glsl-module.so bin/slangd
target_build "$DARWIN_X64_ZIP" darwin-x64 lib/libslang.dylib lib/libslang-glsl-module.dylib bin/slangd
target_build "$DARWIN_ARM64_ZIP" darwin-arm64 lib/libslang.dylib lib/libslang-glsl-module.dylib bin/slangd

echo "restoring .vscodeignore after native build ($TARGET)"
mv .vscodeignore.bak .vscodeignore

# For web build, exclude native/server files
echo "updating .vscodeignore for web build"
cp .vscodeignore .vscodeignore.bak
# Exclude native/server files for web build
echo '' >> .vscodeignore
echo 'media/*.node.js' >> .vscodeignore
echo 'server/dist/nativeServerMain.js' >> .vscodeignore

vsce package --target "web"

echo "restoring .vscodeignore after web build"
mv .vscodeignore.bak .vscodeignore