//
// .-. . . .-. .-. .-. .-. .-. .-. .-.
// |-| |\|  |  |-  `-. |   | | |-  | |
// ` ' ' `  '  `-' `-' `-' `-' '   `-'
//
// .-. .  . .-. .-. .-. .-. .-. .-.
//  |  |\/| |-' | | |(   |  |-  |(
// `-' '  ` '   `-' ' '  '  `-' ' '
//
//  ImportModel.cpp
//
//  Created by Robert Piéchaud on 29/04/15.
//  Copyright (c) 2015 ircam. All rights reserved.
//

#include "ImportModel.h"
#include "ImporterWrapper.h"
#include "Event.h"
#include "Measure.h"
#include "Entry.h"
#include "Pitch.h"
#include "BeatPerMinute.h"
#include <math.h>
#ifdef _WIN32
#include <Winbase.h>
#else
#include <time.h>
#endif

using namespace antescofo;
using namespace std;

ImportModel::ImportModel( ImporterWrapper& wrapper ) :
    wrapper_          ( wrapper ),
    verbose_          ( true )
{
    //NOTHING
}

ImportModel::~ImportModel()
{
    clear();
}

bool ImportModel::save( const string& outputPath )
{
    serialize();
    
    FILE* output = fopen ( outputPath.c_str(), "w" );
    if ( output )
    {
        string content = serialization_.str();
        fprintf( output, "%s", content.c_str() );
        fclose( output );
        return true;
    }
    return false;
}

ostringstream& ImportModel::getSerialization()
{
    serialize();
    return serialization_;
}

string ImportModel::displayScoreInfo() const
{
    string scoreInfo;
    string path = wrapper_.getInputPath();
    size_t sep = path.find_last_of("\\/");
    if ( sep != string::npos )
        path = path.substr( sep + 1, path.size() - sep - 1 );
    scoreInfo = "File: " + path + "\nOrigin: " + fileOrigin_;
    if ( credits_.length() )
        scoreInfo += "\nCredits & info: " + credits_;
    scoreInfo += "\n";
    return scoreInfo;
}

void ImportModel::displayMetadata()
{
    // Front matter
    serialization_ << ";<metadata>" << endl;
    
    // Main matter
    QueryHandler::showQueries(serialization_, events_, &wrapper_);
    
    // Back matter
    serialization_ << ";</metadata>" << endl << endl;
}

void ImportModel::setHeader()
{
    string path = wrapper_.getInputPath();
    size_t sep = path.find_last_of("\\/");
    if ( sep != string::npos )
        path = path.substr( sep + 1, path.size() - sep - 1 );
    
    char bom [4];
    sprintf( bom, "\xef\xbb\xbf" );
    serialization_ << bom; //BOM (Byte order Mark) for UTF8
    serialization_  << "; Antescofo score generated using native importer " << wrapper_.getVersion() << endl
    << "; Copyright (c) IRCAM 2015" << endl
    << "; Designed by Robert Piéchaud" << endl << endl
    << "; Original file: " << path << endl;
    if ( fileOrigin_.size() )
        serialization_ << "; " << fileOrigin_ << endl;
    if ( version_.size() )
        serialization_ << "; " << version_ << endl;
    serialization_ << "; Converted to Antescofo on ";
    setDate( serialization_ );
    if ( credits_.size() )
        serialization_ << "; Credits: " << credits_ << endl;
    serialization_ << endl;
}

void ImportModel::serialize()
{
    serialization_.str( "" );
    // rajout begin
    // We begin by displaying score metadata
    if (wrapper_.displayMetadata())
        displayMetadata();
    // If wrapper asks only for queries, then we do not print score
    if (wrapper_.displayQueriesOnly())
        return;
    // rajout end
    setHeader();
    serialization_ << "; start" << endl;
    auto it = events_.begin();
    int previousMeasure = 0;
    bool setFirst = false;
    while ( it != events_.end() )
    {
        Event* event = *it;
        if ( previousMeasure != event->measure() )
        {
            float duration = getMeasureDuration( previousMeasure );
            if ( previousMeasure != 0 && !areThereNotesInMeasure( previousMeasure ) && duration > 0 )
            {
                serialization_  << "NOTE 0 " << getMeasureDuration( previousMeasure )
                                << " ; empty measure" << endl;
            }
            previousMeasure = event->measure();
            setFirst = true;
        }
        else if ( setFirst && event->hasNotes() )
        {
            event->setFirstInMeasure( true );
        }
        
        //the following scope should ideally be elsewhere (in the musicXML importer itself...) 
        if ( event->features() & GlissandoStart )
        {
            auto itNext = it;
            ++itNext;
            while ( itNext != events_.end() )
            {
                if ( (*itNext)->features() & ( GlissandoEnd | GlissandoStart ) )
                {
                    const std::list<Pitch>& pitches = ((Entry*)(*itNext))->pitches();
                    for ( auto pitch = pitches.begin(); pitch != pitches.end(); ++pitch )
                    {
                        (*it)->addSecondaryPitch( *pitch );
                    }
                    break;
                }
                ++itNext;
            }
        }
        event->serialize( serialization_ );
        if ( event->isFirstInMeasure() )
            setFirst = false;
        ++it;
    }
}

