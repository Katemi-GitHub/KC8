#include <SDL2/SDL.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <unordered_map>
#include <bitset>

using namespace std;

typedef unsigned char   u8;
typedef unsigned short  u16;
typedef unsigned int    u32;
    
struct Mem {
    static constexpr u32 MAX_MEM = 1024 * 64;
    u8 Data[MAX_MEM];

    void Init() {
        string line;
        string token;
        ifstream kateRom( "src/parsed_rom.txt" );
        int i = 0;
        if( kateRom.is_open() ) {
            while( getline( kateRom, line, '\n' ) ) {
                stringstream ss( line );
                string byte;
                while ( getline( ss, byte, ' ' ) ) {
                    u8 value = static_cast< u8 >( stoi( byte, nullptr, 16 ) );
                    Data[i] = value;
                    i++;
                }
            }
        }
        kateRom.close();
    }

    // Read 1 byte
    u8 operator[]( u32 Address ) const {
        return Data[Address];
    }

    // Write 1 byte
    u8& operator[]( u32 Address ) {
        return Data[Address];
    }
};

struct Display {

    void Update( SDL_Renderer* renderer, u16 Data ) {
        u8 RGB = static_cast<u8>(Data >> 8);
        u8 XY = static_cast<u8>(Data);
        SDL_SetRenderDrawColor( renderer, 0, 0, 0, 255 );
        SDL_SetRenderDrawColor( renderer, ( (RGB >> 4 ) & 0b00000011 ) * 85, ( (RGB >> 2 ) & 0b00000011 ) * 85, ( RGB & 0b00000011) * 85, 255 );
        SDL_Rect pixelRect = { ( ( XY >> 4 ) & 0b00001111 ) * 8 * 3, ( XY & 0b00001111 ) * 8 * 3, 8 * 3, 8 * 3 };
        SDL_RenderFillRect( renderer, &pixelRect );
        SDL_RenderPresent( renderer );
    }
};

struct CPU {
    u16 PC; // Program Counter
    u16 SP; // Stack Pointer

    u8 A, I, O, X, Y; // Registers

    u8 C : 1; // Status Flag - Carry Flag
    u8 Z : 1; // Status Flag - Zero Flag
    u8 D : 1; // Status Flag - Decimal mode Flag
    u8 B : 1; // Status Flag - Break command Flag
    u8 V : 1; // Status Flag - Overflow Flag
    u8 N : 1; // Status Flag - Negative Flag

    bool keyPressed = false;
    bool running = true;

     // Registers
    static constexpr u8
        REG_A = 0x01,
        REG_I = 0x02,
        REG_O = 0x03,
        REG_X = 0x04,
        REG_Y = 0x05;

    u8 &FetchRegister( u8 RegID ) {
        switch ( RegID ) {
            case REG_A: { return A; } break;
            case REG_I: { return I; } break;
            case REG_O: { return O; } break;
            case REG_X: { return X; } break;
            case REG_Y: { return Y; } break;
            default: { cout << "WRONG REGISTER ID :" << RegID << endl; } break;
        }
    }

    void Input( u8 Input ) {
        I = Input;
    }

    void Reset( Mem& memory ) {
        PC = 0x0000;
        SP = 0x0000;
        C = Z = D = B = V = N = 0;
        A = I = O = X = Y = 0;
        memory.Init();
    }

    u8 add( u8 R1, u8 R2 ) {
        u16 Sum = static_cast< u16 >( R1 ) + static_cast< u16 >( R2 );
        C = static_cast< u8 >( Sum >> 8 );
        u8 Result = static_cast< u8 >( Sum );
        return Result;
    }

    u8 FetchByte( Mem& memory ) {
        u8 Data = memory[PC];
        PC++;
        return Data;
    }

    u16 FetchWord( Mem& memory ) {
        u16 Data = ( ( static_cast<u16>(memory[PC]) << 8 ) | memory[PC + 1] );
        PC++;
        PC++;
        return Data;
    }

    // Opcodes
    static constexpr u8
        INS_ADD =   0x01,
        INS_SUB =   0x02,
        INS_AND =   0x03,
        INS_OR =    0x04,
        INS_XOR =   0x05,
        INS_NOT =   0x06,
        INS_MOV =   0x07,
        INS_LOAD =  0x08,
        INS_STORE = 0x09,
        INS_INC =   0x0A,
        INS_DEC =   0x0B,
        INS_JMP =   0x0C,
        INS_JZ =    0x0D,
        INS_JNZ =   0x0E,
        INS_CMP =   0x0F,
        INS_SHL =   0x10,
        INS_SHR =   0x11,
        INS_CALL =  0x12,
        INS_RET =   0x13,
        INS_LCD =   0x14,
        INS_CLR =   0x15,
        INS_HLT =   0x16,
        INS_NOP =   0x00;

