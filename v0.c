#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// Dimensions for the map and rendering
#define ROWS 15
#define COLS 15
#define TILE_SIZE 40

// Global variables to follow simple structured programming
char map[ROWS][COLS];
int score = 0;
int lives = 3;
bool isRunning = true;

typedef struct {
    int x;
    int y;
} Entity;

Entity player;
Entity ghost;

// Function to load the map from a text file
void loadMap(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error: Could not open map file!\n");
        isRunning = false;
        return;
    }

    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            fscanf(file, " %c", &map[i][j]);
            
            // Set initial positions based on the file symbols
            if (map[i][j] == 'P') {
                player.x = j;
                player.y = i;
                map[i][j] = ' '; // Clear starting tile
            } else if (map[i][j] == 'F') {
                ghost.x = j;
                ghost.y = i;
                map[i][j] = ' '; // Clear starting tile
            }
        }
    }
    fclose(file);
}

// Function to handle keyboard input
void handleInput(SDL_Event* event) {
    if (event->type == SDL_QUIT) {
        isRunning = false;
    } else if (event->type == SDL_KEYDOWN) {
        int newX = player.x;
        int newY = player.y;

        switch (event->key.keysym.sym) {
            case SDLK_UP:    newY--; break;
            case SDLK_DOWN:  newY++; break;
            case SDLK_LEFT:  newX--; break;
            case SDLK_RIGHT: newX++; break;
            case SDLK_ESCAPE: isRunning = false; break;
        }

        // Prevent moving into walls
        if (map[newY][newX] != '#') {
            player.x = newX;
            player.y = newY;
        }
    }
}

// Function to update the game state
void updateLogic() {
    // Consume pellets and increase score
    if (map[player.y][player.x] == '.') {
        map[player.y][player.x] = ' ';
        score += 10;
        printf("Score: %d\n", score);
    }

    // Basic Ghost AI: Simple tracking rules
    int ghostNewX = ghost.x;
    int ghostNewY = ghost.y;

    // Move only occasionally to slow down the ghost
    if (rand() % 2 == 0) {
        if (player.x > ghost.x) ghostNewX++;
        else if (player.x < ghost.x) ghostNewX--;
        else if (player.y > ghost.y) ghostNewY++;
        else if (player.y < ghost.y) ghostNewY--;
    }

    // Prevent ghost from moving into walls
    if (map[ghostNewY][ghostNewX] != '#') {
        ghost.x = ghostNewX;
        ghost.y = ghostNewY;
    }

    // Check collision between player and ghost
    if (player.x == ghost.x && player.y == ghost.y) {
        lives--;
        printf("You hit a ghost! Lives remaining: %d\n", lives);
        
        if (lives <= 0) {
            printf("Game Over! Final Score: %d\n", score);
            isRunning = false;
        } else {
            // Send player back to a safe spot (temporary logic)
            player.x = 1; 
            player.y = 1;
        }
    }
}

// Function to draw elements on the screen
void renderGame(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Black background
    SDL_RenderClear(renderer);

    // Draw the map elements
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            SDL_Rect tileRect = { j * TILE_SIZE, i * TILE_SIZE, TILE_SIZE, TILE_SIZE };

            if (map[i][j] == '#') {
                SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255); // Blue walls
                SDL_RenderFillRect(renderer, &tileRect);
            } else if (map[i][j] == '.') {
                SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255); // Yellow pellets
                SDL_Rect pelletRect = { j * TILE_SIZE + 15, i * TILE_SIZE + 15, 10, 10 };
                SDL_RenderFillRect(renderer, &pelletRect);
            }
        }
    }

    // Draw Player (Green Square)
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    SDL_Rect playerRect = { player.x * TILE_SIZE, player.y * TILE_SIZE, TILE_SIZE, TILE_SIZE };
    SDL_RenderFillRect(renderer, &playerRect);

    // Draw Ghost (Red Square)
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_Rect ghostRect = { ghost.x * TILE_SIZE, ghost.y * TILE_SIZE, TILE_SIZE, TILE_SIZE };
    SDL_RenderFillRect(renderer, &ghostRect);

    SDL_RenderPresent(renderer);
}

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL Error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Pac-Man Clone", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, COLS * TILE_SIZE, ROWS * TILE_SIZE, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // Ensure you create this file in the same folder as your executable!
    loadMap("map.txt");

    SDL_Event event;
    while (isRunning) {
        while (SDL_PollEvent(&event) != 0) {
            handleInput(&event);
        }

        updateLogic();
        renderGame(renderer);

        SDL_Delay(100); // Frames delay (approx 10 FPS) for retro feel
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}