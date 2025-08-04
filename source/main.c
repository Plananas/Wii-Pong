#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <gccore.h>
#include <wiiuse/wpad.h>
#include <grrlib.h>
#include <ogc/system.h>  // Add this include at the top

#define PADDLE_WIDTH  10.0f
#define PADDLE_HEIGHT 60.0f
#define SCREEN_HEIGHT 480
#define PADDLE_SPEED 5.0f
#define BALL_SIZE 10.0f

// Game state variables
f32 left_paddle_y, right_paddle_y;
f32 ball_x, ball_y, ball_dx, ball_dy;

void initialize_graphics() {
    GRRLIB_Init();
}

void draw_paddle(f32 x, f32 y) {
    GRRLIB_Rectangle(x, y, PADDLE_WIDTH, PADDLE_HEIGHT, 0xFFFFFFFF, true);
}

void draw_ball(f32 x, f32 y) {
    GRRLIB_Rectangle(x, y, BALL_SIZE, BALL_SIZE, 0xFFFFFFFF, true);
}

void draw_objects() {
    draw_paddle(100, left_paddle_y);   // Left Paddle
    draw_paddle(500, right_paddle_y);  // Right Paddle
    draw_ball(ball_x, ball_y);
}

void initialize_objects() {
    left_paddle_y = 200;
    right_paddle_y = 200;
    ball_x = 320;
    ball_y = 240;
    ball_dx = 3;
    ball_dy = 2;

    draw_paddle(100, (int) left_paddle_y);   // Left Paddle
    draw_paddle(500, (int) right_paddle_y);  // Right Paddle
    draw_ball((int)ball_x, (int)ball_y);
    
}

void update_ball() {
    ball_x += ball_dx;
    ball_y += ball_dy;

    // Bounce off top and bottom
    if (ball_y <= 0 || ball_y >= SCREEN_HEIGHT - BALL_SIZE) {
        ball_dy = -ball_dy;
    }
    // Bounce off left and right
    if (ball_x <= 0 || ball_x >= 640 - BALL_SIZE) {
        ball_dx = -ball_dx;
    }
    // Paddle collision (example for left paddle)
    if (ball_x <= 110 && ball_x >= 100 && ball_y + BALL_SIZE >= left_paddle_y && ball_y <= left_paddle_y + PADDLE_HEIGHT) {
        ball_dx = -ball_dx;
        ball_x = 110;
    }
}

int main() {
    VIDEO_Init();
    WPAD_Init();
    initialize_graphics();
    initialize_objects();

    while (1) {
        WPAD_ScanPads();
        u32 held = WPAD_ButtonsHeld(0);
        u32 pressed = WPAD_ButtonsDown(0);

        if (pressed & WPAD_BUTTON_HOME) {
            GRRLIB_Exit();           // Clean up graphics
            SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0); // Return to loader/menu
        }

        // Player 1 (Left paddle) movement
        if (held & WPAD_BUTTON_UP)
            left_paddle_y -= PADDLE_SPEED;
        if (held & WPAD_BUTTON_DOWN)
            left_paddle_y += PADDLE_SPEED;

        // Player 2 (Right paddle) movement
        if (held & WPAD_BUTTON_PLUS)
            right_paddle_y -= PADDLE_SPEED;
        if (held & WPAD_BUTTON_MINUS)
            right_paddle_y += PADDLE_SPEED;

        // Clamp paddle positions within screen bounds
        if (left_paddle_y < 0)
            left_paddle_y = 0;
        if (left_paddle_y > SCREEN_HEIGHT - PADDLE_HEIGHT)
            left_paddle_y = SCREEN_HEIGHT - PADDLE_HEIGHT;

        if (right_paddle_y < 0)
            right_paddle_y = 0;
        if (right_paddle_y > SCREEN_HEIGHT - PADDLE_HEIGHT)
            right_paddle_y = SCREEN_HEIGHT - PADDLE_HEIGHT;

        GRRLIB_FillScreen(0x141414FF);  // Dark gray background


        // Ball Logic
        // Update ball position
        update_ball();
        draw_objects();

        GRRLIB_Render();
        VIDEO_WaitVSync();
    }

    GRRLIB_Exit();
    YS_ResetSystem(SYS_RETURNTOMENU, 0, 0); // Return to loader/menu
    return 0;
}
