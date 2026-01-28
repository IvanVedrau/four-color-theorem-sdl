## 📸 Screenshots

![Gameplay screenshot](Assets/images/Screenshot 2026-01-28 170038.png)


# 🗺️ Four Color Theorem — SDL C Project

An interactive visualization and game based on the **Four Color Theorem**, implemented in **C** using **SDL2**.

The project demonstrates how any planar map can be colored using **no more than four colors**, such that no two adjacent regions share the same color.

---

## 🎮 Gameplay Overview

- The map is generated using a **Voronoi diagram**
- Each region must be colored manually by the player
- Only **4 colors** are available
- Adjacent regions **cannot share the same color**
- The game tracks completion time and stores results in a **Hall of Fame**

---

## 🧠 Theoretical Background

The **Four Color Theorem** states:

> Any planar map can be colored with at most four colors so that no two neighboring regions share the same color.

In this project:
- Regions are represented as points
- A Voronoi diagram defines borders
- Adjacency is detected by sampling neighboring cells
- Color conflicts are checked dynamically

---

## 🛠️ Technical Details

- **Language:** C (C99)
- **Graphics:** SDL2
- **Algorithm:** Voronoi-based region partitioning
- **Adjacency Detection:** Grid sampling
- **Max Regions:** 100
- **Colors:** 4 (Red, Green, Blue, Yellow)

---

## 🎚️ Difficulty Levels

| Difficulty | Regions |
|-----------|---------|
| Easy      | 5       |
| Medium    | 70      |
| Hard      | 100     |

---

## 🕹️ Controls

### Menu
- `1` — Easy
- `2` — Medium
- `3` — Hard

### Game
- `1–4` — Select color
- `Left Mouse Button` — Paint region
- `R` — Restart current difficulty
- `ESC` — Exit

---

## 🏆 Hall of Fame

After winning:
- Enter your name
- Completion time is saved to `hall_of_fame.txt`
- Previous records are displayed on exit

---

## ▶️ How to Build & Run

### Linux / macOS
```bash
gcc main.c -o four_color -lSDL2
./four_color



## 🙏 Acknowledgements

The project uses an existing CLion + SDL2 boilerplate for build system and IDE configuration.

All application logic, algorithms, rendering, and gameplay code were implemented entirely by the author.

