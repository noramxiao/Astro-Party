#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <SDL2/SDL.h>

#include "asset.h"
#include "asset_cache.h"
#include "collision.h"
#include "forces.h"
#include "sdl_wrapper.h"
#include "shapes.h"
#include "entities.h"
#include "bot.h"

const vector_t MIN = {0, 0};
const vector_t MAX = {1000, 500};

// game constants
const size_t INITIAL_GAME_CAPACITY = 5;
const size_t WIN_SCORE = 5;
const size_t SCORE_HEIGHT = 30; // height of entire score bar
const char *FONT_PATH = "assets/Roboto.ttf";
const char *GAME_OVER_MSG = "Game over! Winner is: Player ";
const char *PLAYER_COLOR_NAMES[] = {"Red", "Blue"};
const char *OPP_SELECTION_MSGS[] = {"Player vs. Player", "Play against AI"};
const SDL_Rect MAP_SELECTION_BOX = (SDL_Rect){572, 262, 10, 10};
const SDL_Rect OPP_SELECTION_BOX = (SDL_Rect){530, 325, 10, 10};
const rgb_color_t BLACK = (rgb_color_t){.r = 1, .g = 1, .b = 1};
const rgb_color_t WHITE = (rgb_color_t){1, 1, 1};

// sound constants
const char *SHOOT_SOUND_PATH = "assets/sounds/shoot.wav";
const char *BOOST_SOUND_PATH = "assets/sounds/boost.wav";
const char *BACKGROUND_TRACK = "assets/music/outro.ogg";

// environment constants
const double WALL_DIM = 1;
const double ASTEROID_MASS_DENSITY = 0.1;
const double ELASTICITY = 1;

// ship constants
const double SHIP_MASS = 10;
const double SHIP_BASE = 20;
const double SHIP_HEIGHT = 30;
const double INIT_SHIP_SPEED = 0;
const double INIT_SHIP_ANGLES[] = {
  5 * M_PI / 4,
  M_PI / 4
};
const double PLAYER_ROT_SPEED = -M_PI;
const double PLAYER_ROT_ACCEL = 3;
const double BOOST_VELOCITY = 400;
const double BOOST_ANGLE = -M_PI / 3;
const double BOOST_ROT_SPEED = -3 * M_PI;
const double DOUBLE_TAP_THRESH = 0.2 * CLOCKS_PER_SEC;
const double THRUST_POWER = 3000;
const double DRAG_COEF = 30;
const double ROT_DRAG_FACTOR = 7;
const double RELOAD_TIME = 0.5;

// bullet constants
const double BULLET_RADIUS = 5;
const double BULLET_MASS = 5;
const double BULLET_SPEED = 500;

double rand_double() { return (double)rand() / RAND_MAX; }
void toggle_play(state_t *state);
void toggle_left_map_arrow(state_t *state);
void toggle_right_map_arrow(state_t *state);
void toggle_bot_arrow(state_t *state);
void add_force_creators(state_t *state);

enum mode {
  HOME,
  GAME,
  POST_GAME
};

typedef struct map {
  size_t num_blocks;
  size_t num_asteroids;
  size_t num_blackholes;
  size_t num_bg;
  const char *backdrop_path;
  char **bg_paths;
  vector_t *bg_pos;
  vector_t *bg_sizes;
  double *bg_depth;
  vector_t *block_locations;
  vector_t *block_sizes;
  vector_t *blackhole_locations;
  double *blackhole_masses;
  vector_t *start_pos;
} map_t;

struct state {
  enum mode mode; // Keeps track of what page game is on
  size_t P1_score;
  size_t P2_score;

  size_t map_selected;
  bool bot;
  map_t map;
  
  list_t *home_assets;
  list_t *game_assets;
  list_t *post_game_assets;

  body_t *player1;
  body_t *player2;
  clock_t time_of_last_shot[2];
  
  Mix_Chunk *shoot_sound;
  Mix_Chunk *boost_sound;
  Mix_Music *backing_track;

  scene_t *scene;
  double dt;
  Uint8 *key_state;
};

typedef struct button_info {
  const char *image_path;
  const char *font_path;
  SDL_Rect image_box;
  SDL_Rect text_box;
  rgb_color_t text_color;
  const char *text;
  button_handler_t handler;
} button_info_t;

