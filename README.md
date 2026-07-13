# RetroRacer

An OutRun-style pseudo-3D racing demo in C++20 for Windows Console. It uses the Win32 Console API directly—no third-party engine or download is required.

## Controls

- `W` / Up Arrow — accelerate
- `S` / Down Arrow — brake
- `A`, `D` / Left and Right Arrows — steer
- `Esc` — exit

## Open in Visual Studio

1. Choose **File → Open → Folder**.
2. Select this `RetroRacer` folder.
3. Wait until CMake configuration completes.
4. Select `retro_racer.exe` in the top toolbar and press `Ctrl + F5`.

The executable is produced at `out/build/x64-Debug/retro_racer.exe` for the standard Visual Studio CMake configuration.

## Technical features

- Perspective road generated every frame from the horizon down
- Procedural curves and road/kerb/grass striping
- Delta-time movement, acceleration, braking, off-road slowdown, and steering
- Direct double-buffered output through `WriteConsoleOutputW`
