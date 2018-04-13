//
//  Nosync.h
//  antescofo_converter
//
//  Created by José Echeveste on 13/04/2018.
//  Copyright © 2018 ircam. All rights reserved.
//

#ifndef _ANTESCOFO_IMPORTER_NOSYNC_
#define _ANTESCOFO_IMPORTER_NOSYNC_

#include <stdio.h>
#include <sstream>
#include "Event.h"

namespace antescofo
{
    class NosyncOnNote: public Event   //to be used with TEMPO OFF/ON events
    {
    public:
        explicit NosyncOnNote();
        virtual ~NosyncOnNote();
        
        void        serialize( std::ostringstream& stream ) override;
        EventType   type() const override { return Event_NosyncOnNote; }
        bool        hasNotes() const override;
    };
    
    class NosyncStart: public Event   //to be used with TEMPO OFF/ON events
    {
    public:
        explicit NosyncStart();
        virtual ~NosyncStart();
        
        void        serialize( std::ostringstream& stream ) override;
        EventType   type() const override { return Event_NosyncStart; }
        bool        hasNotes() const override;
    };
    
    class NosyncStop: public Event   //to be used with TEMPO OFF/ON events
    {
    public:
        explicit NosyncStop();
        virtual ~NosyncStop();
        
        void        serialize( std::ostringstream& stream ) override;
        EventType   type() const override { return Event_NosyncStop; }
        bool        hasNotes() const override;
    };
}


#endif // _ANTESCOFO_IMPORTER_NOSYNC_



