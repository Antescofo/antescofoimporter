//
//  Pitch.cpp
//  antescofo_converter
//
//  Created by baba on 17/06/15.
//  Copyright (c) 2015 ircam. All rights reserved.
//

#include "Pitch.h"

using namespace antescofo;
using namespace std;

Pitch::Pitch( int pitch ):
    pitch_( pitch ),
    features_( None )
{
    //NOTHING ELSE
}

Pitch::Pitch( int pitch, const EntryFeatures feature ):
    pitch_( pitch ),
    features_( feature )
{
    //NOTHING ELSE
}

Pitch::Pitch( const Pitch& otherPitch ):
    pitch_( otherPitch.pitch_ ),
    features_( otherPitch.features_ )
{
    //NOTHING ELSE
}

Pitch::~Pitch()
{
    //NOTHING
}

bool Pitch::operator==( const Pitch& otherPitch ) const
{
    return ( pitch_ == otherPitch.pitch_ );
}

bool Pitch::operator<( const Pitch& otherPitch ) const
{
    return abs( pitch_ ) < abs( otherPitch.pitch_ );
}

Pitch& Pitch::operator=( const Pitch& otherPitch )
{
    pitch_ = otherPitch.pitch_;
    features_ = otherPitch.features_;
    return *this;
}

bool Pitch::comparePitch( const Pitch& p1, const Pitch& p2 )
{
    return p1 < p2;
}

string Pitch::serialize() const
{
    int midiCents = pitch_;
    if ( abs( midiCents-( midiCents/100 )*100 ) == 1 )   //pitch marked as trill (ex: 6400-> 6401)
    {
        midiCents = abs( ( midiCents/100 )*100 );
    }
    unsigned int absCents = abs( midiCents );
    if ( features_&DisplayCents || absCents == 0 || absCents%100 != 0 )
        return to_string( midiCents );
    unsigned int relative = ( absCents/100 )%12;
    char symbolic[3];
    char* ptr = symbolic;
    switch ( relative )
    {
        case 0: *ptr = 'C'; break;
        case 1: *ptr++ = 'C'; *ptr = '#'; break;
        case 2: *ptr = 'D'; break;
        case 3: *ptr++ = 'D'; *ptr = '#'; break;
        case 4: *ptr = 'E'; break;
        case 5: *ptr = 'F'; break;
        case 6: *ptr++ = 'F'; *ptr = '#'; break;
        case 7: *ptr = 'G'; break;
        case 8: *ptr++ = 'G'; *ptr = '#'; break;
        case 9: *ptr = 'A'; break;
        case 10: *ptr++ = 'A'; *ptr = '#'; break;
        case 11: *ptr = 'B'; break;
    }
    *(++ptr) = 0;
    return ( (features_&Tiedbackwards)? string( "-"):string("") ) + string( symbolic ) + to_string( ((int) absCents/1200) - 1 );
}

void Pitch::setFeature( const EntryFeatures bits )
{
    features_ |= bits;
}

bool Pitch::isTiedBackwards() const
{
    return ( features_ & Tiedbackwards );
}

bool Pitch::isFlat() const
{
    return ( features_ & Flat );
}

bool Pitch::isSharp() const
{
    return ( features_ & Sharp );
}

bool Pitch::isTrillPitch() const
{
    return ( features_ & Trill );
}

bool Pitch::isRest() const
{
    return ( pitch_ == 0 );
}

