#!/bin/bash

# Setup error handling
set -e
trap 'echo "Error on line $LINENO!"' ERR

# Navigate to ./build/compile_iro
cd build
if [[ -d compile_iro ]]; then
  rm -r compile_iro
fi
mkdir -p compile_iro
cd compile_iro

# Create a python venv with selenium
virtualenv -p python3 env
source env/bin/activate
pip install selenium

# Download geckodriver for selenium to use
wget "https://github.com/mozilla/geckodriver/releases/download/v0.29.1/geckodriver-v0.29.1-linux64.tar.gz"
tar -xvf geckodriver-v0.29.1-linux64.tar.gz
rm geckodriver-v0.29.1-linux64.tar.gz

# Download the iro website for local use
wget -r "https://eeyo.io/iro/" -R "jszip.min.js" # jszip.min.js doesn't exist and will cause a wget error code, so we must ignore it
cd eeyo.io/iro/Iro
wget "https://eeyo.io/iro/Iro/748A06F61FE5D27DCCF83009666C237C.cache.js" # Needed, but not directly referenced in the HTML
cd ../../..