void ImportModel::setDate( ostringstream& stream ) const
{
#ifdef _WIN32
    SYSTEMTIME time;
    GetSystemTime();
    stream << time.wYear << "-" << time.wMonth << "-" << time.wDay << " at " << time.wHour << ":" << time.wMinute;
#else
    time_t t = time( nullptr );
    stream << asctime( localtime( &t ) );
#endif
}

void ImportModel::clear()
{
    serialization_.clear();
    fileOrigin_ = "";
    version_ = "";
    credits_ = "";
    auto it = events_.begin();
    while ( it != events_.end() )
    {
        delete *it;
        it = events_.erase( it );
    }
}

void ImportModel::setFileOrigin( const string& origin )
{
    fileOrigin_ = origin;
}

void ImportModel::setVersion( const string& version )
{
    version_ = version;
}

void ImportModel::setCredits( const string& credits )
{
    credits_ = credits;
}

float ImportModel::addNote( float measure, float start, float duration, Pitch& pitch )
{
    EntryFeatures& features = pitch.features();
    if ( wrapper_.pitchesAsMidiCents() )
        features |= Feature::DisplayCents;
    if ( wrapper_.hasOriginalPitches() )
        features |= Feature::OriginalEnharmony;
    bool splitAndForward = false;
    float forwardDuration ( duration );
    auto it = events_.begin();
    while ( true )
    {
        if ( it == events_.end() )
        {
            emplaceEvent( it, new Entry( measure, start, duration, pitch ) );
            break;
        }
        else if ( (*it)->type() == Event_BeatPerMinute || (*it)->measure() < measure )
        {
            ++it;
            continue;
        }
        else
        {
            Event* event = *it;
            float m = event->measure();
            float s = event->start();
            float d = event->duration();
            EntryFeatures f = event->features();
            if ( event->hasNotes() && m == measure
                && d == 0.0
                && duration == 0.0
                && features & Chord
                && isEqual( s, start ) ) //special case of grace note chord
            {
                while ( it != events_.end() && (*it)->duration() == 0.0 )
                {
                    ++it;
                }
                --it;
                (*it)->addPitch( pitch );
                break;
            }
            else if ( event->hasNotes() && f & AlternateTremolo
                     && features & TremoloEnd
                     && m == measure
                     && isEqual( s, start ) ) //special case of second part of alternate (unmeasured) tremolo
            {
                if ( pitch.midiCents() != 0 )
                    event->addSecondaryPitch( pitch );
                break;
            }
            else if ( event->hasNotes() && m == measure
                     && d > 0
                     && isEqual( s, start )
                     && isEqual( d, duration ) ) //new note and event have same duration & position (no new part)
            {
                if ( pitch.midiCents() != 0 )
                    event->addPitch( pitch );
                break;
            }
            else if ( event->hasNotes() && m == measure
                     && isBefore( s, start )
                     && d > 0
                     && duration > 0
                     && isEqual( start + duration, s + d ) ) // new entry starts amidst the event, both have same end (1 new part)
            {
                if ( pitch.midiCents() != 0 )
                {
                    it = splitEvent( it, start - s );
                    (*it)->addPitch( pitch );
                }
                break;
            }
            else if ( event->hasNotes() && m == measure
                     && d > 0
                     && isEqual( s, start )
                     && isBefore( duration, d ) ) // new entry starts like the event, and finishes before (1 new part)
            {
                if ( pitch.midiCents() != 0 )
                {
                    it = splitEvent( it, duration );
                    --it;
                    (*it)->addPitch( pitch );
                }
                break;
            }
            else if ( event->hasNotes() && m == measure
                     && features&MidiNote
                     && d > 0
                     && duration > 0
                     && isEqual( start, s )
                     && isAfter( duration, d ) ) // new entry starts like the event, and finishes after (no new part, goes on)
            {
                    (*it)->addPitch( pitch );
                    pitch.flagFeatures( Tiedbackwards );
                    features |= Feature::Tiedbackwards;
                    duration -= d;
                    start += d;
            }
            else if ( event->hasNotes() && m == measure
                     && isBefore( s, start )
                     && d > 0
                     && duration > 0
                     && isAfter( s + d, start + duration ) ) //new entry starts amidst the event, and finishes before (2 new part)
            {
                it = splitEvent( it, start - s );
                it = splitEvent( it, duration );
                (*--it)->addPitch( pitch );
                break;
            }
            else if ( event->hasNotes() && m == measure
                     && isBefore( start, s )
                     && d > 0
                     && duration > 0
                     && isBefore( s + d, start + duration ) ) //new entry starts before the event, and finishes after (1 new part, goes on)
            {
                it = emplaceEvent( it, new Entry( measure, start, s - start, pitch ));
                pitch.flagFeatures( Tiedbackwards );
                features |= Feature::Tiedbackwards;
                (*++it)->addPitch( pitch );
                duration -= ( s - start + d );
                start = s + d;
            }
            else if ( event->hasNotes() && m == measure
                     && isBefore( start, s )
                     && d > 0
                     && duration > 0
                     && isEqual( start + duration, s + d ) ) //new entry starts before the event, both have same end (1 new part, stops)
            {
                it = emplaceEvent( it, new Entry( measure, start, s - start, pitch ));
                pitch.flagFeatures( Tiedbackwards );
                (*++it)->addPitch( pitch );
                break;
            }
            else if ( event->hasNotes() && m == measure
                     && isBefore( s, start )
                     && d > 0
                     && duration > 0
                     && isBefore( start, s + d )
                     && isBefore( s + d, start + duration ) ) //new entry starts amidst the event, and finishes after (1 new part, goes on)
            {
                float delta = s + d - start;
                if ( features&MidiNote && delta < EPSILON_MIDI && pitch.midiCents() > 0 )
                {
                    (*it)->changeDuration( start - s );
                }
                else
                {
                    if ( pitch.midiCents() != 0 )
                    {
                        it = splitEvent( it, start - s );
                        (*it)->addPitch( pitch );
                        features |= Feature::Tiedbackwards;
                    }
                    duration = ( start + duration ) - ( s + d );
                    start = s + d;
                    splitAndForward = true;
                }
            }
            else if ( event->hasNotes() && m == measure
                     && isEqual( s, start )
                     && ( d > 0 || ( d == 0 && splitAndForward ))
                     && duration > 0
                     && isBefore( s + d, start + duration ) ) //new entry starts like the event, and finishes after (no part, goes on)
            {
                if ( pitch.midiCents() != 0 )
                {
                    (*it)->addPitch( pitch );
                }
                duration = duration - d;
                start = s + d;
                features |= Feature::Tiedbackwards;
                splitAndForward = true;
            }
            else if ( m > measure || ( m == measure && isAfter( s, start ) ) ) //new entry starts after any event in this measure
            {
                emplaceEvent( it, new Entry( measure, start, duration, pitch ));
                break;
            }
        }
        ++it;
    }
    return forwardDuration;
}

