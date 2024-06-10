#include "sdl_wrapper.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_mixer.h>
#include <assert.h>
#include <float.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

const char WINDOW_TITLE[] = "CS 3";
const int WINDOW_WIDTH = 1000;
const int WINDOW_HEIGHT = 500;
const double MS_PER_S = 1e3;

/**
 * The coordinate at the center of the screen.
 */
vector_t center;
/**
 * The coordinate difference from the center to the top right corner.
 */
vector_t max_diff;
/**
 * The SDL window where the scene is rendered.
 */
SDL_Window *window;
/**
 * The renderer used to draw the scene.
 */
SDL_Renderer *renderer;
/**
 * The keypress handler, or NULL if none has been configured.
 */
key_handler_t key_handler = NULL;
click_handler_t click_handler = NULL;
/**
 * SDL's timestamp when a key was last pressed or released.
 * Used to mesasure how long a key has been held.
 */
uint32_t key_start_timestamp;
/**
 * The value of clock() when time_since_last_tick() was last called.
 * Initially 0.
 */
clock_t last_clock = 0;

/**
 * The last time each keyidx was pressed
*/
double last_presses[] = {0, 0, 0, 0};
/**
 * The last time each keyidx was released
*/
double last_releases[] = {0, 0, 0, 0};

const player_key_t PLAYER_KEYS[] = {P1_TURN, P1_SHOOT, P2_TURN, P2_SHOOT};
const size_t NUM_PLAYER_KEYS = sizeof(PLAYER_KEYS) / sizeof(player_key_t);

Uint16 MUSIC_FORMAT = AUDIO_S16SYS;

/** Computes the center of the window in pixel coordinates */
vector_t get_window_center(void) {
  int *width = malloc(sizeof(*width)), *height = malloc(sizeof(*height));
  assert(width != NULL);
  assert(height != NULL);
  SDL_GetWindowSize(window, width, height);
  vector_t dimensions = {.x = *width, .y = *height};
  free(width);
  free(height);
  return vec_multiply(0.5, dimensions);
}

/**
 * Computes the scaling factor between scene coordinates and pixel coordinates.
 * The scene is scaled by the same factor in the x and y dimensions,
 * chosen to maximize the size of the scene while keeping it in the window.
 */
double get_scene_scale(vector_t window_center) {
  // Scale scene so it fits entirely in the window
  double x_scale = window_center.x / max_diff.x,
         y_scale = window_center.y / max_diff.y;
  return x_scale < y_scale ? x_scale : y_scale;
}

/** Maps a scene coordinate to a window coordinate */
vector_t get_window_position(vector_t scene_pos, vector_t window_center) {
  // Scale scene coordinates by the scaling factor
  // and map the center of the scene to the center of the window
  vector_t scene_center_offset = vec_subtract(scene_pos, center);
  double scale = get_scene_scale(window_center);
  vector_t pixel_center_offset = vec_multiply(scale, scene_center_offset);
  vector_t pixel = {.x = round(window_center.x + pixel_center_offset.x),
                    // Flip y axis since positive y is down on the screen
                    .y = round(window_center.y - pixel_center_offset.y)};
  return pixel;
}

size_t get_keyidx(player_key_t key) {
  for (size_t i = 0; i < NUM_PLAYER_KEYS; i++) {
    if (key == PLAYER_KEYS[i]) {
      return i;
    }
  }
  return -1;
}

void sdl_init(vector_t min, vector_t max) {
  // Check parameters
  assert(min.x < max.x);
  assert(min.y < max.y);

  center = vec_multiply(0.5, vec_add(min, max));
  max_diff = vec_subtract(max, center);
  SDL_Init(SDL_INIT_EVERYTHING);
  window = SDL_CreateWindow(WINDOW_TITLE, SDL_WINDOWPOS_CENTERED,
                            SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT,
                            SDL_WINDOW_RESIZABLE);
  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
  if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 2048)) {
    fprintf(stderr, "Unable to open audio: %s.\n", Mix_GetError());
    exit(1);
  }
  TTF_Init();
}

bool sdl_is_done(void *state) {
  if (key_handler != NULL)
    key_handler(state);

  SDL_Event *event = malloc(sizeof(*event));
  assert(event);

  while (SDL_PollEvent(event)) {
    switch (event->type) {
      case SDL_QUIT:
        free(event);
        return true;
      case SDL_MOUSEBUTTONDOWN:
        if (click_handler == NULL) {
          break;
        }
        click_handler(state, event->motion.x, event->motion.y);
        // break;
      }
  }
  free(event);

  return false;
}

void sdl_clear(void) {
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
  SDL_RenderClear(renderer);
}

