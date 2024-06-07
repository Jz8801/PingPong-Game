#define is_down(b) (input->buttons[b].is_down)
#define pressed(b) (input->buttons[b].is_down && input->buttons[b].changed)
#define released(b) (!input->buttons[b].is_down && input->buttons[b].changed)

float player_1_p, player_1_dp, player_2_p, player_2_dp;
float arena_half_size_x = 85, arena_half_size_y = 45;
float player_half_size_x = 1.5, player_half_size_y = 10;

float ball_p_x, ball_p_y, ball_dp_x = 120, ball_dp_y, ball_half_size = 1;

int player_1_score, player_2_score;

internal void simulate_player(float* p, float* dp, float ddp, float dt) {
	ddp -= *dp * 10.f;

	*p = *p + *dp * dt + ddp * dt * dt * .5f; // player1 position
	*dp = *dp + ddp * dt; // player1 velocity

	if (*p + player_half_size_y > arena_half_size_y) {
		*p = arena_half_size_y - player_half_size_y;
		*dp *= -1;
	}
	else if (*p - player_half_size_y < -arena_half_size_y) {
		*p = -arena_half_size_y + player_half_size_y;
		*dp *= -1;
	}
}

internal bool aabb_vs_aabb(float p1x, float p1y, float hs1x, float hs1y, float p2x, float p2y, float hs2x, float hs2y) {
	return (p1x + hs1x > p2x - hs2x &&
		p1x - hs1x < p2x + hs2x &&
		p1y + hs1y > p2y - hs2y &&
		p1y - hs1y < p2y + hs2y);
}

enum Gamemode {
	GM_MENU,
	GM_GAMEPLAY,
};

Gamemode current_gamemode;
int hot_button;
bool enemy_is_ai;

internal void simulate_game(Input* input, float dt) {
	clear_screen(0xffffff);
	draw_rect(0, 0, arena_half_size_x, arena_half_size_y, 0x000000);

	if (current_gamemode == GM_GAMEPLAY) {
		float player_1_ddp = 0.f; // unit per second instead of frame
		
		if (!enemy_is_ai) {
			if (is_down(BUTTON_UP)) player_1_ddp += 2000;
			if (is_down(BUTTON_DOWN)) player_1_ddp -= 2000;
		}
		else {
			//if (ball_p_y > player_1_p) player_1_ddp += 1000;
			//if (ball_p_y < player_1_p) player_1_ddp -= 1000;
			player_1_ddp = (ball_p_y - player_1_p) * 100;
			if (player_1_ddp > 1300) player_1_ddp = 1300;
			if (player_1_ddp < -1300) player_1_ddp = -1300;
		}

		float player_2_ddp = 0.f; // unit per second instead of frame

		if (is_down(BUTTON_W)) player_2_ddp += 2000;
		if (is_down(BUTTON_S)) player_2_ddp -= 2000;

		simulate_player(&player_1_p, &player_1_dp, player_1_ddp, dt);
		simulate_player(&player_2_p, &player_2_dp, player_2_ddp, dt);

		// Simulate Ball
		{
			ball_p_x += ball_dp_x * dt;
			ball_p_y += ball_dp_y * dt;

			// collision detection
			if (aabb_vs_aabb(ball_p_x, ball_p_y, ball_half_size, ball_half_size, 80, player_1_p, player_half_size_x, player_half_size_y)) {
				ball_p_x = 80 - player_half_size_x - ball_half_size;
				ball_dp_x *= -1;
				// ball_dp_y = player_1_dp * .75f;  ball carries player's velocity
				ball_dp_y = (ball_p_y - player_1_p) + player_1_dp * .7f;
			}
			else if (aabb_vs_aabb(ball_p_x, ball_p_y, ball_half_size, ball_half_size, -80, player_2_p, player_half_size_x, player_half_size_y)) {
				ball_p_x = -80 + player_half_size_x + ball_half_size;
				ball_dp_x *= -1;
				ball_dp_y = (ball_p_y - player_2_p) + player_2_dp * .7f;
			}

			// ball bounce away from the top and bottom of the arena
			if (ball_p_y + ball_half_size > arena_half_size_y) {
				ball_p_y = arena_half_size_y - ball_half_size;
				ball_dp_y *= -1;
			}
			else if (ball_p_y - ball_half_size < -arena_half_size_y) {
				ball_p_y = -arena_half_size_y + ball_half_size;
				ball_dp_y *= -1;
			}

			// scoring
			if (ball_p_x + ball_half_size > arena_half_size_x) {
				ball_dp_x *= -1;
				ball_dp_y = 0;
				ball_p_x = 0;
				ball_p_y = 0;
				player_1_score++;
			}
			else if (ball_p_x - ball_half_size < -arena_half_size_x) {
				ball_dp_x *= -1;
				ball_dp_y = 0;
				ball_p_x = 0;
				ball_p_y = 0;
				player_2_score++;
			}
		}
		/*
		float at_x = -80;
		for (int i = 0; i < player_1_score; i++) {
			draw_rect(at_x, 47.f, 1.f, 1.f, 0xaaaaaa);
			at_x += 2.5f;
		}

		at_x = 80;
		for (int i = 0; i < player_2_score; i++) {
			draw_rect(at_x, 47.f, 1.f, 1.f, 0xaaaaaa);
			at_x -= 2.5f;
		}
		*/
		draw_number(player_1_score, -10, 40, 1.f, 0xbbffbb);
		draw_number(player_2_score, 10, 40, 1.f, 0xbbffbb);

		draw_rect(ball_p_x, ball_p_y, ball_half_size, ball_half_size, 0xffffff);

		draw_rect(80, player_1_p, player_half_size_x, player_half_size_y, 0x00ffff);
		draw_rect(-80, player_2_p, player_half_size_x, player_half_size_y, 0x00ffff);
	} else {
		if (pressed(BUTTON_LEFT) || pressed(BUTTON_RIGHT)) {
			hot_button = !hot_button;
		}

		if (pressed(BUTTON_ENTER)) {
			current_gamemode = GM_GAMEPLAY;
			enemy_is_ai = hot_button ? 0 : 1;
		}

		if (hot_button == 0) {
			draw_text("SINGLE PLAYER", -80, -10, 1, 0x00ffff);
			draw_text("MULTIPLAYER", 15, -10, 1, 0xaaaaaa);
		} else {
			draw_text("SINGLE PLAYER", -80, -10, 1, 0xaaaaaa);
			draw_text("MULTIPLAYER", 15, -10, 1, 0x00ffff);
		}

		draw_text("PINGPONG", -45, 40, 2, 0xffffff);
	}
}