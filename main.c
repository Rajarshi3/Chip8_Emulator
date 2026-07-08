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

void emulate_cycle(Chip8* chip8){


    uint8_t first=(chip8->memory[chip8->pc])>>4;
    uint8_t second=(chip8->memory[chip8->pc]&15;
    uint8_t third=(chip8->memory[(chip8->pc)+1])>>4;
    uint8_t fourth=(chip8->memory[(chip8->pc)+1])&15;
    (chip8->pc)+=2;




    switch(first){
        case(0x0):
            if(third==0xE && fourth==0){
                //00E0 clear screen
                memset((chip8->display), 0, sizeof(chip8->display));
            }
            else if(third==0xE && fourth==0xE){
                //00EE return from a subroutine
                chip8->pc=stack[sp--];
            }
            break;

        case(0x1):/*1nnn jump to location nnn*/ (chip8->pc)=(second<<8)|(third<<4)|(fourth); break;
        case(0x2):/*2nnn CALL addr nnn*/chip8->stack[++(chip8->sp)]=chip8->pc; (chip8->pc)=(second<<8)|(third<<4)|(fourth); break;
        case(0x3):/*3xkk*/(chip8->pc)+=((chip8->reg[second])==(third<<4)|fourth)?2:0; break;
        case(0x4):/*4xkk*/(chip8->pc)+=((chip8->reg[second])!=(third<<4)|fourth)?2:0; break;
        case(0x5):/*5xy0*/(chip8->pc)+=((chip8->reg[second])==(chip8->reg[third]))?2:0; break;
        case(0x6):/*6xkk*/(chip8->reg[second])=(third<<4)|fourth; break;
        case(0x7):/*7xkk*/(chip8->reg[second])+=(third<<4)|fourth; break;
        case(0x8):
            switch(fourth){
                case(0x1):/*8xy1: Vx=Vx|Vy*/(chip8->reg[second])|= (chip8->reg[third]); break;
                case(0x2):/*8xy2: Vx=Vx&Vy*/(chip8->reg[second])&= (chip8->reg[third]); break;
                case(0x3):/*8xy3: Vx=Vx&Vy*/(chip8->reg[second])^= (chip8->reg[third]); break;
                case(0x4):
                    /*8xy4: Vx=Vx+Vy, VF=carry*/
                    uint16_t sum=(chip8->reg[second])+(chip8->reg[third]);
                    chip8->reg[second]=sum&0xFF;//sum
                    chip8->reg[0xF]=(sum>>8)&0xFF/;//carry
                    break;
                case(0x5):
                    /*8xy5 - SUB Vx, Vy*/
                    uint16_t sum=1+(chip8->reg[second])+(chip8->reg[third])^(0xFF);
                    chip8->reg[second]=sum&0xFF;//difference
                    chip8->reg[0xF]=(sum>>8)&0xFF/;//not borrow(if 1 means vx is the actual value if 0 means 2s complement of vx is the value)
                    break;
                case(0x6):
                    /*Shift Vx 1 bit right, store LSB in VF*/
                    chip8->reg[0xF]=(chip8->reg[second])%2==0?0:1;
                    (chip8->reg[second])>>=1;
                    break;
                case(0x7):
                    /*8xy7 Vx = Vy - Vx*/
                    uint16_t sum=1+(chip8->reg[third])+(chip8->reg[second])^(0xFF);
                    chip8->reg[second]=sum&0xFF;//difference
                    chip8->reg[0xF]=(sum>>8)&0xFF/;//not borrow(if 1 means vx is the actual value if 0 means 2s complement of vx is the value)
                    break;
                case(0xE):
                    /*Shift Vx 1 bit left, store MSB in VF*/
                    chip8->reg[0xF]=(chip8->reg[second])<32768?0:1;
                    (chip8->reg[second])<<=1;
                    break;

            }
            break;
        case(0x9):/*9xy0 Skip next instruction if Vx != Vy.*/(chip8->pc)+=((chip8->reg[second])!=(chip8->reg[third]))?2:0; break;
        case(0xA):/*Annn set I(index) to nnn*/(chip8->index)=(second<<8)|(third<<4)|(fourth); break;
        case(0xB):/*Bnnn Jump to location nnn + V0.*/ (chip8->pc)=((second<<8)|(third<<4)|(fourth))+chip8->reg[0];  break;
        case(0xC):
            /*Cxkk Set Vx = random byte AND kk*/
            srand(time(NULL));
            uint8_t random_byte = rand()%256;
            (chip8->reg[second])&=random_byte;
            break;

        case(0xD):
        case(0xE):
            if(third==0x9){(chip8->pc)+=((chip8->keypad[chip8->reg[second]])==1?2:0;}//Skip next instruction if key with the value of Vx is pressed
            else if(third==0xA){(chip8->pc)+=((chip8->keypad[chip8->reg[second]])==0?2:0;}
            break;
        case(0xF):
            uint8_t last=third<<4|fourth;
            switch(last){
                case(0x07):/*Set Vx = delay timer value.*/chip8->reg[second]=chip8->dtimer; break;
                case(0x0A):
            }
            break;
    }


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
