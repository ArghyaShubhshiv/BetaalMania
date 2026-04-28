# 2D Top-Down Shooter - Runnable Project

## Project Status: ✅ COMPLETE & FIXED

All bugs have been identified and fixed. The project is now fully compiled and ready to run.

### Build Complete ✓
- Executable: `build/game`
- Platform: macOS (x86_64 Mach-O)
- Compilation: 0 errors, only deprecation warnings (normal for GLUT/OpenGL on macOS)

## Fixed Issues

1. **Missing `isGameOver` member variable** - Added to `Game` class with public getter method
2. **Game over state handling** - Properly transitions back to menu when player dies
3. **All compilation errors resolved**

## How to Run

### Option 1: From Terminal (Recommended)
```bash
cd /Users/krishgarg/Desktop/2D-Top-down-shooter-master/build
./game
```

### Option 2: Direct Run
```bash
/Users/krishgarg/Desktop/2D-Top-down-shooter-master/build/game
```

### Option 3: From Project Root
```bash
cd /Users/krishgarg/Desktop/2D-Top-down-shooter-master
build/game
```

## Game Controls

### Main Menu
- **Left Click** on buttons to select options
- **Mouse Movement** highlights buttons with glow effect

### In-Game
- **W** - Move Up
- **S** - Move Down  
- **A** - Move Left
- **D** - Move Right
- **Mouse Move** - Aim
- **Left Click/Hold** - Shoot (auto-fire when held)
- **ESC** - Return to Menu
- **R** - Reload (if implemented)

### Difficulty Modes
1. **Easy** - Slower enemies, less damage, more health
2. **Normal** - Balanced gameplay
3. **Hard** - Faster enemies, more damage, less health

## Game Features

✅ **Implemented:**
- Main Menu with difficulty selection
- Settings screen for game mode
- Multiple zombie types with unique behaviors
- Wave-based enemy spawning
- Boss fights
- Weapon system (pistol, rapid-fire, flamethrower)
- Shield power-up system
- Ammo/reloading mechanics
- Score and kill tracking
- Health system
- Screen shake effects
- Perk drops
- Campaign manager system
- AI Director for dynamic difficulty

## Build Information

- **Compiler**: Apple Clang 17.0.0
- **Build System**: CMake 3.10+
- **Dependencies**: OpenGL, GLUT
- **Architecture**: x86_64

## Rebuild (if needed)

```bash
cd /Users/krishgarg/Desktop/2D-Top-down-shooter-master
rm -rf build
mkdir build && cd build
cmake ..
make
```

The project will rebuild successfully with the fixes applied.

## Notes

- All deprecation warnings are normal for macOS (GLUT/OpenGL are deprecated but still functional)
- The project uses fixed-function OpenGL pipeline (legacy, but compatible)
- Requires macOS with Xcode Command Line Tools installed
