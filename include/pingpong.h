#ifndef PINGPONG_H
#define PINGPONG_H

#include <SDL2/SDL.h>

#define WIDTH 900
#define HEIGHT 600

#define PADDLE_WIDTH 10
#define PADDLE_HEIGHT 100



typedef struct  s_sdl {
    SDL_Window*     window;
    SDL_Renderer*   renderer;
} SDLContext;

typedef struct s_paddle {
    SDL_Rect rect;
    int color;
} Paddle;

typedef struct s_ball {
    int x;
    int y;
    int radius;
    int color;
} Ball;

/* window.c */
SDLContext* createWindow(int width, int height, const char* title);
void        destroyWindow(SDLContext* ctx);

/* paddle.c */
void    drawPaddle(SDLContext* ctx, Paddle* pad);
Paddle  initPaddle(int x);

/* ball.c */
void    drawBall(SDLContext* ctx, Ball* ball);
Ball    initBall(int x, int y, int r);

/* game.c */
void    pingpong(int sfd);

#endif

