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

const vector_t MIN = {0, 0};
const vector_t MAX = {1000, 500};

const size_t INITIAL_GAME_CAPACITY = 5;
const size_t WIN_SCORE = 5;
const size_t N_PLAYERS = 2;
const char *TITLE_PATH = "assets/title.png";
const SDL_Rect TITLE_BOX = (SDL_Rect){MAX.x / 4, 100, MAX.x / 2, MAX.y / 3};
const size_t SCORE_HEIGHT = 30; // height of entire score bar

const double INIT_SHIP_SPEED = 0;
const double INIT_SHIP_ANGLES[] = {
  M_PI / 4, 
  5 * M_PI / 4, 
  7 * M_PI / 4,
  3 * M_PI / 4
};
const double PLAYER_ROT_SPEED = -4 * M_PI;
const double BOOST_VELOCITY = 400;
const double BOOST_ANGLE = -M_PI / 2;
const double BOOST_ROT_SPEED = -3 * M_PI;
const double DOUBLE_TAP_THRESH = 0.2 * CLOCKS_PER_SEC;

// sound constants
const char *SHOOT_SOUND_PATH = "assets/sounds/shoot.wav";
const char *BOOST_SOUND_PATH = "assets/sounds/boost.wav";

const double INIT_BULLET_SPEED = 500;

const double WALL_DIM = 1;

const double ELASTICITY = 1;
const double THRUST_POWER = 3000;
const double DRAG_COEF = 30;
const double ROT_DRAG_FACTOR = 7;

const rgb_color_t WHITE = (rgb_color_t){0, 1, 1};

enum mode {
  HOME,
  GAME,
  POST_GAME
};

