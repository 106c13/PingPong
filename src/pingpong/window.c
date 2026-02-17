#include <stdlib.h>
#include "pingpong.h"

SDLContext* createWindow(int width, int height, const char* title) {
    SDLContext* ctx = malloc(sizeof(SDLContext));

    SDL_Init(SDL_INIT_VIDEO);

    ctx->window = SDL_CreateWindow(
        title,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        width, height,
        SDL_WINDOW_OPENGL
    );

   
    ctx->renderer = SDL_CreateRenderer(
            ctx->window, 
            -1, 
            SDL_RENDERER_ACCELERATED
    );

    SDL_SetRenderDrawColor(ctx->renderer, 84, 92, 105, 255);
    SDL_RenderClear(ctx->renderer);
    SDL_RenderPresent(ctx->renderer);

    return ctx;
}

void destroyWindow(SDLContext* ctx) {
    SDL_DestroyRenderer(ctx->renderer);
    SDL_DestroyWindow(ctx->window);
    SDL_Quit();
    free(ctx);
}

