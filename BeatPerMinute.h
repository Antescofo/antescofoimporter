//
// .-. . . .-. .-. .-. .-. .-. .-. .-.
// |-| |\|  |  |-  `-. |   | | |-  | |
// ` ' ' `  '  `-' `-' `-' `-' '   `-'
//
// .-. .  . .-. .-. .-. .-. .-. .-.
//  |  |\/| |-' | | |(   |  |-  |(
// `-' '  ` '   `-' ' '  '  `-' ' '
//
//  BeatPerMinute.h
//
//  Created by Robert Piéchaud on 09/05/15.
//  Copyright (c) 2015 ircam. All rights reserved.
//

#ifndef _ANTESCOFO_IMPORTER_BPM_
#define _ANTESCOFO_IMPORTER_BPM_

#include <stdio.h>
#include <sstream>
#include "Event.h"

namespace antescofo
{
    class BeatPerMinute: public Event
    {
    public:
        explicit BeatPerMinute( int measure, float bpm, float originalBeats, float originalBase );
        explicit BeatPerMinute( int measure, float start, float bpm );  //used mostly for MIDI import
        virtual ~BeatPerMinute();
        
        virtual void        serialize( std::ostringstream& stream );
        virtual EventType   type() const { return Event_BeatPerMinute; }
        virtual bool        hasNotes() const;
        virtual float       start() const;
        
        float   tempo() const { return bpm_; }
        
    private:
        float   bpm_;
        float   start_;
        float   beats_;
        float   base_;
    };
}

#endif //_ANTESCOFO_IMPORTER_BPM_
