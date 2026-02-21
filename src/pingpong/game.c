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
                        (*y) -= 15;
                    break;
                case SDLK_s:
                    if (*y + PADDLE_HEIGHT < HEIGHT)
                        (*y) += 15;
                    break;
            }
            break;
        }
    }
}

void pingpong(int sfd, int* pos) {
    SDLContext* ctx = createWindow(WIDTH, HEIGHT, "Ping Pong");
    Paddle player = initPaddle(5, pos[0]);
    Paddle enemy = initPaddle(WIDTH - PADDLE_WIDTH - 5, pos[1]);
    Ball ball = initBall(pos[2], pos[3], 10);

	bool running = true;

	while (running) {
        char buf[32];


        handleEvents(&running, &player.rect.y);

        SDL_SetRenderDrawColor(ctx->renderer, 30, 30, 30, 255);
        SDL_RenderClear(ctx->renderer);

        drawPaddle(ctx, &player);
        drawPaddle(ctx, &enemy);
        drawBall(ctx, &ball);

        snprintf(buf, sizeof(buf), "%d\n", player.rect.y);
        send(sfd, buf, strlen(buf), 0);
        
        SDL_RenderPresent(ctx->renderer);
        SDL_Delay(10);

        int n = recv(sfd, buf, sizeof(buf), 0);

        if (n > 0) {
			char* p = strchr(buf, ':'); //buf;
            //player.rect.y = (int)strtol(p + 1, &p, 10);
			enemy.rect.y = (int)strtol(p + 1, &p, 10);
			ball.x = (int)strtol(p + 1, &p, 10);
			ball.y = (int)strtol(p + 1, &p, 10);
        } else {
			break;
		}
	}

    destroyWindow(ctx);
}

