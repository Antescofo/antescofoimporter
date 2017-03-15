//
// .-. . . .-. .-. .-. .-. .-. .-. .-.
// |-| |\|  |  |-  `-. |   | | |-  | |
// ` ' ' `  '  `-' `-' `-' `-' '   `-'
//
// .-. .  . .-. .-. .-. .-. .-. .-.
//  |  |\/| |-' | | |(   |  |-  |(
// `-' '  ` '   `-' ' '  '  `-' ' '
//
//  MidiNoteAction.cpp
//
//  Created by Robert Piéchaud on 03/10/15.
//  Copyright (c) 2015 ircam. All rights reserved.
//

#include "MidiNoteAction.H"

using namespace antescofo;

MidiNoteAction::MidiNoteAction():
    delay_      (0),
    note_       (0),
    velocity_   (0),
    channel_    (0)
{
    //NOTHING
}

MidiNoteAction::MidiNoteAction( float delay, int note, int velocity, int channel ):
    delay_      (delay),
    note_       (note),
    velocity_   (velocity),
    channel_    (channel)
{
    //NOTHING
}


MidiNoteAction::MidiNoteAction( const MidiNoteAction& anAction ):
    delay_( anAction.delay_ ),
    note_( anAction.note_ ),
    velocity_( anAction.velocity_ ),
    channel_( anAction.channel_ )
{
    //NOTHING
}

MidiNoteAction::~MidiNoteAction()
{
    //NOTHING
};

void MidiNoteAction::clear()
{
    delay_ = 0;
    note_ = 0;
    velocity_ = 0;
    channel_ = 0;
}

void MidiNoteAction::serialize( std::ostringstream& stream ) const
{
    stream << delay_ << " mnote " << note_ << " " << velocity_ << " " << channel_;
    if ( velocity_ == 0 )
        stream << " ;noteoff";
}
