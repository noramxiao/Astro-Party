#include "vector.h"
#include "list.h"
#include "list.h"

/** 
 * Creates a rectangular shaped list given a width and height in the form of 
 * a vector.
 * 
 * @param centroid centroid of rectangle
 * @param width width of rectangle
 * @param height height of rectangle
 * @return a pointer to the rectangle list
*/
list_t *make_rectangle(vector_t centroid, double width, double height);

/** 
 * Creates an isosceles triangle given a base and height.
 * 
 * @param centroid length of triangle base
 * @param base length of triangle base
 * @param height length of triangle height
 * @return a pointer to the triangle list
*/
list_t *make_iso_triangle(vector_t centroid, double base, double height);

/** 
 * Creates a circular shaped list given a radius.
 * 
 * @param centroid circle radius length
 * @param radius circle radius length
 * @return a pointer to the triangle list
*/
list_t *make_circle(vector_t centroid, double radius);