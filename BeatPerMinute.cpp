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
//  Copyright (c) 2015 ircam. All rights reserved.
//

#include "BeatPerMinute.h"
#include <math.h>

using namespace antescofo;

BeatPerMinute::BeatPerMinute( int measure, float value, float originalBeats, float originalBase ):
    Event       (),
    bpm_        ( value ),
    start_      ( 0.0 ),
    beats_      ( originalBeats ),
    base_       ( originalBase )
{
    measure_ = measure;
}

BeatPerMinute::BeatPerMinute( int measure, float start, float bmp ):
    Event       (),
    bpm_        ( bmp ),
    start_      ( start ),
    beats_      ( 0.0 ),
    base_       ( 0.0 )
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

void BeatPerMinute::serialize( std::ostringstream& stream )
{
    if ( roundf( bpm_ ) == bpm_ )
        stream << "BPM " << (int) bpm_;
    else
        stream << "BPM " << (float) bpm_;
    if ( beats_ && base_ )
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
