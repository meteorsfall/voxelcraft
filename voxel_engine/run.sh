#!/bin/bash
set -e

cargo build
cd target/debug
./voxel_engine
cd ../../
