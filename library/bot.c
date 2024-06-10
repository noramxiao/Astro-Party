#include "bot.h"
#include "body.h"
#include "collision.h"
#include "color.h"
#include "entities.h"
#include "scene.h"
#include "shapes.h"
#include "sdl_wrapper.h"
#include "stdlib.h"
#include "stdio.h"
#include "vector.h"

#include <math.h>

const rgb_color_t DUMMY_COLOR = (rgb_color_t) {.r = 0, .g = 0, .b = 0};

/**
 * Calculate the time at which two bodies are closest given current velocities
*/
double get_time_closest(body_t *b1, body_t *b2) {
	vector_t pos1 = body_get_centroid(b1);
	vector_t v1 = body_get_velocity(b1);
	vector_t pos2 = body_get_centroid(b2);
	vector_t v2 = body_get_velocity(b2);
	vector_t pos_diff = vec_subtract(pos2, pos1);
	vector_t velo_diff = vec_subtract(v2, v1);
	double time_closest = -vec_dot(velo_diff, pos_diff) / vec_dot(velo_diff, velo_diff);
	return fmax(0, time_closest);
}

/**
 * Check if two bodies will collide
 */
bool will_collide(body_t *b1, body_t *b2, double b1_width, double b2_width, scene_t *scene) {
	double time_closest = get_time_closest(b1, b2);

	// set b1 centroid to be at point closest to b2
	vector_t b1_centroid = body_get_centroid(b1);
	vector_t b1_path = vec_multiply(time_closest, body_get_velocity(b1));
	vector_t new_b1_centroid = vec_add(b1_centroid, b1_path);
	body_set_centroid(b1, new_b1_centroid);

	// set b2 centroid to be at point closest to b1
	vector_t b2_centroid = body_get_centroid(b2);
	vector_t b2_path = vec_multiply(time_closest, body_get_velocity(b2));
	vector_t new_b2_centroid = vec_add(b2_centroid, b2_path);
	body_set_centroid(b2, new_b2_centroid);

	// check for collision between bullet and p2 at time_closest
	collision_info_t coll_info = find_collision(b1, b2);
	body_set_centroid(b1, b1_centroid);
	body_set_centroid(b2, b2_centroid);
	if (!coll_info.collided) {
		return false;
	}

	// create bodies for the b1 and b2 paths
	vector_t b1_path_centroid = vec_add(b1_centroid, vec_multiply(0.5, b1_path));
	double b1_path_length = vec_get_length(b1_path);
	list_t *b1_path_rect = make_rectangle(b1_path_centroid, b1_width, b1_path_length);
	body_t *b1_path_body = body_init(b1_path_rect, 0, DUMMY_COLOR);
	vector_t b2_path_centroid = vec_add(b2_centroid, vec_multiply(0.5, b2_path));
	double b2_path_length = vec_get_length(b2_path);
	list_t *b2_path_rect = make_rectangle(b2_path_centroid, b2_width, b2_path_length);
	body_t *b2_path_body = body_init(b2_path_rect, 0, DUMMY_COLOR);

	// check if paths collide with any walls
	size_t n_bodies = scene_bodies(scene);
	for (size_t i = 0; i < n_bodies; i++) {
		body_t *body = scene_get_body(scene, i);
		if (get_type(body) != WALL) {
			continue;
		}
		collision_info_t coll_info = find_collision(b1_path_body, body);
		if (coll_info.collided) {
			body_free(b1_path_body);
			body_free(b2_path_body);
			return false;
		}
	}
	body_free(b1_path_body);
	body_free(b2_path_body);
	
	return true;
}

/**
 * Check if bullet shot by p1 right now will hit p2 given current velocities
*/
bool should_shoot(body_t *p1, body_t *p2, game_info_t *info) {
	double bullet_speed = info->bullet_speed;
	double bullet_radius = info->bullet_radius;
	double ship_base = info->ship_base;
	double ship_height = info->ship_height;
	scene_t *scene = info->scene;

	body_t *bullet = make_bullet(body_get_centroid(p1), body_get_rotation(p1), 
															 bullet_speed, bullet_radius, 0, ship_height);
	bool ret = will_collide(bullet, p2, bullet_radius * 2, ship_base, scene);
	body_free(bullet);
	return ret;
}

/**
 * Check if any bullets in scene will hit ship's current position
 */
bool should_dodge(body_t *ship, game_info_t *info) {
	scene_t *scene = info->scene;
	size_t n_bodies = scene_bodies(scene);
	for (size_t i = 0; i < n_bodies; i++) {
		body_t *body = scene_get_body(scene, i);
		if (get_type(body) != BULLET) {
			continue;
		}
		if (will_collide(body, ship, info->bullet_radius * 2, info->ship_base, info->scene)) {
			return true;
		}
	}
	
	return false;
}

/**
 * Finds the positive modulus between two doubles
*/
double pos_fmod(double a, double b) {
	return fmod(fmod(a, b) + b, b);
}

/** 
 * Calculates clockwise angle difference from v1 to v2
 */
double get_clockwise_angle_diff(vector_t v1, vector_t v2) {
	double angle1 = pos_fmod(atan(v1.y / v1.x), 2 * M_PI);
	double angle2 = pos_fmod(atan(v2.y / v2.x), 2 * M_PI);
	double angle_diff = pos_fmod(angle2 - angle1, 2 * M_PI);
	// return 2 pi - angle diff b/c angle_diff is counterclockwise
	return 2 * M_PI - angle_diff;
}

/**
 * Checks if turning p1 reduces angle between p1 and p2
*/
bool turn_reduces_angle(body_t *p1, body_t *p2, double ship_rot_speed, double dt) {
	// find radius from p1 to p2 during next tick
	vector_t new_p1_centroid = vec_add(body_get_centroid(p1), vec_multiply(dt, body_get_velocity(p1)));
	vector_t new_p2_centroid = vec_add(body_get_centroid(p2), vec_multiply(dt, body_get_velocity(p2)));
	vector_t new_radius = vec_subtract(new_p1_centroid, new_p2_centroid);

	// current angle between forward direction of player (p1) and radius to opp (p2)
	double p1_angle = body_get_rotation(p1);
	vector_t p1_dir = vec_make(1, p1_angle);
	double angle_diff = get_clockwise_angle_diff(p1_dir, new_radius);
	
	// new angle between forward direction of p1 and radius to p2
	double new_p1_angle = p1_angle + ship_rot_speed * dt;
	vector_t new_p1_dir = vec_make(1, new_p1_angle);
	double new_angle_diff = get_clockwise_angle_diff(new_p1_dir, new_radius);
	
	return new_angle_diff < angle_diff;
}

void bot_move(Uint8 *key_state, game_info_t *info, body_t *player)  {
	body_t *opp;
	player_key_t turn_key = P2_TURN;
	player_key_t shoot_key = P2_SHOOT;
	
	// set controls to whichever player bot controls
	if (player == info->p1) {
		opp = info->p2;
		turn_key = P1_TURN;
		shoot_key = P1_SHOOT;
	} else {
		opp = info->p1;
		turn_key = P2_TURN;
		shoot_key = P2_SHOOT;
	}
	
	// shoot if bullet will hit
	bool press_shoot = should_shoot(player, opp, info);
	key_state[shoot_key] = press_shoot;
	
	if (turn_reduces_angle(player, opp, info->ship_rot_speed, info->dt)) {
		key_state[turn_key] = true;
	}

	// boost to dodge if needed
	if (should_dodge(player, info)) {
		bool turning = get_last_press(turn_key) > get_last_release(turn_key);
		key_state[turn_key] = !turning;
	}
}
