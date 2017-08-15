//
// .-. . . .-. .-. .-. .-. .-. .-. .-.
// |-| |\|  |  |-  `-. |   | | |-  | |
// ` ' ' `  '  `-' `-' `-' `-' '   `-'
//
// .-. .  . .-. .-. .-. .-. .-. .-.
//  |  |\/| |-' | | |(   |  |-  |(
// `-' '  ` '   `-' ' '  '  `-' ' '
//
//  MidiNoteAction.h
//
//  Created by Robert Piéchaud on 03/10/15.
//  Copyright (c) 2017 Antescofo. All rights reserved.
//

#ifndef _IMPORTER_MIDI_NOTE_ACTION_
#define _IMPORTER_MIDI_NOTE_ACTION_

#include <sstream>

namespace antescofo
{
    class MidiNoteAction
    {
    public:
        MidiNoteAction();
        MidiNoteAction( float delay, int note, int velocity, int channel );
        MidiNoteAction( const MidiNoteAction& anAction );
        virtual ~MidiNoteAction();
        
        void serialize( std::ostringstream& stream ) const;
        void clear();
        
    public:
        float   delay_;
        int     note_;
        int     velocity_;
        int     channel_;
    };
}

#endif // _IMPORTER_MIDI_NOTE_ACTION_
