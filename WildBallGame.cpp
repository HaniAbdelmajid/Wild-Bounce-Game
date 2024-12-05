#include <SDL.h>
#include <stdio.h>
#include <cstdlib>
#include <ctime>

// Struct to hold all game-related variables
struct GameState {
    bool quit; // Flag to indicate when to exit the game loop
    int no_of_bricks; // Total number of bricks
    int delete_brick_count; // Counter for removed bricks

    int backround_width; // Background width
    int backround_height; // Background height
    int backround_widthmin; // Minimum x-coordinate of the background
    int backround_heightmin; // Minimum y-coordinate of the background

    int paddlebatx; // Paddle's x-coordinate
    int paddlebaty; // Paddle's y-coordinate

    int ballx; // Ball's x-coordinate
    int bally; // Ball's y-coordinate
    int ballvelocx; // Ball's x-velocity
    int ballvelocy; // Ball's y-velocity

    SDL_Rect ballrect; // Rectangle for the ball
    SDL_Rect brickrect[3][7]; // Array of rectangles for bricks

    SDL_Window *window; // SDL window
    SDL_Renderer *rend; // SDL renderer

    SDL_Surface *brick; // Brick surface
    SDL_Surface *ball; // Ball surface
    SDL_Surface *backround; // Background surface
    SDL_Surface *paddlebat; // Paddle surface

    SDL_Texture *bricktexture; // Brick texture
    SDL_Texture *balltexture; // Ball texture
    SDL_Texture *backroundtexture; // Background texture
    SDL_Texture *paddlebattexture; // Paddle texture
};

// Initialize the brick layout
void InitializeBrick(GameState &game) {
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 7; j++) {
            game.brickrect[i][j] = {50 + j * 100, 50 + i * 50, 120, 50};
        }
    }
}

// Handle user input for quitting and moving the paddle
void EventHandler(GameState &game) {
    SDL_Event event;
    SDL_PollEvent(&event);
    
    if (event.type == SDL_QUIT) {
        game.quit = true; // Exit the game loop
    } 
    
    else if (event.type == SDL_KEYDOWN) {
        int paddle_width = 60;

        // Move paddle left
        if (event.key.keysym.sym == SDLK_LEFT && game.paddlebatx > 0) {
            game.paddlebatx -= 80;
            if (game.paddlebatx < 0) {
                game.paddlebatx = 0;
            }
        }

        // Move paddle right
        if (event.key.keysym.sym == SDLK_RIGHT && game.paddlebatx < game.backround_width - paddle_width) {
            game.paddlebatx += 80;
            if (game.paddlebatx > game.backround_width - paddle_width) {
                game.paddlebatx = game.backround_width - paddle_width;
            }
        }
    }
}

// Update ball's position based on its velocity
void moveBall(GameState &game) {
    game.ballx += game.ballvelocx;
    game.bally += game.ballvelocy;
}

// Display the game-over screen and exit the game
void gameOver(GameState &game) {
    SDL_Surface *gameover = SDL_LoadBMP("gameover.bmp");
    SDL_Texture *gameovertexture = SDL_CreateTextureFromSurface(game.rend, gameover);
    SDL_Rect gameoverrect = {0, 0, game.backround_width, game.backround_height};
    SDL_RenderCopy(game.rend, gameovertexture, NULL, &gameoverrect);
    SDL_RenderPresent(game.rend);

    SDL_Delay(10000); // Keep the game-over screen for 10 seconds
    SDL_DestroyTexture(gameovertexture);
    SDL_FreeSurface(gameover);
    SDL_Quit();
    exit(0);
}

// Handle ball collisions with walls, paddle, and out-of-bounds
void ball_collision(GameState &game) {
    if (game.ballx < game.backround_widthmin || game.ballx > game.backround_width - 30) {
        game.ballvelocx = -game.ballvelocx; // Reverse horizontal direction
    }

    if (game.bally < game.backround_heightmin) {
        game.ballvelocy = -game.ballvelocy; // Reverse vertical direction
    }

    if (game.bally > game.backround_height + 60) {
        gameOver(game); // Ball missed the paddle; end the game
    }

    int ballscaling = 20;

    // Check if the ball hits the paddle
    if (game.bally + ballscaling >= game.paddlebaty && game.bally + ballscaling <= game.paddlebaty + 30 &&
        game.ballx + ballscaling >= game.paddlebatx && game.ballx + ballscaling <= game.paddlebatx + 60) {
        game.ballvelocy = -game.ballvelocy; // Reverse vertical direction

        // Add randomness to the ball's horizontal velocity
        int hit_position = game.ballx - game.paddlebatx;
        int paddle_width = 60;

        if (hit_position < paddle_width / 3) {
            game.ballvelocx = (game.ballvelocx > 0 ? -1 : 1) * (rand() % 3 + 2); // Ball hits left side
        } else if (hit_position > 2 * paddle_width / 3) {
            game.ballvelocx = (game.ballvelocx > 0 ? 1 : -1) * (rand() % 3 + 2); // Ball hits right side
        } else {
            game.ballvelocx += (rand() % 5 - 2); // Ball hits the center
        }
    }
}

