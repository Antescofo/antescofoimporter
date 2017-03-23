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
        explicit BeatPerMinute( float measure, float bpm, float originalBeats, float originalBase, bool generated = 0 );
        explicit BeatPerMinute( float measure, float start, float bpm );  //used mostly for MIDI import
        virtual ~BeatPerMinute();
        
        void        serialize( std::ostringstream& stream ) override;
        EventType   type() const override { return Event_BeatPerMinute; }
        bool        hasNotes() const override;
        float       start() const override;
        
        float   tempo() const { return bpm_; }
        rational beatunit() const;
        
    private:
        float   bpm_;
        float   start_;
        float   beats_;
        float   base_;
        bool    generated_;
    };
}

#endif //_ANTESCOFO_IMPORTER_BPM_
