#include "vector.h"
#include "list.h"
#include "list.h"
#include "shapes.h"
#include <stdlib.h>
#include <math.h>

const size_t CIRC_POINTS = 80;
const double PI = 3.141592653589793238462643383279502884197;

list_t *make_rectangle(double width, double height) {
  list_t *points = list_init(4, free);
  vector_t *p1 = malloc(sizeof(vector_t));
  *p1 = (vector_t){0, 0};
  vector_t *p2 = malloc(sizeof(vector_t));
  *p2 = (vector_t){width, 0};
  vector_t *p3 = malloc(sizeof(vector_t));
  *p3 = (vector_t){width, height};
  vector_t *p4 = malloc(sizeof(vector_t));
  *p4 = (vector_t){0, height};

  list_add(points, p1);
  list_add(points, p2);
  list_add(points, p3);
  list_add(points, p4);

  return points;
}

list_t *make_iso_triangle(vector_t centroid, double base, double height) {
	double inradius = base * height / (base + 2 * sqrt(base * base + height * height));
	list_t *points = list_init(3, free);

	vector_t *p1 = malloc(sizeof(vector_t));
	assert(p1);
	*p1 = (vector_t) {.x = 0, .y = 0};
	vector_t *p2 = malloc(sizeof(vector_t));
	assert(p2);
	*p2 = (vector_t) {.x = 0, .y = base};
	vector_t *p3 = malloc(sizeof(vector_t));
	assert(p3);
	*p3 = (vector_t) {.x = base / 2, .y = height};

	list_add(points, p1);
	list_add(points, p2);
	list_add(points, p3);
	
	return points;
}

list_t *make_circle(double radius) {
	vector_t center = (vector_t) {.x = 0, .y = 0};
  list_t *points = list_init(CIRC_POINTS, free);
  for (size_t i = 0; i < CIRC_POINTS; i++) {
    double angle = 2 * PI * i / CIRC_POINTS;
    vector_t *p = malloc(sizeof(vector_t));
    *p = (vector_t){center.x + radius * cos(angle),
                    center.y + radius * sin(angle)};
    list_add(points, p);
  }
  return points;
}