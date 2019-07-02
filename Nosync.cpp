//
//  Nosync.cpp
//  antescofo_importer
//
//  Created by José Echeveste on 13/04/2018.
//  Copyright © 2018 ircam. All rights reserved.
//

#include "Nosync.h"

using namespace antescofo;

NosyncOnNote::NosyncOnNote():
Event  ()
{
    //NOTHING
}

NosyncOnNote::~NosyncOnNote()
{
    //NOTHING
}

bool NosyncOnNote::hasNotes() const
{
    return false;
}

void NosyncOnNote::serialize( std::ostringstream& stream )
{
}

NosyncStart::NosyncStart():
Event  ()
{
    //NOTHING
}

NosyncStart::~NosyncStart()
{
    //NOTHING
}

bool NosyncStart::hasNotes() const
{
    return false;
}

void NosyncStart::serialize( std::ostringstream& stream )
{
}

NosyncStop::NosyncStop():
Event  ()
{
    //NOTHING
}

NosyncStop::~NosyncStop()
{
    //NOTHING
}

bool NosyncStop::hasNotes() const
{
    return false;
}

void NosyncStop::serialize( std::ostringstream& stream )
{
}
