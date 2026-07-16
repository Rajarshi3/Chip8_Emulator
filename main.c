#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <SDL.h>
#include <string.h>
#include <time.h>

#define WINDOW_TITLE "Chip8 Emulator"
#define scale 10
#define run_speed 1

//the game structure
typedef struct{
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
} Game;


//will redesign them(FONTS) later
//took them from cowgods chip8 guide
const uint8_t hex_symbols[80]={
0xF0,0x90,0X90,0X90,0XF0,//0
0X20,0x60,0x20,0x20,0x70,//1
0xF0,0x10,0xF0,0x80,0xF0,//2
0xF0,0x10,0xF0,0x10,0xF0,//3
0x90,0x90,0xF0,0x10,0x10,//4
0xF0,0x80,0xF0,0x10,0xF0,//5
0xF0,0x80,0xF0,0x90,0xF0,//6
0xF0,0x10,0x20,0x40,0x40,//7
0xF0,0x90,0xF0,0x90,0xF0,//8
0xF0,0x90,0xF0,0x10,0xF0,//9
0xF0,0x90,0xF0,0x90,0x90,//A
0xE0,0x90,0xE0,0x90,0xE0,//B
0xF0,0x80,0x80,0x80,0xF0,//C
0xE0,0x90,0x90,0x90,0xE0,//D
0xF0,0x80,0xF0,0x80,0xF0,//E
0xF0,0x80,0xF0,0x80,0x80//F

};

//the chip8 structure
typedef struct{
    bool block; //stopping flag for Fx0A instruction
    uint8_t bindex;//index of register that will take input
    uint8_t memory[4096]; //4KB memory=4096 x 1Byte=4096 x 8bits
    uint8_t reg[16]; //16 x 8bit registers
    uint16_t index; //16 bit index register to store memory addresses
    uint16_t pc; //16 bit program counter to track current instruction
    uint8_t sp; //8 bit stack pointer to point to top of stack memory
    uint16_t stack[16]; //stack memory(an array of 16 16bit values used to store return adresses when calling subroutines)
    uint8_t dtimer; //8bit delay timer register
    uint8_t stimer; //8bit sound timer register
    uint32_t display[64*32]; //storing the state of the default 64x32 display using 32 bits for SDL compatibility
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

//keyboard/input handling function
void handle_event(Chip8* chip8, SDL_Event* event){
    if(!(event->type==SDL_KEYUP || event->type==SDL_KEYDOWN)){return;}
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
        case(SDLK_1): chip8->keypad[0]=key_state; if(chip8->block && event->type==SDL_KEYDOWN){chip8->reg[chip8->bindex]=0;chip8->block=false;} break;
        case(SDLK_2): chip8->keypad[1]=key_state; if(chip8->block && event->type==SDL_KEYDOWN){chip8->reg[chip8->bindex]=1;chip8->block=false;} break;
        case(SDLK_3): chip8->keypad[2]=key_state; if(chip8->block && event->type==SDL_KEYDOWN){chip8->reg[chip8->bindex]=2;chip8->block=false;} break;
        case(SDLK_4): chip8->keypad[3]=key_state; if(chip8->block && event->type==SDL_KEYDOWN){chip8->reg[chip8->bindex]=3;chip8->block=false;} break;

        case(SDLK_q): chip8->keypad[4]=key_state; if(chip8->block && event->type==SDL_KEYDOWN){chip8->reg[chip8->bindex]=4;chip8->block=false;} break;
        case(SDLK_w): chip8->keypad[5]=key_state; if(chip8->block && event->type==SDL_KEYDOWN){chip8->reg[chip8->bindex]=5;chip8->block=false;} break;
        case(SDLK_e): chip8->keypad[6]=key_state; if(chip8->block && event->type==SDL_KEYDOWN){chip8->reg[chip8->bindex]=6;chip8->block=false;} break;
        case(SDLK_r): chip8->keypad[7]=key_state; if(chip8->block && event->type==SDL_KEYDOWN){chip8->reg[chip8->bindex]=7;chip8->block=false;} break;

        case(SDLK_a): chip8->keypad[8]=key_state; if(chip8->block && event->type==SDL_KEYDOWN){chip8->reg[chip8->bindex]=8;chip8->block=false;} break;
        case(SDLK_s): chip8->keypad[9]=key_state; if(chip8->block && event->type==SDL_KEYDOWN){chip8->reg[chip8->bindex]=9;chip8->block=false;} break;
        case(SDLK_d): chip8->keypad[10]=key_state; if(chip8->block && event->type==SDL_KEYDOWN){chip8->reg[chip8->bindex]=10;chip8->block=false;} break;
        case(SDLK_f): chip8->keypad[11]=key_state; if(chip8->block && event->type==SDL_KEYDOWN){chip8->reg[chip8->bindex]=11;chip8->block=false;} break;

        case(SDLK_z): chip8->keypad[12]=key_state; if(chip8->block && event->type==SDL_KEYDOWN){chip8->reg[chip8->bindex]=12;chip8->block=false;} break;
        case(SDLK_x): chip8->keypad[13]=key_state; if(chip8->block && event->type==SDL_KEYDOWN){chip8->reg[chip8->bindex]=13;chip8->block=false;} break;
        case(SDLK_c): chip8->keypad[14]=key_state; if(chip8->block && event->type==SDL_KEYDOWN){chip8->reg[chip8->bindex]=14;chip8->block=false;} break;
        case(SDLK_v): chip8->keypad[15]=key_state; if(chip8->block && event->type==SDL_KEYDOWN){chip8->reg[chip8->bindex]=15;chip8->block=false;} break;


    }

}

