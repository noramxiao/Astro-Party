# Astro Party Game Design Document

## Section 0: Summary

**Working Title:** Astro Party

**Team Members:**
- Justin (Programmer, Physics Integration)
- Marco (Programmer, Graphics and Animation)
- Nora (Programmer, Gameplay and UI Design)

**Concept Statement:** A multiplayer space battle game where players control spaceships with realistic physics, aiming to outmaneuver and outgun their opponents in a variety of space arenas.

## Section 1: Gameplay

**Game Progression:**
- Players start by selecting their spaceships and arena.
- The game begins with players positioned at different points in the arena.
- Players control their ships, trying to eliminate opponents by shooting or ramming them.
- The game continues until only one player remains or the time limit is reached.

**Win and Loss Conditions:**
- **Win:** Be the last player remaining or have the highest score when the time limit is reached.
- **Loss:** Lose all your lives or have the lowest score when the time limit is reached.

**Levels:**
- The game features multiple arenas (levels) with different layouts and hazards.
- Each arena has unique characteristics affecting gameplay.

**Points:**
- Points are awarded for damaging opponents, eliminating opponents, and surviving longer.
- Bonus points for stylish maneuvers and avoiding hazards.

**Controls:**
- **Keyboard:**
  - Arrow keys for movement (up, down, left, right)
  - Spacebar to shoot
  - Shift key for special maneuvers (e.g., boost)
- **Mouse:** Aim and shoot (optional feature)

**Physics:**
- The game uses a custom physics engine to simulate realistic spaceship movement and collisions.
- Physics includes inertia, friction, and collision detection.

**Game Flow:**
1. Main Menu: Choose game mode, arena, and spaceship.
2. Gameplay: Players battle in the selected arena until a win/loss condition is met.
3. End Screen: Display scores and winner, option to restart or return to main menu.

**Graphics:**
- Combination of sprites and vector graphics for spaceships and arenas.
- Basic animations for movement, shooting, and explosions.

## Section 2: Feature Set

### Priority 1 Features (Game cannot work without this)
- **Basic Spaceship Movement:** Implement keyboard controls for spaceship movement (Justin)
- **Collision Detection:** Basic collision detection between spaceships and arena boundaries (Nora)
- **Basic Shooting Mechanic:** Implement shooting and damage system (Marco)
- **Win/Loss Conditions:** Implement game logic to determine win/loss conditions (Justin)

### Priority 2 Features (Minimum grade of C)
- **Arena Selection:** Multiple arenas with different layouts (Nora)
- **Basic HUD:** Display player lives and scores (Marco)
- **Sound Effects:** Basic sound effects for shooting and explosions (Justin)
- **Physics Integration:** Basic implementation of inertia and friction (Nora)

### Priority 3 Features (Minimum grade of B)
- **Special Maneuvers:** Implement boost and evasive maneuvers (Marco)
- **Advanced Collision Handling:** More accurate and detailed collision responses (Justin)
- **Arena Hazards:** Add environmental hazards to arenas (e.g., asteroids, black holes) (Nora)
- **Improved HUD:** Enhanced HUD with additional info (e.g., power-ups, time left) (Marco)

### Priority 4 Features (Minimum grade of A-)
- **Custom Graphics:** Create custom sprites and vector graphics for spaceships and arenas (Marco)
- **Music:** Background music for gameplay (Justin)
- **Multiplayer:** Networked multiplayer mode (Nora)
- **AI Opponents:** Implement AI for solo play or to fill multiplayer slots (Justin)

## Section 3: Timeline

### Week 1
- **Justin:** Basic spaceship movement (P1), Basic shooting mechanic (P1)
- **Marco:** Collision detection (P1), Sound effects (P2)
- **Nora:** Arena selection (P2), Basic HUD (P2)

### Week 2
- **Justin:** Win/Loss conditions (P1), Advanced collision handling (P3)
- **Marco:** Special maneuvers (P3), Improved HUD (P3)
- **Nora:** Physics integration (P2), Arena hazards (P3)

### Week 3
- **Justin:** AI opponents (P4), Music (P4)
- **Marco:** Custom graphics (P4), Multiplayer (P4)
- **Nora:** Finalize and polish all features, ensuring seamless integration (all features)

## Section 4: Disaster Recovery

### Justin
- **Issue:** Falling behind on spaceship movement or shooting mechanic
- **Plan:** Reallocate time from AI opponents (P4) and Music (P4) to complete high-priority tasks.

### Marco
- **Issue:** Difficulty with special maneuvers or HUD
- **Plan:** Simplify special maneuvers and HUD features to ensure completion of basic functionality. Delay custom graphics if necessary.

### Nora
- **Issue:** Problems with physics integration or arena hazards
- **Plan:** Focus on completing essential physics integration first. Simplify or reduce arena hazards if falling behind.

By following this structured plan, we aim to ensure the successful development of Astro Party within the given timeline while maintaining flexibility to handle potential setbacks.



