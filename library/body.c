#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "body.h"

struct body {
  polygon_t *poly;

  double mass;

  vector_t force;
  vector_t impulse;
  bool removed;

  void *info;
  free_func_t info_freer;
};

body_t *body_init(list_t *shape, double mass, rgb_color_t color) {
  assert(shape != NULL);
  body_t *body = body_init_with_info(shape, mass, color, NULL, NULL);
  return body;
}

body_t *body_init_with_info(list_t *shape, double mass, rgb_color_t color,
                            void *info, free_func_t info_freer) {
  assert(shape != NULL);

  body_t *body = malloc(sizeof(body_t));
  assert(body != NULL);

  vector_t init_velocity = (vector_t){.x = 0.0, .y = 0.0};

  body->poly =
      polygon_init(shape, init_velocity, 0.0, color.r, color.g, color.b);
  body->mass = mass;
  body->removed = false;
  body->force = (vector_t){0., 0.};
  body->impulse = (vector_t){0., 0.};
  body->info = info;
  body->info_freer = info_freer;

  return body;
}

void body_free(body_t *body) {
  assert(body != NULL);
  polygon_free(body->poly);
  if (body->info_freer != NULL) {
    body->info_freer(body->info);
  }
  free(body);
}

void body_add_force(body_t *body, vector_t force) {
  assert(body != NULL);
  body->force = vec_add(body->force, force);
}

void body_add_impulse(body_t *body, vector_t impulse) {
  assert(body != NULL);
  body->impulse = vec_add(body->impulse, impulse);
}

void body_remove(body_t *body) {
  assert(body != NULL);
  body->removed = true;
}

bool body_is_removed(body_t *body) {
  assert(body != NULL);
  return body->removed;
}

list_t *body_get_shape(body_t *body) {
  assert(body != NULL);

  list_t *polygon_points = polygon_get_points(body->poly);
  size_t num_points = list_size(polygon_points);

  list_t *shape_points = list_init(num_points, free);

  for (size_t i = 0; i < num_points; i++) {
    vector_t *point = (vector_t *)list_get(polygon_points, i);
    vector_t *new_point = malloc(sizeof(vector_t));
    assert(new_point != NULL);
    *new_point = *point;
    list_add(shape_points, new_point);
  }

  return shape_points;
}

vector_t body_get_centroid(body_t *body) {
  assert(body != NULL);
  return polygon_get_center(body->poly);
}

vector_t body_get_velocity(body_t *body) {
  assert(body != NULL);
  return polygon_get_velocity(body->poly);
}

rgb_color_t *body_get_color(body_t *body) {
  assert(body != NULL);
  return polygon_get_color(body->poly);
}

void body_set_color(body_t *body, rgb_color_t *col) {
  assert(body != NULL && col != NULL);
  polygon_set_color(body->poly, col);
}

void body_set_centroid(body_t *body, vector_t x) {
  assert(body != NULL);
  polygon_set_center(body->poly, x);
}

void body_set_velocity(body_t *body, vector_t v) {
  assert(body != NULL);
  polygon_set_velocity(body->poly, v);
}

double body_get_rotation(body_t *body) {
  assert(body != NULL);
  return polygon_get_rotation(body->poly);
}

void body_set_rotation(body_t *body, double angle) {
  assert(body != NULL);
  polygon_set_rotation(body->poly, angle);
}

void body_tick(body_t *body, double dt) {
  assert(body != NULL);
  vector_t old_vel = polygon_get_velocity(body->poly);

  vector_t force_vel = vec_multiply(dt / body->mass, body->force);
  vector_t impulse_vel = vec_multiply(1. / body->mass, body->impulse);

  vector_t new_vel = vec_add(vec_add(old_vel, force_vel), impulse_vel);
  vector_t avg_vel = vec_multiply(0.5, vec_add(old_vel, new_vel));

  polygon_set_velocity(body->poly, avg_vel);
  polygon_move(body->poly, dt);
  polygon_set_velocity(body->poly, new_vel);

  body_reset(body);
}

double body_get_mass(body_t *body) {
  assert(body != NULL);
  return body->mass;
}

polygon_t *body_get_polygon(body_t *body) {
  assert(body != NULL);
  return body->poly;
}

void *body_get_info(body_t *body) {
  assert(body != NULL);
  return body->info;
}

void body_reset(body_t *body) {
  body->force = VEC_ZERO;
  body->impulse = VEC_ZERO;
}