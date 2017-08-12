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

#include "rational.h"
#include <stdio.h>
#include <sstream>
#include <math.h>
#include <deque>

static const float EPSILON = 0.01;
static const float EPSILON_MIDI = 0.0457;  //used for MIDI quantification: represents the difference between a 16th note (as part of a 6-uplet)
                                    //and of regular 32nd note (we might add some customizable quantification precision in the future...)
static const float THETA = 0.2;
static const float THETA_MIDI = 0.095;

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
        SlashedGraceNote    = 0x00000080,
        GlissandoStart      = 0x00000100,
        GlissandoEnd        = 0x00000200,
        MeasureRest         = 0x00000400,
        Chord               = 0x00000800,
        Harmonic            = 0x00001000,
        DoubleHarmonic      = 0x00002000,
        NaturalHarmonic     = 0x00004000,
        SquareNotehead      = 0x00008000,
        Fermata             = 0x00010000,
        Cue                 = 0x00020000,
        Tiedbackwards       = 0x00100000,
        Transposed          = 0x00800000,
        DisplayCents        = 0x01000000,
        OriginalEnharmony   = 0x02000000,
        GeneratedTempo      = 0x10000000,
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
        Action_ActionGroup
    };
    
    class Pitch;
    
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
        virtual void          setAsRest() {}
        virtual float         start() const;
        virtual float         duration() const;
        virtual void          changeStart( float newTime ) {}
        virtual void          changeDuration( float newTime ) {}
        virtual void          tiePitches() {}
        virtual bool          addPitch( const Pitch& pitch ) { return false; }
        virtual int           primaryPitchCount() const { return 0; }
        virtual void          addSecondaryPitch( const Pitch& pitch ) {}
        virtual EntryFeatures features() const { return 0; }
        virtual void          addFeatures( EntryFeatures feature ) {}
        virtual void          removeFeatures( EntryFeatures feature ) {}
        virtual Event*        duplicate() const;
        
        float measure() const;
        bool  isFirstInMeasure() const;
        void  setFirstInMeasure( bool status );
        
        //virtual void queryPulseChange(std::deque<std::pair<float, std::string> >& pulseChangePositions) { } // by default, do nothing
        //virtual void queryTempoBeatUnitChanges(std::deque<std::pair<float, rational> >&) const { } // by default, do nothing
        
    protected:
        bool isEqual( float t1, float t2 ) const;
        
    protected:
        float   measure_;
        bool    isFirstInMeasure_;
    };
}

#endif // _ANTESCOFO_IMPORTER_EVENT_
