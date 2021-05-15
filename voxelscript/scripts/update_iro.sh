#!/bin/bash

# Setup error handling
set -e
trap 'echo "Error on line $LINENO!"' ERR

# Filepaths for iro/tm
IRO_FILE="./voxelscript_extension/voxelscript_highlighter/voxelscript.iro"
TM_FILE="./voxelscript_extension/voxelscript_highlighter/syntaxes/voxelscript.tmLanguage"
IRO_HASH_FILE="./build/compile_iro/iro_hash.txt"
TM_HASH_FILE="./build/compile_iro/tm_hash.txt"

source ./build/compile_iro/env/bin/activate # For python venv
export PATH="$PATH:$(pwd)/build/compile_iro" # For geckodriver

# If the .tmLanguage doesn't exist, or the tmLanguage/iro file has changed, then we should regenerate the tmLanguage file
SHOULD_MAKE_TM=false
if ! [[ -f "$TM_FILE" ]]; then
  echo ".tmLanguage file not found: Generating from .iro file..."
  SHOULD_MAKE_TM=true
else
  if [[ ! -f "$TM_HASH_FILE" || ! -f "$IRO_HASH_FILE" ]]; then
    echo "New ./build directory. Regenerating .tmLanguage file..."
    SHOULD_MAKE_TM=true
  else
    IRO_HASH="$(sha256sum "$IRO_FILE")"
    TM_HASH="$(sha256sum "$TM_FILE")"
    if [[ "$IRO_HASH" != "$(cat "$IRO_HASH_FILE")" ]]; then
      echo "Iro file has changed. Regenerating .tmLanguage file..."
      SHOULD_MAKE_TM=true
    elif [[ "$TM_HASH" != "$(cat "$TM_HASH_FILE")" ]]; then
      echo ".tmLanguage file has been tampered with. Regenerating .tmLanguage file..."
      SHOULD_MAKE_TM=true
    fi
  fi
fi

# Regenerate tmLanguage file
if [[ "$SHOULD_MAKE_TM" == "true" ]]; then
  python3 ./scripts/iro_to_textmate.py "file://$(pwd)/build/compile_iro/eeyo.io/iro/index.html" ./scripts/iro_to_textmate.py < "$IRO_FILE" > "$TM_FILE"

  if ! cat "$TM_FILE" | grep voxelscript &>/dev/null; then
    echo ".tmLanguage file doesn't have the phrase \"voxelscript\" in it. Something probably went wrong."
    rm -f "$IRO_HASH_FILE" "$TM_HASH_FILE" "$TM_FILE"
    exit 1
  fi

  sha256sum "$IRO_FILE" > "$IRO_HASH_FILE"
  sha256sum "$TM_FILE" > "$TM_HASH_FILE"
else
  echo ".tmLanguage is up-to-date!"
fi
