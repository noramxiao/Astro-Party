#include "polygon.h"
#include "color.h"
#include "list.h"
#include <assert.h>
#include <math.h>
#include <stdlib.h>

typedef struct polygon {
  list_t *points;
  vector_t velocity;
  double rotation_speed;
  double rotation;
  rgb_color_t *color;
} polygon_t;

polygon_t *polygon_init(list_t *points, vector_t initial_velocity,
                        double rotation_speed, double red, double green,
                        double blue) {
  polygon_t *ret = malloc(sizeof(polygon_t));
  assert(ret != NULL);
  ret->points = points;
  ret->velocity = initial_velocity;
  ret->rotation_speed = rotation_speed;
  ret->rotation = 0.0;
  ret->color = color_init(red, green, blue);
  return ret;
}

list_t *polygon_get_points(polygon_t *polygon) { return polygon->points; }

void polygon_move(polygon_t *polygon, double time_elapsed) {
  vector_t displacement = vec_multiply(time_elapsed, polygon->velocity);
  polygon_translate(polygon, displacement);
  double angle = polygon->rotation_speed * time_elapsed;
  polygon_rotate(polygon, angle, polygon_centroid(polygon));
  polygon->rotation += angle;
}

void polygon_set_velocity(polygon_t *polygon, vector_t vel) {
  polygon->velocity = vel;
}

vector_t polygon_get_velocity(polygon_t *polygon) { return polygon->velocity; }

void polygon_free(polygon_t *polygon) {
  list_free(polygon->points);
  color_free(polygon->color);
  free(polygon);
}

double polygon_area(polygon_t *polygon) {
  double area = 0;
  list_t *points = polygon->points;
  size_t n = list_size(points);
  for (size_t i = 0; i < n; i++) {
    area += vec_cross(*(vector_t *)list_get(points, i),
                      *(vector_t *)list_get(points, (i + 1) % n));
  }
  area = fabs(area / 2);
  return area;
}

vector_t polygon_centroid(polygon_t *polygon) {
  vector_t ret;
  list_t *points = polygon->points;
  size_t n = list_size(points);
  double cx = 0;
  double cy = 0;
  for (size_t i = 0; i < n; i++) {
    cx += (((vector_t *)list_get(points, i))->x +
           ((vector_t *)list_get(points, (i + 1) % n))->x) *
          vec_cross(*(vector_t *)list_get(points, i),
                    *(vector_t *)list_get(points, (i + 1) % n));
  }
  for (size_t i = 0; i < n; i++) {
    cy += (((vector_t *)list_get(points, i))->y +
           ((vector_t *)list_get(points, (i + 1) % n))->y) *
          vec_cross(*(vector_t *)list_get(points, i),
                    *(vector_t *)list_get(points, (i + 1) % n));
  }
  double area = polygon_area(polygon);
  ret.x = cx / (6 * area);
  ret.y = cy / (6 * area);
  return ret;
}

double polygon_rot_inertia(polygon_t *polygon, double mass) {
  double numer = 0;
  double denom = 0;
  list_t *points = polygon->points;
  size_t n = list_size(points);
  for (size_t i = 0; i < n; i++) {
    vector_t p1 = *(vector_t*)list_get(points, i);
    vector_t p2 = *(vector_t*)list_get(points, (i + 1) % n);
    numer += fabs(vec_cross(p1, p2)) * (vec_dot(p1, p1) + vec_dot(p1, p2) + vec_dot(p2, p2));
    denom += fabs(vec_cross(p1, p2));
  }
  denom *= 6;
  return mass * numer / denom;
}

void polygon_translate(polygon_t *polygon, vector_t translation) {
  list_t *points = polygon->points;
  size_t n = list_size(points);
  for (size_t i = 0; i < n; i++) {
    vector_t *p = (vector_t *)list_get(points, i);
    vector_t next = vec_add(*p, translation);
    p->x = next.x;
    p->y = next.y;
  }
}

void polygon_rotate(polygon_t *polygon, double angle, vector_t point) {
  list_t *points = polygon->points;
  size_t n = list_size(points);
  for (size_t i = 0; i < n; i++) {
    vector_t *p = (vector_t *)list_get(points, i);
    vector_t diff = vec_subtract(*p, point);
    diff = vec_rotate(diff, angle);
    vector_t next = vec_add(point, diff);
    p->x = next.x;
    p->y = next.y;
  }
}

rgb_color_t *polygon_get_color(polygon_t *polygon) { return polygon->color; }

void polygon_set_color(polygon_t *polygon, rgb_color_t *color) {
  color_free(polygon->color);
  polygon->color = color_init(color->r, color->g, color->b);
}

void polygon_set_center(polygon_t *polygon, vector_t centroid) {
  vector_t current_centroid = polygon_centroid(polygon);
  vector_t translation = vec_subtract(centroid, current_centroid);
  polygon_translate(polygon, translation);
}

vector_t polygon_get_center(polygon_t *polygon) {
  return polygon_centroid(polygon);
}

void polygon_set_rotation(polygon_t *polygon, double rot) {
  vector_t center = polygon_get_center(polygon);
  double current_rotation = polygon_get_rotation(polygon);
  double rotation_angle = rot - current_rotation;
  polygon_rotate(polygon, rotation_angle, center);
  polygon->rotation = rot;
}

double polygon_get_rotation(polygon_t *polygon) { return polygon->rotation; }

double polygon_get_rot_speed(polygon_t *polygon) { 
  assert(polygon);
  return polygon->rotation_speed; 
}

void polygon_set_rot_speed(polygon_t *polygon, double rot_speed) { 
  assert(polygon);
  polygon->rotation_speed = rot_speed;
}
