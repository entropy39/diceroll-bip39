#include "waveshare_platform.h"

#include "layout.h"

#include <SDL.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

enum
{
    DISPLAY_SCALE = 2,
    DISPLAY_WIDTH = DICEROLL_DISPLAY_WIDTH * DISPLAY_SCALE,
    DISPLAY_HEIGHT = DICEROLL_DISPLAY_HEIGHT * DISPLAY_SCALE,
    WINDOW_WIDTH = 1920,
    WINDOW_HEIGHT = 1080,
    DISPLAY_X = ( WINDOW_WIDTH - DISPLAY_WIDTH ) / 2,
    DISPLAY_Y = ( WINDOW_HEIGHT - DISPLAY_HEIGHT ) / 2
};

static SDL_Window *window;
static SDL_Renderer *renderer;
static SDL_Texture *texture;
static SDL_Cursor *finger_cursor;
static uint16_t framebuffer[ DICEROLL_DISPLAY_WIDTH * DICEROLL_DISPLAY_HEIGHT ];

static void fail( const char *operation )
{
    fprintf( stderr, "%s: %s\n", operation, SDL_GetError( ) );
    exit( EXIT_FAILURE );
}

static void present( void )
{
    SDL_Rect display_rect = { DISPLAY_X, DISPLAY_Y, DISPLAY_WIDTH, DISPLAY_HEIGHT };

    if( SDL_UpdateTexture( texture, NULL, framebuffer, DICEROLL_DISPLAY_WIDTH * sizeof( uint16_t ) ) != 0 )
        fail( "SDL_UpdateTexture" );
    if( SDL_SetRenderDrawColor( renderer, 0, 0, 0, 255 ) != 0 )
        fail( "SDL_SetRenderDrawColor" );
    if( SDL_RenderClear( renderer ) != 0 )
        fail( "SDL_RenderClear" );
    if( SDL_RenderCopy( renderer, texture, NULL, &display_rect ) != 0 )
        fail( "SDL_RenderCopy" );
    SDL_RenderPresent( renderer );
}

uint16_t *waveshare_platform_display_init( void )
{
    if( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_TIMER ) != 0 )
        fail( "SDL_Init" );

    window = SDL_CreateWindow( "Diceroll BIP-39 Demonstration", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_BORDERLESS );
    if( window == NULL )
        fail( "SDL_CreateWindow" );

    SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "0" );
    renderer = SDL_CreateRenderer( window, -1, SDL_RENDERER_SOFTWARE );
    if( renderer == NULL )
        fail( "SDL_CreateRenderer" );

    texture = SDL_CreateTexture( renderer, SDL_PIXELFORMAT_RGB565, SDL_TEXTUREACCESS_STREAMING, DICEROLL_DISPLAY_WIDTH, DICEROLL_DISPLAY_HEIGHT );
    if( texture == NULL )
        fail( "SDL_CreateTexture" );

    finger_cursor = SDL_CreateSystemCursor( SDL_SYSTEM_CURSOR_HAND );
    if( finger_cursor == NULL )
        fail( "SDL_CreateSystemCursor" );
    SDL_SetCursor( finger_cursor );

    return framebuffer;
}

void waveshare_platform_touch_init( void )
{
}

bool waveshare_platform_touch_read( uint16_t *x, uint16_t *y )
{
    SDL_Event event;
    int mouse_x;
    int mouse_y;
    uint32_t buttons;

    while( SDL_PollEvent( &event ) )
    {
        if( event.type == SDL_QUIT ||
                ( event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE ) )
        {
            SDL_Quit( );
            exit( EXIT_SUCCESS );
        }
    }

    present( );
    buttons = SDL_GetMouseState( &mouse_x, &mouse_y );
    if( ( buttons & SDL_BUTTON( SDL_BUTTON_LEFT ) ) == 0U )
        return false;
    if( mouse_x < DISPLAY_X || mouse_y < DISPLAY_Y || mouse_x >= DISPLAY_X + DISPLAY_WIDTH || mouse_y >= DISPLAY_Y + DISPLAY_HEIGHT )
        return false;

    *x = ( uint16_t )( ( mouse_x - DISPLAY_X ) / DISPLAY_SCALE );
    *y = ( uint16_t )( ( mouse_y - DISPLAY_Y ) / DISPLAY_SCALE );
    return true;
}

uint64_t waveshare_platform_time_us( void )
{
    return SDL_GetTicks64( ) * 1000U;
}

void waveshare_platform_sleep_ms( uint32_t milliseconds )
{
    SDL_Delay( milliseconds );
}
