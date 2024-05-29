#include "vector.h"
#include "list.h"
#include "list.h"

/** 
 * Creates a rectangular shaped list given a width and height in the form of 
 * a vector.
 * 
 * @param centroid centroid of the rectangle
 * @param dimensions width and height of rectangle in vector format
 * @return a pointer to the rectangle list
*/
list_t *make_rectangle(vector_t centroid, vector_t dimensions);

/** 
 * Creates a triangular shaped list given a base and height.
 * 
 * @param centroid centroid of the triangle
 * @param base length of triangle base
 * @param height length of triangle height
 * @return a pointer to the triangle list
*/
list_t *make_triangle(vector_t centroid, double base, double height);

/** 
 * Creates a circular shaped list given a radius.
 * 
 * @param centroid centroid of the circle
 * @param radius circle radius length
 * @return a pointer to the triangle list
*/
list_t *make_circle(vector_t centroid, double radius);