typedef struct image_info {
  const char *image_path;
  const char *font_path;
  SDL_Rect image_box;
  SDL_Rect text_box;
  rgb_color_t text_color;
  const char *text;
} image_info_t;

image_info_t home_images[] = {
  { .image_path = "assets/space.jpg",
    .image_box = (SDL_Rect){MIN.x, MIN.y, MAX.x, MAX.y}},
  { .image_path = "assets/title.png",
    .image_box = (SDL_Rect){MAX.x / 4, 20, MAX.x / 2, MAX.y / 3}},
  { .image_path = "assets/box.jpeg",
    .image_box = (SDL_Rect){525, 250, 200, 50},
    .font_path = "assets/Roboto.ttf",
    .text_box = (SDL_Rect){530, 260, 200, 75},
    .text_color = BLACK,
    .text = "Map "},
  { .image_path = "assets/box.jpeg",
    .image_box = (SDL_Rect){525, 315, 200, 50}},
  { .image_path = "assets/instructions.png",
    .image_box = (SDL_Rect){100, 185, 350, 315}
  }
};

image_info_t post_game_images[] = {
  { .image_path = "assets/space.jpg",
    .image_box = (SDL_Rect){MIN.x, MIN.y, MAX.x, MAX.y}},
  { .image_path = "assets/box.jpeg",
    .image_box = (SDL_Rect){300, 200, 400, 50}}
};

button_info_t button_templates[] = {
    {.image_path = "assets/play_button.png",
     .image_box = (SDL_Rect){520, 390, 200, 75},
     .handler = (void *)toggle_play},
    {.image_path = "assets/left_arrow.png",
     .image_box = (SDL_Rect){445, 245, 75, 75},
     .handler = (void*)toggle_left_map_arrow},
    {.image_path = "assets/right_arrow.png",
     .image_box = (SDL_Rect){725, 245, 75, 75},
     .handler = (void*)toggle_right_map_arrow},
    {.image_path = "assets/left_arrow.png",
     .image_box = (SDL_Rect){445, 310, 75, 75},
     .handler = (void*)toggle_bot_arrow},
    {.image_path = "assets/right_arrow.png",
     .image_box = (SDL_Rect){725, 310, 75, 75},
     .handler = (void*)toggle_bot_arrow}
};

map_t maps[] = {
  {
    .num_blocks = 3,
    .num_asteroids = 10,
    .num_bg = 3,
    .backdrop_path = "assets/space1.png",
    .bg_paths = (char *[]){"assets/neptune.png", "assets/planet1.png", "assets/earthlike.png"},
    .bg_pos = (vector_t[]){(vector_t){400, 150}, (vector_t){500, 200}, (vector_t){250, 280}},
    .bg_sizes = (vector_t[]){(vector_t){300, 300}, (vector_t){150, 150}, (vector_t){120, 120}},
    .bg_depth = (double[]){6, 3, 2},
    .block_locations = (vector_t[]){(vector_t){100, 100}, 
    (vector_t){200, 200}, 
    (vector_t){300, 300}},
    .block_sizes = (vector_t[]){(vector_t){100, 100}, 
    (vector_t){100, 100}, 
    (vector_t){100, 100}},
    .start_pos = (vector_t[]){(vector_t){100, 300}, 
    (vector_t){700, 200}}
  },
  {
    .num_blocks = 4,
    .num_asteroids = 7,
    .num_bg = 0,
    .backdrop_path = "assets/space2.jpg",
    .block_locations = (vector_t[]){(vector_t){100, 100},
    (vector_t){250, 250},
    (vector_t){400, 100},
    (vector_t){100, 400}},
    .block_sizes = (vector_t[]){(vector_t){100, 100},
    (vector_t){75, 75},
    (vector_t){100, 100},
    (vector_t){75, 75}},
    .start_pos = (vector_t[]){(vector_t){300, 100},
    (vector_t){600, 400}}
  },
  {
    .num_blocks = 5,
    .num_asteroids = 12,
    .num_bg = 0,
    .backdrop_path = "assets/space3.jpg",
    .block_locations = (vector_t[]){(vector_t){150, 150},
    (vector_t){300, 300},
    (vector_t){450, 150},
    (vector_t){600, 300},
    (vector_t){750, 150}},
    .block_sizes = (vector_t[]){(vector_t){120, 120},
    (vector_t){80, 80},
    (vector_t){100, 100},
    (vector_t){80, 80},
    (vector_t){120, 120}},
    .start_pos = (vector_t[]){(vector_t){50, 450},
    (vector_t){950, 50}}
  },
  {
    .num_blocks = 6,
    .num_asteroids = 15,
    .num_bg = 0,
    .backdrop_path = "assets/space4.jpg",
    .block_locations = (vector_t[]){(vector_t){600, 300},
    (vector_t){200, 400},
    (vector_t){350, 150},
    (vector_t){500, 350},
    (vector_t){650, 100},
    (vector_t){800, 300}},
    .block_sizes = (vector_t[]){(vector_t){90, 90},
    (vector_t){110, 110},
    (vector_t){95, 95},
    (vector_t){110, 110},
    (vector_t){90, 90},
    (vector_t){95, 95}},
    .start_pos = (vector_t[]){(vector_t){400, 50},
    (vector_t){900, 450}}
  }
};

