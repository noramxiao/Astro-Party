# Space Wars Game Design Document

## Section 0: Summary

**Working Title:** Space Wars

**Team Members:**
- Justin (Physics Integration)
- Marco (Graphics and Animation)
- Nora (Gameplay and UI Design)

**Concept Statement:** A multiplayer space battle game where players control spaceships with realistic physics, aiming to outmaneuver and outgun their opponents in a variety of space arenas.

## Section 1: Gameplay

**Game Progression:**
- The game begins with players positioned at different locations on the map.
- Players control their ships, trying to eliminate opponents by shooting them.
- The player who shoots their opponent wins the round.
- First player to 5 rounds wins.

**Controls:**
- **Keyboard:**
  - Players use two keys each (one for shooting, one for turning)
  - Default keybinds: player 1 uses "W" and "Q" to turn and shoot, and player 2 uses "M" and "N" to turn and shoot, respectively.
- **Mouse:** 
  - Players can use the mouse to interact with the UI to select the map/start the game.

**Physics:**
- The physics engine will calculte the movement of the ships as players turn and shoot, including:
- Position of the ship with a fixed velocity and input-based rotation
- Acceleration when the player double taps the turn button
- Collisions with the walls and other spaceships

**Game Flow:**
1. Main Menu: Choose the map and whether to play against AI and start the game.
2. Gameplay: Players battle in the selected arena until a player triumphs.
3. End Screen: Display winner.

**Graphics:**
- Vector graphics spaceships, bullets, obstacles, borders.
- Animation for spaceship movement and shooting.

## Section 2: Feature Set

### Priority 1 Features
- **Controls:** Implementation of keyboard controls (Marco)
- **Vector Graphics:** Create graphics for spaceships, obstacles, and bullets (Marco)
- **Map:** Implementation of arena borders and obstacles (Justin)
- **Game Flow:** Implement logic to determine winner/loser and when to switch scenes (Nora)

### Priority 2 Features
- **Advanced Movement:** Implement double tap turn button to boost (Marco)
- **Advanced Physics:** Implementation of drag and thrust forces for spaceship velocity cap (Justin)
- **Basic HUD:** Display player lives and scores (Nora)

### Priority 3 Features
- **Sound Effects:** Sound effects for shooting and explosions (Marco)
- **Camera movement:** Dynamically zoom in on the scene of action (Justin)
- **UI:** Create UI for homepage and render images, backgrounds, and text for all pages (Nora)

### Priority 4 Features
- **Multiple arenas:** Create multiple arenas and allow selection (Nora)
- **Music:** Background music for gameplay (Justin)
- **AI Opponents:** Implement AI to take over player 2 (Marco)

## Section 3: Timeline

### Week 1
- **Marco:** Spaceship Controls, Vector Graphics
- **Justin:** Map Creation, Advanced Physics
- **Nora:** Game Flow

### Week 2
- **Marco:** Advanced Movement, Sound Effects
- **Justin:** Advanced Physics (cont.)
- **Nora:** Basic HUD, UIs

### Week 3
- **Justin:** Music
- **Marco:** AI Opponents
- **Nora:** Multiple Arenas, Custom Graphics

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
