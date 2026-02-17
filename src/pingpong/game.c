#include <sys/socket.h>
#include <string.h>
#include <stdbool.h>
#include "pingpong.h"

static void handleEvents(bool* running, int* y) {
    SDL_Event e;


    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            *running = 0;
        } else if (e.type == SDL_KEYDOWN) {
            switch (e.key.keysym.sym) {
                case SDLK_ESCAPE:
                    *running = false; 
                    break;
                case SDLK_w:
                    if (*y > 0)
                        (*y) -= 10;
                    break;
                case SDLK_s:
                    if (*y + PADDLE_HEIGHT < HEIGHT)
                        (*y) += 10;
                    break;
            }
            break;
        }
    }
}

void pingpong(int sfd) {
    SDLContext* ctx = createWindow(WIDTH, HEIGHT, "Ping Pong");
    Paddle player = initPaddle(0);
    Paddle enemy = initPaddle(WIDTH - PADDLE_WIDTH);
    Ball ball = initBall( WIDTH / 2, HEIGHT / 2, 10);

	bool running = true;

	while (running) {
        char buf[32];

        int n = recv(sfd, buf, 32, MSG_DONTWAIT);

        //printf("Recv: %s\n", buf);
        if (n > 0) {
            enemy.rect.y = (int)strtol(buf, NULL, 10);
        }

        handleEvents(&running, &player.rect.y);

        SDL_SetRenderDrawColor(ctx->renderer, 30, 30, 30, 255);
        SDL_RenderClear(ctx->renderer);

        drawPaddle(ctx, &player);
        drawPaddle(ctx, &enemy);
        drawBall(ctx, &ball);

        snprintf(buf, sizeof(buf), "%d", player.rect.y);
        send(sfd, buf, strlen(buf), 0);
        
        SDL_RenderPresent(ctx->renderer);
        SDL_Delay(10);
	}
    destroyWindow(ctx);
}

