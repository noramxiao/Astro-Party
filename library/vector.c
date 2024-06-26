#include "vector.h"
#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const vector_t VEC_ZERO = {(double)0., (double)0.};

vector_t vec_add(vector_t v1, vector_t v2) {
  vector_t ret;
  ret.x = v1.x + v2.x;
  ret.y = v1.y + v2.y;
  return ret;
}

vector_t vec_subtract(vector_t v1, vector_t v2) {
  vector_t ret;
  ret.x = v1.x - v2.x;
  ret.y = v1.y - v2.y;
  return ret;
}

vector_t vec_negate(vector_t v) {
  vector_t ret;
  ret.x = -v.x;
  ret.y = -v.y;
  return ret;
}

vector_t vec_multiply(double scalar, vector_t v) {
  vector_t ret;
  ret.x = v.x * scalar;
  ret.y = v.y * scalar;
  return ret;
}

double vec_dot(vector_t v1, vector_t v2) {
  double ret = v1.x * v2.x + v1.y * v2.y;
  return ret;
}

double vec_cross(vector_t v1, vector_t v2) { return v1.x * v2.y - v1.y * v2.x; }

vector_t vec_rotate(vector_t v, double angle) {
  vector_t ret;
  ret.x = v.x * cos(angle) - v.y * sin(angle);
  ret.y = v.y * cos(angle) + v.x * sin(angle);
  return ret;
}

double vec_get_length(vector_t v) { return sqrt(pow(v.x, 2) + pow(v.y, 2)); }

vector_t vec_project(vector_t v1, vector_t v2) {
  double dot = vec_dot(v1, v2);
  double length = vec_get_length(v2);
  vector_t ret = vec_multiply(dot / (length * length), v2);
  return ret;
}

vector_t vec_make(double magnitude, double angle) {
  vector_t ret = (vector_t) {.x = 0, .y = magnitude};
  ret = vec_rotate(ret, angle);
  return ret;
}

vector_t vec_unit(vector_t v) {
  double magnitude = sqrt(v.x * v.x + v.y * v.y);
  if (magnitude == 0.) {
    return VEC_ZERO;
  }
  return vec_multiply(1.0 / magnitude, v);
}