void reset_game(state_t *state) {
  size_t n_bodies = scene_bodies(state->scene);

  for (size_t i = 0; i < n_bodies; i++) {
    body_t *body = scene_get_body(state->scene, i);
    if (get_type(body) == BULLET) {
      body_remove(body);
    }
  }

  // reset players's velocity and position
  body_set_centroid(state->player1, state->map.start_pos[0]);
  body_set_velocity(state->player1, (vector_t) {0, 0});
  body_set_centroid(state->player2, state->map.start_pos[1]);
  body_set_velocity(state->player2, (vector_t) {0, 0});

  // reset players's forces and impulses
  body_reset(state->player1);
  body_reset(state->player2);
}

void score_hit(body_t *body1, body_t *body2, vector_t axis, void *aux,
                double force_const) {
  state_t *state = aux;
  entity_info_t *ship_info = body_get_info(body1);
  if(ship_info->team == 1){
    state->P1_score++;
  }
  if(ship_info->team == 0){
    state->P2_score++;
  }
  reset_game(state);
}


void add_ship(state_t *state, vector_t pos, size_t team) {
  vector_t velocity = vec_make(INIT_SHIP_SPEED, INIT_SHIP_ANGLES[team]);
  body_t *ship_body = make_ship(pos, team, velocity, INIT_SHIP_ANGLES[team], 
                                SHIP_BASE, SHIP_HEIGHT, SHIP_MASS);
  scene_add_body(state->scene, ship_body);
}

void add_bounds(state_t *state) {
  list_t *wall1_shape =
      make_rectangle((vector_t){MAX.x, MAX.y / 2}, WALL_DIM, MAX.y);
  body_t *wall1 = body_init_with_info(wall1_shape, INFINITY, WHITE,
                                      entity_info_init(WALL, 100), free);
  list_t *wall2_shape =
      make_rectangle((vector_t){0, MAX.y / 2}, WALL_DIM, MAX.y);
  body_t *wall2 = body_init_with_info(wall2_shape, INFINITY, WHITE,
                                      entity_info_init(WALL, 100), free);
  list_t *ceiling_shape =
      make_rectangle((vector_t){MAX.x / 2, MAX.y}, MAX.x, WALL_DIM);
  body_t *ceiling = body_init_with_info(ceiling_shape, INFINITY, WHITE,
                                        entity_info_init(WALL, 100), free);
  list_t *ground_shape =
      make_rectangle((vector_t){MAX.x / 2, 0}, MAX.x, WALL_DIM);
  body_t *ground = body_init_with_info(ground_shape, INFINITY, WHITE,
                                       entity_info_init(WALL, 100), free);
  scene_add_body(state->scene, wall1);
  scene_add_body(state->scene, wall2);
  scene_add_body(state->scene, ceiling);
  scene_add_body(state->scene, ground);
}

void add_obstacles(state_t *state){
  add_bounds(state);
  for(size_t i = 0; i < state->map.num_blocks; i++){
    list_t *block_shape = make_rectangle(state->map.block_locations[i], state->map.block_sizes[i].x, state->map.block_sizes[i].y);
    body_t *block = body_init_with_info(block_shape, INFINITY, WHITE,
                                      entity_info_init(WALL, 100), free);
    scene_add_body(state->scene, block);
  }
}

