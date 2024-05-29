#include "vector.h"
#include "list.h"
#include "body.h"
#include "color.h"

typedef enum {
	SHIP,
	PILOT,
	BULLET,
	ASTEROID,
	WALL,
	BACKGROUND
} entity_type_t;

/** 
 * Creates a body for a spaceship. Will just be an isosceles triangle for now.
 * 
 * @param centroid centroid of the spaceship
 * @param color color of the spaceship
 * @param init_velocity initial velocity of the spaceship
 * @return a pointer to the ship body
*/
body_t *make_ship(vector_t centroid, rgb_color_t color, vector_t init_velocity);

/** 
 * Creates a body for a pilot. Will just be a rectangle for now.
 * 
 * @param centroid centroid of the pilot
 * @param color color of the pilot
 * @param init_velocity initial velocity of the pilot
 * @return a pointer to the pilot body
*/
body_t *make_pilot(vector_t centroid, rgb_color_t color, vector_t init_velocity);

/** 
 * Creates a body for an asteroid. Will just be a grey circle for now.
 * 
 * @param centroid centroid of the asteroid
 * @param radius radius of asteroid
 * @param init_velocity initial velocity of asteroid
 * @return a pointer to the asteroid body
*/
body_t *make_asteroid(vector_t centroid, double radius, vector_t init_velocity);