#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdlib.h>
#include <time.h>

#define SCREEN_WIDTH 700
#define SCREEN_HEIGHT 750
#define SPACESHIP_WIDTH 60
#define SPACESHIP_HEIGHT 60
#define BULLET_WIDTH 20
#define BULLET_HEIGHT 20
#define BULLET_SPEED 5
#define SPACESHIP_SPEED 5
#define MAX_BULLETS 10
#define MAX_DEBRIS 15
#define TICK_INTERVAL 16

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
SDL_Rect bullets[MAX_BULLETS];
SDL_Rect debris[MAX_DEBRIS];
TTF_Font* font = NULL;
int score = 0;
int gameOver = 0;
int bulletIndex = 0;

SDL_Texture* spaceshipTexture = NULL;
SDL_Texture* debrisTexture = NULL;
SDL_Texture* bulletTexture = NULL;
SDL_Texture* backgroundTexture = NULL;
SDL_Rect spaceshipRect;

// Variable for smooth spaceship movement
int spaceshipVelocityX = 0;
int spaceshipVelocityY = 0;

Uint32 next_game_tick = 0;

int initSDL() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL initialization failed: %s\n", SDL_GetError());
        return -1;
    }

    // Initialize SDL_image
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        printf("SDL_image initialization failed: %s\n", IMG_GetError());
        return -1;
    }

    // Initialize SDL_ttf
    if (TTF_Init() == -1) {
        printf("SDL_ttf initialization failed: %s\n", TTF_GetError());
        return -1;
    }

    window = SDL_CreateWindow("Space Debris Shooter", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);

    if (window == NULL) {
        printf("Window creation failed: %s\n", SDL_GetError());
        return -1;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if (renderer == NULL) {
        printf("Renderer creation failed: %s\n", SDL_GetError());
        return -1;
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    return 0;
}

void loadAssets() {
    SDL_Surface* backgroundSurface = IMG_Load("two.png");
    if (backgroundSurface == NULL) {
        printf("Failed to load background image: %s\n", IMG_GetError());
        return;
    }

    backgroundTexture = SDL_CreateTextureFromSurface(renderer, backgroundSurface);
    SDL_FreeSurface(backgroundSurface);

    SDL_Surface* spaceshipSurface = IMG_Load("rocket.png");
    if (spaceshipSurface == NULL) {
        printf("Failed to load spaceship image: %s\n", IMG_GetError());
        return;
    }

    spaceshipTexture = SDL_CreateTextureFromSurface(renderer, spaceshipSurface);
    SDL_FreeSurface(spaceshipSurface);

    SDL_Surface* debrisSurface = IMG_Load("asteroid.png");
    if (debrisSurface == NULL) {
        printf("Failed to load debris image: %s\n", IMG_GetError());
        return;
    }

    debrisTexture = SDL_CreateTextureFromSurface(renderer, debrisSurface);
    SDL_FreeSurface(debrisSurface);

    SDL_Surface* bulletSurface = IMG_Load("bullet.png");
    if (bulletSurface == NULL) {
        printf("Failed to load bullet image: %s\n", IMG_GetError());
        return;
    }

    bulletTexture = SDL_CreateTextureFromSurface(renderer, bulletSurface);
    SDL_FreeSurface(bulletSurface);

    spaceshipRect.x = (SCREEN_WIDTH - SPACESHIP_WIDTH) / 2;
    spaceshipRect.y = SCREEN_HEIGHT - SPACESHIP_HEIGHT - 20;
    spaceshipRect.w = SPACESHIP_WIDTH;
    spaceshipRect.h = SPACESHIP_HEIGHT;

    for (int i = 0; i < MAX_BULLETS; ++i) {
        bullets[i].w = BULLET_WIDTH;
        bullets[i].h = BULLET_HEIGHT;
        bullets[i].x = -BULLET_WIDTH;
        bullets[i].y = -BULLET_HEIGHT;
    }

    srand(time(NULL));
    for (int i = 0; i < MAX_DEBRIS; ++i) {
        int debrisSize = rand() % 30 + 20;
        debris[i].w = debrisSize;
        debris[i].h = debrisSize;
        debris[i].x = rand() % (SCREEN_WIDTH - debris[i].w);
        debris[i].y = -rand() % SCREEN_HEIGHT;
    }

    font = TTF_OpenFont("game_over.ttf", 48);
    if (font == NULL) {
        printf("Failed to load font: %s\n", TTF_GetError());
        return;
    }
}

void handleInput(SDL_Event* event) {
    const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);

    spaceshipVelocityX = 0;
    spaceshipVelocityY = 0;

    if (currentKeyStates[SDL_SCANCODE_LEFT] || currentKeyStates[SDL_SCANCODE_A]) {
        spaceshipVelocityX = -SPACESHIP_SPEED;
    }
    if (currentKeyStates[SDL_SCANCODE_RIGHT] || currentKeyStates[SDL_SCANCODE_D]) {
        spaceshipVelocityX = SPACESHIP_SPEED;
    }
    if (currentKeyStates[SDL_SCANCODE_UP] || currentKeyStates[SDL_SCANCODE_W]) {
        spaceshipVelocityY = -SPACESHIP_SPEED;
    }
    if (currentKeyStates[SDL_SCANCODE_DOWN] || currentKeyStates[SDL_SCANCODE_S]) {
        spaceshipVelocityY = SPACESHIP_SPEED;
    }

    if (!gameOver && event->type == SDL_KEYDOWN && event->key.repeat == 0) {
        switch (event->key.keysym.sym) {
            case SDLK_SPACE:
                bullets[bulletIndex].x = spaceshipRect.x + (SPACESHIP_WIDTH - BULLET_WIDTH) / 2;
                bullets[bulletIndex].y = spaceshipRect.y - BULLET_HEIGHT;
                bulletIndex = (bulletIndex + 1) % MAX_BULLETS;
                break;
        }
    }
}

