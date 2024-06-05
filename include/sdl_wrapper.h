#ifndef __SDL_WRAPPER_H__
#define __SDL_WRAPPER_H__

#include "color.h"
#include "list.h"
#include "polygon.h"
#include "scene.h"
#include "state.h"
#include "vector.h"
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <stdbool.h>

// Values passed to a key handler when the given arrow key is pressed
typedef enum {
  P1_TURN = SDL_SCANCODE_W,
  P1_SHOOT = SDL_SCANCODE_Q,
  P2_TURN = SDL_SCANCODE_M,
  P2_SHOOT = SDL_SCANCODE_N,
} player_key_t;

/**
 * The possible types of key events.
 * Enum types in C are much more primitive than in Java; this is equivalent to:
 * typedef unsigned int KeyEventType;
 * #define KEY_PRESSED 0
 * #define KEY_RELEASED 1
 */
typedef enum { KEY_PRESSED, KEY_RELEASED } key_event_type_t;

/**
 * A keypress handler.
 * When a key is pressed or released, the handler is passed its char value.
 * Most keys are passed as their char value, e.g. 'a', '1', or '\r'.
 * Arrow keys have the special values listed above.
 *
 * @param key a character indicating which key was pressed
 * @param type the type of key event (KEY_PRESSED or KEY_RELEASED)
 * @param held_time if a press event, the time the key has been held in seconds
 */
typedef void (*key_handler_t)(const Uint8 *key_state, void *state);
                              
/**
 * A click handler.
 * When a mouse click is pressed or released, the handler is passed its char value.
 *
 * @param state a struct containing information about the environment
 * @param x the x coord of the click
 * @param held_time the y coord of the click
 */
typedef void (*click_handler_t)(void *state, double x, double y);

/**
 * Initializes the SDL window and renderer.
 * Must be called once before any of the other SDL functions.
 *
 * @param min the x and y coordinates of the bottom left of the scene
 * @param max the x and y coordinates of the top right of the scene
 */
void sdl_init(vector_t min, vector_t max);

/**
 * Processes all SDL events and returns whether the window has been closed.
 * This function must be called in order to handle inputs.
 *
 * @return true if the window was closed, false otherwise
 */
bool sdl_is_done(void *state);

/**
 * Clears the screen. Should be called before drawing polygons in each frame.
 */
void sdl_clear(void);

/**
 * Loads image as a surface and converts it to a texture.
 *
 * @param img_path the file path of the image to be loaded
 *
 * @return SDL_Texture object generated from the image
 */
SDL_Texture *sdl_load_image(const char *img_path);

/**
 * Loads font as a TTF_Font.
 *
 * @param font_path the file path of the font to be loaded
 * @param ptsize the size of the font to be loaded
 *
 * @return TTF_Font object generated from the font
 */
TTF_Font *sdl_load_font(const char *font_path, int ptsize);

/**
 * Loads color as a SDL_Color.
 *
 * @param r the red value of the color (0 to 255)
 * @param g the green value of the color (0 to 255)
 * @param b the blue value of the color (0 to 255)
 * @param a the alpha value of the color(0 to 1)
 *
 * @return SDL_Color object generated from the color
 */
SDL_Color *sdl_load_color(Uint8 r, Uint8 b, Uint8 g, Uint8 a);

/**
 * Draws the image by copying the texture to the rendering target.
 *
 * @param texture the texture to be copied to the rendering target
 * @param pos the position of the texture in the window
 * @param size the size of the texture
 */
void sdl_draw_image(SDL_Texture *texture, vector_t pos, vector_t size);

/**
 * Draws the text by copying the texture to the rendering target.
 *
 * @param font the font for the text
 * @param string the text content
 * @param forecol the text color
 * @param pos the position of the text
 */
void sdl_draw_text(TTF_Font *font, const char *string, SDL_Color *forecol,
                   vector_t pos);

/**
 * Draws a polygon from the given list of vertices and a color.
 *
 * @param poly a struct representing the polygon
 * @param color the color used to fill in the polygon
 */
void sdl_draw_polygon(polygon_t *poly, rgb_color_t color);

/**
 * Displays the rendered frame on the SDL window.
 * Must be called after drawing the polygons in order to show them.
 */
void sdl_show(void);

/**
 * Draws all bodies in a scene.
 * This internally calls sdl_draw_polygon()
 * so those functions should not be called directly.
 *
 * @param scene the scene to draw
 * @param aux an additional body to draw (can be NULL if no additional bodies)
 */
void sdl_render_scene(scene_t *scene, void *aux);
void sdl_render_scene_cam(scene_t *scene, void *aux, vector_t cam_center, vector_t cam_size);

/**
 * Registers a function to be called every time a key is pressed.
 * Overwrites any existing handler.
 *
 * Example:
 * ```
 * void on_key(char key, key_event_type_t type, double held_time) {
 *     if (type == KEY_PRESSED) {
 *         switch (key) {
 *             case 'a':
 *                 printf("A pressed\n");
 *                 break;
 *             case UP_ARROW:
 *                 printf("UP pressed\n");
 *                 break;
 *         }
 *     }
 * }
 * int main(void) {
 *     sdl_on_key(on_key);
 *     while (!sdl_is_done());
 * }
 * ```
 *
 * @param handler the function to call with each key press
 */
void sdl_on_key(key_handler_t handler);

/**
 * Registers a function to be called every time a mouse is clicked.
 * Overwrites any existing handler.
 *
 * @param handler the function to call with each mouse click
 */
void sdl_on_click(click_handler_t handler);

/**
 * Returns last time key was pressed
 * 
 * @param key key to check
*/
double get_last_press(player_key_t key);

/**
 * Sets last time key was pressed
 * 
 * @param key key to check
*/
void set_last_press(player_key_t key);

/**
 * Returns last time key was released
 * 
 * @param key key to check
*/
double get_last_release(player_key_t key);

/**
 * Sets last time key was released
 * 
 * @param key key to check
*/
void set_last_release(player_key_t key);

/**
 * Get duration of time key has been held for
 * 
 * @param key key to find time since last press
 * 
 * @return time in seconds that key has been held
*/
double get_time_held(player_key_t key);

/**
 * Load a .wav sound file
 * 
 * @param sound_path path to wav file
 * 
 * @return pointer to Mix_Chunk
*/
Mix_Chunk *sdl_load_sound(const char* sound_path);

/**
 * Play a sound
 * 
 * @param sound sound to play
*/
void sdl_play_sound(Mix_Chunk *sound);

/**
 * Finds the smallest bounding box for a body
 *
 * @param body the body to find the bounding box for
 * @return an SDL_Rect with the top left corner and width and weight
 */
SDL_Rect get_bounding_box(body_t *body);

/**
 * Gets the amount of time that has passed since the last time
 * this function was called, in seconds.
 *
 * @return the number of seconds that have elapsed
 */
double time_since_last_tick(void);

double get_window_width(void);

double get_window_height(void);

#endif // #ifndef __SDL_WRAPPER_H__