float ImportModel::addRepeatedNotes( float measure, float initial, float duration, float divisions, Pitch& pitch )
{
    float fullDuration ( duration );
    float start = initial;
    for ( int i = 0; i < divisions; ++i )
    {
        addNote( measure, start, duration/divisions, pitch );
        start += duration/divisions;
    }
    return fullDuration;
}

deque<Event*>::iterator ImportModel::splitEvent( deque<Event*>::iterator it, float splitTime )
{
    if ( splitTime >= (*it)->duration() )
        return it;
    Event* newEvent = (*it)->duplicate();
    newEvent->changeDuration( (*it)->duration() - splitTime );
    newEvent->changeStart( (*it)->start() + splitTime );
    newEvent->tiePitches();
    (*it)->changeDuration( splitTime );
    (*it)->removeFeatures( MeasureRest );
    if ( newEvent->features()&GlissandoStart )
    {
        auto itnext = it + 1;
        while ( itnext != events_.end() && !((*itnext)->features()&GlissandoEnd))
            ++itnext;
        if (itnext != events_.end() )
        {
            Entry* entryAfter = (Entry*) *itnext;
            Entry* newEntry = (Entry*) newEvent;
            std::list<Pitch>& pitchesNew = newEntry->pitches();
            std::list<Pitch>& pitchesAfter = entryAfter->pitches();
            if ( pitchesNew.size() == pitchesAfter.size() )
            {
                for ( auto it1 = pitchesNew.begin(), it2 = pitchesAfter.begin(); it1 != pitchesNew.end(); ++it1, ++it2 )
                {
                    float factor = (*it)->duration() / ((*it)->duration() + newEvent->duration());
                    int inBetween = it1->midiCents() + ( (int)((float)( it2->midiCents() - it1->midiCents() )*factor)/100)*100;
                    (*it1).setMidiCents( inBetween );
                    (*it1).unflagFeatures( OriginalEnharmony );
                    (*it1).setTied( false );
                }
            }
        }
    }
    ++it;
    it = emplaceEvent( it, newEvent );
    return it;
}

