#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>

#define WINDOW_TITLE "Chip8 Emulator"
#define scale 10
#define run_speed 1

//the game structure
typedef struct{
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
} Game;

//the chip8 structure
typedef struct{
    uint8_t memory[4096]; //4KB memory=4096 x 1Byte=4096 x 8bits
    uint8_t reg[16]; //16 x 8bit registers
    uint16_t index; //16 bit index register to store memory addresses
    uint16_t pc; //16 bit program counter to track current instruction
    uint8_t sp; //8 bit stack pointer to point to top of stack memory
    uint16_t stack[16]; //stack memory(an array of 16 16bit values used to store return adresses when calling subroutines)
    uint8_t dtimer; //8bit delay timer register
    uint8_t stimer; //8bit sound timer register
    uint32_t display[64*32]; //storing the state of the default 64x32 display
    uint8_t keypad[16]; //storing the state of keypad
} Chip8;

//returns false if SDL is initialised properly
//call SDL_Init first then create window then create renderer
bool init(Game* game){

    if(SDL_Init(SDL_INIT_VIDEO| SDL_INIT_AUDIO| SDL_INIT_TIMER)){
        fprintf(stderr, "SDL_Init failed \n %s\n", SDL_GetError());
        return true;
    }
    game->window=SDL_CreateWindow(WINDOW_TITLE, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, scale*64, scale*32, SDL_WINDOW_RESIZABLE);
    if(!game->window){
        fprintf(stderr, "SDL_CreateWindow failed \n %s\n", SDL_GetError());
        return true;
    }
    game->renderer=SDL_CreateRenderer(game->window, -1, SDL_RENDERER_ACCELERATED);
    if(!game->renderer){
        fprintf(stderr, "SDL_CreateRenderer failed \n %s\n", SDL_GetError());
        return true;
    }
    game->texture=SDL_CreateTexture(game->renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 64, 32);
    if(!game->texture){
        fprintf(stderr, "SDL_CreateTexture failed \n %s\n", SDL_GetError());
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
    SDL_DestroyTexture(game->texture);
    SDL_DestroyRenderer(game->renderer);
    SDL_DestroyWindow(game->window);
    SDL_Quit();
    exit(exit_status);

}
//main loop for detecting inputs and running the emulator
void run(Chip8* chip8){
    bool quit=false;
    SDL_Event event;
    while(!quit){
        while(SDL_PollEvent(&event)){
            if(event.type==SDL_QUIT){quit=true;}
            handle_event(chip8, &event);
        }

        for(int i=0;i<10*run_speed;i++){
            emulate_cycle(chip8);
        }

        update_timer(chip8);
        render_screen(chip8);

        //so the main loop runs with a 16ms delay which means
        /*
        Here is what the timeline of one loop iteration looks like:
        T = 0.0ms: The loop starts. SDL_PollEvent checks your keyboard.
        T = 0.1ms: The CPU executes 10 instructions and updates the screen array.
        T = 0.2ms: SDL renders the pixels to the window.
        T = 0.2ms to 16.2ms: SDL_Delay(16) puts your program completely to sleep. It does nothing.
        T = 16.2ms: The loop restarts, and SDL_PollEvent checks your keyboard again.
        */

        SDL_Delay(16);
    }

}

//keyboard/input handling function
void handle_event(Chip8* chip8, SDL_Event* event){

    /*
    SDL_Event is a enum that has a member called type
    if(event->type==SDL_KEYDOWN) it means a keyboard key has been pressed.
    SDL_KEYDOWN is an instance of SDL_KeyboardEvent structure.
    SDL_KeyboardEvent structure has member called keysym which denotes which key was pressed.
    keysym is an instance of SDL_Keysym struct which has a member called sym that denotes virtual keycode of the key pressed.
    sym is an instance of SDL_KeyCode enum.
    Refer to https://wiki.libsdl.org/SDL2/SDL_KeyCode to find the keycodes of keyboard keys.
    */

    uint8_t key_state=(event->type==SDL_KEYDOWN)? 1:0;

    /*The keyboard is supposed to look like this
    123C 1234
    456D qwer
    789E asdf
    A0BF zxcv
    */
    switch(event->key.keysym.sym){
        case(SDLK_1): chip8->keypad[0]=key_state; break;
        case(SDLK_2): chip8->keypad[1]=key_state; break;
        case(SDLK_3): chip8->keypad[2]=key_state; break;
        case(SDLK_4): chip8->keypad[3]=key_state; break;

        case(SDLK_q): chip8->keypad[4]=key_state; break;
        case(SDLK_w): chip8->keypad[5]=key_state; break;
        case(SDLK_e): chip8->keypad[6]=key_state; break;
        case(SDLK_r): chip8->keypad[7]=key_state; break;

        case(SDLK_a): chip8->keypad[8]=key_state; break;
        case(SDLK_s): chip8->keypad[9]=key_state; break;
        case(SDLK_d): chip8->keypad[10]=key_state; break;
        case(SDLK_f): chip8->keypad[11]=key_state; break;

        case(SDLK_z): chip8->keypad[12]=key_state; break;
        case(SDLK_x): chip8->keypad[13]=key_state; break;
        case(SDLK_c): chip8->keypad[14]=key_state; break;
        case(SDLK_v): chip8->keypad[15]=key_state; break;


    }

}

/*void emulate_cycle(Chip8* chip8){
    uint16_t instruction= ((chip8->memory[chip8->pc])<<8)||(chip8->memory[(chip8->pc)+1]);
    chip8->pc+=2;

    switch(instruction){
        case()
    }


}*/


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
