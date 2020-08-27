# VoxelCraft

This repository aims to implement a fully features voxel engine, designed to be used with a simple scripting language. Modders should have the full freedom to easily add content to the game, and users should be able to seamlessly enjoy servers with arbitrary modpacks!

## Setup

On Ubuntu, run

```./setup
cmake .
make
./vc
```

On Windows, run

```setup
cnmake
nmake
./vc
```

And then VoxelCraft should begin running!

- Windows will assume that you have Visual Studio Community installed, along with OpenGL.

## Use

Simply explore the world!

Implemented features are the following:
- Escape to free and lock the mouse
- Left click to damage blocks
- Right cick to place blocks
- Scripting language that transpiles to js