Event* ImportModel::findMeasure( float measure ) const
{
    Event* measureEvent = nullptr;
    deque<Event*>::const_iterator it = events_.begin();
    while ( it != events_.end() )
    {
        Event* event = *it;
        if ( event->isMeasure() && event->measure() == measure )
        {
            measureEvent = event;
            break;
        }
        ++it;
    }
    return measureEvent;
}

float ImportModel::getMeasureDuration( float measure ) const
{
    float duration = 0.0;
    deque<Event*>::const_iterator it = events_.begin();
    bool found = false;
    while ( it != events_.end() )
    {
        const Event* event = *it;
        if ( event->hasNotes() && event->measure() == measure )
        {
            duration += event->duration();
            found = true;
        }
        else if ( found && event->measure() != measure )
            break;
        ++it;
    }
    return duration;
}

float ImportModel::getMeasureAccumulutatedBeats( float measure ) const
{
    float duration = 0.0;
    deque<Event*>::const_iterator it = events_.begin();
    bool found = false;
    while ( it != events_.end() )
    {
        const Event* event = *it;
        if ( event->hasNotes() && event->measure() == measure )
        {
            duration += event->duration();
            found = true;
        }
        else if ( found && event->measure() != measure )
            break;
        ++it;
    }
    return duration;
}

bool ImportModel::areThereNotesInMeasure( float measure ) const
{
    bool notes = false;
    auto it = events_.begin();
    while ( it != events_.end() )
    {
        const Event* event = *it;
        if ( event->hasNotes() && event->measure() == measure )
        {
            notes = true;
            break;
        }
        ++it;
    }
    return notes;
}

void ImportModel::appendEvent( Event* event )
{
    events_.push_back( event );
}

deque<Event*>::iterator ImportModel::emplaceEvent( deque<Event*>::iterator it, Event* event )
{
    return events_.insert( it, event ); //emplace
}

void ImportModel::insertFirstEventInMeasure( Event* event )
{
    auto it = events_.begin();
    while ( it != events_.end() && (*it)->measure() < event->measure() )
    {
        ++it;
    }
    events_.insert( it, event );
}

void ImportModel::insertOrReplaceEvent( Event* event )
{
    auto it = events_.begin();
    while ( it != events_.end() &&
            ((*it)->measure() < event->measure() || ((*it)->measure() == event->measure() && (*it)->start() < event->start() ) ) )
    {
        ++it;
    }
    if ( it != events_.end()
        && (*it)->measure() == event->measure()
        && (*it)->start() == event->start()
        && (*it)->type() == event->type() )
    {
        const Event* item = *it;
        *it = event;
        delete item;
    }
    else
        events_.insert( it, event );
}

void ImportModel::replaceEvent( Event* event )
{
    auto it = events_.begin();
    while ( it != events_.end() )
    {
        const Event* item = *it;
        if (  item->measure() == event->measure() &&
              item->start() == event->start() &&
              item->type() == event->type() )
        {
            *it = event;
            delete item;
            break;
        }
        ++it;
    }
}

void ImportModel::queryTempi( std::vector<std::string>& tempi )
{
    auto it = events_.begin();
    while ( it != events_.end() )
    {
        Event* event = *it;
        if ( event->type() != Event_BeatPerMinute )
        {
            ++it;
            continue;
        }
        ++it;
    }
}

float ImportModel::fractionToFloat(std::string& str)
{
    std::stringstream sstr (str);
    int n = 0, d = 1; char c;
    sstr >> n >> c >> d;
    return (d ? ((float(n))/d) : 0.f);
}

/*
float ImportModel::queryFirstMeasureDuration()
{
    for (auto event : events_)
    {
        if (event->type() == Event_Measure) // if event is a measure...
        {
            Measure* measure = dynamic_cast<Measure*>(event);
            if (measure->accumBeats())  // accumBeats == 0 means we are on the first measure bar line
                return (measure->accumBeats() / measure->metricFactor()) ;
        }
    }
    return 0.f;
}
*/

