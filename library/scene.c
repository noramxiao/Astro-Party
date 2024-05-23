#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "body.h"
#include "forces.h"
#include "scene.h"

const size_t INITIAL_SIZE = 5;

struct scene {
  size_t num_bodies;
  list_t *bodies;
  list_t *force_creators;
};

typedef struct force_instance {
  force_creator_t force_creator;
  list_t *bodies;
  void *aux;
} force_instance_t;

force_instance_t *force_instance_init(force_creator_t force_creator,
                                      list_t *bodies, void *aux) {
  force_instance_t *force = malloc(sizeof(force_instance_t));
  assert(force != NULL);

  force->force_creator = force_creator;
  force->bodies = bodies;
  force->aux = aux;

  return force;
}

void force_instance_free(force_instance_t *force_instance) {
  list_free(force_instance->bodies);
  list_t *aux_bodies = body_aux_bodies(((body_aux_t *)force_instance->aux));
  list_free(aux_bodies);
  free(force_instance->aux);
  free(force_instance);
}

scene_t *scene_init(void) {
  scene_t *scene = malloc(sizeof(scene_t));
  assert(scene != NULL);

  scene->bodies = list_init(INITIAL_SIZE, (free_func_t)body_free);
  scene->force_creators =
      list_init(INITIAL_SIZE, (free_func_t)force_instance_free);

  return scene;
}

void scene_free(scene_t *scene) {
  list_free(scene->bodies);
  list_free(scene->force_creators);
  free(scene);
}

size_t scene_bodies(scene_t *scene) { return list_size(scene->bodies); }

body_t *scene_get_body(scene_t *scene, size_t index) {
  return (body_t *)list_get(scene->bodies, index);
}

void scene_add_body(scene_t *scene, body_t *body) {
  list_add(scene->bodies, body);
}

void scene_remove_body(scene_t *scene, size_t index) {
  assert(scene != NULL);
  assert(index < scene_bodies(scene));
  body_remove(scene_get_body(scene, index));
}

force_instance_t *scene_get_force(scene_t *scene, size_t index) {
  return (force_instance_t *)list_get(scene->force_creators, index);
}

void scene_add_force_creator(scene_t *scene, force_creator_t force_creator,
                             void *aux) {
  scene_add_bodies_force_creator(scene, force_creator, aux, list_init(0, free));
}

void scene_add_bodies_force_creator(scene_t *scene, force_creator_t forcer,
                                    void *aux, list_t *bodies) {
  force_instance_t *force = force_instance_init(forcer, bodies, aux);
  list_add(scene->force_creators, force);
}

void scene_tick(scene_t *scene, double dt) {
  for (size_t i = 0; i < list_size(scene->force_creators); i++) {
    force_instance_t *force = scene_get_force(scene, i);
    force->force_creator(force->aux);
  }

  for (ssize_t i = 0; i < (ssize_t)(scene_bodies(scene)); i++) {
    body_t *curr_body = scene_get_body(scene, i);

    // Check if the current body is removed
    if (body_is_removed(curr_body)) {
      // Check if any force creators contain body
      for (ssize_t j = 0; j < (ssize_t)(list_size(scene->force_creators));
           j++) {
        force_instance_t *force_instance = scene_get_force(scene, j);
        list_t *force_bodies = force_instance->bodies;
        for (size_t k = 0; k < list_size(force_bodies); k++) {
          if (list_get(force_bodies, k) == curr_body) {
            force_instance_t *removed = list_remove(scene->force_creators, j);
            force_instance_free(removed);
            j--; // decrement so we don't skip next force
            break;
          }
        }
      }

      body_t *removed = list_remove(scene->bodies, i);
      body_free(removed);
      i--;
    } else {
      body_tick(curr_body, dt);
    }
  }
}
