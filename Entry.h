//
// .-. . . .-. .-. .-. .-. .-. .-. .-.
// |-| |\|  |  |-  `-. |   | | |-  | |
// ` ' ' `  '  `-' `-' `-' `-' '   `-'
//
// .-. .  . .-. .-. .-. .-. .-. .-.
//  |  |\/| |-' | | |(   |  |-  |(
// `-' '  ` '   `-' ' '  '  `-' ' '
//
//  Entry.h
//
//  Created by Robert Piéchaud on 09/05/15.
//  Copyright (c) 2015 ircam. All rights reserved.
//

#ifndef _ANTESCOFO_IMPORTER_ENTRY_
#define _ANTESCOFO_IMPORTER_ENTRY_

#include <stdio.h>
#include <sstream>
#include <list>
#include "Event.h"

namespace antescofo
{
    enum CentsInterval
    {
        Unison      = 0,
        MinorThird  = 300,
        MajorThird  = 400,
        Forth       = 500,
        Fifth       = 700,
        MajorSixth  = 900,
        Octave      = 1200,
        MajorTenth  = 1600
    };
    
    class Entry: public Event
    {
    public:
        explicit Entry( int measure, float start, float duration, int centsPitch, bool displayCents, EntryFeatures features = None );
        Entry( const Entry& from );
        virtual ~Entry();
        
        virtual void          serialize( std::ostringstream& stream );
        virtual EventType     type() const { return Event_Entry; }
        virtual bool          hasNotes() const;
        virtual float         start() const;
        virtual void          changeStart( float newTime );
        virtual void          changeDuration( float newTime );
        virtual float         duration() const;
        virtual bool          addPitch( int midiCents, EntryFeatures feature );
        virtual void          addSecondaryPitch( int midiCents, EntryFeatures feature );
        virtual EntryFeatures features() { return features_; }
        virtual void          addFeatures( EntryFeatures feature );
        virtual void          tiePitches();
        virtual Event*        duplicate() const;
        virtual bool          isRest() const;
        
        const std::list<int>& pitches() const;
        std::list<int>&       pitches();
        const std::list<int>& secondaryPitches() const;
        std::list<int>&       secondaryPitches();
        
        bool isTiedTo( Entry* entry );
        
    private:
        std::string formatDuration() const;
        std::string serializePitch( int pitch ) const;
        void        serializeNote( std::ostringstream& stream );
        void        serializeChord( std::ostringstream& stream );
        void        serializeFastRepeatedTremolo( std::ostringstream& stream );
        void        serializeTrill( std::ostringstream& stream );
        void        serializeMulti( std::ostringstream& stream );
        int         getHarmonicSoundingPitch( int lowPitch, int writtenHarmonic ) const;
        
    private:
        float               start_;
        float               duration_;
        std::list<int>      centsPitches_;
        std::list<int>      secondaryPitches_; //for alternate tremolos & trills
        const bool          displayCents_;
        EntryFeatures       features_;
    };
}

#endif // _ANTESCOFO_IMPORTER_ENTRY_
