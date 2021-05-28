
#include <stdbool.h>
#include <stdint.h>
#include "platform.h"
#include "internals.h"
#include "specialize.h"
#include "softfloat.h"


float32_t f32_div_( float32_t a, float32_t b )
{
    union ui32_f32 uA;
    uint_fast32_t uiA;
    bool signA;
    int_fast16_t expA;
    uint_fast32_t sigA;
    union ui32_f32 uB;
    uint_fast16_t uiB;
    bool signB;
    int_fast16_t expB;
    uint_fast32_t sigB;
    bool signZ;
    int_fast16_t expZ;
    union ui32_f32 uZ;

    
    /**
     * get the signal、exp and frac bits of a、b
     */
    uA.f = a;
    uiA = uA.ui;
    signA = signF32UI( uiA );
    expA  = expF32UI( uiA );
    sigA  = fracF32UI( uiA );
    uB.f = b;
    uiB = uB.ui;
    signB = signF32UI( uiB );
    expB  = expF32UI( uiB );
    sigB  = fracF32UI( uiB );
    
    signZ = signA ^ signB;
    /**
     * handle NaN and inf
     */
    if ( expA == 0xFF ) 
    {
        if ( sigA )
        {//a is NaN
            uZ.ui = signZ ? 0xffc00000 : 0x7fc00000;//-NaN or NaN
            return uZ.f;
        }
        if ( expB == 0xFF ) 
        {
            if ( sigB )
            {//b is NaN
                uZ.ui = signZ ? 0xffc00000 : 0x7fc00000;//-NaN or NaN
                return uZ.f;
            }
            //b is inf, invalid
            softfloat_raiseFlags( softfloat_flag_invalid );
            uZ.ui = signZ ? 0xffc00000 : 0x7fc00000;//-NaN or NaN
            return uZ.f;
        }
        //a is inf, return inf
        uZ.ui = signZ ? 0xff800000 : 0x7f800000;//-inf or inf
        return uZ.f;
    }
    if ( expB == 0xFF ) 
    {
        if ( sigB )
        {//b is NaN
            uZ.ui = signZ ? 0xffc00000 : 0x7fc00000;//-NaN or NaN
            return uZ.f;
        }
        //b is inf, return 0
        uZ.ui = signZ ? 0x80000000 : 0x0;
        return uZ.f;
    }

    /** handle 0
     */
    if ( ! ( expB | sigB ) )
    { // b is 0
        if ( ! (expA | sigA) ) 
        {//a isn't 0, invalid
            softfloat_raiseFlags( softfloat_flag_invalid );
            uZ.ui = signZ ? 0xffc00000 : 0x7fc00000;//-NaN or NaN
            return uZ.f;
        }
        //a is 0
        softfloat_raiseFlags( softfloat_flag_infinite );
        uZ.ui = signZ ?  0xff800000 : 0x7f800000;//-inf or inf
        return uZ.f;
    }
    if ( ! (expA | sigA) ) 
    {//return 0
        uZ.ui = signZ ? 0x80000000 : 0x0;
        return uZ.f;
    }

    float32_t makeup24 = { 0xc000000 };
    //in order to make recip(b) != inf
    if( ( b.v & 0x7FFFFFFF ) <= 0x200000 )
    {//a and b mul 2^23

        b.v += ( 24 << 23 );
        b = signB ? f32_add( b, makeup24 ) : f32_sub( b, makeup24 );

        if( 0 == expA )
        {
            a.v += ( 24 << 23 );
            a = signA ? f32_add( a, makeup24 ) : f32_sub( a, makeup24 ); 
        }
        else
        {
            expA += 23;
            if( expA > 254)
            {
                a.v =  signA ? 0xff800000 : 0x7f800000;//-inf or inf
            }
            else
            {
                a.v += ( (uint32_t)23 << 23 );
            }
        } 
    }

    //use mul and recip to compute div
    b = f32_reciprocal( b );
    return f32_mul( a, b );
}

