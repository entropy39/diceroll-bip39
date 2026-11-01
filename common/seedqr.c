#include "seedqr.h"

int seedqr_encode( const MnemonicState *state, char *text, size_t capacity )
{
    if( text == NULL || capacity == 0U )
        return -1;
    text[ 0 ] = '\0';
    if( state == NULL || ( state->word_count != 12U && state->word_count != 24U ) ||
        !mnemonic_state_entropy_complete( state ) ||
        capacity < ( size_t )state->word_count * 4U + 1U )
        return -1;

    for( uint8_t word = 1U; word <= state->word_count; ++word )
    {
        uint16_t index;
        int result = word == state->word_count
                   ? mnemonic_state_get_final_word_index( state, &index )
                   : mnemonic_state_get_word_index( state, word, &index );
        if( result != 0 )
        {
            volatile char *wipe = text;
            for( size_t i = 0U; i < ( size_t )state->word_count * 4U + 1U; ++i )
                wipe[ i ] = 0;
            return -1;
        }
        for( int digit = 3; digit >= 0; --digit )
        {
            text[ ( word - 1U ) * 4U + digit ] = ( char )( '0' + index % 10U );
            index /= 10U;
        }
    }
    text[ state->word_count * 4U ] = '\0';
    return 0;
}
