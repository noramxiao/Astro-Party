# Space Wars Game Design Document

## Section 0: Summary

**Working Title:** Space Wars

**Team Members:**
- Justin (Programmer, Physics Integration)
- Marco (Programmer, Graphics and Animation)
- Nora (Programmer, Gameplay and UI Design)

**Concept Statement:** A multiplayer space battle game where players control spaceships with realistic physics, aiming to outmaneuver and outgun their opponents in a variety of space arenas.

## Section 1: Gameplay

**Game Progression:**
- The game begins with players positioned at opposite corners of the map.
- Players control their ships, trying to eliminate opponents by shooting them.
- The player who shoots the opponent twice (once in ship mode, once in pilot mode) wins the round.
- First player to 5, 10, or 15 rounds (player choice) wins.

**Controls:**
- **Keyboard:**
  - Players use two keys each (one for shooting, one for turning)
  - Default keybinds: player 1 uses "W" and "Q" to turn and shoot, and player 2 uses "<" and ">" to turn and shoot.
- **Mouse:** 
  - Players can use the mouse to interact with the UI to select the map/start the game.

**Physics:**
- The physics engine will calculte the movement of the ships as players turn and shoot, including:
- Position of the ship with a fixed velocity and input-based rotation
- Recoil when the player shoots
- Acceleration when the player double taps the turn button
- Collisions with the walls and other spaceships
- Gravity between the spaceships and planets placed around the map.

**Game Flow:**
1. Main Menu: Choose the map and number of rounds needed to win and start the game.
2. Gameplay: Players battle in the selected arena until once player dies.
3. End Screen: Display scores and winner, option to restart with same settings or return to main menu.

**Graphics:**
- Sprites for spaceships, bullets, pilots, and obstacles such as planets in the arena.
- No vector graphics needed for now
- Animation for spaceship movement and shooting.

## Section 2: Feature Set

### Priority 1 Features
- **Spaceship Movement and Shooting:** Implement keyboard controls/animation for basic spaceship movement (constant velocity + rotation) and shooting (Marco)
- **Collision Detection:** Collision detection between spaceships, bullets, and arena boundaries (Justin)
- **Game Flow:** Implement logic to determine winner/loser and when to switch scenes (Nora)

### Priority 2 Features
- **Advanced Movement:** Implement double tap turn button to boost and recoil when spaceship shoots (Marco)
- **Arena Selection:** Multiple arenas with different layouts (Nora)
- **Basic HUD:** Display player lives and scores (Nora)
- **Physics Integration:** Implementation of friction when spaceship is in contact with arena border (Justin)

### Priority 3 Features
- **Sound Effects:** Basic sound effects for shooting and explosions (Justin)
- **Advanced Collision Handling:** More accurate and detailed collision responses (Marco)
- **Arena Hazards:** Add environmental hazards to arenas (e.g., asteroids, black holes) (Nora)

### Priority 4 Features
- **Custom Graphics:** Create custom sprites for spaceships and arenas (Nora)
- **Multiple arenas:** Create multiple arenas (Nora)
- **Music:** Background music for gameplay (Justin)
- **AI Opponents:** Implement AI for solo play or to fill multiplayer slots (Marco)

## Section 3: Timeline

### Week 1
- **Marco:** Spaceship Movement and Shooting
- **Justin:** Collision Detection
- **Nora:** Game Flow

### Week 2
- **Justin:** Physics Integration, Sound Effects
- **Marco:** Advanced Movement
- **Nora:** Arena Selection, Basic HUD

### Week 3
- **Justin:** Music
- **Marco:** Advanced Collision Handling, AI Opponents
- **Nora:** Arena Hazards, Custom Graphics

## Section 4: Disaster Recovery

### Justin
- **Issue:** Falling behind on spaceship movement or shooting mechanic
- **Plan:** Reallocate time from Music to complete high-priority tasks.

### Marco
- **Issue:** Difficulty with special maneuvers or HUD
- **Plan:** Simplify special maneuvers and HUD features to ensure completion of basic functionality. Reallocate time from AI.

### Nora
- **Issue:** Problems with physics integration or arena hazards
- **Plan:** Focus on completing essential physics integration first. Simplify or reduce arena hazards if falling behind.
