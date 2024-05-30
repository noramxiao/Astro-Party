#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "asset.h"
#include "asset_cache.h"
#include "collision.h"
#include "forces.h"
#include "sdl_wrapper.h"
#include "entities.h"

const vector_t MIN = {0, 0};
const vector_t MAX = {1000, 500};

const size_t INITIAL_CAPACITY = 5;
const size_t WIN_SCORE = 5;
const size_t N_PLAYERS = 2;

const double SHIP_MASS = 10.0;
const size_t CIRC_NPOINTS = 100;

rgb_color_t PLAYER_COLORS[] = (rgb_color_t[]){(rgb_color_t){1, 0, 0}, (rgb_color_t){0, 0, 1}};

enum mode {
  HOME,
  GAME,
  POST_GAME
};

typedef struct map {
  size_t num_blocks;
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
  
  list_t *assets;

  body_t *ship1;
  body_t *ship2;

  scene_t *scene;
};

typedef struct map {
  size_t num_blocks;
  const char *bg_path;
  vector_t *block_locations;
  vector_t *block_sizes;
} map_t;

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
    .bg_path = "assets/frogger-background.png",
    .block_locations = (vector_t[]){(vector_t){100, 100}, 
    (vector_t){200, 200}, 
    (vector_t){300, 300}},
    .block_sizes = (vector_t[]){(vector_t){100, 100}, 
    (vector_t){100, 100}, 
    (vector_t){100, 100}},
    .start_pos = (vector_t[]){(vector_t){100, 300}, 
    (vector_t){400, 200}}
  }
}

button_info_t button_templates[] = {
    {.image_path = "assets/play_button.png",
     .image_box = (SDL_Rect){0, 200, 100, 100},
     .handler = (void *)toggle_play},
};



/** Make a rectangle-shaped body object.
 *
 * @param center a vector representing the center of the body.
 * @param width the width of the rectangle
 * @param height the height of the rectangle
 * @return pointer to the rectangle-shaped body
 */
list_t *make_rectangle(vector_t center, double width, double height) {
  list_t *points = list_init(4, free);
  vector_t *p1 = malloc(sizeof(vector_t));
  *p1 = (vector_t){center.x - width / 2, center.y - height / 2};

  vector_t *p2 = malloc(sizeof(vector_t));
  *p2 = (vector_t){center.x + width / 2, center.y - height / 2};

  vector_t *p3 = malloc(sizeof(vector_t));
  *p3 = (vector_t){center.x + width / 2, center.y + height / 2};

  vector_t *p4 = malloc(sizeof(vector_t));
  *p4 = (vector_t){center.x - width / 2, center.y + height / 2};

  list_add(points, p1);
  list_add(points, p2);
  list_add(points, p3);
  list_add(points, p4);

  return points;
}




void add_ship(state_t *state, vector_t pos, size_t team) {
  list_t *ship_shape = make_ship(pos, team, (vector_t){0, 0});
  body_t *ship_body = body_init_with_info(ship_shape, SHIP_MASS, PLAYER_COLORS[team],
                                          entity_info_init(SHIP), free);
  scene_add_body(state->scene, ship_body);
}