SDL_Texture *sdl_load_image(const char *img_path) {
  SDL_Texture *img = IMG_LoadTexture(renderer, img_path);

  return img;
}

TTF_Font *sdl_load_font(const char *font_path, int ptsize) {
  TTF_Font *font = TTF_OpenFont(font_path, ptsize);
  return font;
}

SDL_Color *sdl_load_color(Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
  SDL_Color *col = malloc(sizeof(SDL_Color));
  col->r = r;
  col->g = g;
  col->b = b;
  col->a = a;
  return col;
}

void sdl_draw_image(SDL_Texture *texture, vector_t pos, vector_t size) {
  SDL_Rect rect;
  rect.x = pos.x;
  rect.y = pos.y;
  rect.w = size.x;
  rect.h = size.y;
  SDL_RenderCopy(renderer, texture, NULL, &rect);
}

void sdl_draw_text(TTF_Font *font, const char *string, SDL_Color *forecol,
                   vector_t pos) {
  SDL_Rect rect;
  rect.x = pos.x;
  rect.y = pos.y;
  TTF_SizeText(font, string, &rect.w, &rect.h);
  SDL_Surface *text = TTF_RenderText_Solid(font, string, *forecol);
  SDL_Texture *caption = SDL_CreateTextureFromSurface(renderer, text);
  SDL_RenderCopy(renderer, caption, NULL, &rect);
}

void sdl_draw_polygon_cam(polygon_t *poly, rgb_color_t color, vector_t cam_center, vector_t cam_size) {
  list_t *points = polygon_get_points(poly);
  // Check parameters
  size_t n = list_size(points);
  assert(n >= 3);

  vector_t window_center = get_window_center();

  // Convert each vertex to a point on screen
  int16_t *x_points = malloc(sizeof(*x_points) * n),
          *y_points = malloc(sizeof(*y_points) * n);
  assert(x_points != NULL);
  assert(y_points != NULL);
  for (size_t i = 0; i < n; i++) {
    vector_t *vertex = list_get(points, i);
    vector_t pixel = get_window_position(*vertex, window_center);
    x_points[i] = (pixel.x - cam_center.x + cam_size.x/2)*WINDOW_WIDTH/cam_size.x;
    y_points[i] = (pixel.y + cam_center.y - WINDOW_HEIGHT + cam_size.y/2)*WINDOW_HEIGHT/cam_size.y;
  }

  // Draw polygon with the given color
  filledPolygonRGBA(renderer, x_points, y_points, n, color.r * 255,
                    color.g * 255, color.b * 255, 255);
  free(x_points);
  free(y_points);
}
void sdl_draw_polygon(polygon_t *poly, rgb_color_t color) {
  list_t *points = polygon_get_points(poly);
  // Check parameters
  size_t n = list_size(points);
  assert(n >= 3);

  vector_t window_center = get_window_center();

  // Convert each vertex to a point on screen
  int16_t *x_points = malloc(sizeof(*x_points) * n),
          *y_points = malloc(sizeof(*y_points) * n);
  assert(x_points != NULL);
  assert(y_points != NULL);
  for (size_t i = 0; i < n; i++) {
    vector_t *vertex = list_get(points, i);
    vector_t pixel = get_window_position(*vertex, window_center);
    x_points[i] = pixel.x;
    y_points[i] = pixel.y;
  }

  // Draw polygon with the given color
  filledPolygonRGBA(renderer, x_points, y_points, n, color.r * 255,
                    color.g * 255, color.b * 255, 255);
  free(x_points);
  free(y_points);
}

void sdl_show(void) {
  // Draw boundary lines
  vector_t window_center = get_window_center();
  vector_t max = vec_add(center, max_diff),
           min = vec_subtract(center, max_diff);
  vector_t max_pixel = get_window_position(max, window_center),
           min_pixel = get_window_position(min, window_center);
  SDL_Rect *boundary = malloc(sizeof(*boundary));
  boundary->x = min_pixel.x;
  boundary->y = max_pixel.y;
  boundary->w = max_pixel.x - min_pixel.x;
  boundary->h = min_pixel.y - max_pixel.y;
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderDrawRect(renderer, boundary);
  free(boundary);

  SDL_RenderPresent(renderer);
}

void sdl_render_scene(scene_t *scene, void *aux) {
  size_t body_count = scene_bodies(scene);
  for (size_t i = 0; i < body_count; i++) {
    body_t *body = scene_get_body(scene, i);
    list_t *shape = body_get_shape(body);
    polygon_t *poly = polygon_init(shape, (vector_t){0, 0}, 0, 0, 0, 0);
    sdl_draw_polygon(poly, *body_get_color(body));
    list_free(shape);
  }
  if (aux != NULL) {
    body_t *body = aux;
    sdl_draw_polygon(body_get_polygon(body), *body_get_color(body));
  }
}

