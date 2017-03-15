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
//  Copyright (c) 2015 ircam. All rights reserved.
//

#include "Event.h"

using namespace antescofo;

Event::Event():
    measure_          ( -1 ),
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

int Event::measure() const
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

