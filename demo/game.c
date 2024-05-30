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
#include "entities.h"
#include "shapes.h"

const vector_t MIN = {0, 0};
const vector_t MAX = {1000, 500};

const size_t INITIAL_GAME_CAPACITY = 5;
const size_t WIN_SCORE = 5;
const size_t N_PLAYERS = 2;
const char *TITLE_PATH = "assets/title.png";
const SDL_Rect TITLE_BOX = (SDL_Rect){100, 100, MAX.x / 2, MAX.y / 4};
const size_t SCORE_HEIGHT = 75; // height of entire score bar

const vector_t VEC_ZERO = (vector_t){.x = 0.0, .y = 0.0};
const vector_t INIT_VELOS[] = {
  (vector_t) {.x = 80, .y = 80},
  (vector_t) {.x = -80, .y = -80},
  (vector_t) {.x = 80, .y = -80},
  (vector_t) {.x = -80, .y = 80},
};

const double INIT_ANGLES[] = {
  M_PI / 4 - M_PI / 2, 
  5 * M_PI / 4 - M_PI / 2, 
  7 * M_PI / 4 - M_PI / 2,
  3 * M_PI / 4 - M_PI / 2
};

const double PLAYER_ROT_SPEED = -3 * M_PI;

const double WALL_DIM = 1;
const double ELASTICITY = 1;
const double THRUST_POWER = 50;
const double DRAG_COEF = 1;

rgb_color_t white = (rgb_color_t){0, 1, 1};

enum mode {
  HOME,
  GAME,
  POST_GAME
};

typedef struct map {
  size_t num_blocks;
  size_t num_asteroids;
  const char *bg_path;
  vector_t *block_locations;
  vector_t *block_sizes;
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
    .num_asteroids = 5,
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
     .image_box = (SDL_Rect){0, 200, 100, 100},
     .handler = (void *)toggle_play},
};

void add_ship(state_t *state, vector_t pos, size_t team) {
  body_t *ship_body = make_ship(pos, team, INIT_VELOS[team], INIT_ANGLES[team]);
  scene_add_body(state->scene, ship_body);
}