void QueryHandler::showPulseChangesAsNim( std::ostringstream& stream ) const
{
    /// STEP 1. Pulse changes
    //// Get pulse changes
    auto const& pulseChanges = pulseChangePositions;

    //// Write pulse changes them as NIM
    // 1. (initialization) open NIM
    stream << "@eval_when_load"<< ' ' << "{\n$pulses_nim := NIM {";
    // 2. (recursion) fill NIM
    // Handle empty
    if (pulseChanges.empty())
    {
        stream << " 0 1, 1 1";
        cerr << "Warning, score has no pulses (no measures?)" << endl;
    }
    else
    {
        bool firstItemWritten = false;
        float lastPosition = 0.0;
        std::string lastPulse = "";
        for (auto const& pair : pulseChanges)
        {
            float const currentPosition = pair.first;
            std::string const currentPulse = pair.second;
            if (!firstItemWritten || (currentPosition > lastPosition))
            {
                if (currentPulse == "")     // security
                {
                    std::cerr << "Warning! empty pulse at beat position " << currentPosition << endl;
                    continue;
                }
                
                if (!firstItemWritten)  // case of first element
                {
                    // Write content
                    stream << " (" << currentPosition << ") (" << currentPulse << ")";
                    // Set flag
                    firstItemWritten = true;
                }
                else    // case of further elements
                {
                    // Write separator + content
                    stream << ", (" << (currentPosition - lastPosition) << ") (" << lastPulse << "), (0) (" << currentPulse << ")";
                }
                // Update Position
                lastPosition = currentPosition;
                lastPulse = currentPulse;
            }
        }
        // Case of NIM with one element: re-write last value with non-zero delay
        if (pulseChanges.size() == 1)
            stream << ", (1) (" << pulseChanges.back().second << ")";
    }
    // 3. (finalization) close NIM
    stream << " }\n}" << endl;
    
    
    /// STEP 2. initial pulse phase
    //TODO: store default pulse value elsewhere
    float const initialPulse = (pulseChanges.empty() ? 1. : rational(pulseChanges.front().second).toFloat());
    float const pickupDuration = firstMeasureDuration;
    //std::cerr << "pickup duration is " << pickupDuration << ", initial pulse is " << initialPulse << std::endl;
    if (pickupDuration > 0.f)
    {
        float const phase = fmod(pickupDuration, initialPulse);
        if (phase)
        {
            float const delta = (pickupDuration-initialPulse);
            stream << "@eval_when_load"<< ' ' << "{\n$pulse_pos := " <<  delta << "\n}" << endl;
        }
    }
    
    stream << endl;
}

void ImportModel::beautify()
{
    consolidateNotesAndRests();
    consolidateTemposAndMeasures();
}

void ImportModel::consolidateNotesAndRests()
{
    auto it = events_.begin();
    while ( it != events_.end() )
    {
        Event* event = *it;
        if ( event->type() != Event_Entry )
        {
            ++it;
            continue;
        }
        auto itNext = it;
        ++itNext;
        if ( itNext == events_.end() )
            break;
        else if ( (*itNext)->type() != Event_Entry )
        {
            ++it;
            continue;
        }
        if ( event->isRest() && ( ( (Entry*) event )->isTiedTo( (Entry*) *itNext )
                                 || ( (*itNext)->isRest() && event->measure() == (*itNext)->measure() ) ) )
        {
            event->changeDuration( event->duration() + (*itNext)->duration() );
            it = events_.erase( itNext );
            --it;
            continue;
        }
        ++it;
    }
}

void ImportModel::consolidateTemposAndMeasures()
{
    bool notesFound = false;
    for ( long i = events_.size() - 1; i > 0; --i )
    {
        Event* event = events_[i];
        if ( !notesFound && event->hasNotes() )
            notesFound = true;
        if ( !notesFound &&
            ( event->type() == Event_BeatPerMinute || ( event->type() == Event_Measure && wrapper_.inputIsMIDI() ) ) )
        {
            auto it = events_.begin() + i;
            events_.erase( it );
            delete event;
            continue;
        }
        if ( event->type() != Event_BeatPerMinute )
        {
            continue;
        }
        auto eventBefore = events_[i - 1];
        if ( eventBefore->type() != Event_BeatPerMinute || event->measure() != eventBefore->measure() )
        {
            continue;
        }
        if ( event->start() > eventBefore->start() && ((BeatPerMinute*)event)->tempo() == ((BeatPerMinute*)event)->tempo() )
        {
            auto it = events_.begin() + i;
            events_.erase( it );
            delete event;
        }
        else if ( event->start() == eventBefore->start() )
        {
            auto it = events_.begin() + i - 1;
            events_.erase( it );
            delete eventBefore;
        }
    }
}