void add_bounds(state_t *state) {
  list_t *wall1_shape =
      make_rectangle((vector_t){MAX.x, MAX.y / 2}, WALL_DIM, MAX.y);
  body_t *wall1 = body_init_with_info(wall1_shape, INFINITY, white,
                                      entity_info_init(WALL), free);
  list_t *wall2_shape =
      make_rectangle((vector_t){0, MAX.y / 2}, WALL_DIM, MAX.y);
  body_t *wall2 = body_init_with_info(wall2_shape, INFINITY, white,
                                      entity_info_init(WALL), free);
  list_t *ceiling_shape =
      make_rectangle((vector_t){MAX.x / 2, MAX.y}, MAX.x, WALL_DIM);
  body_t *ceiling = body_init_with_info(ceiling_shape, INFINITY, white,
                                        entity_info_init(WALL), free);
  list_t *ground_shape =
      make_rectangle((vector_t){MAX.x / 2, 0}, MAX.x, WALL_DIM);
  body_t *ground = body_init_with_info(ground_shape, INFINITY, white,
                                       entity_info_init(WALL), free);
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
void init_map(state_t *state, map_t map){
  add_ship(state, map.start_pos[0], 0);
  add_ship(state, map.start_pos[1], 1);


  add_bounds(state);
  for(size_t i = 0; i < map.num_blocks; i++){
    list_t *block_shape = make_rectangle(map.block_locations[i], map.block_sizes[i].x, map.block_sizes[i].y);
    body_t *block = body_init_with_info(block_shape, INFINITY, white,
                                      entity_info_init(WALL), free);
    scene_add_body(state->scene, block);
  }
}

void on_key(char key, key_event_type_t type, double held_time, state_t *state) {
  /*TODO: edit with Space Wars implementation*/
  body_t *froggy = scene_get_body(state->scene, 0);
  vector_t translation = (vector_t){0, 0};
  if (type == KEY_PRESSED && type != KEY_RELEASED) {
    switch (key) {
    case P1_TURN:
      translation.x = -H_STEP;
      break;
    case RIGHT_ARROW:
      translation.x = H_STEP;
      break;
    case UP_ARROW:
      translation.y = V_STEP;
      break;
    case DOWN_ARROW:
      if (body_get_centroid(froggy).y > START_POS.y) {
        translation.y = -V_STEP;
      }
      break;
    }
    vector_t new_centroid = vec_add(body_get_centroid(froggy), translation);
    body_set_centroid(froggy, new_centroid);
  }
}

void toggle_play(state_t *state) {
  state->mode = GAME;
}

void handle_buttons(state_t *state, double x, double y) {
  size_t n_assets = list_size(state->assets);
  for (size_t i = 0; i < n_assets; i++) {
    asset_t *asset = list_get(state->assets, i);
    if (asset_get_type(asset) == ASSET_BUTTON) {
      asset_on_button_click(asset, state, x, y);
    }
  }
}

void on_click(state_t *state, double x, double y) {
  switch (state->mode) {
    case HOME:
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
 * Initializes and stores the button assets in the state.
 */
void create_buttons(state_t *state) {
  size_t n_buttons = sizeof(button_templates) / sizeof(button_templates[0]);
  for (size_t i = 0; i < n_buttons; i++) {
    button_info_t info = button_templates[i];
    asset_t *button = create_button_from_info(state, info);
    list_add(state->assets, button);
  }
}

void home_init(state_t *state) {
  create_buttons(state);
}

state_t *emscripten_init() {
  asset_cache_init();
  sdl_init(MIN, MAX);
  srand(time(NULL));
  state_t *state = malloc(sizeof(state_t));
  state->mode = HOME;
  state->P1_score = 0;
  state->P2_score = 0;
  state->assets = list_init(INITIAL_CAPACITY, asset_)
  state->map = maps[0];
  state->scene = scene_init();
  assert(state->scene);

  init_map(state, state->map);
  state->ship1 = scene_get_body(state->scene, 0);
  state->ship2 = scene_get_body(state->scene, 1);


  // BACKGROUND
  SDL_Rect background_bbox = (SDL_Rect){
      .x = MIN.x, .y = MIN.y, .w = MAX.x - MIN.x, .h = MAX.y - MIN.y};
  asset_t *background_asset =
      asset_make_image(BACKGROUND_PATH, background_bbox);
  list_add(state->assets, background_asset);

  sdl_on_key((key_handler_t)on_key);
  sdl_on_click((click_handler_t)on_click);
  return state;
}

void update_score(state_t *state) {
  bool p1 = false;
  bool p2 = false;
  size_t n_bodies = scene_bodies(state->scene);

  for (size_t i = 0; i < n_bodies; i++) {
    void *info = body_get_info(scene_get_body(state->scene, i));
    if (info == P1_SHIP || info == P1_PILOT) {
      p1 = true;
    } else if (info == P2_SHIP || info == P2_PILOT) {
      p2 = true;
    }
  }

  if (!(p1 && p2)) {
    if (p1) {
      state->P1_score++;
    } else if (p2) {
      state->P2_score++;
    }
  }
}

void render_scene(state_t *state, void *aux) {
  switch (state->mode) {
    case HOME:

  }  
}

bool emscripten_main(state_t *state) {
  double dt = time_since_last_tick();

  switch (state->mode) {
    case HOME: {

    }
    case GAME: {

    }
  }
  scene_tick(state->scene, dt);

  update_score(state);
  if (state->P1_score > WIN_SCORE || state->P2_score > WIN_SCORE) { return true; }

  render_scene(state->scene, NULL);
  return false;
}

void emscripten_free(state_t *state) {
  list_free(state->assets);
  scene_free(state->scene);
  asset_cache_destroy();
  free(state);
}