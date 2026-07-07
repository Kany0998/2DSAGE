\# 2DSAGE



2DSAGE is a 2D game engine written in C++ using SDL2.



The project is being developed as part of an engineering thesis. The engine is being extended with RPG mechanics, artificial intelligence, physics, audio, scripting, and an ECS architecture based on EnTT.



\## Main technologies



\- C++20

\- SDL2

\- SDL2\_image

\- SDL2\_ttf

\- SDL2\_mixer

\- GLM

\- Lua

\- sol2

\- EnTT

\- Dear ImGui

\- CMake

\- Conan 2



\## Current status



The project currently supports:



\- SDL2 rendering and input

\- textures and fonts

\- Lua scripting

\- Dear ImGui debug interface

\- CMake build configuration

\- dependency management with Conan 2

\- EnTT dependency prepared for ECS migration



The existing custom ECS will be gradually replaced with EnTT.



\## Requirements



\### Windows



Install:



\- Visual Studio 2026 Community with Desktop development with C++

\- CMake

\- Python 3

\- Conan 2

\- Git



The project is currently tested on Windows x64.



\## Installing Conan



```powershell

python -m pip install --upgrade pip

python -m pip install conan

conan profile detect --force

```



\## Clone the repository



```powershell

git clone https://github.com/Kany0998/2DSAGE.git

cd 2DSAGE

```



Replace `REPOSITORY\_URL` with the actual repository address.



\## Install dependencies



From the project root directory run:



```powershell

conan install . --output-folder=build --build=missing -s build\_type=Debug -s compiler.cppstd=20 -c tools.cmake.cmaketoolchain:generator=Ninja

```



Conan downloads and prepares the required dependencies.



\## Configure the project



```powershell

cmake --preset windows-debug

```



\## Build the project



```powershell

cmake --build --preset windows-debug

```



The executable is generated in:



```text

build/visual-studio-debug/Debug/2DSAGE.exe

```



\## Opening in Visual Studio



Do not open the old `2DSAGE.sln` file.



Open the project as a CMake folder:



```text

File → Open → Folder

```



Select the project directory and choose:



```text

Windows Debug - Conan

```



as the active CMake preset.



Set `2DSAGE.exe` as the startup target.



Run with:



```text

F5

```



or without the debugger:



```text

Ctrl + F5

```



\## Project structure



```text

2DSAGE/

├── assets/

├── src/

├── libs/

├── CMakeLists.txt

├── CMakePresets.json

├── conanfile.py

└── README.md

```



The `libs` directory is temporary. Legacy copies of Lua, sol2, and GLM will be removed after the migration is completed.



\## Dependencies



Dependencies are declared in `conanfile.py`.



Current dependencies include:



\- SDL2

\- SDL2\_image

\- SDL2\_ttf

\- SDL2\_mixer

\- GLM

\- Lua

\- sol2

\- EnTT



Dependencies should not be installed manually or configured using hardcoded local paths.



\## Assets



The application expects the `assets` directory to be available next to the executable.



CMake automatically copies the assets after building.



Do not use absolute paths such as:



```text

C:\\Users\\Username\\Desktop\\2DSAGE\\assets

```



All asset paths should be relative to the application directory.



\## Branches



The CMake and Conan migration is currently developed on:



```text

cmake-conan-migration

```



\## Planned development



\- migration from the custom ECS to EnTT

\- RPG character statistics

\- level progression

\- inventory and equipment

\- loot tables

\- A\* pathfinding

\- environmental physics

\- audio system

\- playable RPG demonstration



\## Linux



Linux support is planned but is currently optional and not yet fully tested.



A separate Linux build will be required. Windows executables cannot be run natively on Linux.



\## License



License information will be added later.

