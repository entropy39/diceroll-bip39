/* Exercise the real app with timed touch input and an in-memory display. */
#include "app.h"
#include "waveshare_platform.h"
#include "layout.h"
#include <assert.h>
#include <setjmp.h>
#include <stdio.h>
#include <string.h>

static uint16_t pixels[800 * 480];
static uint16_t completed[800 * 480];
static unsigned tick;
static unsigned entropy_bits;
static unsigned words;
static unsigned finish_tick;
static jmp_buf done;
static const char *image_path;

uint16_t *waveshare_platform_display_init( void ) { return pixels; }
void waveshare_platform_touch_init( void ) { }
uint64_t waveshare_platform_time_us( void ) { return ( uint64_t )tick * 5000U; }

bool waveshare_platform_touch_read( uint16_t *x, uint16_t *y )
{
    *x = 400; *y = 390;
    if( tick < 2 )
    {
        *x = words == 12 ? 200 : 500; *y = 200;
        return true;
    }
    if( tick >= 5 && tick <= finish_tick )
        return ( tick - 5 ) % 5 == 0;
    unsigned t = tick - finish_tick;
    if( ( t >= 10 && t < 15 ) || ( t >= 45 && t < 50 ) )
    {
        /* Both halves of the former 0/1 area must open the same QR. */
        *x = t < 45 ? 400 : 700;
        return true;
    }
    if( ( t >= 25 && t < 30 ) || ( t >= 60 && t < 65 ) )
    {
        *x = 600;
        return true;
    }
    /* Exercise the existing protected Restart after returning from QR. */
    if( t >= 80 && t < 290 ) { *x = 50; return true; }
    if( t >= 300 && t < 302 ) { *x = 500; *y = 200; return true; }
    return false;
}

static void save_image( void )
{
    if( image_path == NULL ) return;
    FILE *file = fopen( image_path, "wb" );
    assert( file != NULL );
    fprintf( file, "P6\n800 480\n255\n" );
    for( unsigned i = 0; i < 800 * 480; ++i )
    {
        uint16_t p = pixels[i];
        unsigned char rgb[] = { ( (p >> 11) & 31 ) * 255 / 31,
                                ( (p >> 5) & 63 ) * 255 / 63,
                                (p & 31) * 255 / 31 };
        assert( fwrite( rgb, 1, 3, file ) == 3 );
    }
    assert( fclose( file ) == 0 );
}

void waveshare_platform_sleep_ms( uint32_t milliseconds )
{
    assert( milliseconds == 5 );
    ++tick;
    if( tick == finish_tick + 6 )
    {
        /* The final input must leave the words visible, with the QR button. */
        assert( pixels[400 * 800 + 300] == 0xc618 );
        memcpy( completed, pixels, sizeof( pixels ) );
    }
    if( tick == finish_tick + 20 || tick == finish_tick + 55 )
    {
        /* The QR's quiet zone is white; no word grid remains behind it. */
        unsigned left = words == 12 ? 9 : 18;
        unsigned top = words == 12 ? 9 : 18;
        assert( pixels[top * 800 + left] == 0xffff );
        if( tick == finish_tick + 20 ) save_image( );
    }
    if( tick == finish_tick + 38 || tick == finish_tick + 73 )
        assert( memcmp( completed, pixels, sizeof( pixels ) ) == 0 );
    if( tick == finish_tick + 310 )
    {
        /* Restart still requires a hold, and the new phrase is incomplete. */
        assert( pixels[400 * 800 + 600] == 0x4208 );
        assert( memcmp( completed, pixels, sizeof( pixels ) ) != 0 );
        longjmp( done, 1 );
    }
    assert( tick < finish_tick + 400 );
}

int main( int argc, char **argv )
{
    words = argc > 1 && strcmp( argv[1], "24" ) == 0 ? 24 : 12;
    entropy_bits = words == 12 ? 128 : 256;
    finish_tick = 5 + ( entropy_bits - 1 ) * 5;
    if( argc > 2 ) image_path = argv[2];
    if( setjmp( done ) == 0 ) app_run( );
    printf( "Waveshare %u-word flow: completion, QR, return, reopen, restart passed\n", words );
    return 0;
}
