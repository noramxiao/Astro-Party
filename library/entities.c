#include "vector.h"
#include "list.h"
#include "body.h"
#include "color.h"
#include "shapes.h"
#include "entities.h"

// color constants
const rgb_color_t RED = (rgb_color_t) {.r = 255, .g = 0, .b = 0}; // p1 color
const rgb_color_t BLUE = (rgb_color_t) {.r = 0, .g = 0, .b = 255}; // p2 color

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

typedef struct entity_info {
	entity_type_t type;
} entity_info_t;

entity_info_t *make_type_info(entity_type_t type) {
  entity_info *info = malloc(sizeof(entity_info_t));
  info->type = type;
  return info;
}

entity_type_t get_type(body_t *body) {
  return *(entity_type_t)(body_get_info(body)->type);
}

body_t *make_ship(vector_t centroid, rgb_color_t color, vector_t init_velocity) {
	list_t *ship = make_triangle(centroid, SHIP_BASE, SHIP_HEIGHT, color, init_velocity);
	body_t *ret = body_init_with_info(ship, SHIP_MASS, )
	return ship;
}

body_t *make_pilot(vector_t centroid, rgb_color_t color, vector_t init_velocity) {
	list_t *pilot = make_rectangle(centroid, PILOT_RECT_DIMS, )
}