//
// .-. . . .-. .-. .-. .-. .-. .-. .-.
// |-| |\|  |  |-  `-. |   | | |-  | |
// ` ' ' `  '  `-' `-' `-' `-' '   `-'
//
// .-. .  . .-. .-. .-. .-. .-. .-.
//  |  |\/| |-' | | |(   |  |-  |(
// `-' '  ` '   `-' ' '  '  `-' ' '
//
//  MIDIDefines.h
//
//  Created by Robert Piéchaud on 02/10/15.
//  Copyright (c) 2017 Antescofo. All rights reserved.
//

#ifndef _IMPORTER_MIDI_DEFINES_
#define _IMPORTER_MIDI_DEFINES_

#define MIDI_MAX_SIZE 10000000

enum
{
    MIDI_HEADER = 'MThd',
    MIDI_TRACK = 'MTrk'
};

enum
{
    MIDI_NOTE_OFF               = 0x8,
    MIDI_NOTE_ON                = 0x9,
    MIDI_NOTE_AFTERTOUCH        = 0xA,
    MIDI_NOTE_CONTROL_CHANGE    = 0xB,
    MIDI_NOTE_PROGRAM_CHANGE    = 0xC,
    MIDI_NOTE_PRESSURE          = 0xD,
    MIDI_NOTE_PROGRAM_PITCHBEND = 0xE
};

enum
{
    MIDI_META_TRACKNAME     = 0x03,
    MIDI_META_ENDOFTRACK    = 0x2F,
    MIDI_META_TEMPO         = 0x51,
    MIDI_META_TIMESIGNATURE = 0x58,
    MIDI_META_KEYSIGNATURE  = 0x59
};

extern const char* GMSoundSet [];

#endif //_IMPORTER_MIDI_DEFINES_
