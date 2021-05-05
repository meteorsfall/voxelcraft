#!/bin/bash
set -e

npm run package_compiler
sudo cp ./build/voxelscript_compiler /usr/local/bin/voxelc

EMCC_HASH="$((find ./build/emscripten -type f -print0  | sort -z | xargs -0 sha1sum; \
 find ./build/emscripten \( -type f -o -type d \) -print0 | sort -z | \
   xargs -0 stat -c '%n %a') | sha1sum | awk '{print $1}')"

# The /usr/local/share/voxelc directory only depends on ./build/emscripten,
# So we only update that directory if the hash has changed from any existent installation
if [[ ! -f /usr/local/share/voxelc/hash || "$(cat /usr/local/share/voxelc/hash)" != "$EMCC_HASH" ]]; then
  sudo rm -rf /usr/local/share/voxelc
  sudo cp -r ./build/emscripten /usr/local/share/voxelc
  sudo ln -fs /usr/local/share/voxelc/voxelc-emcc /usr/local/bin/voxelc-emcc
  sudo /usr/local/share/voxelc/init-emcc
  echo "$EMCC_HASH" | sudo tee /usr/local/share/voxelc/hash
fi

