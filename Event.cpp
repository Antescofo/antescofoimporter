//
// .-. . . .-. .-. .-. .-. .-. .-. .-.
// |-| |\|  |  |-  `-. |   | | |-  | |
// ` ' ' `  '  `-' `-' `-' `-' '   `-'
//
// .-. .  . .-. .-. .-. .-. .-. .-.
//  |  |\/| |-' | | |(   |  |-  |(
// `-' '  ` '   `-' ' '  '  `-' ' '
//
//  Event.cpp
//
//  Created by Robert Piéchaud on 09/05/15.
//  Copyright (c) 2017 Antescofo. All rights reserved.
//

#include "Event.h"

using namespace antescofo;

Event::Event():
    measure_          ( "" ),
    isFirstInMeasure_ ( false )
{
    //NOTHING
}

Event::Event( const Event& from ):
    measure_          ( from.measure_ ),
    isFirstInMeasure_ ( from.isFirstInMeasure_ )
{
    //NOTHING
}

Event::~Event()
{
    //NOTHING
}

const std::string Event::measure() const
{
    return measure_;
}

bool Event::isMeasure() const
{
    return false;
}

float Event::duration() const
{
    return 0.0;
}

float Event::start() const
{
    return -1.0;
}

Event* Event::duplicate() const
{
    return nullptr;
}

void Event::setFirstInMeasure( bool status )
{
    isFirstInMeasure_ = status;
}

bool Event::isFirstInMeasure() const
{
    return isFirstInMeasure_;
}

bool Event::isEqual( float t1, float t2 ) const
{
    if ( features() & MidiNote )
        return fabs( t1 - t2 ) < EPSILON_MIDI;
    return fabs( t1 - t2 ) < EPSILON;
}
