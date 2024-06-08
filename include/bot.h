#include "list.h"
#include "body.h"
#include "scene.h"
#include "sdl_wrapper.h"

typedef struct game_info {
	body_t *p1;
	body_t *p2;
	double bullet_speed;
	double bullet_radius;
	double ship_base;
	double ship_height;
	scene_t *scene;
	double dt;
} game_info_t;

/**
 * Estimates if a bullet shot by p1 now will hit p2 given that p2 moves at
 * a constant velocity by finding the time t at which the distance between 
 * the centroid of a shot bullet and the centroid of p2's ship is minimized.
 * t is calculated by an equation found by taking the partial derivative
 * of the distance of the 2 centroids given their parametric equations.
 * 
 * @param p1 the body of player 1's ship
 * @param p2 the body of player 2's ship
 * @param info information about the game such as constants, etc...
 * 
 * @return if bullet shot now by p1 will hit p2
*/
bool bullet_will_hit(body_t *p1, body_t *p2, game_info_t *info);

/**
 * Modifies keyboard state to reflect user input for bot
 * 
 * @param key_state keyboard state that is the output of SDL_GetKeyboardState(NULL)
 * @param info information about the game such as constants, etc...
 * @param player pointer to player body that bot should control
*/
void bot_move(Uint8 *key_state, game_info_t *info, body_t *player);