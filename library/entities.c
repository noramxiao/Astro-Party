#include "vector.h"
#include "list.h"
#include "body.h"
#include "color.h"
#include "shapes.h"
#include "entities.h"
#include <assert.h>
#include <stdlib.h>
#include <assert.h>

// color constants
const rgb_color_t RED = (rgb_color_t) {.r = 1, .g = 0, .b = 0}; 
const rgb_color_t BLUE = (rgb_color_t) {.r = 0, .g = 0, .b = 1}; 
const rgb_color_t PURPLE = (rgb_color_t) {.r = 0.6, .g = 0.1, .b = 1}; 
const rgb_color_t PLAYER_COLORS[] = {RED, BLUE};
const rgb_color_t GRAY = (rgb_color_t) {.r = 0.5, .g = 0.5, .b = 0.5};

entity_info_t *entity_info_init(entity_type_t type, size_t player_idx) {
	entity_info_t *ret = malloc(sizeof(entity_info_t));
	assert(ret);
	ret->type = type;
	ret->team = player_idx;
	return ret;
}

size_t get_team(body_t *body) {
	entity_info_t *info = body_get_info(body);
  return info->team;
}

void entity_info_free(entity_info_t *info) {
	free(info);
}

entity_type_t get_type(body_t *body) {
	entity_info_t *info = body_get_info(body);
  return info->type;
}

body_t *make_ship(vector_t centroid, size_t player_idx, vector_t velocity, 
									double angle, const double base, const double height, 
									const double mass) {
	list_t *ship = make_iso_triangle(centroid, base, height);
	entity_info_t *ship_info = entity_info_init(SHIP, player_idx);
	rgb_color_t color = PLAYER_COLORS[player_idx];
	body_t *ret = body_init_with_info(ship, mass, color, ship_info, (free_func_t) entity_info_free);
	body_set_centroid(ret, centroid);
	body_set_velocity(ret, velocity);
	body_set_rotation(ret, angle);
	return ret;
}

body_t *make_pilot(vector_t centroid, size_t player_idx, vector_t velocity, 
									 double angle, double mass, vector_t dims) {
	list_t *pilot = make_rectangle(centroid, dims.x, dims.y);
	entity_info_t *pilot_info = entity_info_init(PILOT, player_idx);
	rgb_color_t color = PLAYER_COLORS[player_idx];
	body_t *ret = body_init_with_info(pilot, mass, color, pilot_info, (free_func_t) entity_info_free);
	body_set_centroid(ret, centroid);
	body_set_velocity(ret, velocity);
	body_set_rotation(ret, angle);
	return ret;
}

body_t *make_bullet(vector_t ship_centroid, double ship_angle, double speed, 
										const double radius, const double mass, 
										const double ship_height) {
	list_t *bullet = make_circle(ship_centroid, radius);
	entity_info_t *bullet_info = entity_info_init(BULLET, 100);
	body_t *ret = body_init_with_info(bullet, mass, GRAY, bullet_info, (free_func_t) entity_info_free);

	double dist_from_centroid = ship_height * 2 / 3 + radius + 1;
	vector_t disp = vec_make(dist_from_centroid, ship_angle);
	vector_t bullet_centroid = vec_add(ship_centroid, disp);
	body_set_centroid(ret, bullet_centroid);
	
	vector_t velocity = vec_make(speed, ship_angle);
	body_set_velocity(ret, velocity);

	return ret;
}

body_t *make_asteroid(vector_t centroid, double radius, vector_t velocity, 
											const double density) {
	list_t *asteroid = make_circle(centroid, radius);
	entity_info_t *asteroid_info = entity_info_init(ASTEROID, 100);
	double mass = radius * radius * density;
	body_t *ret = body_init_with_info(asteroid, mass, GRAY, asteroid_info, (free_func_t)entity_info_free);
	body_set_centroid(ret, centroid);
	body_set_velocity(ret, velocity);
	return ret;
}

body_t *make_blackhole(vector_t centroid, double mass, const double radius) {
	list_t *blackhole = make_circle(centroid, radius);
	entity_info_t *blackhole_info = entity_info_init(BLACKHOLE, 100);
	body_t *ret = body_init_with_info(blackhole, mass, PURPLE, blackhole_info, (free_func_t)entity_info_free);
	body_set_centroid(ret, centroid);
	body_set_velocity(ret, (vector_t){0, 0});
	return ret;
}