void sdl_render_scene_cam(scene_t *scene, void *aux, vector_t cam_center, vector_t cam_size) {
  size_t body_count = scene_bodies(scene);
  for (size_t i = 0; i < body_count; i++) {
    body_t *body = scene_get_body(scene, i);
    list_t *shape = body_get_shape(body);
    polygon_t *poly = polygon_init(shape, (vector_t){0, 0}, 0, 0, 0, 0);
    sdl_draw_polygon_cam(poly, *body_get_color(body), cam_center, cam_size);
    list_free(shape);
  }
  if (aux != NULL) {
    body_t *body = aux;
    sdl_draw_polygon(body_get_polygon(body), *body_get_color(body));
  }
}

void sdl_on_key(key_handler_t handler) { key_handler = handler; }

Uint8 *sdl_get_keystate() {
  const Uint8 *sdl_key_state = SDL_GetKeyboardState(NULL);
  Uint8 *key_state = malloc(sizeof(bool) * 4);
  key_state[P1_TURN] = sdl_key_state[SDL_SCANCODE_W];
  key_state[P1_SHOOT] = sdl_key_state[SDL_SCANCODE_Q];
  key_state[P2_TURN] = sdl_key_state[SDL_SCANCODE_M];
  key_state[P2_SHOOT] = sdl_key_state[SDL_SCANCODE_N];
  return key_state;
}

void sdl_on_click(click_handler_t handler) { click_handler = handler; }

void set_last_press(player_key_t key) {
  size_t idx = get_keyidx(key);
  last_presses[idx] = clock();
}

double get_last_press(player_key_t key) {
  size_t idx = get_keyidx(key);
  if (!last_presses[idx]) {
    set_last_press(key);
  }
  return last_presses[idx];
}

void set_last_release(player_key_t key) {
  size_t idx = get_keyidx(key);
  last_releases[idx] = clock();
}

double get_last_release(player_key_t key) {
  size_t idx = get_keyidx(key);
  if (!last_releases[idx]) {
    set_last_release(key);
  }
  return last_releases[idx];
}

double get_time_held(player_key_t key) {
  double now = clock();
  double time_since_last_press = now - get_last_press(key);
  double time_since_last_release = now - get_last_release(key);
  if (time_since_last_press < time_since_last_release) {
    return time_since_last_release / CLOCKS_PER_SEC;
  }
  return 0;
}

Mix_Chunk *sdl_load_sound(const char* sound_path) {
  return Mix_LoadWAV(sound_path);
}

Mix_Music *sdl_load_music(const char* sound_path) {
  Mix_Music *mus = Mix_LoadMUS(sound_path);
  if( mus == NULL )
  {
    printf( "Failed to load beat music! SDL_mixer Error: %s\n", Mix_GetError() );
    exit(1);
  }
  return mus;
}

void sdl_play_sound(Mix_Chunk *sound) {
  Mix_PlayChannel(-1, sound, 0);
}

void sdl_play_music(Mix_Music *music) {
  Mix_PlayMusic(music, -1 );
}

SDL_Rect get_bounding_box(body_t *body) {
  double min_x = DBL_MAX;
  double max_x = -DBL_MAX;
  double min_y = DBL_MAX;
  double max_y = -DBL_MAX;
  list_t *shape = body_get_shape(body);
  for (int i = 0; i < list_size(shape); i++) {
    vector_t point = *(vector_t *)list_get(shape, i);
    min_x = fmin(min_x, point.x);
    max_x = fmax(max_x, point.x);
    min_y = fmin(min_y, point.y);
    max_y = fmax(max_y, point.y);
  }
  list_free(shape);

  vector_t top_left = (vector_t){.x = min_x, .y = max_y};
  vector_t window_center = get_window_center();
  vector_t pixel = get_window_position(top_left, window_center);
  SDL_Rect ret = (SDL_Rect){
      .x = pixel.x, .y = pixel.y, .w = max_x - min_x, .h = max_y - min_y};
  return ret;
}

double time_since_last_tick(void) {
  clock_t now = clock();
  double difference = last_clock
                          ? (double)(now - last_clock) / CLOCKS_PER_SEC
                          : 0.0; // return 0 the first time this is called
  last_clock = now;
  return difference;
}

double get_window_width(){
  return WINDOW_WIDTH;
}

double get_window_height(){
  return WINDOW_HEIGHT;
}