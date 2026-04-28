# 🎮 BetaalMania

![BetaalMania screenshot](https://imgs.search.brave.com/mes4XYswQEF6TxoavHYDSj6_SqIZQmuoxiLC7Kemq6s/rs:fit:860:0:0:0/g:ce/aHR0cHM6Ly9nYW1h/dmVyc2UuY29tL2Mv/aS9nL3BpeGVsLXpv/bWJpZS1zaG9vdGVy/LTIwMTcuanBnPzE0/OTIwNjU3NDg)

## Overview
BetaalMania is a fast‑paced 2D top‑down shooter where you pilot a lone hero against endless waves of enemies in a neon‑lit arena. Built as a university lab assignment, the game showcases modern C++ graphics programming using **OpenGL**, **GLUT**, and **SDL2**, wrapped in a clean CMake build system.

---

## ✨ Features
- **Dynamic Wave System** – Enemies spawn in increasingly challenging patterns.
- **Smooth Player Controls** – WASD movement & mouse aiming with responsive shooting.
- **Multiple Weapon Types** – Primary gun, spread shotgun, and charged laser.
- **Boss Battles** – Epic multi‑phase bosses with unique attack patterns.
- **Pixel‑Art UI** – Health bar, score counter, ammo display with neon accents.
- **Audio Feedback** – Sound effects for shooting, explosions, and power‑ups.
- **Cross‑Platform Build** – CMake configuration works on Windows, macOS, and Linux.

---

## 🎮 Controls
| Action | Key |
|--------|-----|
| Move Up | **W** |
| Move Down | **S** |
| Move Left | **A** |
| Move Right | **D** |
| Aim / Shoot | **Mouse Left Click** |
| Switch Weapon | **Mouse Wheel** |
| Pause | **Esc** |

---

## 🛠️ Technologies Used
| Component | Library / Tool |
|-----------|----------------|
| Language | C++17 |
| Graphics | OpenGL, GLUT |
| Window & Input | SDL2 |
| Build System | CMake |
| Asset Loading | stb_image (for PNG/BMP textures) |
| Version Control | Git |

---

## 📦 Build & Run
```bash
# Clone the repository (if you haven't already)
git clone https://github.com/yourusername/BetaalMania.git
cd BetaalMania

# Create a build directory
mkdir build && cd build

# Generate makefiles with CMake
cmake ..

# Compile the game
cmake --build .

# Run the executable
./game   # on Linux/macOS
game.exe  # on Windows
```
> **Note:** Ensure that the required libraries (OpenGL, GLUT, SDL2) are installed on your system and reachable by the compiler.

---

## 🤝 Contributing
Feel free to fork the project and submit pull requests. Guidelines:
- Follow the existing coding style.
- Keep new features well‑documented.
- Write unit tests for any core logic.

---

## 📜 License
This project is licensed under the **MIT License** – see the `LICENSE` file for details.

---

*Happy shooting!*
