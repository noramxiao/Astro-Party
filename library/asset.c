#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <assert.h>

#include "asset.h"
#include "asset_cache.h"
#include "color.h"
#include "sdl_wrapper.h"

typedef struct asset {
  asset_type_t type;
  SDL_Rect bounding_box;
} asset_t;

typedef struct text_asset {
  asset_t base;
  TTF_Font *font;
  const char *text;
  rgb_color_t color;
} text_asset_t;

typedef struct image_asset {
  asset_t base;
  SDL_Texture *texture;
  body_t *body;
} image_asset_t;

typedef struct button_asset {
  asset_t base;
  image_asset_t *image_asset;
  text_asset_t *text_asset;
  button_handler_t handler;
  bool is_rendered;
} button_asset_t;

/**
 * Allocates memory for an asset with the given parameters.
 *
 * @param ty the type of the asset
 * @param bounding_box the bounding box containing the location and dimensions
 * of the asset when it is rendered
 * @return a pointer to the newly allocated asset
 */
static asset_t *asset_init(asset_type_t ty, SDL_Rect bounding_box) {
  asset_t *new;
  switch (ty) {
  case ASSET_IMAGE: {
    new = malloc(sizeof(image_asset_t));
    break;
  }
  case ASSET_FONT: {
    new = malloc(sizeof(text_asset_t));
    break;
  }
  case ASSET_BUTTON: {
    new = malloc(sizeof(button_asset_t));
    break;
  }
  default: {
    assert(false && "Unknown asset type");
  }
  }
  assert(new);
  new->type = ty;
  new->bounding_box = bounding_box;
  return new;
}

asset_type_t asset_get_type(asset_t *asset) { return asset->type; }

asset_t *asset_make_image(const char *filepath, SDL_Rect bounding_box) {
  SDL_Texture *texture =
      (SDL_Texture *)asset_cache_obj_get_or_create(ASSET_IMAGE, filepath);

  image_asset_t *image_asset =
      (image_asset_t *)asset_init(ASSET_IMAGE, bounding_box);
  assert(image_asset);
  image_asset->texture = texture;
  image_asset->body = NULL;

  return (asset_t *)image_asset;
}

asset_t *asset_make_image_with_body(const char *filepath, body_t *body) {
  SDL_Rect bbox = get_bounding_box(body);
  image_asset_t *ret = (image_asset_t *)asset_make_image(filepath, bbox);
  ret->body = body;
  assert(ret->body);
  return (asset_t *)ret;
}

asset_t *asset_make_text(const char *filepath, SDL_Rect bounding_box,
                         const char *text, rgb_color_t color) {
  TTF_Font *font =
      (TTF_Font *)asset_cache_obj_get_or_create(ASSET_FONT, filepath);

  text_asset_t *text_asset =
      (text_asset_t *)asset_init(ASSET_FONT, bounding_box);
  assert(text_asset);
  text_asset->font = font;
  text_asset->text = text;
  text_asset->color = color;

  return (asset_t *)text_asset;
}

asset_t *asset_make_button(SDL_Rect bounding_box, asset_t *image_asset,
                           asset_t *text_asset, button_handler_t handler) {
  button_asset_t *button =
      (button_asset_t *)asset_init(ASSET_BUTTON, bounding_box);
  assert(button);
  button->image_asset = (image_asset_t *)image_asset;
  button->text_asset = (text_asset_t *)text_asset;
  button->handler = handler;
  button->is_rendered = false;

  return (asset_t *)button;
}

void asset_on_button_click(asset_t *button, state_t *state, double x,
                           double y) {
  vector_t pos = (vector_t){.x = (double)(button->bounding_box.x),
                            .y = (double)(button->bounding_box.y)};
  vector_t size = (vector_t){.x = (double)(button->bounding_box.w),
                             .y = (double)(button->bounding_box.h)};

  if (x > pos.x && x < pos.x + size.x && y > pos.y && y < pos.y + size.y &&
      ((button_asset_t *)button)->is_rendered) {
    ((button_asset_t *)button)->handler(state);
  }
  ((button_asset_t *)button)->is_rendered = false;
}

