/* 	Header file for pong game.
	Includes various declarations for code
	in project.
*/
extern const uint8_t const test[512];
void display_game(int x, const uint8_t *data);
void matrix_to_array();
int power(int a, int b);
void paddle_l_init();
void paddle_r_init();
void move_l_paddle_up();
void move_r_paddle_up();
void move_l_paddle_down();
void move_r_paddle_down();