typedef struct map {
  size_t num_blocks;
  size_t num_asteroids;
  size_t num_blackholes;
  const char *bg_path;
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

  map_t map;
  
  list_t *home_assets;
  list_t *game_assets;

  body_t *player1;
  body_t *player2;
  
  Mix_Chunk *shoot_sound;
  Mix_Chunk *boost_sound;

  scene_t *scene;
  double dt;
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

void toggle_play(state_t *state);

map_t maps[] = {
  {
    .num_blocks = 3,
    .num_asteroids = 10,
    .bg_path = "assets/space.png",
    .block_locations = (vector_t[]){(vector_t){100, 100}, 
    (vector_t){200, 200}, 
    (vector_t){300, 300}},
    .block_sizes = (vector_t[]){(vector_t){100, 100}, 
    (vector_t){100, 100}, 
    (vector_t){100, 100}},
    .start_pos = (vector_t[]){(vector_t){100, 300}, 
    (vector_t){400, 200}}
  }
};

double rand_double() { return (double)rand() / RAND_MAX; }

button_info_t button_templates[] = {
    {.image_path = "assets/play_button.png",
     .image_box = (SDL_Rect){400, 300, 200, 75},
     .handler = (void *)toggle_play},
};

void add_ship(state_t *state, vector_t pos, size_t team) {
  vector_t velocity = vec_make(INIT_SHIP_SPEED, INIT_SHIP_ANGLES[team]);
  body_t *ship_body = make_ship(pos, team, velocity, INIT_SHIP_ANGLES[team]);
  scene_add_body(state->scene, ship_body);
}

void reset_game(state_t *state) {
  // clear and re-add bricks. Which function initializes bricks in the
  // beginning of the game?
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
  entity_info_t *bullet_info = body_get_info(body2);
  if(bullet_info->team == 0){
    state->P1_score++;
  }
  if(bullet_info->team == 1){
    state->P2_score++;
  }
  reset_game(state);
}

/**
 * Collision handler that prevents object from phasing through wall
*/
void wall_collision_handler(body_t *body, body_t *wall, vector_t axis, void *aux, double force_const) {
  entity_info_t *info = body_get_info(body);
  switch (info->type) {
    case ASTEROID:
    case SHIP:
      physics_collision_handler(body, wall, axis, aux, 0.2 * ELASTICITY); // ship doesn't bounce against wall
      collision_info_t coll_info = find_collision(body, wall);
      double overlap = coll_info.overlap;
      vector_t body_dir = vec_unit(body_get_velocity(body));
      vector_t remove_overlap = vec_multiply(-overlap, body_dir);
      vector_t new_centroid = vec_add(body_get_centroid(body), remove_overlap);
      body_set_centroid(body, new_centroid);
      body_set_velocity(body, VEC_ZERO);
      break;
    default:
      break;
    }
} 

void add_bullet(state_t *state, body_t *ship) {
  vector_t ship_centroid = body_get_centroid(ship);
  double ship_angle = body_get_rotation(ship);
  body_t *bullet = make_bullet(ship_centroid, ship_angle, INIT_BULLET_SPEED);
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

/** adds ships and initializes the map
 *
 * @param state the state
 * @param map the map
 */    

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
    body_t *asteroid = make_asteroid(pos, 10 + rand_double() * 30, (vector_t){0, 0});
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

void init_map(state_t *state){
  map_t map = state->map;

  add_ship(state, map.start_pos[0], 0);
  add_ship(state, map.start_pos[1], 1);

  add_obstacles(state);
  add_asteroids(state);

  SDL_Rect background_bbox = (SDL_Rect){
      .x = MIN.x, .y = MIN.y, .w = MAX.x - MIN.x, .h = MAX.y - MIN.y};
  asset_t *background_asset =
      asset_make_image(map.bg_path, background_bbox);
  list_add(state->game_assets, background_asset);
}

void on_key(Uint8 *key_state, state_t *state) {
  if (state->mode != GAME) { return; }

  body_t *p1 = state->player1;
  body_t *p2 = state->player2;

  double dt = state->dt;
  double now = clock(); 

  if (key_state[P1_TURN]) {
    double time_since_last_press = now - get_last_press(P1_TURN);
    double time_since_last_release = now - get_last_release(P1_TURN);
    double time_held = dt;
    if (time_since_last_press < time_since_last_release) {
      time_held = fmax(time_held, time_since_last_release / CLOCKS_PER_SEC);
    }
    double rot_speed = PLAYER_ROT_SPEED * fmin(2, 1 + log(time_held * 5 + 1));
    double da = rot_speed * dt;
    double curr_angle = body_get_rotation(p1);
    body_set_rotation(p1, curr_angle + da);
    set_last_press(P1_TURN);
  } else {
    double time_since_last_press = now - get_last_press(P1_TURN);
    double time_since_last_release = now - get_last_release(P1_TURN);
    if (time_since_last_press < time_since_last_release && time_since_last_release < DOUBLE_TAP_THRESH) {
      double angle = body_get_rotation(p1);
      vector_t boost_impulse = vec_make(body_get_mass(p1) * BOOST_VELOCITY, angle + BOOST_ANGLE);
      body_add_impulse(p1, boost_impulse);
      body_add_rot_impulse(p1, body_get_rot_inertia(p1) * BOOST_ROT_SPEED);
      sdl_play_sound(state->boost_sound);
    }
    set_last_release(P1_TURN);
  }

  if (key_state[P2_TURN]) {
    double time_since_last_press = now - get_last_press(P2_TURN);
    double time_since_last_release = now - get_last_release(P2_TURN);
    double time_held = dt;
    if (time_since_last_press < time_since_last_release) {
      time_held = fmax(time_held, time_since_last_release / CLOCKS_PER_SEC);
    }
    double rot_speed = PLAYER_ROT_SPEED * fmin(2, 1 + log(time_held * 5 + 1));
    double da = rot_speed * dt;
    double curr_angle = body_get_rotation(p2);
    body_set_rotation(p2, curr_angle + da);
    set_last_press(P2_TURN);
  } else {
    double time_since_last_press = now - get_last_press(P2_TURN);
    double time_since_last_release = now - get_last_release(P2_TURN);
    if (time_since_last_press < time_since_last_release && time_since_last_release < DOUBLE_TAP_THRESH) {
      double angle = body_get_rotation(p1);
      vector_t boost_impulse = vec_make(body_get_mass(p2) * BOOST_VELOCITY, angle + BOOST_ANGLE);
      body_add_impulse(p2, boost_impulse);
      body_add_rot_impulse(p2, body_get_rot_inertia(p2) * BOOST_ROT_SPEED);
      sdl_play_sound(state->boost_sound);
    }
    set_last_release(P2_TURN);
  }

  if (key_state[P1_SHOOT]) {
    add_bullet(state, p1);
    sdl_play_sound(state->shoot_sound);
    set_last_press(P1_SHOOT);
  }

  if (key_state[P2_SHOOT]) {
    add_bullet(state, p2);
    sdl_play_sound(state->shoot_sound);
    set_last_press(P2_SHOOT);
  }
}

void toggle_play(state_t *state) {
  state->mode = GAME;
}

void handle_buttons(state_t *state, double x, double y) {
  size_t n_assets = list_size(state->home_assets);
  for (size_t i = 0; i < n_assets; i++) {
    asset_t *asset = list_get(state->home_assets, i);
    if (asset_get_type(asset) == ASSET_BUTTON) {
      asset_on_button_click(asset, state, x, y);
    }
  }
}

void on_click(state_t *state, double x, double y) {
  switch (state->mode) {
    case HOME:
      handle_buttons(state, x, y);
      break;
    case GAME:
      break;
    case POST_GAME:
      break;
    default:
      break;
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
 * Initializes and stores the button assets in the state.
 */
void create_buttons(state_t *state) {
  size_t n_buttons = sizeof(button_templates) / sizeof(button_templates[0]);
  for (size_t i = 0; i < n_buttons; i++) {
    button_info_t info = button_templates[i];
    asset_t *button = create_button_from_info(state, info);
    list_add(state->home_assets, button);
  }

}

void home_init(state_t *state) {
  create_buttons(state);

  asset_t *title = asset_make_image(TITLE_PATH, TITLE_BOX);
  list_add(state->home_assets, title);
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
        if(t == SHIP || t == ASTEROID){
          create_physics_collision(state->scene, body2, body, ELASTICITY);
        } else if (t == WALL) {
          create_collision(state->scene, body, body2, (collision_handler_t) wall_collision_handler, NULL, ELASTICITY);
        }
      }
      break;
    case ASTEROID:
      create_drag(state->scene, DRAG_COEF, body);
      for (size_t j = i+1; j < scene_bodies(state->scene); j++) {
        body_t *body2 = scene_get_body(state->scene, j);
        entity_type_t t = get_type(body2);
        if(t == SHIP || t == ASTEROID){
          create_physics_collision(state->scene, body2, body, ELASTICITY);
        } else if (t == WALL) {
          create_collision(state->scene, body, body2, (collision_handler_t) wall_collision_handler, NULL, ELASTICITY);
        }
      }
      break;
    case WALL:
      for (size_t j = i+1; j < scene_bodies(state->scene); j++) {
        body_t *body2 = scene_get_body(state->scene, j);
        entity_type_t t = get_type(body2);
        if (t == SHIP || t == ASTEROID){
          create_collision(state->scene, body2, body, (collision_handler_t) wall_collision_handler, NULL, ELASTICITY);
        }
      }
      break;
    default:
      break;
    }
  }
}

void render_assets(list_t *assets) {
  size_t n_assets = list_size(assets);
  for (size_t i = 0; i < n_assets; i++) {
    asset_render(list_get(assets, i));
  }
}



/** 
 * Renders score as a progress bar at the top of the screen.
*/
void render_scores(state_t *state) {
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

vector_t calc_cam_size(state_t *state){
  vector_t diff = vec_subtract(body_get_centroid(state->player1), body_get_centroid(state->player2));
  diff.x = fmax(fabs(diff.x) * 1.3, 100);
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
  state->home_assets = list_init(INITIAL_GAME_CAPACITY, (free_func_t) asset_destroy);
  state->game_assets = list_init(INITIAL_GAME_CAPACITY, (free_func_t) asset_destroy);
  state->map = maps[0];
  state->scene = scene_init();
  
  home_init(state);
  init_map(state);
  state->player1 = scene_get_body(state->scene, 0);
  state->player2 = scene_get_body(state->scene, 1);

  state->shoot_sound = Mix_LoadWAV(SHOOT_SOUND_PATH);
  state->boost_sound = Mix_LoadWAV(BOOST_SOUND_PATH);

  sdl_on_key((key_handler_t)on_key);
  sdl_on_click((click_handler_t)on_click);
  add_force_creators(state);
  
  return state;
}

bool emscripten_main(state_t *state) {
  double dt = time_since_last_tick();

  switch (state->mode) {
    case HOME: {
      sdl_clear();
      render_assets(state->home_assets);
      sdl_show();
      break;
    }
    case GAME: {
      scene_tick(state->scene, dt);

      if (state->P1_score > WIN_SCORE || state->P2_score > WIN_SCORE) { return true; }

      sdl_clear();
      render_assets(state->game_assets);
      vector_t cam_center = vec_multiply(0.5, vec_add(body_get_centroid(state->player1), body_get_centroid(state->player2)));
      sdl_render_scene_cam(state->scene, NULL, cam_center, calc_cam_size(state));
      //sdl_render_scene(state->scene, NULL);
      render_scores(state);
      sdl_show();

      state->dt = dt;
      sdl_is_done(state);
      break;
    }
    case POST_GAME: {
      break;
    }
  }

  return false;
}

void emscripten_free(state_t *state) {
  list_free(state->home_assets);
  list_free(state->game_assets);
  Mix_FreeChunk(state->shoot_sound);
  Mix_FreeChunk(state->boost_sound);
  scene_free(state->scene);
  asset_cache_destroy();
  free(state);
}