//
// .-. . . .-. .-. .-. .-. .-. .-. .-.
// |-| |\|  |  |-  `-. |   | | |-  | |
// ` ' ' `  '  `-' `-' `-' `-' '   `-'
//
// .-. .  . .-. .-. .-. .-. .-. .-.
//  |  |\/| |-' | | |(   |  |-  |(
// `-' '  ` '   `-' ' '  '  `-' ' '
//
//  Event.h
//
//  Created by Robert Piéchaud on 09/05/15.
//  Copyright (c) 2015 ircam. All rights reserved.
//

#ifndef _ANTESCOFO_IMPORTER_EVENT_
#define _ANTESCOFO_IMPORTER_EVENT_

#include <stdio.h>
#include <sstream>
#include <math.h>

static const float EPSILON = 0.01;
static const float THETA = 0.05;
static const float ALPHA = 0.04;

namespace antescofo
{
    typedef unsigned int EntryFeatures;
    enum Feature
    {
        None                = 0x00000000,
        Staccato            = 0x00000001,
        FastRepeatedTremolo = 0x00000002,
        Trill               = 0x00000004,
        WholeToneTrill      = 0x00000008,
        AlternateTremolo    = 0x00000010,
        TremoloEnd          = 0x00000020,
        GraceNote           = 0x00000040,
        Chord               = 0x00000080,
        GlissandoStart      = 0x00000100,
        GlissandoEnd        = 0x00000200,
        Harmonic            = 0x00001000,
        DoubleHarmonic      = 0x00002000,
        NaturalHarmonic     = 0x00004000,
        SquareNotehead      = 0x00008000,
        Fermata             = 0x00010000,
        Tiedbackwards       = 0x00100000,
        Flat                = 0x00200000,
        Sharp               = 0x00400000,
        DisplayCents        = 0x00800000,
        MidiNote            = 0x80000000
    };
    
    enum EventType
    {
        Event_Measure = 1,
        Event_Entry,
        Event_Tempo,
        Event_BeatPerMinute,
        Event_RepeatBar,
        Event_RepeatEnding,
    };
    
    class Event
    {
    public:
        Event();
        Event( const Event& );
        virtual ~Event();
        
        virtual void        serialize( std::ostringstream& stream ) = 0;
        virtual EventType   type() const = 0;
        virtual bool        hasNotes() const = 0;
        
        virtual bool          isMeasure() const;
        virtual bool          isRest() const { return false; }
        virtual float         start() const;
        virtual float         duration() const;
        virtual void          changeStart( float newTime ) {}
        virtual void          changeDuration( float newTime ) {}
        virtual void          tiePitches() {}
        virtual bool          addPitch( int midiCents, EntryFeatures feature ) { return false; }
        virtual void          addSecondaryPitch( int midiCents, EntryFeatures feature ) {}
        virtual EntryFeatures features() { return 0; }
        virtual void          addFeatures( EntryFeatures feature ) {}
        virtual Event*        duplicate() const;
        
        int  measure() const;
        bool isFirstInMeasure() const;
        void setFirstInMeasure( bool status );
        
    protected:
        int     measure_;
        bool    isFirstInMeasure_;
    };
}

inline bool isEqual( float t1, float t2 )
{
    return fabs( t1 - t2 ) < EPSILON;
}

inline bool isAfter( float t1, float t2 )
{
    return ( t1 - t2 ) > EPSILON;
}

inline bool isBefore( float t1, float t2 )
{
    return ( t2 - t1 ) > EPSILON;
}

#endif // _ANTESCOFO_IMPORTER_EVENT_
