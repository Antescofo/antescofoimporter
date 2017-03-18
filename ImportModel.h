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
#import <vector>

namespace antescofo
{
    class Event;
    class ImporterWrapper;
    class Pitch;
    
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
        void  appendEvent( Event* event );
        void  insertOrReplaceEvent( Event* event );
        void  insertFirstEventInMeasure( Event* event );
        void  replaceEvent( Event* event );
        float addNote( float measure, float start, float duration, Pitch& pitch );
        float addRepeatedNotes( float measure, float start, float duration, float divisions, Pitch& pitch );
        Event* findMeasure( float measure ) const;
        float getMeasureDuration( float measure ) const;
        float getMeasureAccumulutatedBeats( float measure ) const;
        bool  areThereNotesInMeasure( float measure ) const;
        void  beautify();
        
        void  queryTempi( std::vector<std::string>& tempi );
        float queryFirstMeasureDuration();
        std::deque<std::pair<float, std::string> > queryPulseChanges(); //TODO: const;  // retrieve the list of pulse changes
        void showPulseChangesAsNim( std::ostringstream& stream ); //TODO: const;    // retrieve the list of pulse changes as
    
        static float fractionToFloat(std::string& str);
        
        std::string         displayScoreInfo() const;
        std::ostringstream& getSerialization();
        
    private:
        void serialize();
        void setHeader();
        void displayMetadata();
        void setDate( std::ostringstream& stream ) const;
        std::deque<Event*>::iterator splitEvent( std::deque<Event*>::iterator it, float splitTime );
        std::deque<Event*>::iterator emplaceEvent( std::deque<Event*>::iterator it, Event* event );
        void consolidateNotesAndRests();
        void checkTies();
        void consolidateTemposAndMeasures();
        inline bool isEqual( float t1, float t2 );
        inline bool isAfter( float t1, float t2 );
        inline bool isBefore( float t1, float t2 );
        
    private:
        ImporterWrapper&    wrapper_;
        std::deque<Event*>  events_;
        bool                verbose_;
        std::ostringstream  serialization_;
        std::string         fileOrigin_;
        std::string         version_;
        std::string         credits_;
    };
}

#endif //_ANTESCOFO_IMPORT_MODEL_
