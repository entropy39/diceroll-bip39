#include "waveshare_platform.h"

#include "layout.h"

#include <SDL.h>
#include <emscripten.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum
{
    WEB_FRAME_MS = 16
};

static SDL_Window *window;
static SDL_Surface *window_surface;
static SDL_Surface *framebuffer_surface;
static uint16_t framebuffer[ DICEROLL_DISPLAY_WIDTH * DICEROLL_DISPLAY_HEIGHT ];
static uint16_t presented_framebuffer[ DICEROLL_DISPLAY_WIDTH * DICEROLL_DISPLAY_HEIGHT ];
static bool frame_presented;
static bool pointer_down;
static uint16_t pointer_x;
static uint16_t pointer_y;

static void fail( const char *operation )
{
    fprintf( stderr, "%s: %s\n", operation, SDL_GetError( ) );
    exit( EXIT_FAILURE );
}

static void present( void )
{
    if( frame_presented &&
            memcmp( framebuffer, presented_framebuffer,
                    sizeof( framebuffer ) ) == 0 )
        return;

    if( SDL_BlitSurface( framebuffer_surface, NULL, window_surface, NULL ) != 0 )
        fail( "SDL_BlitSurface" );
    if( SDL_UpdateWindowSurface( window ) != 0 )
        fail( "SDL_UpdateWindowSurface" );
    memcpy( presented_framebuffer, framebuffer, sizeof( framebuffer ) );
    frame_presented = true;
}

static uint16_t clamp_coordinate( float value, uint16_t limit )
{
    if( value <= 0.0f )
        return 0U;
    if( value >= ( float )limit )
        return ( uint16_t )( limit - 1U );
    return ( uint16_t )value;
}

static void process_events( void )
{
    SDL_Event event;

    while( SDL_PollEvent( &event ) )
    {
        switch( event.type )
        {
            case SDL_MOUSEBUTTONDOWN:
                if( event.button.button == SDL_BUTTON_LEFT )
                {
                    pointer_down = true;
                    pointer_x = clamp_coordinate( ( float )event.button.x,
                                                  DICEROLL_DISPLAY_WIDTH );
                    pointer_y = clamp_coordinate( ( float )event.button.y,
                                                  DICEROLL_DISPLAY_HEIGHT );
                }
                break;
            case SDL_MOUSEBUTTONUP:
                if( event.button.button == SDL_BUTTON_LEFT )
                    pointer_down = false;
                break;
            case SDL_MOUSEMOTION:
                if( pointer_down )
                {
                    pointer_x = clamp_coordinate( ( float )event.motion.x,
                                                  DICEROLL_DISPLAY_WIDTH );
                    pointer_y = clamp_coordinate( ( float )event.motion.y,
                                                  DICEROLL_DISPLAY_HEIGHT );
                }
                break;
            case SDL_FINGERDOWN:
            case SDL_FINGERMOTION:
                pointer_down = true;
                pointer_x = clamp_coordinate(
                                event.tfinger.x * DICEROLL_DISPLAY_WIDTH,
                                DICEROLL_DISPLAY_WIDTH );
                pointer_y = clamp_coordinate(
                                event.tfinger.y * DICEROLL_DISPLAY_HEIGHT,
                                DICEROLL_DISPLAY_HEIGHT );
                break;
            case SDL_FINGERUP:
                pointer_down = false;
                break;
            default:
                break;
        }
    }
}

uint16_t *waveshare_platform_display_init( void )
{
    if( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_TIMER ) != 0 )
        fail( "SDL_Init" );

    window = SDL_CreateWindow( "Dice Roll BIP-39 Demonstration",
                               SDL_WINDOWPOS_UNDEFINED,
                               SDL_WINDOWPOS_UNDEFINED,
                               DICEROLL_DISPLAY_WIDTH,
                               DICEROLL_DISPLAY_HEIGHT, 0 );
    if( window == NULL )
        fail( "SDL_CreateWindow" );

    window_surface = SDL_GetWindowSurface( window );
    if( window_surface == NULL )
        fail( "SDL_GetWindowSurface" );

    framebuffer_surface = SDL_CreateRGBSurfaceFrom(
                              framebuffer, DICEROLL_DISPLAY_WIDTH,
                              DICEROLL_DISPLAY_HEIGHT, 16,
                              DICEROLL_DISPLAY_WIDTH * sizeof( uint16_t ),
                              0xf800U, 0x07e0U, 0x001fU, 0U );
    if( framebuffer_surface == NULL )
        fail( "SDL_CreateRGBSurfaceFrom" );

    return framebuffer;
}

void waveshare_platform_touch_init( void )
{
}

bool waveshare_platform_touch_read( uint16_t *x, uint16_t *y )
{
    process_events( );
    present( );
    if( !pointer_down )
        return false;

    *x = pointer_x;
    *y = pointer_y;
    return true;
}

uint64_t waveshare_platform_time_us( void )
{
    return ( uint64_t )emscripten_get_now( ) * 1000U;
}

void waveshare_platform_sleep_ms( uint32_t milliseconds )
{
    emscripten_sleep( milliseconds < WEB_FRAME_MS ? WEB_FRAME_MS : milliseconds );
}
