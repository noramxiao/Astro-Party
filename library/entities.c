#include "vector.h"
#include "list.h"
#include "body.h"
#include "color.h"
#include "shapes.h"
#include "entities.h"
#include <assert.h>
#include <stdlib.h>

// color constants
const rgb_color_t RED = (rgb_color_t) {.r = 255, .g = 0, .b = 0}; 
const rgb_color_t BLUE = (rgb_color_t) {.r = 0, .g = 0, .b = 255}; 
const rgb_color_t PLAYER_COLORS[] = {RED, BLUE};
const rgb_color_t GRAY = (rgb_color_t) {.r = 150, .g = 150, .b = 150};

// ship constants
const double SHIP_MASS = 10;
const double SHIP_BASE = 20;
const double SHIP_HEIGHT = 30;

// pilot constants
const double PILOT_MASS = 5;
const vector_t PILOT_RECT_DIMS = (vector_t) {.x = 10, .y = 20};
const double PILOT_CIRC_RAD = 15;

// asteroid constants
const double ASTEROID_MASS_DENSITY = 1;

entity_info_t *entity_info_init(entity_type_t type, size_t player_idx) {
	entity_info_t *ret = malloc(sizeof(entity_info_t));
	assert(ret);
	ret->type = type;
	ret->player = player_idx;
	return ret;
}

void entity_info_free(entity_info_t *info) {
	free(info);
}

entity_type_t get_type(body_t *body) {
	entity_info_t *info = body_get_info(body);
  return info->type;
}

body_t *make_ship(vector_t centroid, size_t player_idx, vector_t init_velocity) {
	list_t *ship = make_iso_triangle(centroid, SHIP_BASE, SHIP_HEIGHT);
	entity_info_t *ship_info = entity_info_init(SHIP, player_idx);
	rgb_color_t color = PLAYER_COLORS[player_idx];
	body_t *ret = body_init_with_info(ship, SHIP_MASS, color, ship_info, (free_func_t)entity_info_free);
	body_set_centroid(ret, centroid);
	return ret;
}

body_t *make_pilot(vector_t centroid, size_t player_idx, vector_t init_velocity) {
	list_t *pilot = make_rectangle(centroid, PILOT_RECT_DIMS.x, PILOT_RECT_DIMS.y);
	entity_info_t *pilot_info = entity_info_init(PILOT, player_idx);
	rgb_color_t color = PLAYER_COLORS[player_idx];
	body_t *ret = body_init_with_info(pilot, PILOT_MASS, color, pilot_info, (free_func_t)entity_info_free);
	body_set_centroid(ret, centroid);
	return ret;
}

body_t *make_asteroid(vector_t centroid, double radius, vector_t init_velocity) {
	list_t *asteroid = make_circle(centroid, radius);
	entity_info_t *asteroid_info = entity_info_init(ASTEROID, 100);
	double mass = radius * radius * ASTEROID_MASS_DENSITY;
	body_t *ret = body_init_with_info(asteroid, mass, GRAY, asteroid_info, (free_func_t)entity_info_free);
	body_set_centroid(ret, centroid);
	return ret;
}