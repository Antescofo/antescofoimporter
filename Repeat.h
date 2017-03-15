//
// .-. . . .-. .-. .-. .-. .-. .-. .-.
// |-| |\|  |  |-  `-. |   | | |-  | |
// ` ' ' `  '  `-' `-' `-' `-' '   `-'
//
// .-. .  . .-. .-. .-. .-. .-. .-.
//  |  |\/| |-' | | |(   |  |-  |(
// `-' '  ` '   `-' ' '  '  `-' ' '
//
//  Repeat.h
//
//  Created by Robert Piéchaud on 09/05/15.
//  Copyright (c) 2015 ircam. All rights reserved.
//

#ifndef _ANTESCOFO_IMPORTER_Repeat_
#define _ANTESCOFO_IMPORTER_Repeat_

#include <stdio.h>
#include <sstream>
#include "Event.h"

namespace antescofo
{
    class Repeat: public Event   //to be used with Repeat OFF/ON events
    {
    public:
        explicit Repeat( int measure, int direction, int ending );
        virtual ~Repeat();
        
        virtual void        serialize( std::ostringstream& stream );
        virtual EventType   type() const;
        virtual bool        hasNotes() const;
        virtual float       start() const;
        
    private:
        int     direction_;
        int     ending_;
    };
}


#endif // _ANTESCOFO_IMPORTER_Repeat_