void add_asteroids(state_t *state){
  for(size_t i = 0; i < state->map.num_asteroids; i++){
    bool pos_found = false;
    vector_t pos = (vector_t) {rand_double()*MAX.x, rand_double()*MAX.y};
    body_t *asteroid = make_asteroid(pos, 10 + rand_double() * 30, VEC_ZERO, 
                                     ASTEROID_MASS_DENSITY);
    while(!pos_found){
      pos_found = true;
      pos = (vector_t) {rand_double()*MAX.x, rand_double()*MAX.y};
      body_set_centroid(asteroid, pos);
      size_t n_bodies = scene_bodies(state->scene);
      
      for (size_t i = 0; i < n_bodies; i++) {
        body_t *body = scene_get_body(state->scene, i);
        if(find_collision(body, asteroid).collided){
          pos_found = false;
          break;
        }
      }
    }
    scene_add_body(state->scene, asteroid);
  }
}

/**
 * Initializes map elements such as ships, obstacles, asteroids, and background images
 * and adds them to the scene and state.
 * 
 * @param state the state
 */
void map_init(state_t *state){
  map_t map = maps[state->map_selected];
  state->map = map;

  add_ship(state, map.start_pos[0], 0);
  add_ship(state, map.start_pos[1], 1);

  add_obstacles(state);
  add_asteroids(state);

  if (map.backdrop_path != NULL) {
    SDL_Rect background_bbox = (SDL_Rect){
      .x = MIN.x, .y = MIN.y, .w = MAX.x - MIN.x, .h = MAX.y - MIN.y};
    asset_t *background_asset =
      asset_make_image(map.backdrop_path, background_bbox);
    list_add(state->game_assets, background_asset);
  }
  
  for(size_t i = 0; i < map.num_bg; i++){
    SDL_Rect background_bbox = (SDL_Rect){
      .x = map.bg_pos[i].x, .y = map.bg_pos[i].y, .w = map.bg_sizes[i].x, .h = map.bg_sizes[i].y};
    asset_t *background_asset =
      asset_make_image(map.bg_paths[i], background_bbox);
    list_add(state->game_assets, background_asset);
  }
}

void handle_turn(body_t *ship, double time_held, double dt) {
  // increase rotation speed at log rate
  double rot_speed = PLAYER_ROT_SPEED * fmin(2, 1 + log(time_held * PLAYER_ROT_ACCEL + 1));
  double da = rot_speed * dt;
  double curr_angle = body_get_rotation(ship);
  body_set_rotation(ship, curr_angle + da);
}

void handle_boost(body_t *ship, double time_since_turn_pressed, 
                  double time_since_turn_released, Mix_Chunk *boost_sound) {
  if (time_since_turn_pressed < time_since_turn_released && 
      time_since_turn_released < DOUBLE_TAP_THRESH) {
    double angle = body_get_rotation(ship);
    vector_t boost_impulse = vec_make(body_get_mass(ship) * BOOST_VELOCITY, angle + BOOST_ANGLE);
    body_add_impulse(ship, boost_impulse);
    body_add_rot_impulse(ship, body_get_rot_inertia(ship) * BOOST_ROT_SPEED);
    sdl_play_sound(boost_sound);
  }
}

void handle_shoot(state_t *state, body_t *ship) {
  // check if player has shot before reload time is up
  double now = clock();
  double time_since_shot;
  if (ship == state->player1) {
    time_since_shot = now - state->time_of_last_shot[0];
  } else {
    time_since_shot = now - state->time_of_last_shot[1];
  }
  time_since_shot = time_since_shot / CLOCKS_PER_SEC;
  if (time_since_shot < RELOAD_TIME) {
    return;
  }

  sdl_play_sound(state->shoot_sound);
  
  vector_t ship_centroid = body_get_centroid(ship);
  double ship_angle = body_get_rotation(ship);
  body_t *bullet = make_bullet(ship_centroid, ship_angle, BULLET_SPEED, BULLET_RADIUS,
                               BULLET_MASS, SHIP_HEIGHT);
  scene_t *scene = state->scene;
  scene_add_body(scene, bullet);
  for (int i = 0; i < scene_bodies(scene); i++) {
    body_t *body = scene_get_body(scene, i);
    if (body == bullet) {
      continue;
    }
    if (get_type(body) == BULLET || get_type(body) == ASTEROID) {
      create_destructive_collision(scene, body, bullet);
    } else if (get_type(body) == WALL) {
      create_destroy_first_collision(scene, bullet, body);
    } else if (get_type(body) == SHIP) {
      create_collision(scene, body, bullet, (collision_handler_t) score_hit, state, ELASTICITY);
    }
  }

  // update time of last shot by player
  if (ship == state->player1) {
    state->time_of_last_shot[0] = now;
  } else {
    state->time_of_last_shot[1] = now;
  }
}

