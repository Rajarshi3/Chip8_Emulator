#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>

#define WINDOW_TITLE "Chip 8 Emulator"

#define WINDOW_WIDTH 64
#define WINDOW_HEIGHT 32

//the game structure
typedef struct{
    SDL_Window* window;
    SDL_Renderer* renderer;
} Game;

//the chip8 structure
typedef struct{
    uint8_t memory[4096];
    uint8_t reg[16];
    uint16_t index;
    uint16_t pc;
    uint8_t sp;
    uint16_t stack[16];
    uint8_t dtimer;
    uint8_t stimer;
    uint32_t display[64*32];
    uint8_t keypad[16];
} Chip8;

//returns false if SDL is initialised properly
//call SDL_Init first then create window then create renderer
bool init(Game* game){

    if(SDL_Init(SDL_INIT_VIDEO| SDL_INIT_AUDIO| SDL_INIT_TIMER)){
        fprintf(stderr, "SDL_Init failed \n %s\n", SDL_GetError());
        return true;
    }
    game->window=SDL_CreateWindow(WINDOW_TITLE, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE);
    if(!game->window){
        fprintf(stderr, "SDL_CreateWindow failed \n %s\n", SDL_GetError());
        return true;
    }
    game->renderer=SDL_CreateRenderer(game->window, -1, SDL_RENDERER_ACCELERATED);
    if(!game->renderer){
        fprintf(stderr, "SDL_CreateRenderer failed \n %s\n", SDL_GetError());
        return true;
    }
    return false;

}

//final cleanup function
void cleanup(Game* game, int exit_status){
    /*if(game->renderer){
        SDL_DestroyRenderer(game->renderer);
    }
    if(game->window){
        SDL_DestroyWindow(game->window);
    }
    SDL_Quit();
    exit(exit_status);*/
    SDL_DestroyRenderer(game->renderer);
    SDL_DestroyWindow(game->window);
    SDL_Quit();
    exit(exit_status);

}

//main function
int main(int argc, char* argv[]){
    Game game={0};
    if(init(&game)){
        printf("init didnt work\n");
        cleanup(&game, EXIT_FAILURE);
        exit(1);
    }
    SDL_RenderClear(game.renderer);
    SDL_RenderPresent(game.renderer);
    SDL_Delay(2000);
    printf("Went well\n");
    cleanup(&game, EXIT_SUCCESS);
    exit(0);
}
