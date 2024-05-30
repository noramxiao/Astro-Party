#ifndef __ENTITIES_H__
#define __ENTITIES_H__

#include "vector.h"
#include "list.h"
#include "body.h"
#include "color.h"

extern const rgb_color_t PLAYER_COLORS[];

typedef enum {
	SHIP,
	PILOT,
	BULLET,
	ASTEROID,
	WALL,
	BACKGROUND
} entity_type_t;

typedef struct entity_info {
	entity_type_t type;
	size_t team;
} entity_info_t;

entity_type_t get_type(body_t *body);

size_t get_team(body_t* body);

entity_info_t *entity_info_init(entity_type_t type, size_t player_idx);

/** 
 * Creates a body for a spaceship. Will just be an isosceles triangle for now.
 * 
 * @param centroid centroid of the spaceship
 * @param player_idx which player the ship belongs to
 * @param init_velocity initial velocity of the spaceship
 * @return a pointer to the ship body
*/
body_t *make_ship(vector_t centroid, size_t player_idx, vector_t velocity, double angle);

/** 
 * Creates a body for a pilot. Will just be a rectangle for now.
 * 
 * @param centroid centroid of the pilot
 * @param player_idx which player the pilot belongs to
 * @param init_velocity initial velocity of the pilot
 * @return a pointer to the pilot body
*/
body_t *make_pilot(vector_t centroid, size_t player_idx, vector_t velocity, double angle);

/** 
 * Creates a body of a circle shape for a bullet.
 * 
 * @param centroid centroid of the bullet
 * @param velocity velocity of the bullet
 * @return a pointer to the bullet body
*/
body_t *make_bullet(vector_t ship_centroid, double ship_angle, double init_speed);

/** 
 * Creates a body for an asteroid. Will just be a grey circle for now.
 * 
 * @param centroid centroid of the asteroid
 * @param radius radius of asteroid
 * @param init_velocity initial velocity of asteroid
 * @return a pointer to the asteroid body
*/
body_t *make_asteroid(vector_t centroid, double radius, vector_t init_velocity);

#endif