void on_key(state_t *state) {
  if (state->mode != GAME || !state->key_state) { 
    return; 
  }

  body_t *p1 = state->player1;
  body_t *p2 = state->player2;

  double dt = state->dt;
  double now = clock(); 

  Uint8 *key_state = state->key_state;
  if (key_state[P1_TURN]) {
    double time_held = fmax(dt, get_time_held(P1_TURN));
    handle_turn(p1, time_held, dt);
    set_last_press(P1_TURN);
  } else {
    double time_since_last_press = now - get_last_press(P1_TURN);
    double time_since_last_release = now - get_last_release(P1_TURN);
    if (time_since_last_press < time_since_last_release) {
      handle_boost(p1, time_since_last_press, time_since_last_release, state->boost_sound);
      set_last_release(P1_TURN);
    }
  }

  if (key_state[P2_TURN]) {
    double time_held = fmax(dt, get_time_held(P2_TURN));
    handle_turn(p2, time_held, dt);
    set_last_press(P2_TURN);
  } else {
    double time_since_last_press = now - get_last_press(P2_TURN);
    double time_since_last_release = now - get_last_release(P2_TURN);
    if (time_since_last_press < time_since_last_release) {
      handle_boost(p2, time_since_last_press, time_since_last_release, state->boost_sound);
      set_last_release(P2_TURN);
    }
  }

  if (key_state[P1_SHOOT]) {
    handle_shoot(state, p1);
    set_last_press(P1_SHOOT);
  }

  if (key_state[P2_SHOOT]) {
    handle_shoot(state, p2);
    set_last_press(P2_SHOOT);
  }

  free(key_state);
  state->key_state = NULL;
}

/**
 * Called when the play button is clicked. Initializes the game page based on home page
 * selections.
 * 
 * @param state the state
 */
void toggle_play(state_t *state) {
  state->mode = GAME;
  map_init(state);
  state->player1 = scene_get_body(state->scene, 0);
  state->player2 = scene_get_body(state->scene, 1);

  add_force_creators(state);
}

/**
 * Called when left arrow button is clicked to toggle between map options. Updates map
 * selection in the state by decrementing the map option.
 * 
 * @param state the state
 */
void toggle_left_map_arrow(state_t *state) {
  if (state->map_selected == 0) {
    state->map_selected = 3;
    return;
  }
  state->map_selected--;
}

/**
 * Called when left arrow button is clicked to toggle between map options. Updates map
 * selection in the state by incrementing the map option.
 * 
 * @param state the state
 */
void toggle_right_map_arrow(state_t *state) {
  state->map_selected++;
  state->map_selected %= 4;
}

/**
 * Called when toggling between opponent options in the home page. Updates opponent
 * selection in the state.
 * 
 * @param state the state
 */
void toggle_bot_arrow(state_t *state) {
  state->bot = !state->bot;
}

/**
 * on_click helper function. Passes coords to button assets.
 * 
 * @param state the state
 * @param x x coord of the click
 * @param y y coord of the click
 */
void handle_buttons(state_t *state, double x, double y) {
  size_t n_assets = list_size(state->home_assets);
  for (size_t i = 0; i < n_assets; i++) {
    asset_t *asset = list_get(state->home_assets, i);
    if (asset_get_type(asset) == ASSET_BUTTON) {
      asset_on_button_click(asset, state, x, y);
    }
  }
}

/**
 * Called when the user clicks the screen. Passes x, y coordinates to be handled.
 * 
 * @param state the state
 * @param x x coord of the click
 * @param y y coord of the click
 */
void on_click(state_t *state, double x, double y) {
  if (state->mode == HOME) { // mouse input only needed in home page
    handle_buttons(state, x, y);
  }
}

/**
 * Using `info`, initializes a button in the scene.
 *
 * @param info the button info struct used to initialize the button
 */
