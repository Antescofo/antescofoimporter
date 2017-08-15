//
// .-. . . .-. .-. .-. .-. .-. .-. .-.
// |-| |\|  |  |-  `-. |   | | |-  | |
// ` ' ' `  '  `-' `-' `-' `-' '   `-'
//
// .-. .  . .-. .-. .-. .-. .-. .-.
//  |  |\/| |-' | | |(   |  |-  |(
// `-' '  ` '   `-' ' '  '  `-' ' '
//
//  Tempo.h
//
//  Created by Robert Piéchaud on 09/05/15.
//  Copyright (c) 2017 Antescofo. All rights reserved.
//

#ifndef _ANTESCOFO_IMPORTER_TEMPO_
#define _ANTESCOFO_IMPORTER_TEMPO_

#include <stdio.h>
#include <sstream>
#include "Event.h"

namespace antescofo
{
    class Tempo: public Event   //to be used with TEMPO OFF/ON events
    {
    public:
        explicit Tempo( bool status );
        virtual ~Tempo();
        
        void        serialize( std::ostringstream& stream ) override;
        EventType   type() const override { return Event_Tempo; }
        bool        hasNotes() const override;
        
    private:
        bool    status_;
    };
}


#endif // _ANTESCOFO_IMPORTER_TEMPO_
