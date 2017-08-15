//
// .-. . . .-. .-. .-. .-. .-. .-. .-.
// |-| |\|  |  |-  `-. |   | | |-  | |
// ` ' ' `  '  `-' `-' `-' `-' '   `-'
//
// .-. .  . .-. .-. .-. .-. .-. .-.
//  |  |\/| |-' | | |(   |  |-  |(
// `-' '  ` '   `-' ' '  '  `-' ' '
//
//  Tempo.cpp
//
//  Created by Robert Piéchaud on 09/05/15.
//  Copyright (c) 2017 Antescofo. All rights reserved.
//

#include "Tempo.h"

using namespace antescofo;

Tempo::Tempo( bool status ):
    Event  (),
    status_ ( status )
{
    //NOTHING
}

Tempo::~Tempo()
{
    //NOTHING
}

bool Tempo::hasNotes() const
{
    return false;
}

void Tempo::serialize( std::ostringstream& stream )
{
    stream << "TEMPO ";
    if ( status_ )
        stream << "ON\n";
    else
        stream << "OFF\n";
}
