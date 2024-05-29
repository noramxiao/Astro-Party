#include "vector.h"
#include "list.h"
#include "body.h"
#include "color.h"
#include "shapes.h"
#include "entities.h"
#include <stdlib.h>

// color constants
const rgb_color_t RED = (rgb_color_t) {.r = 255, .g = 0, .b = 0}; // p1 color
const rgb_color_t BLUE = (rgb_color_t) {.r = 0, .g = 0, .b = 255}; // p2 color
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

entity_info_t *entity_info_init(entity_type_t type) {
	entity_info_t *ret = malloc(sizeof(entity_info_t));
	assert(ret);
	ret->type = type;
	return ret;
}

entity_info_free(entity_info_t *info) {
	free(info);
}

body_t *make_ship(vector_t centroid, rgb_color_t color, vector_t init_velocity) {
	list_t *ship = make_triangle(centroid, SHIP_BASE, SHIP_HEIGHT);
	entity_info_t *ship_info = entity_info_init(SHIP);
	body_t *ret = body_init_with_info(ship, SHIP_MASS, color, ship_info, entity_info_free);
	return ret;
}

body_t *make_pilot(vector_t centroid, rgb_color_t color, vector_t init_velocity) {
	list_t *pilot = make_rectangle(centroid, PILOT_RECT_DIMS);
	entity_info_t *pilot_info = entity_info_init(PILOT);
	body_t *ret = body_init_with_info(pilot, PILOT_MASS, color, pilot_info, entity_info_free);
	return ret;
}

body_t *make_asteroid(vector_t centroid, double radius, vector_t init_velocity);