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
//  Copyright (c) 2017 Antescofo. All rights reserved.
//

#ifndef _ANTESCOFO_IMPORT_MODEL_
#define _ANTESCOFO_IMPORT_MODEL_

#include "Event.h"
#include "rational.h"
#include <string>
#include <deque>
#include <vector>

namespace antescofo
{
    class Event;
    class ImporterWrapper;
    class Pitch;
    class QueryHandler;
    
    
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
        Event* findFirstBeatPerMinute() const;
        float getMeasureDuration( float measure ) const;
        float getMeasureAccumulutatedBeats( float measure ) const;
        bool  areThereNotesInMeasure( float measure ) const;
        void  beautify();
        void  addWaitForNote(std::string position);
        
        void  queryTempi( std::vector<std::string>& tempi );
        //float queryFirstMeasureDuration();
        //std::deque<std::pair<float, std::string> > queryPulseChanges(); //TODO: const;  //<! retrieve the list of pulse changes
        //void showPulseChangesAsNim( std::ostringstream& stream ); //TODO: const;    //<! print the list of pulse changes in ostream
        
        //std::deque<std::pair<float, rational> > queryTempoBeatUnitChanges() const;  // retrieve the list of beat units given in tempo marks
        //void showTempoBeatUnitChangesAsNim( std::ostringstream& stream ) const;  // print the list of beat units in ostream

        //void showPulseAsNim( std::ostringstream& stream ) const;
        
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
        void consolidateTemposAndMeasures();
        void checkTies();
        
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
        std::vector<std::string> waitForNoteArray_;
    };
    
    class Measure;
    class BeatPerMinute;
    
    class QueryHandler
    {
    public:
        QueryHandler(std::deque<Event* > const& events, ImporterWrapper const* = NULL);
        
        static void showQueries(std::ostringstream&, std::deque<Event* > const&, ImporterWrapper const* = NULL);
        
        //static QueryHandler performQueries(ImportModel const&, ImporterWrapper const*);
        
    protected:
        virtual void showQueriesOn(std::ostringstream&) const; //<! show all queries requested by the wrapper_
        void performQueries(std::deque<Event* > const& events);
        void performQueries(Event const*);
        void performQueriesOn(Event const*);
        void performQueriesOn(Measure const*);
        void performQueriesOn(BeatPerMinute const*);

        ImporterWrapper const*    wrapper_;   //<! the wrapper option will say which queries are performed in performQueries and printed in showQueries
        
        // individual queries
        // Pulse changes
        void showPulseChangesAsNim( std::ostringstream& ) const;
        void queryPulseChange(Measure const*);
    public:
        rational inferePulseSignature(std::string const& timeSignature) const; //<! infère et retourne la pulse correspondante à la timeSignature
    protected:
        static float const EPSILON;
        
        std::deque<std::pair<float, std::string> > pulseChangePositions;
        rational getCurrentPulseDuration() const;// { return (pulseChangePositions.empty() ? rational(1) : rational(pulseChangePositions.back().second) ); }
        
        // Pulses
        void queryPulses(Event const* measure);
        void queryPulses(Measure const* measure);
        void addPulses(float const duration);
        std::deque<float> pulses; //<! list of element, each one represents beat duration until next beat
        void showPulsesAsNim( std::ostringstream& stream ) const;
        float currentPulsePhaseDuration;     //<!
        float accumBeats_;

        // First measure duration
        bool queryFirstMeasureDuration(Measure const* measure);
        float firstMeasureDuration;
        
        // Beat unit of tempo indication
        bool queryTempoBeatUnitChanges(Measure const*);  // retrieve the list of beat units given in tempo marks
        bool queryTempoBeatUnitChanges(BeatPerMinute const*);
        void showTempoBeatUnitChangesAsNim( std::ostringstream& ) const;  // print the list of beat units in ostream
        std::deque<std::pair<float, rational> > tempoBeatUnitChanges;
        rational currentTimeSignature;  //<! last time signature
        rational currentTempoMarkBeatUnit;  //<! beat unit of last tempo mark
        rational currentTempoMarkTimeSignature; //<! time signature of last tempo mark
        
    public:
        // Wait for notes
        static void showWaitForNotes(std::vector<std::string> const& array, std::ostringstream& stream );
    };
}

#endif //_ANTESCOFO_IMPORT_MODEL_