void asset_render(asset_t *asset) {
  vector_t pos = (vector_t){.x = (double)(asset->bounding_box.x),
                            .y = (double)(asset->bounding_box.y)};
  vector_t size = (vector_t){.x = (double)(asset->bounding_box.w),
                             .y = (double)(asset->bounding_box.h)};

  switch (asset->type) {
  case ASSET_IMAGE: {
    image_asset_t *image_asset = (image_asset_t *)asset;
    if (image_asset->body) {
      SDL_Rect bbox = get_bounding_box(image_asset->body);
      pos = (vector_t){.x = (double)(bbox.x), .y = (double)(bbox.y)};
      size = (vector_t){.x = (double)(bbox.w), .y = (double)(bbox.h)};
    }
    sdl_draw_image(image_asset->texture, pos, size);
    break;
  }
  case ASSET_FONT: {
    text_asset_t *text_asset = (text_asset_t *)asset;
    SDL_Color *color = sdl_load_color(text_asset->color.r, text_asset->color.g,
                                      text_asset->color.b, 0);
    sdl_draw_text(text_asset->font, text_asset->text, color, pos);
    break;
  }
  case ASSET_BUTTON: {
    button_asset_t *button = (button_asset_t *)asset;
    if (button->image_asset != NULL) {
      asset_render((asset_t *)button->image_asset);
    }
    if (button->text_asset != NULL) {
      asset_render((asset_t *)button->text_asset);
    }
    button->is_rendered = true;
    break;
  }
  default: {
    assert(false);
  }
  }
}

void asset_render_cam(asset_t *asset, vector_t cam_center, vector_t cam_size) {
  vector_t pos = (vector_t){.x = (double)((asset->bounding_box.x - cam_center.x + cam_size.x/2)*get_window_width()/cam_size.x), 
      .y = (double)((asset->bounding_box.y + cam_center.y - get_window_height() + cam_size.y/2)*get_window_height()/cam_size.y)};
  vector_t size = (vector_t){.x = (double)(asset->bounding_box.w*get_window_width()/cam_size.x), .y = (double)(asset->bounding_box.h*get_window_height()/cam_size.y)};

  switch (asset->type) {
  case ASSET_IMAGE: {
    image_asset_t *image_asset = (image_asset_t *)asset;
    if (image_asset->body) {
      SDL_Rect bbox = get_bounding_box(image_asset->body);
      pos = (vector_t){.x = (double)((bbox.x - cam_center.x + cam_size.x/2)*get_window_width()/cam_size.x), 
      .y = (double)((bbox.y + cam_center.y - get_window_height() + cam_size.y/2)*get_window_height()/cam_size.y)};
      size = (vector_t){.x = (double)(bbox.w*get_window_width()/cam_size.x), .y = (double)(bbox.h*get_window_height()/cam_size.y)};
    }
    sdl_draw_image(image_asset->texture, pos, size);
    break;
  }
  case ASSET_FONT: {
    text_asset_t *text_asset = (text_asset_t *)asset;
    SDL_Color *color = sdl_load_color(text_asset->color.r, text_asset->color.g,
                                      text_asset->color.b, 0);
    sdl_draw_text(text_asset->font, text_asset->text, color, pos);
    break;
  }
  case ASSET_BUTTON: {
    button_asset_t *button = (button_asset_t *)asset;
    if (button->image_asset != NULL) {
      asset_render((asset_t *)button->image_asset);
    }
    if (button->text_asset != NULL) {
      asset_render((asset_t *)button->text_asset);
    }
    button->is_rendered = true;
    break;
  }
  default: {
    assert(false);
  }
  }
}

void asset_destroy(asset_t *asset) { free(asset); }