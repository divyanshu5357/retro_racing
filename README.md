# RetroRacer

RetroRacer is a C++20 pseudo-3D arcade racing game for Windows, inspired by classic OutRun-style racers. It generates a curved road in real time inside the Windows console—no third-party game engine or graphics library is required.

## Preview

The game renders a moving perspective road with grass, striped kerbs, a rival car, speed control, and off-road slowdown.

## Features

- Pseudo-3D perspective road renderer
- Procedural curves, grass, road, and kerb striping
- Real-time acceleration, braking, and steering
- Off-road speed penalty
- Delta-time game loop for consistent movement
- Double-buffered Win32 Console rendering with `WriteConsoleOutputW`
- No external dependencies

## Controls

| Key | Action |
| --- | --- |
| `W` or Up Arrow | Accelerate |
| `S` or Down Arrow | Brake |
| `A` / Left Arrow | Steer left |
| `D` / Right Arrow | Steer right |
| `Esc` | Quit the game |

## Requirements

- Windows 10 or Windows 11
- Visual Studio Community with the **Desktop development with C++** workload
- CMake (included with Visual Studio's C++ tools)

## Build and run in Visual Studio

1. Open Visual Studio.
2. Select **File → Open → Folder**.
3. Choose the `RetroRacer` project folder.
4. Wait for CMake configuration to finish.
5. Select `retro_racer.exe` from the top toolbar.
6. Press `Ctrl + F5` to build and run the game.

The generated executable is normally located at:

```text
out/build/x64-Debug/retro_racer.exe
```

## Project structure

```text
RetroRacer/
├── src/
│   └── main.cpp          # Game loop, input, road renderer, and console output
├── CMakeLists.txt        # CMake build configuration
├── .gitignore            # Generated-file exclusions for Git
├── LICENSE               # Project license (if added)
└── README.md
```

## How it works

For each frame, RetroRacer calculates the road width and centre position for every screen row below the horizon. The apparent road width increases closer to the player, creating depth. Sine-based curves shift the road centre, while player input changes speed and lateral position.

## Future improvements

- Lap timer and high-score system
- Multiple tracks and track segments
- Sound effects and music
- Opponent cars with collision detection
- Textured sprites and weather effects
