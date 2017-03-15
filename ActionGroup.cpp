//
// .-. . . .-. .-. .-. .-. .-. .-. .-.
// |-| |\|  |  |-  `-. |   | | |-  | |
// ` ' ' `  '  `-' `-' `-' `-' '   `-'
//
// .-. .  . .-. .-. .-. .-. .-. .-.
//  |  |\/| |-' | | |(   |  |-  |(
// `-' '  ` '   `-' ' '  '  `-' ' '
//
//  ActionGroup.cpp
//
//  Created by Robert Piéchaud on 03/10/15.
//  Copyright (c) 2015 ircam. All rights reserved.
//

#include "ActionGroup.h"
#include "MIDIDefines.h"

using namespace antescofo;
using namespace std;

ActionGroup::ActionGroup( const string& name  ):
    Event       (),
    GMpatch_    ( -1 ),
    name_       ( name )
{
    measure_ = 0;
}

ActionGroup::~ActionGroup()
{
    //NOTHING
}

bool ActionGroup::hasNotes() const
{
    return false;
}

EventType ActionGroup::type() const
{
    return Action_ActionGroup;
}

void ActionGroup::setGMpatch( int patch )
{
    GMpatch_ = patch;
}

void ActionGroup::appendMidiNote( const MidiNoteAction& note )
{
    notes_.push_back( note );
}

void ActionGroup::serialize( std::ostringstream& stream )
{
    stream << endl << "group " << name_ << " @tight @local {" << endl;
    if ( GMpatch_ >= 0 )
        stream << "    ; General MIDI sound: " << GMSoundSet[GMpatch_] << endl;
    stream << "    ; delay msgrecvr pitch velocity channel" << endl;
    for ( auto cit = notes_.begin(); cit != notes_.end(); ++cit )
    {
        stream << "    ";
        (*cit).serialize( stream );
        stream << endl;
    }
    stream << "}" << std::endl;
}
