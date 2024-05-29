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
const size_t INITIAL_CAPACITY = 5;
const size_t WIN_SCORE = 5;
const size_t N_PLAYERS = 2;

const double SHIP_MASS = 10.0;

enum mode {
  HOME,
  GAME,
  POST_GAME
};

struct state {
  enum mode mode; // Keeps track of what page game is on
  size_t P1_score;
  size_t P2_score;
  
  list_t *assets;
  scene_t *scene;
};

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

double rand_double(double low, double high) {
  return (high - low) * rand() / RAND_MAX + low;
}

state_t *emscripten_init() {
  asset_cache_init();
  sdl_init(MIN, MAX);
  state_t *state = malloc(sizeof(state_t));
  state->mode = HOME;
  state->P1_score = 0;
  state->P2_score = 0;
  state->scene = scene_init();
  srand(time(NULL));

  // BACKGROUND
  SDL_Rect background_bbox = (SDL_Rect){
      .x = MIN.x, .y = MIN.y, .w = MAX.x - MIN.x, .h = MAX.y - MIN.y};
  asset_t *background_asset =
      asset_make_image(BACKGROUND_PATH, background_bbox);
  list_add(state->body_assets, background_asset);

  sdl_on_key((key_handler_t)on_key);
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
    } else {
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
  scene_tick(state->scene, dt);

  update_score(state);
  if (state->P1_score > WIN_SCORE || state->P2_score > WIN_SCORE) { return true; }

  render_scene(state->scene, NULL);
  return false;
}

void emscripten_free(state_t *state) {
  list_free(state->body_assets);
  scene_free(state->scene);
  asset_cache_destroy();
  free(state);
}