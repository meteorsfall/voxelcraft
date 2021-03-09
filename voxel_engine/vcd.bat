nmake debug || EXIT /B
if exist mods\compile_main.trigger (
    wsl asc mods/main.ts --runtime half --explicitStart -b mods/main.wasm
    del mods\compile_main.trigger
)
cd build/debug
start /WAIT /B vc.exe