// Check if two rectangles are colliding
bool ball_brick_collision_detect(SDL_Rect rect1, SDL_Rect rect2) {
    return !(rect1.x > rect2.x + rect2.w || rect1.x + rect1.w < rect2.x ||
             rect1.y > rect2.y + rect2.h || rect1.y + rect1.h < rect2.y);
}

// Handle collisions between the ball and bricks
void ball_brick_collision(GameState &game) {
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 7; j++) {
            if (ball_brick_collision_detect(game.brickrect[i][j], game.ballrect)) {
                game.brickrect[i][j] = {0, 0, 0, 0}; // Remove the brick
                game.delete_brick_count++;

                game.ballvelocy = -game.ballvelocy; // Reverse vertical direction
                game.ballvelocx += (rand() % 5 - 2); // Add randomness to horizontal velocity
            }
        }
    }
}

// Free all resources before exiting the game
void Destroy(GameState &game) {
    SDL_DestroyTexture(game.paddlebattexture);
    SDL_DestroyTexture(game.bricktexture);
    SDL_DestroyTexture(game.backroundtexture);
    SDL_DestroyTexture(game.balltexture);
    SDL_FreeSurface(game.paddlebat);
    SDL_FreeSurface(game.brick);
    SDL_FreeSurface(game.backround);
    SDL_FreeSurface(game.ball);
    SDL_DestroyRenderer(game.rend);
    SDL_DestroyWindow(game.window);
}

// Display the winning screen
void winning(GameState &game) {
    SDL_Surface *win = SDL_LoadBMP("win.bmp");
    SDL_Texture *wintexture = SDL_CreateTextureFromSurface(game.rend, win);
    SDL_FreeSurface(win);

    SDL_Rect winrect = {50, 50, 700, 500};

    Uint32 startTime = SDL_GetTicks();
    while (SDL_GetTicks() - startTime < 5000) { // Show for 5 seconds
        SDL_RenderClear(game.rend);
        SDL_RenderCopy(game.rend, wintexture, NULL, &winrect);
        SDL_RenderPresent(game.rend);
    }

    SDL_DestroyTexture(wintexture);
}

int main(int argc, char **argv) {
    SDL_Init(SDL_INIT_VIDEO);

    srand(static_cast<unsigned int>(time(nullptr))); // Seed the random number generator

    GameState game = {};
    game.quit = false;
    game.no_of_bricks = 21;
    game.delete_brick_count = 0;

    game.backround_width = 800;
    game.backround_height = 600;

    game.paddlebatx = game.backround_width / 2;
    game.paddlebaty = game.backround_height - 30;

    game.ballx = game.paddlebatx + 20;
    game.bally = game.paddlebaty - 30;
    game.ballvelocx = 1;
    game.ballvelocy = 1;

    game.window = SDL_CreateWindow("WildBall", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, 0);
    game.rend = SDL_CreateRenderer(game.window, -1, 0);

    SDL_Rect backroundrect = {0, 0, 800, 600};
    InitializeBrick(game);

    game.ball = SDL_LoadBMP("ball.bmp");
    game.backround = SDL_LoadBMP("backround.bmp");
    game.paddlebat = SDL_LoadBMP("paddlebat.bmp");
    game.brick = SDL_LoadBMP("brick.bmp");

    game.balltexture = SDL_CreateTextureFromSurface(game.rend, game.ball);
    game.backroundtexture = SDL_CreateTextureFromSurface(game.rend, game.backround);
    game.paddlebattexture = SDL_CreateTextureFromSurface(game.rend, game.paddlebat);
    game.bricktexture = SDL_CreateTextureFromSurface(game.rend, game.brick);

	// While loop to exit loop if it does not equal quit
    while (!game.quit) {
        Uint32 frameStart = SDL_GetTicks();

        EventHandler(game);

        game.ballrect = {game.ballx, game.bally, 20, 30};
        SDL_Rect paddlebatrect = {game.paddlebatx, game.paddlebaty, 60, 30};

        moveBall(game);
        ball_collision(game);
        ball_brick_collision(game);

        if (game.delete_brick_count == game.no_of_bricks) {
            winning(game);
            break;
        }

        SDL_RenderClear(game.rend);
        SDL_RenderCopy(game.rend, game.backroundtexture, NULL, &backroundrect);

        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 7; j++) {
                SDL_RenderCopy(game.rend, game.bricktexture, NULL, &game.brickrect[i][j]);
            }
        }

        SDL_RenderCopy(game.rend, game.balltexture, NULL, &game.ballrect);
        SDL_RenderCopy(game.rend, game.paddlebattexture, NULL, &paddlebatrect);
        SDL_RenderPresent(game.rend);
		
		// To help reduce the ball being to quickly for the user to even react to by adding frame delay
        Uint32 frameTime = SDL_GetTicks() - frameStart;
        if (frameTime < 2) {
            SDL_Delay(2 - frameTime);
        }
    }

    Destroy(game);
    
    //Quit the game
    SDL_Quit();
    return 0;
}