asset_t *create_button_from_info(state_t *state, button_info_t info) {
  asset_t *image_asset = NULL;
  asset_t *text_asset = NULL;
  if (info.image_path != NULL) {
    image_asset = asset_make_image(info.image_path, info.image_box);
  }
  if (info.font_path != NULL) {
    text_asset = asset_make_text(info.font_path, info.text_box, info.text,
                                 info.text_color);
  }

  asset_t *button =
      asset_make_button(info.image_box, image_asset, text_asset, info.handler);

  asset_cache_register_button(button);

  return button;
}

/**
 * Using `info`, initializes an image and adds to list.
 *
 * @param list list to add the image asset to
 * @param info the image info struct used to initialize the image
 * @param info_size the size of the array of image info's
 */
void add_image_from_info(list_t *list, image_info_t info[], size_t info_size) {
  for (size_t i = 0; i < info_size; i++) {
    image_info_t img = info[i];
    asset_t *image_asset = NULL;
    asset_t *text_asset = NULL;
    if (img.image_path != NULL) {
      image_asset = asset_make_image(img.image_path, img.image_box);
      list_add(list, image_asset);
    }
    if (img.font_path != NULL) {
      text_asset = asset_make_text(img.font_path, img.text_box, img.text,
                                  img.text_color);
      list_add(list, text_asset);
  }
  }
}

/**
 * Initializes and stores the button assets in the state.
 * 
 * @param state the state
 */
void create_buttons(state_t *state) {
  size_t n_buttons = sizeof(button_templates) / sizeof(button_templates[0]);
  for (size_t i = 0; i < n_buttons; i++) {
    button_info_t info = button_templates[i];
    asset_t *button = create_button_from_info(state, info);
    list_add(state->home_assets, button);
  }
}

/**
 * Initializes images and buttons in the home page and adds them to the state.
 * 
 * @param state the state
 */
void home_init(state_t *state) {
  size_t size = sizeof(home_images) / sizeof(home_images[0]);
  
  add_image_from_info(state->home_assets, home_images, size);
  create_buttons(state);
}

void add_force_creators(state_t *state) {
  for (size_t i = 0; i < scene_bodies(state->scene); i++) {
    body_t *body = scene_get_body(state->scene, i);
    switch (get_type(body)) {
    case SHIP:
      create_thrust(state->scene, THRUST_POWER, body);
      create_drag(state->scene, DRAG_COEF, body);
      double rot_inertia = body_get_rot_inertia(body);
      double rot_drag_coef = ROT_DRAG_FACTOR * rot_inertia;
      create_rot_drag(state->scene, rot_drag_coef, body);
      for (size_t j = i+1; j < scene_bodies(state->scene); j++) {
        body_t *body2 = scene_get_body(state->scene, j);
        entity_type_t t = get_type(body2);
        if(t == SHIP || t == ASTEROID || t == WALL){
          create_compound_collision(state->scene, body, body2, ELASTICITY);
        }
      }
      break;
    case ASTEROID:
      create_drag(state->scene, DRAG_COEF, body);
      for (size_t j = i+1; j < scene_bodies(state->scene); j++) {
        body_t *body2 = scene_get_body(state->scene, j);
        entity_type_t t = get_type(body2);
        if(t == ASTEROID || t == WALL){
          create_compound_collision(state->scene, body, body2, ELASTICITY);
        }
      }
      break;
    default:
      break;
    }
  }
}

/**
 * Renders a list of assets to the screen.
 * 
 * @param assets list_t of assets to be rendered.
 */
void render_assets(list_t *assets) {
  size_t n_assets = list_size(assets);
  for (size_t i = 0; i < n_assets; i++) {
    asset_render(list_get(assets, i));
  }
}

void render_bg_track(state_t *state, vector_t cam_pos, vector_t cam_size) {
  vector_t center = vec_multiply(0.5, MAX);
  asset_render_cam(list_get(state->game_assets, 0), center, MAX);

  map_t map = (map_t) state->map;
  double cam_dist = cam_size.x/MAX.x;
  for(size_t i = 0; i < map.num_bg; i++){
    vector_t scaled_diff = vec_multiply(1/map.bg_depth[i], vec_subtract(cam_pos, center));
    double scale = (map.bg_depth[i] + cam_dist)/(1 + map.bg_depth[i]);
    asset_render_cam(list_get(state->game_assets, i+1), vec_add(center, scaled_diff), vec_multiply(scale, MAX));
  }
  
}

