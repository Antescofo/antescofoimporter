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
//  Copyright (c) 2017 Antescofo. All rights reserved.
//

#ifndef _REPEAT_EVENT_
#define _REPEAT_EVENT_

#include <stdio.h>
#include <sstream>
#include "Event.h"

namespace antescofo
{
    class Repeat: public Event   //to be used with Repeat OFF/ON events
    {
    public:
        explicit Repeat( float measure, int direction, int ending );
        virtual ~Repeat();
        
        void        serialize( std::ostringstream& stream ) override;
        EventType   type() const override;
        bool        hasNotes() const override;
        float       start() const override;
        
    private:
        int     direction_;
        int     ending_;
    };
}


#endif // _REPEAT_EVENT_