void add_bounds(state_t *state) {
  list_t *wall1_shape =
      make_rectangle((vector_t){MAX.x, MAX.y / 2}, WALL_DIM, MAX.y);
  body_t *wall1 = body_init_with_info(wall1_shape, INFINITY, white,
                                      entity_info_init(WALL, 100), free);
  list_t *wall2_shape =
      make_rectangle((vector_t){0, MAX.y / 2}, WALL_DIM, MAX.y);
  body_t *wall2 = body_init_with_info(wall2_shape, INFINITY, white,
                                      entity_info_init(WALL, 100), free);
  list_t *ceiling_shape =
      make_rectangle((vector_t){MAX.x / 2, MAX.y}, MAX.x, WALL_DIM);
  body_t *ceiling = body_init_with_info(ceiling_shape, INFINITY, white,
                                        entity_info_init(WALL, 100), free);
  list_t *ground_shape =
      make_rectangle((vector_t){MAX.x / 2, 0}, MAX.x, WALL_DIM);
  body_t *ground = body_init_with_info(ground_shape, INFINITY, white,
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
    body_t *block = body_init_with_info(block_shape, INFINITY, white,
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
  body_t *p1 = state->player1;
  body_t *p2 = state->player2;
  double dt = state->dt;

  if (key_state[SDL_SCANCODE_W]) {
    double da = PLAYER_ROT_SPEED * dt;
    double curr_angle = body_get_rotation(p1);
    body_set_rotation(p1, curr_angle + da);
    vector_t curr_velocity = body_get_velocity(p1);
    vector_t new_velocity = vec_rotate(curr_velocity, da);
    body_set_velocity(p1, new_velocity);
  }

  if (key_state[SDL_SCANCODE_M]) {
    double da = PLAYER_ROT_SPEED * dt;
    double curr_angle = body_get_rotation(p2);
    body_set_rotation(p2, curr_angle + da);
    vector_t curr_velocity = body_get_velocity(p2);
    vector_t new_velocity = vec_rotate(curr_velocity, da);
    body_set_velocity(p2, new_velocity);
  }
}

void toggle_play(state_t *state) {
  state->mode = GAME;
  init_map(state);
  state->player1 = scene_get_body(state->scene, 0);
  state->player2 = scene_get_body(state->scene, 1);
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

bool update_score(state_t *state) {
//   bool p1 = false;
//   bool p2 = false;
//   size_t n_bodies = scene_bodies(state->scene);

//   entity_type_t t1 = get_type(state->player1);
//   entity_type_t t2 = get_type(state->player2);

//   p1 = t1 == SHIP || t1 == PILOT;
//   p2 = t2 == SHIP || t2 == PILOT;

//   if (!(p1 && p2)) {
//     if (p1) {
//       state->P1_score++;
//       return true;
//     } else if (p2) {
//       state->P2_score++;
//       return true;
//     }
//   }
  return false;

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

void add_force_creators(state_t *state) {
  for (size_t i = 0; i < scene_bodies(state->scene); i++) {
    body_t *body = scene_get_body(state->scene, i);
    switch (get_type(body)) {
    case SHIP:
      create_thrust(state->scene, THRUST_POWER, body);
      create_drag(state->scene, DRAG_COEF, body);
      for (size_t j = i+1; j < scene_bodies(state->scene); j++) {
        body_t *body2 = scene_get_body(state->scene, j);
        entity_type_t t = get_type(body2);
        if(t == SHIP || t == WALL || t == ASTEROID){
          create_physics_collision(state->scene, body2, body, ELASTICITY);
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
  vector_t centroid = (vector_t){.x = width / 2.0, .y = SCORE_HEIGHT / 4.0};
  list_t *rectangle_pts = make_rectangle(centroid, width, height);
  polygon_t *rectangle = polygon_init(rectangle_pts, VEC_ZERO, 0.0, p1_color.r, 
                                      p1_color.g, p1_color.b);
  sdl_draw_polygon(rectangle, p1_color.r, p1_color.g, p1_color.b);

  // player 2
  size_t p2 = state->P2_score;
  rgb_color_t p2_color = PLAYER_COLORS[1];
  size_t width_2 = p2 * (MAX.x / WIN_SCORE);
  size_t height_2 = SCORE_HEIGHT / 2;
  vector_t centroid_2 = (vector_t){.x = width_2 / 2.0, .y = 0.75 * SCORE_HEIGHT};
  list_t *rectangle_pts_2 = make_rectangle(centroid_2, width_2, height_2);
  polygon_t *rectangle_2 = polygon_init(rectangle_pts_2, VEC_ZERO, 0.0, p2_color.r, 
                                      p2_color.g, p2_color.b);
  sdl_draw_polygon(rectangle_2, p2_color.r, p2_color.g, p2_color.b);
}

state_t *emscripten_init() {
  asset_cache_init();
  sdl_init(MIN, MAX);
  
  srand(time(NULL));
  state_t *state = malloc(sizeof(state_t));
  state->mode = GAME;
  state->P1_score = 0;
  state->P2_score = 0;
  state->home_assets = list_init(INITIAL_GAME_CAPACITY, (free_func_t) asset_destroy);
  state->game_assets = list_init(INITIAL_GAME_CAPACITY, (free_func_t) asset_destroy);
  state->map = maps[0];
  state->scene = scene_init();
  
  //home_init(state);
  init_map(state);
  state->player1 = scene_get_body(state->scene, 0);
  state->player2 = scene_get_body(state->scene, 1);

  sdl_on_key((key_handler_t)on_key);
  sdl_on_click((click_handler_t)on_click);
  add_force_creators(state);
  
  return state;
}

bool emscripten_main(state_t *state) {
  double dt = time_since_last_tick();

  switch (state->mode) {
    case HOME: {
      render_assets(state->home_assets);
      break;
    }
    case GAME: {
      scene_tick(state->scene, dt);

      if (state->P1_score > WIN_SCORE || state->P2_score > WIN_SCORE) { return true; }

      render_assets(state->game_assets);
      render_scores(state);
      sdl_render_scene(state->scene, NULL);
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
  scene_free(state->scene);
  asset_cache_destroy();
  free(state);
}