//function for fetch-decode-execute cycle of chip8 instructions
void emulate_cycle(Chip8* chip8){

    if(chip8->block==true){return;}//block execution

    uint8_t first=(chip8->memory[chip8->pc])>>4;
    uint8_t second=(chip8->memory[chip8->pc])&15;
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
                chip8->pc=chip8->stack[(chip8->sp)--];
            }
            break;

        case(0x1):/*1nnn jump to location nnn*/ (chip8->pc)=(second<<8)|(third<<4)|(fourth); break;
        case(0x2):/*2nnn CALL addr nnn*/chip8->stack[++(chip8->sp)]=chip8->pc; (chip8->pc)=(second<<8)|(third<<4)|(fourth); break;
        case(0x3):/*3xkk*/(chip8->pc)+=((chip8->reg[second])==((third<<4)|fourth))?2:0; break;
        case(0x4):/*4xkk*/(chip8->pc)+=((chip8->reg[second])!=((third<<4)|fourth))?2:0; break;
        case(0x5):/*5xy0*/(chip8->pc)+=((chip8->reg[second])==(chip8->reg[third]))?2:0; break;
        case(0x6):/*6xkk*/(chip8->reg[second])=(third<<4)|fourth; break;
        case(0x7):/*7xkk*/(chip8->reg[second])+=(third<<4)|fourth; break;
        case(0x8):
            switch(fourth){
                case(0x1):/*8xy0: Set Vx = Vy*/(chip8->reg[second])=(chip8->reg[third]);break;
                case(0x1):/*8xy1: Vx=Vx|Vy*/(chip8->reg[second])|= (chip8->reg[third]); break;
                case(0x2):/*8xy2: Vx=Vx&Vy*/(chip8->reg[second])&= (chip8->reg[third]); break;
                case(0x3):/*8xy3: Vx=Vx^Vy*/(chip8->reg[second])^= (chip8->reg[third]); break;
                case(0x4):{
                    /*8xy4: Vx=Vx+Vy, VF=carry*/
                    uint16_t sum=(chip8->reg[second])+(chip8->reg[third]);
                    chip8->reg[second]=sum&0xFF;//sum
                    chip8->reg[0xF]=(sum>>8)&0xFF;//carry
                    break;}

                case(0x5):{
                    /*8xy5 - SUB Vx, Vy*/
                    uint16_t sum=1+(chip8->reg[second])+((chip8->reg[third])^(0xFF));
                    chip8->reg[second]=sum&0xFF;//difference
                    chip8->reg[0xF]=(sum>>8)&0xFF;//not borrow(if 1 means vx is the actual value if 0 means 2s complement of vx is the value)
                    break;}
                case(0x6):
                    /*Shift Vx 1 bit right, store LSB in VF*/
                    chip8->reg[0xF]=(chip8->reg[second])%2==0?0:1;
                    (chip8->reg[second])>>=1;
                    break;
                case(0x7):{
                    /*8xy7 Vx = Vy - Vx*/
                    uint16_t sum=1+(chip8->reg[third])+((chip8->reg[second])^(0xFF));
                    chip8->reg[second]=sum&0xFF;//difference
                    chip8->reg[0xF]=(sum>>8)&0xFF;//not borrow(if 1 means vx is the actual value if 0 means 2s complement of vx is the value)
                    break;}
                case(0xE):
                    /*8xyE Shift Vx 1 bit left, store MSB in VF*/
                    chip8->reg[0xF]=(chip8->reg[second])<0x80?0:1;
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
            (chip8->reg[second])=random_byte&((third<<4)|fourth));
            break;

        case(0xD):/*Draw opcode: DXYN*/
            //basic idea is in original chip8 it was 1 bit per pixel now it is 1 byte so go figure
            uint8_t start_x=(chip8->reg[second])%64;
            uint8_t start_y=(chip8->reg[third])%32;
            bool collision=false;
            for(int i=0;i<fourth;i++){
                for(int j=0;j<8;j++){
                    uint8_t currentx=(start_x+j)%64;
                    uint8_t currenty=(start_y+i)%32;
                    uint32_t wrindex=64*(currenty)+currentx;
                    uint32_t towrite=((chip8->memory[(chip8->index)+i])>>j)&1;
                   if((chip8->display[wrindex]==1)&&(towrite==1)){collision=true;};//collision
                    chip8->display[wrindex]^=towrite;
                }
                chip8->reg[0xF]=(collision==true)?1:0;
            }
            break;
        case(0xE):
            if(third==0x9){(chip8->pc)+=(chip8->keypad[chip8->reg[second]])==1?2:0;}//Skip next instruction if key with the value of Vx is pressed
            else if(third==0xA){(chip8->pc)+=(chip8->keypad[chip8->reg[second]])==0?2:0;}
            break;
        case(0xF):
            uint8_t last=third<<4|fourth;
            switch(last){
                case(0x07):/*Set Vx = delay timer value.*/chip8->reg[second]=chip8->dtimer; break;
                case(0x0A):/*Fx0A Wait for key press, store key value in Vx.*/chip8->block=true; chip8->bindex=second; break;
                case(0x15):/*Set delay timer = Vx*/chip8->dtimer=chip8->reg[second];break;
                case(0x18):/*Set sound timer = Vx.*/chip8->stimer=chip8->reg[second];break;
                case(0x1E):/*Fx1E Set I = I + Vx.*/ chip8->index+=chip8->reg[second];break;
                case(0x29):/*Set I = location of sprite for digit Vx.*/chip8->index=0x50+(5*(chip8->reg[second])); break;
                case(0x33):
                    /*Store BCD Vx in memloc I, I+1, and I+2*/
                    uint8_t a=chip8->reg[second];
                    chip8->memory[chip8->index]=a/100;
                    chip8->memory[(chip8->index)+1]=(a/10)-(10*(a/100));
                    chip8->memory[(chip8->index)+2]=a-(10*(a/10));
                    break;
                case(0x55):
                    /*Fx55 Store registers V0 through Vx in memory starting at location I*/
                    for(uint8_t i=0;i<=second;i++){
                        chip8->memory[(chip8->index)+i]=chip8->reg[i];
                    }
                    break;
                case(0x65):
                    /*Fx65 Read registers V0 through Vx from memory starting at location I.*/
                    for(uint8_t i=0;i<=second;i++){
                        chip8->reg[i]=chip8->memory[(chip8->index)+i];
                    }
                    break;

            }

    }


}

