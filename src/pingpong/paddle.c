#include "pingpong.h"

void drawPaddle(SDLContext* ctx, Paddle* pad) {
    int r, g, b;

    r = pad->color >> 16 & 0xFF;
    g = (pad->color >> 8) & 0xFF;
    b = pad->color & 0xFF;

    SDL_SetRenderDrawColor(ctx->renderer, r, g, b, 255);
    SDL_RenderFillRect(ctx->renderer, &pad->rect);
}

Paddle initPaddle(int x, int y) {
    Paddle pad;

    pad.rect.x = x;
    pad.rect.y = y;
    pad.rect.w = PADDLE_WIDTH;
    pad.rect.h = PADDLE_HEIGHT;
    pad.color = 0xFFFFFF;

    return pad;
}
