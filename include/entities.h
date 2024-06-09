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
	BACKGROUND,
	BLACKHOLE
} entity_type_t;

typedef struct entity_info {
	entity_type_t type;
	size_t team;
} entity_info_t;

entity_type_t get_type(body_t *body);

size_t get_team(body_t* body);

entity_info_t *entity_info_init(entity_type_t type, size_t player_idx);

/** 
 * Creates a isosceles triangle shaped body for a spaceship.
 * 
 * @param centroid centroid of the spaceship
 * @param player_idx which player the ship belongs to
 * @param velocity initial velocity of the spaceship
 * @param angle initial angle of the spaceship
 * @param base base length of the spaceship
 * @param height height of the spaceship
 * @param mass mass of the spaceship
 * 
 * @return a pointer to the ship body
*/
body_t *make_ship(vector_t centroid, size_t player_idx, vector_t velocity, 
									double angle, const double base, const double height, 
									const double mass);

/** 
 * Creates a body of a circle shape for a bullet.
 * 
 * @param ship_centroid centroid of the ship
 * @param ship_angle angle of the ship
 * @param speed initial speed of bullet
 * @param radius radius of bullet
 * @param mass mass of bullet
 * @param ship_height height of ship
 * 
 * @return a pointer to the bullet body
*/
body_t *make_bullet(vector_t ship_centroid, double ship_angle, double speed, 
										const double radius, const double mass, 
										const double ship_height);

/** 
 * Creates a circular body for an asteroid.
 * 
 * @param centroid centroid of the asteroid
 * @param radius radius of asteroid
 * @param velocity initial velocity of asteroid
 * @param density density of asteroid
 * 
 * @return a pointer to the asteroid body
*/
body_t *make_asteroid(vector_t centroid, double radius, vector_t velocity, 
											const double density);

/** 
 * Creates a body for black hole asteroid.
 * 
 * @param centroid centroid of the asteroid
 * @param mass mass of black hole
 * @param radius radius of black hole
 * 
 * @return a pointer to the asteroid body
*/
body_t *make_blackhole(vector_t centroid, double mass, const double radius);

#endif