bool ImportModel::isEqual( float t1, float t2 )
{
    if ( wrapper_.inputIsMIDI() )
        return fabs( t1 - t2 ) < EPSILON_MIDI;
    return fabs( t1 - t2 ) < EPSILON;
}

bool ImportModel::isAfter( float t1, float t2 )
{
    if ( wrapper_.inputIsMIDI() )
        return ( t1 - t2 ) > EPSILON_MIDI;
    return ( t1 - t2 ) > EPSILON;
}

bool ImportModel::isBefore( float t1, float t2 )
{
    if ( wrapper_.inputIsMIDI() )
        return ( t2 - t1 ) > EPSILON_MIDI;
    return ( t2 - t1 ) > EPSILON;
}

float const QueryHandler::EPSILON = 0.005f; // in beats

void QueryHandler::showQueries(std::ostringstream& o, std::deque<Event*> const& events, ImporterWrapper const* wrapper)
{
    /// Step 1. perform queries
    QueryHandler queries(events, wrapper);
    
    /// Step 2. print queries
    queries.showQueriesOn(o);
}

QueryHandler::QueryHandler( std::deque<Event* > const& events, ImporterWrapper const* wrapper) : wrapper_(wrapper), firstMeasureDuration(0.), currentPulsePhaseDuration(0), accumBeats_(0.)
{
    performQueries(events);
}

/*! @brief  Launch queries by iterating on all Events
 */
void QueryHandler::performQueries(std::deque<Event* > const& events)
{
    /// Recursion: Pass successive every Events to the queryHandler
    for (auto event : events)
    {
        performQueries(event);
    }
    
    /// Finalization (if needed)
}

/*! @brief  Print selected queries
 *
 *  À terme, il faudra que cette fonction n'imprime que les Queries sélectionnées
 */
void QueryHandler::showQueriesOn(std::ostringstream& o) const
{
    showPulseChangesAsNim(o);
    showPulsesAsNim(o);
}

/*! @brief   This function selects the queries to perform on each event according to its type
 */
void QueryHandler::performQueries(Event const* event)
{
    if (!event)
        return;
    
    if (event->type() == Event_Measure) // if event is a measure...
    {
        // queries that are measure-specific
        performQueriesOn(dynamic_cast<Measure const*>(event));
    }
    else
    {
        // queries that generic for all other events
        performQueriesOn(event);
    }
}

/*! @brief  Perform queries specific to Measures
 */
void QueryHandler::performQueriesOn(Measure const* measure)
{
    if (!measure)
        return;
    
    queryFirstMeasureDuration(measure);
    queryPulseChange(measure);
    queryPulses(measure);
    queryTempoBeatUnitChanges(measure);
}

/*! @brief  Perform queries for other kind of events
 */
void QueryHandler::performQueriesOn(Event const* event)
{
    if (!event)
        return;
    
    queryPulses(event);
}

/*! @brief  Extracts the pulse of input measure, and add it to pulseChangePositions if there is a change
 *
 *  Pour l'instant, on ne gère pas les compound measures
 */
void QueryHandler::queryPulseChange(Measure const* measure)
{
    // Get last pulse unit
    std::string const& lastPulse = (pulseChangePositions.empty() ? "" : pulseChangePositions.back().second);
    // Get pulse unit of current measure
    std::string const& currentPulse = inferePulseSignature(measure->timeSignature()).toString();
    // Insert current pulse but ONLY on changes
    if (currentPulse != lastPulse)
    {
        // get beat position of last change
        float const lastPosition = (pulseChangePositions.empty() ? 0.f : pulseChangePositions.back().first);
        // get current position
        float const currentPosition = measure->accumBeats();
        // should not happen!
        if (currentPosition == lastPosition)
        {
            if (!pulseChangePositions.empty())
            {
                std::cerr << "Warning, simultaneous pulse changes at position " << currentPosition << ", current pulse=" << currentPulse << " and last pulse=" << lastPulse << std::endl;
                pulseChangePositions.pop_back();
            }
        }
        else if (currentPosition < lastPosition) // should NOT happen at all !
            return;
        // get current position
        pulseChangePositions.emplace_back(currentPosition, currentPulse);
    }
}