void render_bg_zoom(state_t *state, vector_t cam_pos, vector_t cam_size) {
  vector_t center = vec_multiply(0.5, MAX);
  asset_render_cam(list_get(state->game_assets, 0), center, vec_multiply(0.7, cam_size));

  map_t map = (map_t) state->map;
  for(size_t i = 0; i < map.num_bg; i++){
    vector_t scaled_diff = vec_multiply(1/map.bg_depth[i], vec_subtract(cam_pos, center));
    asset_render_cam(list_get(state->game_assets, i+1), vec_add(center, scaled_diff), cam_size);
  }
  
}

/** 
 * Renders score as a progress bar at the top of the screen. Game page only.
 * 
 * @param state the state
*/
void game_render_scores(state_t *state) {
  // player 1
  size_t p1 = state->P1_score;
  rgb_color_t p1_color = PLAYER_COLORS[0];
  size_t width = p1 * (MAX.x / WIN_SCORE);
  size_t height = SCORE_HEIGHT / 2;
  vector_t centroid = (vector_t){.x = width / 2.0, .y = MAX.y - SCORE_HEIGHT / 4.0};
  list_t *rectangle_pts = make_rectangle(centroid, width, height);
  polygon_t *rectangle = polygon_init(rectangle_pts, VEC_ZERO, 0.0, p1_color.r, 
                                      p1_color.g, p1_color.b);
  sdl_draw_polygon(rectangle, p1_color);

  // player 2
  size_t p2 = state->P2_score;
  rgb_color_t p2_color = PLAYER_COLORS[1];
  size_t width_2 = p2 * (MAX.x / WIN_SCORE);
  size_t height_2 = SCORE_HEIGHT / 2;
  vector_t centroid_2 = (vector_t){.x = width_2 / 2.0, .y = MAX.y - 0.75 * SCORE_HEIGHT};
  list_t *rectangle_pts_2 = make_rectangle(centroid_2, width_2, height_2);
  polygon_t *rectangle_2 = polygon_init(rectangle_pts_2, VEC_ZERO, 0.0, p2_color.r, 
                                      p2_color.g, p2_color.b);
  sdl_draw_polygon(rectangle_2, p2_color);
}

/**
 * Displays which map and what opponent type the user has selected. Home page only.
 * 
 * @param state the state
*/
void home_render_selected(state_t *state) {
  // Map selection
  SDL_Rect map_box = MAP_SELECTION_BOX;
  char *map_selected = " ";

  switch (state->map_selected) {
    case 0:
      map_selected = "1";
      break;
    case 1:
      map_selected = "2";
      break;
    case 2:
      map_selected = "3";
      break;
    default:
      map_selected = "4";
      break;
  }
  asset_t *map_text = asset_make_text(FONT_PATH, map_box, map_selected, WHITE);
  asset_render(map_text);

  // Opponent selection
  SDL_Rect opp_box = OPP_SELECTION_BOX;
  char *opp_selected = " ";
  if (state->bot) {
    opp_selected = strdup(OPP_SELECTION_MSGS[1]);
    assert(opp_selected);
  } else {
    opp_selected = strdup(OPP_SELECTION_MSGS[0]);
    assert(opp_selected);
  }
  asset_t *opp_text = asset_make_text(FONT_PATH, opp_box, opp_selected, WHITE);
  asset_render(opp_text);
}

/**
 * Initializes images and buttons in the post game page and adds them to the state.
 * 
 * @param state the state
 */
void post_game_init(state_t *state) {
  size_t size = sizeof(post_game_images) / sizeof(post_game_images[0]);
  add_image_from_info(state->post_game_assets, post_game_images, size);

  char *msg = strdup(GAME_OVER_MSG);
  assert(msg);
  char *color = " ";
  ssize_t winner = state->P1_score - state->P2_score; 
  if (winner > 0) {
    color = strdup(PLAYER_COLOR_NAMES[0]);
    assert(color);
    msg = strcat(msg, color);
  } else {
    color = strdup(PLAYER_COLOR_NAMES[1]);
    assert(color);
    msg = strcat(msg, color);
  }

  SDL_Rect box = (SDL_Rect){post_game_images[1].image_box.x + 15, 
                            post_game_images[1].image_box.y + 15, MAX.x / 4, MAX.y / 4};
  asset_t *msg_asset = asset_make_text(FONT_PATH, box, msg, WHITE);

  list_add(state->post_game_assets, msg_asset);
}