    void Execute( Mem& memory, Display& display, SDL_Renderer* renderer ) {
        while ( running ) {
            u8 Ins = FetchByte( memory );

            unordered_map<SDL_Keycode, u8> keyMap = {
                { SDLK_z, 0x01 },
                { SDLK_x, 0x02 },
                { SDLK_RIGHT, 0x03 },
                { SDLK_LEFT, 0x04 },
                { SDLK_DOWN, 0x05 },
                { SDLK_UP, 0x06 }
            };

            SDL_Event event;
            while ( SDL_PollEvent( &event ) ) {
                if ( event.type == SDL_KEYDOWN && !keyPressed ) {
                    SDL_Keycode keycode = event.key.keysym.sym;
                    if ( keyMap.count( keycode ) > 0 ) {
                        Input( keyMap[keycode] );
                    }
                    keyPressed = true;
                } else if ( event.type == SDL_KEYUP ) {
                    keyPressed = false;
                }
            }

            switch ( Ins ) {
                case INS_ADD: {
                    u8 R1 = FetchRegister( FetchByte( memory ) );
                    u8 R2 = FetchRegister( FetchByte( memory ) );
                    R1 = add( R1, R2 );
                } break;
                case INS_SUB: {
                    u8 R1 = FetchRegister( FetchByte( memory ) );
                    u8 R2 = FetchRegister( FetchByte( memory ) );
                    R1 = add( R1, ~R2 );
                } break;
                case INS_AND: {
                    u8 R1 = FetchRegister( FetchByte( memory ) );
                    u8 R2 = FetchRegister( FetchByte( memory ) );
                    R1 =  R1 & R2;
                } break;
                case INS_OR: {
                    u8 R1 = FetchRegister( FetchByte( memory ) );
                    u8 R2 = FetchRegister( FetchByte( memory ) );
                    R1 = R1 | R2;
                } break;
                case INS_XOR: {
                    u8 R1 = FetchRegister( FetchByte( memory ) );
                    u8 R2 = FetchRegister( FetchByte( memory ) );
                    R1 = ~(R1 | R2);
                } break;
                case INS_NOT: {
                    u8 R1 = FetchRegister( FetchByte( memory ) );
                    R1 = ~R1;
                } break;
                case INS_MOV: {
                    u8 R1 = FetchRegister( FetchByte( memory ) );
                    u8 R2 = FetchRegister( FetchByte( memory ) );
                    R1 = R2;
                } break;
                case INS_LOAD: {
                    u8 R1 = FetchRegister( FetchByte( memory ) );
                    u8 R2 = FetchByte( memory );
                    R1 = memory[R2];
                } break;
                case INS_STORE: {
                    u8 R1 = FetchByte( memory );
                    u8 R2 = FetchRegister( FetchByte( memory ) );
                    memory[R1] = R2;
                } break;
                case INS_INC: {
                    u8 R1 = FetchByte( memory );
                    R1 = add( R1, u8( 1 ) );
                } break;
                case INS_DEC: {
                    u8 R1 = FetchByte( memory );
                    R1 = add( R1, ~( u8( 1 ) ) );
                } break;
                case INS_JMP: {
                    PC = FetchWord( memory );
                } break;
                case INS_JZ: {
                    if (Z) {
                        PC = FetchWord( memory );
                    }
                } break;
                case INS_JNZ: {
                    if (!Z) {
                        PC = FetchWord( memory );
                    }
                } break;
                case INS_CMP: {
                    u8 R1 = FetchRegister( FetchByte( memory ) );
                    u8 R2 = FetchRegister( FetchByte( memory ) );
                    if ( R1 == R2 ) {
                        Z = 1;
                    } else {
                        Z = 0;
                    }
                } break;
                case INS_SHL: {
                    u8 R1 = FetchRegister( FetchByte( memory ) );
                    R1 = ( R1 << FetchByte( memory ) );
                } break;
                case INS_SHR: {
                    u8 R1 = FetchRegister( FetchByte( memory ) );
                    R1 = ( R1 >> FetchByte( memory ) );
                } break;
                case INS_CALL: {
                    SP = add( PC, u8( 1 ) );
                    PC = FetchWord( memory );
                } break;
                case INS_RET: {
                    PC = SP;
                } break;
                case INS_LCD: {
                    display.Update( renderer, FetchWord( memory ) );
                } break;
                case INS_CLR: { SDL_RenderClear( renderer ); } break;
                case INS_HLT: { running = false; } break;
                case INS_NOP: {} break;
                default: {} break;
            }
        }
    }
};

int main( int argc, char* args[] ) {
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window = SDL_CreateWindow(
        "KC8",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        128 * 3,
        128 * 3,
        SDL_WINDOW_SHOWN
    );
    SDL_Renderer* renderer = SDL_CreateRenderer(
        window,
        -1,
        0
    );

    CPU cpu;
    Mem mem;
    Display display;

    cpu.Reset( mem );
    cpu.Execute( mem, display, renderer );

    SDL_DestroyRenderer( renderer );
    SDL_DestroyWindow( window );
    SDL_Quit();

    return 0;
}
