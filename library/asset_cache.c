#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <assert.h>
#include <png.h>
#include <stdio.h>
#include <stdlib.h>

#include "asset_cache.h"
#include "list.h"
#include "sdl_wrapper.h"

static list_t *ASSET_CACHE;

const size_t FONT_SIZE = 18;
const size_t INITIAL_CAPACITY = 5;

typedef struct {
  asset_type_t type;
  const char *filepath;
  void *obj;
} entry_t;

static void asset_cache_free_entry(entry_t *entry) {
  switch (entry->type) {
  case ASSET_IMAGE: {
    SDL_DestroyTexture((SDL_Texture *)entry->obj);
    break;
  }
  case ASSET_FONT: {
    TTF_CloseFont((TTF_Font *)entry->obj);
    break;
  }
  case ASSET_BUTTON: {
    free((asset_t *)entry->obj);
    break;
  }
  default: {
    assert(false);
  }
  }
  free((void *)entry->filepath);
  free(entry);
}

void asset_cache_init() {
  ASSET_CACHE =
      list_init(INITIAL_CAPACITY, (free_func_t)asset_cache_free_entry);
}

void asset_cache_destroy() { list_free(ASSET_CACHE); }

/**
 * Helper method to check if entry already corresponds to `filepath`.
 *
 * @param ty the type of asset
 * @param filepath the file path
 *
 * @return entry_t object of corresponding entry if it exists, NULL otherwise
 */
static entry_t *asset_cache_filepath_exists(asset_type_t ty,
                                            const char *filepath) {
  size_t length = list_size(ASSET_CACHE);

  for (size_t i = 0; i < length; i++) {
    entry_t *e = (entry_t *)list_get(ASSET_CACHE, i);
    if (e->filepath == NULL) {
      continue;
    }
    if (strcmp(e->filepath, filepath) == 0 && e->type == ty) {
      return e;
    }
  }
  return NULL;
}

void *asset_cache_obj_get_or_create(asset_type_t ty, const char *filepath) {
  entry_t *entry = asset_cache_filepath_exists(ty, filepath);

  if (!entry) {
    entry = malloc(sizeof(entry_t));
    assert(entry);
    entry->type = ty;
    entry->filepath = strdup(filepath);
    assert(entry->filepath);

    switch (ty) {
    case ASSET_IMAGE: {
      SDL_Texture *texture = sdl_load_image(filepath);
      assert(texture);
      entry->obj = texture;
      break;
    }
    case ASSET_FONT: {
      TTF_Font *font = sdl_load_font(filepath, FONT_SIZE);
      assert(font);
      entry->obj = font;
      break;
    }
    default: {
      assert(false);
    }
    }

    list_add(ASSET_CACHE, entry);
  }

  return entry->obj;
}

void asset_cache_register_button(asset_t *button) {
  entry_t *entry = malloc(sizeof(entry_t));
  assert(entry);
  entry->type = ASSET_BUTTON;
  entry->filepath = NULL;
  entry->obj = button;
  list_add(ASSET_CACHE, entry);
}

void asset_cache_handle_buttons(state_t *state, double x, double y) {

  size_t length = list_size(ASSET_CACHE);
  for (size_t i = 0; i < length; i++) {
    entry_t *e = (entry_t *)list_get(ASSET_CACHE, i);
    if (e->type == ASSET_BUTTON) {
      asset_on_button_click((asset_t *)e->obj, state, x, y);
    }
  }
}
