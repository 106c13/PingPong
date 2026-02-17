#include <math.h>
#include "pingpong.h"

Ball initBall(int x, int y, int r) {
    Ball ball;

    ball.x = x;
    ball.y = y;
    ball.radius = r;
    ball.color = 0xFFFFFF;
    
    return ball;
}

void drawAAPixel(SDL_Renderer* renderer, int x, int y, int color, float brightness)
{
    int r = ((color >> 16) & 0xFF) * brightness;
    int g = ((color >> 8) & 0xFF) * brightness;
    int b = (color & 0xFF) * brightness;

    SDL_SetRenderDrawColor(renderer, r, g, b, 255);
    SDL_RenderDrawPoint(renderer, x, y);
    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 255);
}

void drawBall(SDLContext* ctx, Ball* ball)
{
    int cx = ball->x;
    int cy = ball->y;
    int r  = ball->radius;
    int color = ball->color;

    SDL_SetRenderDrawBlendMode(ctx->renderer, SDL_BLENDMODE_BLEND);

    for (float x = -r; x <= r; x += 0.5)
    {
        float yf = sqrtf((float)(r*r - x*x));
        int y_int = (int)yf;
        float frac = yf - y_int;

        // draw main vertical span
        SDL_RenderDrawLine(ctx->renderer,
                           cx + x,
                           cy - y_int,
                           cx + x,
                           cy + y_int);

        // top edge AA
        drawAAPixel(ctx->renderer, cx + x, cy - y_int - 1, color, frac);
        // bottom edge AA
        drawAAPixel(ctx->renderer, cx + x, cy + y_int + 1, color, frac);
    }
}
