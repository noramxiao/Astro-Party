#include "vector.h"
#include "list.h"
#include "body.h"
#include "color.h"

const double SHIP_MASS;
const double PILOT_MASS;
const double ASTEROID_MASS_DENSITY;

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
 * Creates a body for a pilot. Will just be a rectangle and a circle for now.
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

/** 
 * Creates a rectangular shaped body given a width and height in the form of 
 * a vector and a color.
 * 
 * @param centroid centroid of the rectangle
 * @param dimensions width and height of rectangle in vector format
 * @param color color of the rectangle
 * @param init_velocity initial velocity of the rectangle
 * @return a pointer to the rectangle body
*/
body_t *make_rectangle(vector_t centroid, vector_t dimensions, rgb_color_t color, vector_t init_velocity);

/** 
 * Creates a triangular shaped body given a base, height, and color.
 * 
 * @param centroid centroid of the triangle
 * @param base length of triangle base
 * @param height length of triangle height
 * @param color color of the triangle
 * @param init_velocity initial velocity of the triangle
 * @return a pointer to the triangle body
*/
body_t *make_triangle(vector_t centroid, double base, double height, rgb_color_t color, vector_t init_velocity);

/** 
 * Creates a circular shaped body given a radius and color.
 * 
 * @param centroid centroid of the circle
 * @param radius circle radius length
 * @param color color of the circle
 * @param init_velocity initial velocity of the circle
 * @return a pointer to the triangle body
*/
body_t *make_circle(vector_t centroid, double radius, rgb_color_t color, vector_t init_velocity);