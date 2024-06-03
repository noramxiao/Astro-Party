#include "bot.h"
#include "body.h"
#include "collision.h"
#include "color.h"
#include "entities.h"
#include "shapes.h"
#include "vector.h"

const vector_t UP_UNIT_VEC = (vector_t) {.x = 0, .y = 1};

const rgb_color_t BLACK = (rgb_color_t) {.r = 0, .g = 0, .b = 0};

bool bullet_will_hit(body_t *p1, body_t *p2, game_info_t *info) {
	const double bullet_dist = info->bullet_dist;
	const double bullet_speed = info->bullet_speed;
	const double bullet_radius = info->bullet_radius;
	const double ship_base = info->ship_base;
	const double ship_height = info->ship_height;

	vector_t v1 = body_get_velocity(p1);
	vector_t p1_pos = body_get_centroid(p1);
	double p1_angle = body_get_rotation(p1);
	vector_t p1_dir = vec_rotate(UP_UNIT_VEC, p1_angle);
	vector_t bullet_disp = vec_make(bullet_dist, p1_dir);
	vector_t bullet_pos = vec_add(p1_pos, bullet_disp);
	vector_t bullet_velo = vec_make(bullet_speed, p1_dir);
	vector_t p2_pos = body_get_centroid(p2);
	vector_t p2_velo = body_get_velocity(p2);

	// find time of minimum distance between bullet and p2
	vector_t velo_diff = vec_subtract(p2_velo, bullet_velo);
	vector_t pos_diff = vec_subtract(p2_pos, bullet_pos);
	double time_closest = vec_dot(velo_diff, pos_diff) / vec_dot(velo_diff, velo_diff);

	// find the actual distance by plugging time back in
	double dist = vec_get_length(pos_diff + vec_multiply(time_closest, velo_diff));
	// estimate if bullet and p2 will collide by checking if dist is less than
	// the sum of their radii (for p2 we assume radius = average of ship base and
	// ship height divided by 2)
	double radii_sum = bullet_radius + (ship_base + ship_height) / 4;
	if (dist > radii_sum) {
		return false;
	}
	
	// create body for bullet path to use for collision checking
	vector_t bullet_path = vec_multiply(time_closest, bullet_velo);
	vector_t collision_pos = vec_add(bullet_pos, bullet_path);
	vector_t path_centroid = vec_multiply(0.5, vec_add(bullet_pos, bullet_velo));
	double path_length = vec_get_length(bullet_path);
	list_t *path_rect = make_rectangle(path_centroid, bullet_radius, vec_get_length(bullet_path));
	body_t *path_body = body_init(path_rect, 0, BLACK);

	// check if bullet collides with any walls (asteroids are fine) before ship
	list_t *bodies = info->bodies;
	size_t n_bodies = list_size(bodies);
	for (size_t i = 0; i < n_bodies; i++) {
		body_t *body = list_get(bodies, i);
		if (get_type(body) != WALL) {
			continue;
		}
		collision_info_t *coll_info = find_collision(path_body, body);
		if (coll_info->collided) {
			return false;
		}
	}
	body_free(path_body);
	
	return true;
}

void bot_move(Uint8 *key_state, game_info_t *info, body_t *player)  {
	body_t *opp;
	player_key_t turn_key;
	player_key_t shoot_key;
	if (player == info->p1) {
		opp = info->p2;
		turn_key = P2_TURN;
		shoot_key = P2_SHOOT;
	} else {
		opp = info->p1;
		turn_key = P1_TURN;
		shoot_key = P1_SHOOT;
	}
	
	// shoot if bullet will hit, turn otherwise
	bool should_shoot = bullet_will_hit(player, opp, info);
	key_state[shoot_key] = should_shoot;
	key_state[turn_key] = !should_shoot;
}