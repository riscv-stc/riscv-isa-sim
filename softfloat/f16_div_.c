
#include <stdbool.h>
#include <stdint.h>
#include "platform.h"
#include "internals.h"
#include "specialize.h"
#include "softfloat.h"


float16_t f16_div_( float16_t a, float16_t b )
{
    union ui16_f16 uA;
    uint_fast16_t uiA;
    bool signA;
    int_fast8_t expA;
    uint_fast16_t sigA;
    union ui16_f16 uB;
    uint_fast16_t uiB;
    bool signB;
    int_fast8_t expB;
    uint_fast16_t sigB;
    bool signZ;
    int_fast8_t expZ;
    union ui16_f16 uZ;


    /**
     * get the signal、exp and frac bits of a、b
     */
    uA.f = a;
    uiA = uA.ui;
    signA = signF16UI( uiA );
    expA  = expF16UI( uiA );
    sigA  = fracF16UI( uiA );
    uB.f = b;
    uiB = uB.ui;
    signB = signF16UI( uiB );
    expB  = expF16UI( uiB );
    sigB  = fracF16UI( uiB );
    
    signZ = signA ^ signB;
    /**
     * handle NaN and inf
     */
    if ( expA == 0x1F ) 
    {
        if ( sigA )
        {//a is NaN
            uZ.ui = signZ ? 0xfe00 : 0x7e00; //-NaN or NaN
            return uZ.f;
        }
        if ( expB == 0x1F ) 
        {
            if ( sigB )
            {//b is NaN
                uZ.ui = signZ ? 0xfe00 : 0x7e00;//-NaN or NaN
                return uZ.f;
            }
            //b is inf, invalid
            softfloat_raiseFlags( softfloat_flag_invalid );
            uZ.ui = signZ ? 0xfe00 : 0x7e00;//-NaN or NaN
            return uZ.f;
        }
        //a is inf, return inf
        uZ.ui = signZ ? 0xfc00 : 0x7c00;//-inf or inf
        return uZ.f;
    }
    if ( expB == 0x1F ) 
    {
        if ( sigB )
        {//b is NaN
            uZ.ui = signZ ? 0xfe00 : 0x7e00;//-NaN or NaN
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
            uZ.ui = signZ ? 0xfe00 : 0x7e00;//-NaN or NaN
            return uZ.f;
        }
        //a is 0
        softfloat_raiseFlags( softfloat_flag_infinite );
        uZ.ui = signZ ? 0xfc00 : 0x7c00;//-inf or inf
        return uZ.f;
    }
    if ( ! (expA | sigA) ) 
    {//return 0
        uZ.ui = signZ ? 0x8000 : 0x0;
        return uZ.f;
    }

    float16_t makeup11 = { 0x2c00 };
    //in order to make recip(b) != inf
    if( ( b.v & 0x7FFF ) <= 0x100 )
    {//a and b mul 2^10

        b.v += ( 11 << 10 );
        b = signB ? f16_add( b, makeup11 ) : f16_sub( b, makeup11 );

        if( 0 == expA )
        {
            a.v += ( 11 << 10 );
            a = signA ? f16_add( a, makeup11 ) : f16_sub( a, makeup11 ); 
        }
        else
        {
            expA += 10;
            if( expA > 30)
            {
                a.v =  signA ? 0xfc00 : 0x7c00;//-inf or inf
            }
            else
            {
                a.v += ( (uint16_t)10 << 10 );
            }
        } 
    }

    //use mul and recip to compute div
    b = f16_reciprocal( b );
    return f16_mul( a, b );
}

