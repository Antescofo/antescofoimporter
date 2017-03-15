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
        features |= DisplayCents;
    if ( wrapper_.hasOriginalPitches() )
        features |= OriginalEnharmony;
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
                    features |= Tiedbackwards;
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
                features |= Tiedbackwards;
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
                        features |= Tiedbackwards;
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
                features |= Tiedbackwards;
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
        if ( event->isRest() && ( (Entry*) event )->isTiedTo( (Entry*) *itNext ) )
        {
            event->changeDuration( event->duration() + (*itNext)->duration() );
            it = events_.erase( itNext );
            --it;
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
