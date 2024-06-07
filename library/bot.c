#include "bot.h"
#include "body.h"
#include "collision.h"
#include "color.h"
#include "entities.h"
#include "scene.h"
#include "shapes.h"
#include "stdlib.h"
#include "stdio.h"
#include "vector.h"

#include <math.h>

const vector_t UP_UNIT_VEC = (vector_t) {.x = 0, .y = 1};
const rgb_color_t DUMMY_COLOR = (rgb_color_t) {.r = 0, .g = 0, .b = 0};

bool bullet_will_hit(body_t *p1, body_t *p2, game_info_t *info) {
	const double bullet_speed = info->bullet_speed;
	const double bullet_radius = info->bullet_radius;
	const double ship_base = info->ship_base;
	const double ship_height = info->ship_height;
	const double bullet_dist = ship_height * 2 / 3 + bullet_radius + 1;

	double p1_angle = body_get_rotation(p1);
	vector_t spawn_disp = vec_make(bullet_dist, p1_angle);
	vector_t p1_pos = body_get_centroid(p1);
	vector_t bullet_spawn = vec_add(p1_pos, spawn_disp);
	vector_t bullet_velo = vec_make(bullet_speed, p1_angle);
	vector_t p2_pos = body_get_centroid(p2);
	vector_t p2_velo = body_get_velocity(p2);

	// find time of minimum distance between bullet and p2
	vector_t velo_diff = vec_subtract(p2_velo, bullet_velo);
	vector_t pos_diff = vec_subtract(p2_pos, bullet_spawn);
	double time_closest = -vec_dot(velo_diff, pos_diff) / vec_dot(velo_diff, velo_diff);

	// find the actual distance by plugging time back in
	vector_t min_disp = vec_add(pos_diff, vec_multiply(time_closest, velo_diff));
	double min_dist = vec_get_length(min_disp);
	// estimate if bullet and p2 will collide by checking if dist is less than
	// the sum of their radii (for p2 we assume radius = average of ship base and
	// ship height divided by 2)
	double radii_sum = bullet_radius + (ship_base + ship_height) / 4;
	if (min_dist > radii_sum) {
		return false;
	}
	
	// create body for bullet path to use for collision checking
	vector_t bullet_path = vec_multiply(time_closest, bullet_velo);
	vector_t path_centroid = vec_add(bullet_spawn, vec_multiply(0.5, bullet_path));
	double path_length = vec_get_length(bullet_path);
	list_t *path_rect = make_rectangle(path_centroid, bullet_radius, path_length);
	body_t *path_body = body_init(path_rect, 0, DUMMY_COLOR);

	// check if bullet collides with any walls before reaching ship
	scene_t *scene = info->scene;
	size_t n_bodies = scene_bodies(scene);
	for (size_t i = 0; i < n_bodies; i++) {
		body_t *body = scene_get_body(scene, i);
		if (get_type(body) != WALL) {
			continue;
		}
		collision_info_t coll_info = find_collision(path_body, body);
		if (coll_info.collided) {
			return false;
		}
	}
	body_free(path_body);
	
	return true;
}

/**
 * Finds the positive modulus between two doubles
*/
double pos_fmod(double a, double b) {
	return fmod(fmod(a, b) + b, b);
}

/**
 * Checks if turning reduces angle between p1 and p2
*/
bool turn_reduces_angle(body_t *p1, body_t *p2, double dt) {
	// current angle between forward direction of player (p1) and radius to opp (p2)
	double p1_angle = body_get_rotation(p1);
	p1_angle = pos_fmod(p1_angle, 2 * M_PI);
	vector_t radius = vec_subtract(body_get_centroid(p2), body_get_centroid(p1));
	double rad_angle = atan(radius.y / radius.x) - M_PI / 2;
	rad_angle = pos_fmod(rad_angle, 2 * M_PI);
	double angle_diff = rad_angle - p1_angle;
	angle_diff = pos_fmod(angle_diff, 2 * M_PI);
	
	// new angle between forward direction of p1 and radius to p2
	double new_p1_angle = p1_angle + body_get_rot_speed(p1) * dt;
	new_p1_angle = pos_fmod(p1_angle, 2 * M_PI);
	vector_t new_p1_centroid = vec_add(body_get_centroid(p1), vec_multiply(dt, body_get_velocity(p1)));
	vector_t new_p2_centroid = vec_add(body_get_centroid(p2), vec_multiply(dt, body_get_velocity(p2)));
	vector_t new_radius = vec_subtract(new_p1_centroid, new_p2_centroid);
	double new_rad_angle = atan(new_radius.y / new_radius.x) - M_PI / 2;
	new_rad_angle = pos_fmod(new_rad_angle, 2 * M_PI);
	double new_angle_diff = new_rad_angle - new_p1_angle;
	new_angle_diff = pos_fmod(new_angle_diff, 2 * M_PI);
	
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
	bool should_shoot = bullet_will_hit(player, opp, info);
	key_state[shoot_key] = should_shoot;
	
	// turn with 99% chance otherwise (could use the turn_reduces_angle 
	// function to "track" player, but AI is already too good)
	if (!should_shoot) {
		key_state[turn_key] = rand() % 100;
	}
}