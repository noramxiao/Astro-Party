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
	double ship_rot_speed;
	double double_tap_thresh;
	scene_t *scene;
	double dt;
} game_info_t;

/**
 * Modifies keyboard state to reflect user input for bot
 * 
 * @param key_state keyboard state that is the output of SDL_GetKeyboardState(NULL)
 * @param info information about the game such as constants, etc...
 * @param player pointer to player body that bot should control
*/
void bot_move(Uint8 *key_state, game_info_t *info, body_t *player);