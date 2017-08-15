//
// .-. . . .-. .-. .-. .-. .-. .-. .-.
// |-| |\|  |  |-  `-. |   | | |-  | |
// ` ' ' `  '  `-' `-' `-' `-' '   `-'
//
// .-. .  . .-. .-. .-. .-. .-. .-.
//  |  |\/| |-' | | |(   |  |-  |(
// `-' '  ` '   `-' ' '  '  `-' ' '
//
//  ActionGroup.h
//
//  Created by Robert Piéchaud on 03/10/15.
//  Copyright (c) 2017 Antescofo. All rights reserved.
//

#ifndef _IMPORTER_ACTION_GROUP_
#define _IMPORTER_ACTION_GROUP_

#include <stdio.h>
#include <vector>
#include "Event.h"
#include "MidiNoteAction.h"

namespace antescofo
{
    class ActionGroup: public Event
    {
    public:
        explicit ActionGroup( const std::string& name );
        virtual ~ActionGroup();
        
        void        serialize( std::ostringstream& stream ) override;
        EventType   type() const override;
        bool        hasNotes() const override;
        void        setGMpatch( int patch );
        void        appendMidiNote( const MidiNoteAction& note );
        
    private:
        std::string name_;
        int         GMpatch_;
        std::vector<MidiNoteAction> notes_;
    };
}


#endif // _IMPORTER_ACTION_GROUP_