vector_t calc_cam_size(state_t *state){
  vector_t diff = vec_subtract(body_get_centroid(state->player1), body_get_centroid(state->player2));
  diff.x = fmax(fabs(diff.x) * 1.3, 300);
  diff.y = fabs(diff.y) * 1.3;
  if(diff.x > 2*diff.y){
    return (vector_t) {diff.x, diff.x/2};
  }
  else{
    return (vector_t) {2*diff.y, diff.y};
  }
}

state_t *emscripten_init() {
  asset_cache_init();
  sdl_init(MIN, MAX);
  
  srand(time(NULL));
  state_t *state = malloc(sizeof(state_t));
  state->mode = HOME;
  state->P1_score = 0;
  state->P2_score = 0;
  state->time_of_last_shot[0] = 0;
  state->time_of_last_shot[1] = 0;
  state->bot = false;
  state->map_selected = 0;
  state->home_assets = list_init(INITIAL_GAME_CAPACITY, (free_func_t) asset_destroy);
  state->game_assets = list_init(INITIAL_GAME_CAPACITY, (free_func_t) asset_destroy);
  state->post_game_assets = list_init(INITIAL_GAME_CAPACITY, (free_func_t) asset_destroy);
  state->dt = 0;
  state->key_state = NULL;
  state->scene = scene_init();
  state->shoot_sound = sdl_load_sound(SHOOT_SOUND_PATH);
  state->boost_sound = sdl_load_sound(BOOST_SOUND_PATH);
  state->backing_track = sdl_load_music(BACKGROUND_TRACK);
  
  home_init(state);
  sdl_play_music(state->backing_track);

  sdl_on_key((key_handler_t)on_key);
  sdl_on_click((click_handler_t)on_click);
  
  return state;
}

bool emscripten_main(state_t *state) {
  double dt = time_since_last_tick();

  switch (state->mode) {
    case HOME: {
      sdl_clear();
      render_assets(state->home_assets);
      home_render_selected(state);
      sdl_show();
      break;
    }
    case GAME: {
      scene_tick(state->scene, dt);

      // game over
      if (state->P1_score >= WIN_SCORE || state->P2_score >= WIN_SCORE) { 
        state->mode = POST_GAME;
        post_game_init(state);
      }

      // render scene with camera
      sdl_clear();
      vector_t cam_center = vec_multiply(0.5, vec_add(body_get_centroid(state->player1), 
                                        body_get_centroid(state->player2)));
      render_bg_track(state, cam_center, calc_cam_size(state));
      sdl_render_scene_cam(state->scene, NULL, cam_center, calc_cam_size(state));
      game_render_scores(state);
      sdl_show();

      // bot update
      state->key_state = sdl_get_keystate();
      if (state->bot) {
        game_info_t *info = malloc(sizeof(game_info_t));
        assert(info);
        *info = (game_info_t) {
          .p1 = state->player1,
          .p2 = state->player2,
          .bullet_speed = BULLET_SPEED,
          .bullet_radius = BULLET_RADIUS,
          .ship_base = SHIP_BASE,
          .ship_height = SHIP_HEIGHT,
          .ship_rot_speed = PLAYER_ROT_SPEED, 
          .double_tap_thresh = DOUBLE_TAP_THRESH,
          .scene = state->scene,
          .dt = state->dt
        };
        bot_move(state->key_state, info, state->player2);
        free(info);
      }
      
      state->dt = dt;
      sdl_is_done(state);
      break;
    }
    case POST_GAME: {
      sdl_clear();
      render_assets(state->post_game_assets);
      sdl_show();
      break;
    }
  }

  return false;
}

void emscripten_free(state_t *state) {
  list_free(state->home_assets);
  list_free(state->game_assets);
  list_free(state->post_game_assets);
  Mix_FreeChunk(state->shoot_sound);
  Mix_FreeChunk(state->boost_sound);
  Mix_FreeMusic(state->backing_track);
  scene_free(state->scene);
  asset_cache_destroy();
  free(state);
}