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
    isFirstInMeasure_ ( false ),
    jumps_            ()
{
    //NOTHING
}

Event::Event( const Event& from ):
    measure_          ( from.measure_ ),
    isFirstInMeasure_ ( from.isFirstInMeasure_ ),
    jumps_            ( from.jumps_ )
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

void Event::addJump( const std::string& label )
{
    if ( label.empty() )
        return;
    for ( auto it = jumps_.begin(); it != jumps_.end(); ++it )
    {
        if ( *it == label )
            return;
    }
    jumps_.push_back( label );
}

void Event::clearJumps()
{
    jumps_.clear();
}

void Event::serializeJumps( std::ostringstream& stream ) const
{
    if ( jumps_.empty() )
        return;
    stream << " @jump ";
    bool first = true;
    for ( auto it = jumps_.begin(); it != jumps_.end(); ++it )
    {
        if ( first )
            first = false;
        else
            stream << ", ";
        stream << *it;
    }
}

bool Event::isEqual( float t1, float t2 ) const
{
    if ( features() & MidiNote )
        return fabs( t1 - t2 ) < EPSILON_MIDI;
    return fabs( t1 - t2 ) < EPSILON;
}
