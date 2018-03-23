//
// .-. . . .-. .-. .-. .-. .-. .-. .-.
// |-| |\|  |  |-  `-. |   | | |-  | |
// ` ' ' `  '  `-' `-' `-' `-' '   `-'
//
// .-. .  . .-. .-. .-. .-. .-. .-.
//  |  |\/| |-' | | |(   |  |-  |(
// `-' '  ` '   `-' ' '  '  `-' ' '
//
//  BeatPerMinute.cpp
//
//  Created by Robert Piéchaud on 09/05/15.
//  Copyright (c) 2017 Antescofo. All rights reserved.
//

#include "BeatPerMinute.h"
#include <math.h>

using namespace antescofo;

BeatPerMinute::BeatPerMinute( float measure, float start, float value, float originalBeats, float originalBase, bool generated ):
Event       (),
bpm_        ( value ),
start_      ( start ),
beats_      ( originalBeats ),
base_       ( originalBase ),
generated_  ( generated )
{
    measure_ = measure;
}

BeatPerMinute::BeatPerMinute( float measure, float start, float bmp ):
    Event       (),
    bpm_        ( bmp ),
    start_      ( start ),
    beats_      ( 0.0 ),
    base_       ( 0.0 ),
    generated_  ( false )
{
    measure_ = measure;
}

BeatPerMinute::~BeatPerMinute()
{
    //NOTHING
}

bool BeatPerMinute::hasNotes() const
{
    return false;
}

float BeatPerMinute::start() const
{
    return start_;
}


/*! \brief  Copy a BeatPerMinute event at given score position
 */
BeatPerMinute* BeatPerMinute::copyAt( float measure, float start, BeatPerMinute const& bpm) {
    return new BeatPerMinute(measure, start, bpm.bpm_, bpm.beats_, bpm.base_, false);
}


void BeatPerMinute::serialize( std::ostringstream& stream )
{
    //stream.precision(4);
    if ( roundf( bpm_ ) == bpm_ )
        stream << "BPM " << (int) bpm_;
    else
    {
        char buffer [12];
        sprintf( buffer, "%.2f", bpm_ );
        stream << "BPM " << buffer;
    }
    if ( measure_ > 1 && generated_ )
        stream << " @modulate";
    if ( beats_ && base_ && beats_ != bpm_  )
    {
        int base = (int) ((float) base_*8 );
        stream << " ; ( ← ";
        switch ( base )
        {
            case 2: stream << "𝅘𝅥𝅯"; break;
            case 3: stream << "𝅘𝅥𝅯."; break;
            case 4: stream << "𝅘𝅥𝅮"; break;
            case 6: stream << "𝅘𝅥𝅮."; break;
            case 12: stream << "𝅘𝅥."; break;
            case 16: stream << "𝅗𝅥"; break;
            case 24: stream << "𝅗𝅥."; break;
            case 32: stream << "𝅝"; break;
            case 48: stream << "𝅝."; break;
            default:
            case 8: stream << "𝅘𝅥";
                break;
        }
        stream << " =" << beats_ << ") ";
    }
    stream << std::endl;
}