/*! @brief  returns duration of first measure (0.0 if no measure)
 *
 *  Look for the first measure and returns its beat position
 *
 */
bool QueryHandler::queryFirstMeasureDuration(Measure const* measure)
{
    // Do nothing if query is over
    if (this->firstMeasureDuration > 0.0)
        return true;
    
    if (measure->accumBeats() > 0.0)
    {
        this->firstMeasureDuration = measure->accumBeats();
        return true;
    }
    else
        return false;
}


/*! @brief  Returns the Pulse unit corresponding to input time signature
 *
 *  This function contains the musicologic rules to translate a time signature into a pulse
 *  So far, it is not suitable for compound time signature, odd time signature
 */
rational QueryHandler::inferePulseSignature(std::string const& timeSignature) const
{
    // if there is an error, returns 1 = 1/1
    rational pulse(1, 1);
    // Parse timeSignature as rational number
    rational r(timeSignature);
    if (r.getNumerator() == 0)    // parsing error
        return pulse;
    long int const denom = r.getDenominator();
    long int const num = r.getNumerator();
    
    // Inférence de la pulse à partir de la time-signature
    // see: http://www2.siba.fi/muste1/index.php?id=98&la=en
    
    if (denom == 4)
        pulse.set(1, 1);        // TODO: traiter le cas de la pulsation à la blanche pointée
    else if (denom == 3)
        pulse.set(4, 3);
    else if (denom == 2)
        pulse.set(2, 1);
    else if (denom == 1)
        pulse.set(4, 1);
    else if (denom == 8)    // cas particulier: 6/8, 9/8, 12/8, etc. --> pulse = 1.5 (3/2)  // sinon, pulse = 0.5
    {
        if ((num % 3 == 0) && (num > 3))
            pulse.set(3,2);     // doted quarter note
        else
            pulse.set(1,2);           // eight note
    }
    else if (denom == 16)    // cas particulier: 3/16, 6/16, 9/16, 12/16, etc. --> pulse = 3/4 // sinon, pulse = 1/4
    {
        if ((num % 3 == 0) /*&& (num > 3)*/)
            pulse.set(3,4);     // doted eight note
        else
            pulse.set(1,4);           // sixteenth note
    }
    else    // normalement, on ne devrait pas tomber sur ce cas
    {
        // Change numerator to quarter note
        pulse.setNumerator(4);
        // (optional) Reduce
        pulse.rationalise();
    }
    
    return pulse;
}


/*! @brief  Retrieve the list of beat units given in tempo marks
 */
bool QueryHandler::queryTempoBeatUnitChanges(Measure const* measure)
{//TODO: finir cette fonction
    /*
   auto const& tempoBeatUnitChanges = this->tempoBeatUnitChanges;

    // Get new time signature
    rational const& newTimeSignature = measure->timeSignature();
    // Insert current pulse but ONLY on changes
    if ((currentTimeSignature != newTimeSignature)  // if there is a time signature change
        && (newTimeSignature))  // if new time signature is valid
    {
        // get beat position of last change
        float const lastPosition = (tempoBeatUnitChanges.empty() ? 0.f : tempoBeatUnitChanges.back().first);
        // get current position
        float const currentPosition = measure->accumBeats();
        // should not happen!
        if (currentPosition == lastPosition)
        {
            if (!pulseChangePositions.empty())
            {
                //std::cerr << "Warning, simultaneous pulse changes at position " << currentPosition << ", current pulse=" << currentBeatUnit << " and last pulse=" << lastBeatUnit << std::endl;
                pulseChangePositions.pop_back();
            }
        }
        else if (currentPosition < lastPosition) // should NOT happen at all !
            return false;
        // get current position
        tempoBeatUnitChanges.emplace_back(currentPosition, currentBeatUnit);
    }
    
    // update currentTimeSignature
     = newTimeSignature;
    //currentTempoMarkBeatUnit;
    */
    
    return false;
}

/*! @brief  Fill pulses deque with values that represents the beat duration until next pulse
 */
void QueryHandler::queryPulses(Event const* event)
{
    // Add event duration to keep track of current beat position
    accumBeats_ += event->duration();   // Rmk: do NOT use this for MEASURE, since the measure duration is WRONG
    
    // Insert pulses and keep track of current pulse phase
    addPulses(event->duration());
}