void updateGame() {
    if (gameOver) {
        return;
    }

    spaceshipRect.x += spaceshipVelocityX;
    spaceshipRect.y += spaceshipVelocityY;

    if (spaceshipRect.x < 0) {
        spaceshipRect.x = 0;
    }
    if (spaceshipRect.x > SCREEN_WIDTH - SPACESHIP_WIDTH) {
        spaceshipRect.x = SCREEN_WIDTH - SPACESHIP_WIDTH;
    }
    if (spaceshipRect.y < 0) {
        spaceshipRect.y = 0;
    }
    if (spaceshipRect.y > SCREEN_HEIGHT - SPACESHIP_HEIGHT) {
        spaceshipRect.y = SCREEN_HEIGHT - SPACESHIP_HEIGHT;
    }

    for (int i = 0; i < MAX_BULLETS; ++i) {
        if (bullets[i].y >= 0) {
            bullets[i].y -= BULLET_SPEED;
        }
    }

    for (int i = 0; i < MAX_DEBRIS; ++i) {
        if (debris[i].y <= SCREEN_HEIGHT) {
            debris[i].y += 2;

            for (int j = 0; j < MAX_BULLETS; ++j) {
                if (bullets[j].y >= 0 && SDL_HasIntersection(&bullets[j], &debris[i])) {
                    debris[i].w = rand() % 30 + 20;
                    debris[i].h = debris[i].w;
                    debris[i].x = rand() % (SCREEN_WIDTH - debris[i].w);
                    debris[i].y = -rand() % SCREEN_HEIGHT;

                    score++;
                }
            }

            if (SDL_HasIntersection(&spaceshipRect, &debris[i])) {
                gameOver = 1;
            }
        } else {
            debris[i].w = rand() % 30 + 20;
            debris[i].h = debris[i].w;
            debris[i].x = rand() % (SCREEN_WIDTH - debris[i].w);
            debris[i].y = -rand() % SCREEN_HEIGHT;
        }
    }
}

void render() {
    SDL_RenderCopy(renderer, backgroundTexture, NULL, NULL);
    SDL_RenderCopy(renderer, spaceshipTexture, NULL, &spaceshipRect);

    for (int i = 0; i < MAX_BULLETS; ++i) {
        if (bullets[i].y >= 0) {
            SDL_RenderCopy(renderer, bulletTexture, NULL, &bullets[i]);
        }
    }

    for (int i = 0; i < MAX_DEBRIS; ++i) {
        SDL_Rect destRect = {debris[i].x, debris[i].y, debris[i].w, debris[i].h};
        SDL_RenderCopy(renderer, debrisTexture, NULL, &destRect);
    }

    SDL_Color textColor = {255, 255, 255, 255};
    char scoreText[50];
    sprintf(scoreText, "SCORE: %d", score);
    SDL_Surface* scoreSurface = TTF_RenderText_Solid(font, scoreText, textColor);
    SDL_Texture* scoreTexture = SDL_CreateTextureFromSurface(renderer, scoreSurface);
    SDL_FreeSurface(scoreSurface);

    SDL_Rect scoreRect = {SCREEN_WIDTH - 120, 20, 100, 30};
    SDL_RenderCopy(renderer, scoreTexture, NULL, &scoreRect);
    SDL_DestroyTexture(scoreTexture);

    if (gameOver) {
        SDL_Color bgColor = {255, 255, 255, 255};
        SDL_SetRenderDrawColor(renderer, bgColor.r, bgColor.g, bgColor.b, bgColor.a);
        SDL_Rect bgRect = {0, SCREEN_HEIGHT / 2 - 50, SCREEN_WIDTH, 60};
        SDL_RenderFillRect(renderer, &bgRect);

        SDL_Color gameOverColor = {255, 0, 0, 255};
        char gameOverText[50];
        sprintf(gameOverText, "GAME OVER");
        SDL_Surface* gameOverSurface = TTF_RenderText_Solid(font, gameOverText, gameOverColor);
        SDL_Texture* gameOverTexture = SDL_CreateTextureFromSurface(renderer, gameOverSurface);
        SDL_FreeSurface(gameOverSurface);

        SDL_Rect gameOverRect = {(SCREEN_WIDTH - 200) / 2, SCREEN_HEIGHT / 2 - 55, 200, 60};
        SDL_RenderCopy(renderer, gameOverTexture, NULL, &gameOverRect);
        SDL_DestroyTexture(gameOverTexture);
    }

    SDL_RenderPresent(renderer);
}

void closeSDL() {
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_DestroyTexture(backgroundTexture);
    SDL_DestroyTexture(spaceshipTexture);
    SDL_DestroyTexture(debrisTexture);
    SDL_DestroyTexture(bulletTexture);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}

Uint32 time_left(void) {
    Uint32 now = SDL_GetTicks();
    return (next_game_tick > now) ? next_game_tick - now : 0;
}

int main(int argc, char* args[]) {
    if (initSDL() == -1) {
        return -1;
    }

    loadAssets();

    int quit = 0;
    SDL_Event e;

    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = 1;
            } else {
                handleInput(&e);
            }
        }

        Uint32 ticks = SDL_GetTicks();
        updateGame();
        render();

        next_game_tick += TICK_INTERVAL;
        Uint32 sleep_time = time_left();
        if (sleep_time > 0) {
            SDL_Delay(sleep_time);
        }
    }

    closeSDL();

    return 0;
}
