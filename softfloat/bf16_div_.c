
#include <stdbool.h>
#include <stdint.h>
#include "platform.h"
#include "internals.h"
#include "specialize.h"
#include "softfloat.h"


bfloat16_t bf16_div_( bfloat16_t a, bfloat16_t b )
{
    union ui16_bf16 uA;
    uint_fast16_t uiA;
    bool signA;
    int_fast16_t expA;
    uint_fast8_t sigA;
    union ui16_bf16 uB;
    uint_fast16_t uiB;
    bool signB;
    int_fast16_t expB;
    uint_fast8_t sigB;
    bool signZ;
    int_fast16_t expZ;
    union ui16_bf16 uZ;
    
    /**
     * get the signal、exp and frac bits of a、b
     */
    uA.f = a;
    uiA = uA.ui;
    signA = signBF16UI( uiA );
    expA  = expBF16UI( uiA );
    sigA  = fracBF16UI( uiA );
    uB.f = b;
    uiB = uB.ui;
    signB = signBF16UI( uiB );
    expB  = expBF16UI( uiB );
    sigB  = fracBF16UI( uiB );
    
    signZ = signA ^ signB;
    /**
     * handle NaN and inf
     */
    if ( expA == 0xFF ) 
    {
        if ( sigA )
        {//a is NaN
            uZ.ui = signZ ? 0xffc0 : 0x7fc0;//-NaN or NaN
            return uZ.f;
        }
        if ( expB == 0xFF ) 
        {
            if ( sigB )
            {//b is NaN
                uZ.ui = signZ ? 0xffc0 : 0x7fc0;//-NaN or NaN
                return uZ.f;
            }
            //b is inf, invalid
            softfloat_raiseFlags( softfloat_flag_invalid );
            uZ.ui = signZ ? 0xffc0 : 0x7fc0;//-NaN or NaN
            return uZ.f;
        }
        //a is inf, return inf
        uZ.ui = signZ ? 0xff80 : 0x7f80;//-inf or inf
        return uZ.f;
    }
    if ( expB == 0xFF ) 
    {
        if ( sigB )
        {//b is NaN
            uZ.ui = signZ ? 0xffc0 : 0x7fc0;//-NaN or NaN
            return uZ.f;
        }
        //b is inf, return 0
        uZ.ui = signZ ? 0x8000 : 0x0;
        return uZ.f;
    }

    /** handle 0
     */
    if ( ! ( expB | sigB ) )
    { // b is 0
        if ( ! (expA | sigA) ) 
        {//a isn't 0, invalid
            softfloat_raiseFlags( softfloat_flag_invalid );
            uZ.ui = signZ ? 0xffc0 : 0x7fc0;//-NaN or NaN
            return uZ.f;
        }
        //a is 0
        softfloat_raiseFlags( softfloat_flag_infinite );
        uZ.ui = signZ ?  0xff80 : 0x7f80;//-inf or inf
        return uZ.f;
    }
    if ( ! (expA | sigA) )
    {//return 0
            uZ.ui = signZ ? 0x8000 : 0x0;
            return uZ.f;
    }

    bfloat16_t makeup8 = { 0x400 };
    //in order to make recip(b) != inf
    if( ( b.v & 0x7FFF ) <= 0x20 )
    {//a and b mul 2^7

        b.v += ( 8 << 7 );
        b = signB ? bf16_add( b, makeup8 ) : bf16_sub( b, makeup8 );

        if( 0 == expA )
        {
            a.v += ( 8 << 7 );
            a = signA ? bf16_add( a, makeup8 ) : bf16_sub( a, makeup8 ); 
        }
        else
        {
            expA += 7;
            if( expA > 254)
            {
                a.v =  signA ? 0xff80 : 0x7f80;//-inf or inf
            }
            else
            {
                a.v += ( (uint16_t)7 << 7 );
            }
        } 
    }

    //use mul and recip to compute div
    b = bf16_reciprocal( b );
    return bf16_mul( a, b );
}

