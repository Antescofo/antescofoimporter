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
//  Copyright (c) 2017 Antescofo. All rights reserved.
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
        explicit BeatPerMinute( const std::string& measure, float start, float bpm, float originalBeats, float originalBase, bool generated = 0 );
        explicit BeatPerMinute( const std::string& measure, float start, float bpm );  //used mostly for MIDI import
        virtual ~BeatPerMinute();
        
        static BeatPerMinute* copyAt( const std::string& measure, float start, BeatPerMinute const& bpm);
        
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
