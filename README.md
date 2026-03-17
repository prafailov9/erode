# erode

For first-time setup, or after deleting "/build" folder, follow these steps:

1. clean build project by attaching dep toolchain and exporting a build/compile_commands.json, used by clangd.
cmake -S . -B build `
-G Ninja `
-DCMAKE_TOOLCHAIN_FILE=C:/dev/vcpkg/scripts/buildsystems/vcpkg.cmake `
-DCMAKE_EXPORT_COMPILE_COMMANDS=ON

2. build/compile_commands.json should exist. Copy to root dir.
Copy-Item build/compile_commands.json .

3. Restart clangd server
Open Command Pallete: Ctrl + Shift + P
Type: "clangd: Restart Language Server"

If anything breaks:
- switching generator (VS → Ninja)
- changing toolchain
- changing dependencies
- things randomly stop working

clean first: Remove-Item -Recurse -Force build
then follow steps.