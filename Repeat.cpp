//
// .-. . . .-. .-. .-. .-. .-. .-. .-.
// |-| |\|  |  |-  `-. |   | | |-  | |
// ` ' ' `  '  `-' `-' `-' `-' '   `-'
//
// .-. .  . .-. .-. .-. .-. .-. .-.
//  |  |\/| |-' | | |(   |  |-  |(
// `-' '  ` '   `-' ' '  '  `-' ' '
//
//  Repeat.cpp
//
//  Created by Robert Piéchaud on 09/05/15.
//  Copyright (c) 2017 Antescofo. All rights reserved.
//

#include "Repeat.h"

using namespace antescofo;

Repeat::Repeat( const std::string& measure, int direction, int ending ):
    Event  (),
    direction_ ( direction ),
    ending_    ( ending )
{
    measure_ = measure;
}

Repeat::~Repeat()
{
    //NOTHING
}

bool Repeat::hasNotes() const
{
    return false;
}

float Repeat::start() const
{
    if ( direction_ == -1 )
        return 99999;
    return 0.0;
}


EventType Repeat::type() const
{
    if ( ending_ > 0 )
        return Event_RepeatEnding;
    return Event_RepeatBar;
}

void Repeat::serialize( std::ostringstream& stream )
{
    if ( direction_ != 0 )
    {
        stream << "; repeat (";
        if ( direction_ == 1 )
            stream << "opening";
        else if ( direction_ == -1 )
            stream << "closing";
        stream << ")";
    }
    else
    {
        if ( ending_ == 1 )
            stream << "; 1st repeat ending";
        else if ( ending_ == 2 )
            stream << "; 2nd repeat ending";
    }
    stream << std::endl;
}