//function to render screen using SDL
void render_screen(Chip8* chip8, Game* game){
    if(SDL_UpdateTexture(game->texture, NULL, chip8->display, 64*sizeof(uint32_t))){
        fprintf(stderr, "SDL_UpdateTexture() failed \n %s\n", SDL_GetError());
    }
    if(SDL_RenderClear(game->renderer)){
        fprintf(stderr, "SDL_RenderClear() failed \n %s\n", SDL_GetError());
    }
    if(SDL_RenderCopy(game->renderer, game->texture, NULL, NULL)){
        fprintf(stderr, "SDL_RenderCopy() failed \n %s\n", SDL_GetError());
    }
    if(SDL_RenderPresent(game->renderer)){
        fprintf(stderr, "SDL_RenderPresent() failed \n %s\n", SDL_GetError());
    }
}


//will look at integrating with SDL to make audio work later
void update_timer(Chip8* chip8, Game* game){
chip8->dtimer-=(chip8->dtimer>0)?1:0;
chip8->stimer-=(chip8->stimer>0)?1:0;
}

//main loop for detecting inputs and running the emulator
void run(Chip8* chip8, Game* game){
    bool quit=false;
    SDL_Event event;
    while(!quit){
        while(SDL_PollEvent(&event)){
            if(event.type==SDL_QUIT){quit=true;}
            handle_event(chip8, &event);
        }

        //emulate cycle runs at about 600Hz
        for(int i=0;i<10*run_speed;i++){
            emulate_cycle(chip8);
        }
        //timer runs ar 60Hz
        update_timer(chip8,game);
        render_screen(chip8, game);

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


//function to copy fonts to:
//1.copy fonts to memory of chip8
//2.copy the ROM to the memory of chip8
//3.set the sp and pc to appropriate values

int processor_init(Chip8* chip8, char* rom_address){

    //copy fonts to chip8 memory
    for(int i=0;i<80;i++){
        chip8->memory[0x50+i]=hex_symbols[i];    }


    //copying the rom
    //opening the file
    FILE* rom;
    if((rom=fopen(rom_address, "rb"))==NULL){
        //printf("Cannot open file\n");
        fprintf(stderr,"Cannot open file\n");
        exit(1);
    }
    //moving to the end of the file
    fseek(rom,0,SEEK_END);

    //finding the size of the file using ftell
    long int size_of_rom=ftell(rom);
    if(size_of_rom<0){
        //printf("ftell() error finding the size of file\n");
        fprintf(stderr, "ftell() error finding the size of file\n");
        fclose(rom);
        return 1;
    }

    //checking if sizeofrom exceeds the space available in chip8->memory[4096]
    if(size_of_rom>4096-0x200){
        //printf("The ROM exceeds the available memory in chip8\n");
        fprintf(stderr, "The ROM exceeds the available memory in chip8\n");
        fclose(rom);
        return 1;
    }
    //returning to the beginning of the file
    rewind(rom);

    //allocating memory for the buffer
    uint8_t* temp=malloc(size_of_rom);
    if(!temp){fprintf(stderr,"In processor_init allocating buffer memory failed\n");fclose(rom); return 1;}

    //reading from file to buffer
    int read_bytes=fread(temp,1, size_of_rom, rom);
    if(read_bytes!=size_of_rom){fprintf(stderr,"In processor_init fread() failed\n");fclose(rom); free(temp); return 1;}

    for(int i=0; i<size_of_rom;i++){
        chip8->memory[0x200+i]=temp[i];
    }
    free(temp);
    fclose(rom);

    //initialising the pc and sp
    chip8->pc=0x200;
    chip8->sp=0;
    return 0;

}

//main function
int main(int argc, char* argv[]){
     //FILE* rom;
     char* rom_address;
    if(argc<2){
            //printf("program executed for less than expected arguments\n");
            fprintf(stderr,"program executed for less than expected arguments\n");
            exit(1);
        }
    else if(argc==2){
        /*if((rom=fopen(argv[1], "rb"))==NULL){
            //printf("Cannot open file\n");
            fprintf(stderr,"Cannot open file\n");
            exit(1);
        }*/
        if(argv[1]==NULL){fprintf(stderr,"File does not exist\n");exit(1);}
        rom_address=argv[1];
    }
    else{
        //printf("Too many arguments\n");
        fprintf(stderr,"Too many arguments\n");
        exit(1);
    }

    Game game={0};
    if(init(&game)){
        //printf("init didnt work\n");
        fprintf(stderr,"init didnt work\n");
        cleanup(&game, EXIT_FAILURE);
        exit(1);
    }

    Chip8 chip8={0};
    if(processor_init(&chip8,rom_address)){
        //printf("processor_init failed\n");
        fprintf(stderr,"processor_init failed\n");
        cleanup(&game, EXIT_FAILURE);
        exit(1);
    };

    run(&chip8, &game);
    cleanup(&game, EXIT_SUCCESS);

    exit(0);
}
