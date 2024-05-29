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

const vector_t MIN = {0, 0};
const vector_t MAX = {1000, 500};

const double SHIP_MASS = 10.0;

const double WALL_DIM = 1;

const double USER_HEIGHT = 25;
const vector_t USER_INIT_POS = {500, 25};



rgb_color_t white = (rgb_color_t){1, 1, 1};

enum Mode {
  HOME,
  GAME,
  POST_GAME
};



struct state {
  enum Mode mode; // Keeps track of what page game is on
  size_t P1_score;
  size_t P2_score;
  


  scene_t *scene;
};

void on_key(char key, key_event_type_t type, double held_time, state_t *state) {
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

double rand_double(double low, double high) {
  return (high - low) * rand() / RAND_MAX + low;
}

state_t *emscripten_init() {
  asset_cache_init();
  sdl_init(MIN, MAX);
  state_t *state = malloc(sizeof(state_t));
  state->points = 0;
  srand(time(NULL));
  state->scene = scene_init();
  state->body_assets = list_init(2, (free_func_t)asset_destroy);

  body_t *froggy = make_frog(OUTER_RADIUS, INNER_RADIUS, VEC_ZERO);
  body_set_centroid(froggy, RESET_POS);

  scene_add_body(state->scene, froggy);

  SDL_Rect background_bbox = (SDL_Rect){
      .x = MIN.x, .y = MIN.y, .w = MAX.x - MIN.x, .h = MAX.y - MIN.y};
  asset_t *background_asset =
      asset_make_image(BACKGROUND_PATH, background_bbox);
  list_add(state->body_assets, background_asset);

  asset_t *frog_asset = asset_make_image_with_body(FROGGER_PATH, froggy);
  list_add(state->body_assets, frog_asset);

  for (size_t r = 3; r < ROWS + 3; r++) {
    double cx = 0;
    double cy = r * V_STEP + body_get_centroid(froggy).y;
    double multiplier = 0;
    if (r % 2 == 0) {
      multiplier = 1;
    } else {
      multiplier = -1;
    }
    if ((double)rand() / RAND_MAX < VEL_MULT_PROB) {
      multiplier *= EXTRA_VEL_MULT;
    }
    while (cx < MAX.x) {
      double w = rand_double(OBS_WIDTHS.x, OBS_WIDTHS.y);
      body_t *obstacle = make_obstacle(w, OBSTACLE_HEIGHT, (vector_t){cx, cy});
      cx += w + rand_double(OBS_SPACING.x, OBS_SPACING.y);

      body_set_velocity(obstacle, vec_multiply(multiplier, BASE_OBJ_VEL));
      scene_add_body(state->scene, obstacle);

      create_collision(state->scene, froggy, obstacle, reset_user_handler, NULL,
                       0);

      asset_t *log_asset = asset_make_image_with_body(LOG_PATH, obstacle);
      list_add(state->body_assets, log_asset);
    }
  }
  sdl_on_key((key_handler_t)on_key);
  return state;
}

bool emscripten_main(state_t *state) {
  double dt = time_since_last_tick();
  player_wrap_edges(state);
  for (int i = 1; i < scene_bodies(state->scene); i++) {
    wrap_edges(scene_get_body(state->scene, i));
  }
  sdl_clear();
  for (size_t i = 0; i < list_size(state->body_assets); i++) {
    asset_render(list_get(state->body_assets, i));
  }
  sdl_show();

  scene_tick(state->scene, dt);
  return false;
}

void emscripten_free(state_t *state) {
  list_free(state->body_assets);
  scene_free(state->scene);
  asset_cache_destroy();
  free(state);
}