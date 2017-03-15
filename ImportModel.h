//
// .-. . . .-. .-. .-. .-. .-. .-. .-.
// |-| |\|  |  |-  `-. |   | | |-  | |
// ` ' ' `  '  `-' `-' `-' `-' '   `-'
//
// .-. .  . .-. .-. .-. .-. .-. .-.
//  |  |\/| |-' | | |(   |  |-  |(
// `-' '  ` '   `-' ' '  '  `-' ' '
//
//  Model.h
//
//  Created by Robert Piéchaud on 29/04/15.
//  Copyright (c) 2015 ircam. All rights reserved.
//

#ifndef _ANTESCOFO_IMPORT_MODEL_
#define _ANTESCOFO_IMPORT_MODEL_

#include "Event.h"
#include <string>
#include <deque>

namespace antescofo
{
    class Event;
    class ImporterWrapper;
    
    class ImportModel
    {
    public:
        ImportModel( ImporterWrapper& wrapper );
        virtual ~ImportModel();
        
        bool  save( const std::string& outputPath );
        void  clear();
        void  setFileOrigin( const std::string& origin );
        void  setVersion( const std::string& version );
        void  setCredits( const std::string& credits );
        void  setPitchesAsMidiCents( bool status );
        void  appendEvent( Event* event );
        void  insertOrReplaceEvent( Event* event );
        void  insertFirstEventInMeasure( Event* event );
        void  replaceEvent( Event* event );
        float addNote( int measure, float start, float duration, int cents, EntryFeatures features = None );
        float addRepeatedNotes( int measure, float start, float duration, float divisions, int cents );
        Event* findMeasure( int measure ) const;
        float getMeasureDuration( int measure ) const;
        bool  areThereNotesInMeasure( int measure ) const;
        void  beautify();
        
        std::ostringstream& getSerialization();
        
    private:
        void serialize();
        void setHeader();
        void setDate( std::ostringstream& stream ) const;
        std::deque<Event*>::iterator splitEvent( std::deque<Event*>::iterator it, float splitTime );
        std::deque<Event*>::iterator emplaceEvent( std::deque<Event*>::iterator it, Event* event );
        void consolidateNotesAndRests();
        void consolidateTempos();
        bool showMidiCents() const;
        
    private:
        ImporterWrapper&    wrapper_;
        std::deque<Event*>  events_;
        bool                verbose_;
        bool                displayMidiCents_;
        std::ostringstream  serialization_;
        std::string         fileOrigin_;
        std::string         version_;
        std::string         credits_;
    };
}

#endif //_ANTESCOFO_IMPORT_MODEL_
