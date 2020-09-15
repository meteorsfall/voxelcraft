nmake debug || EXIT /B
if exist mods\compile_main.trigger (
    wsl asc mods/main.ts --runtime half --explicitStart -b mods/main.wasm
    del mods\compile_main.trigger
)
start /WAIT /B build/debug/vc.exe
