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

Pitch::Pitch():
    midiCents_  ( 0 ),
    note_       ( 0 ),
    accidental_ ( 0 ),
    octave_     ( 0 ),
    features_   ( None )
{
    //NOTHING ELSE
}

Pitch::Pitch( int cents ):
    midiCents_  ( cents ),
    note_       ( 0 ),
    accidental_ ( 0 ),
    octave_     ( 0 ),
    features_   ( None )
{
    if ( cents < 0 )
        features_ |= Tiedbackwards;
}

Pitch::Pitch( int cents, const EntryFeatures feature ):
    midiCents_( cents ),
    note_       ( 0 ),
    accidental_ ( 0 ),
    octave_     ( 0 ),
    features_( feature )
{
    features_ &= ~OriginalEnharmony;
}

Pitch::Pitch( const Pitch& otherPitch ):
    midiCents_( otherPitch.midiCents_ ),
    note_       ( otherPitch.note_ ),
    accidental_ ( otherPitch.accidental_ ),
    octave_     ( otherPitch.octave_ ),
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
    return ( midiCents_ == otherPitch.midiCents_ );
}

bool Pitch::operator==( int cents ) const
{
    return ( midiCents_ == cents );
}

bool Pitch::operator<( const Pitch& otherPitch ) const
{
    return midiCents_ < otherPitch.midiCents_;
}

Pitch& Pitch::operator=( const Pitch& otherPitch )
{
    midiCents_ = otherPitch.midiCents_;
    features_ = otherPitch.features_;
    note_ = otherPitch.note_;
    accidental_ = otherPitch.accidental_;
    octave_ = otherPitch.octave_;
    return *this;
}

bool Pitch::comparePitch( const Pitch& p1, const Pitch& p2 )
{
    return p1 < p2;
}

void Pitch::resetNote()
{
    note_ = 0;
    accidental_ = 0;
    octave_ = 0;
}

string Pitch::serialize() const
{
    bool isTied = (features_&Tiedbackwards) && midiCents_ != 0;
    char symbolic[4];
    char* ptr = symbolic;
    int midiCents = midiCents_;
    if ( midiCents-( midiCents/100 )*100 == 1 )   //pitch marked as trill (ex: 6400-> 6401)
    {
        midiCents = ( midiCents/100 )*100;
    }
    int microtone = midiCents%100;
    if ( microtone == 0 && features_&OriginalEnharmony && note_ != 0 && !( features_& (Transposed&NaturalHarmonic&Trill) ) )
    {
        *ptr = note_;
        if ( accidental_ == -2 )
        {
            *(++ptr) = 'b';
            *(++ptr) = 'b';
        }
        else if ( accidental_ == -1 )
        {
            *(++ptr) = 'b';
        }
        else if ( accidental_ == 1 )
        {
            *(++ptr) = '#';
        }
        else if ( accidental_ == 2 )
        {
            *(++ptr) = 'x';
        }
        *(++ptr) = 0;
        string display = (isTied? string( "-"):string("") ) + symbolic + to_string( octave_ );
        /*
        if ( microtone )
        {
            display += (accidental_>=0?"+":"-");
            display += to_string(microtone);
        }
        */
        return display;
    }
    if ( features_&DisplayCents || midiCents == 0 || midiCents%100 != 0 )
        return ((isTied && midiCents != 0)? string( "-"):string("") ) + to_string( midiCents );
    unsigned int relative = ( midiCents/100 )%12;
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
    return ( isTied? string( "-"):string("") ) + symbolic + to_string( ((int) midiCents/1200) - 1 );
}

void Pitch::setFeatureBits( const EntryFeatures bits )
{
    features_ |= bits;
}

bool Pitch::isTiedBackwards() const
{
    return ( features_ & Tiedbackwards );
}

bool Pitch::isFlat() const
{
    return ( accidental_ == -1 );
}

bool Pitch::isSharp() const
{
    return ( accidental_ == 1 );
}

bool Pitch::isTrillPitch() const
{
    return ( features_ & Trill );
}

bool Pitch::isRest() const
{
    return ( midiCents_ == 0 );
}

void Pitch::setTied( bool status )
{
    if ( status)
    {
        features_ |= Tiedbackwards;
    }
    else
    {
        features_ &= ~Tiedbackwards;
    }
}