void QueryHandler::queryPulses(Measure const* measure)
{
    // Add event duration to keep track of current beat position
    float const lastAccumBeats = accumBeats_;
    accumBeats_ = measure->accumBeats();
    
    // Insert pulses and keep track of current pulse phase
    addPulses(accumBeats_ - lastAccumBeats);

    // (in case) Insert the "reminder pulse" if there is such
    // Rounding strategy
    if (currentPulsePhaseDuration > EPSILON)        //TODO: no longer use rounding -> this requires using
    {
        // Rounding strategy
        //TODO: no longer use rounding
        float const& currentPulse = this->getCurrentPulseDuration().toFloat();
        if (fabs(currentPulsePhaseDuration - currentPulse) <= EPSILON)
            pulses.emplace_back(currentPulse);
        else    // normal behavior
            pulses.emplace_back(currentPulsePhaseDuration);
    }
    
    // Always reset current duration
    currentPulsePhaseDuration = 0;
}

/** This method should be called by all queryPulse
 * It insert as many pulses as they can fit in duration, and keep track of pulse phase accordingly (in currentPulsePhaseDuration)
 */
void QueryHandler::addPulses(float duration)
{
    // Add event duration to keep track of current beat phase
    currentPulsePhaseDuration += duration;
    
    float const& currentPulse = this->getCurrentPulseDuration().toFloat();
    
    // while currentPulsePhaseDuration is greater than pulse duration, then add a pulse
    while (currentPulsePhaseDuration > currentPulse)
    {
        currentPulsePhaseDuration -= currentPulse;
        pulses.emplace_back(currentPulse);
    }
}

/**
 * Returns current pulse unit
 */
rational QueryHandler::getCurrentPulseDuration() const
{
    return (pulseChangePositions.empty() ? rational(1) : rational(pulseChangePositions.back().second) );
}


/**
 * This functions prints a NIM that tells the pulse phase, i.e. the remaining beat duration until next beat
 */
void QueryHandler::showPulsesAsNim( std::ostringstream& stream ) const
{
    int const maxItemPerLine = 10;
    int itemNumber = 0.;
    
    // Nim opening
    stream << "@eval_when_load"<< ' ' << "{\n$pulse_phase_nim := NIM {";
    bool first = false;
    
    for (float pulseDuration : pulses)
    {
        // Jump line if too many items
        ++itemNumber;
        if (itemNumber % maxItemPerLine == 0)
            stream << std::endl;
        
        // write separator
        if (first)
            stream << ",";
        else
            first = true;
        
        // write pulse
        stream << " 0 (" << pulseDuration << "), (" << pulseDuration << ") (" << 0 << ")";
    }
    
    /*
    // Case of all first measure except the last one
    for (auto event : events_)
    {
        if (event->type() == Event_Measure) // if event is a measure...
        {
            Measure const* measure = dynamic_cast<Measure const*>(event);
            measureBeatPosition = measure->accumBeats();
            measureDuration = measure->duration();
            if (measureBeatPosition > lastMeasureBeatPosition)  // TO
            {
                // Jump line if too many items
                ++itemNumber;
                if (itemNumber % maxItemPerLine == 0)
                    stream << std::endl;
                
                // Get pulse unit of current measure
                std::string const& currentPulse = QueryHandler::inferePulseSignature(measure->timeSignature());
                
                // Write all beats in measure
            }
            lastMeasureBeatPosition = measureBeatPosition;
        }
    }
    
    // Case of last measure*/
    
    
    // Nim ending
    stream << " }\n}" << endl;
}


bool QueryHandler::queryTempoBeatUnitChanges(BeatPerMinute const* bpm)
{
    //TODO: finir
    /*
    auto const& changes = this->tempoBeatUnitChanges;
    // Get
    auto const& beatUnit = bpm->beatunit();
    
    // Filter for change
    if (currentTempoMarkBeatUnit != beatUnit)
    {
        
    }
    
    // update current attributs
    currentTempoMarkTimeSignature = currentTimeSignature;    // we assume that time signature was ALREADY changed BEFORE this BPM
    currentTempoMarkBeatUnit = beatUnit;
    */
    return false;
}

void QueryHandler::showTempoBeatUnitChangesAsNim( std::ostringstream& stream ) const  // print the list of beat units in ostream
{
    //TODO: écrire cette fonction
}


//TODO: SUPPRIMER cette fonction
/*
std::deque<std::pair<float, std::string> > ImportModel::queryPulseChanges()
{
    // Create an empty deque
    std::deque<std::pair<float, std::string> > pulseChangePositions;
    // Pass it to events
    for ( auto const& event : events_ )
    {
        event->queryPulseChange(pulseChangePositions);  // so far, only MEASURES append something
    }
    
    return pulseChangePositions;
}*/
