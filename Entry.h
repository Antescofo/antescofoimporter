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
//  Copyright (c) 2017 Antescofo. All rights reserved.
//

#ifndef _ANTESCOFO_IMPORTER_ENTRY_
#define _ANTESCOFO_IMPORTER_ENTRY_

#include <stdio.h>
#include <sstream>
#include <list>
#include <utility>
#include "Event.h"
#include "Pitch.h"
#include "SimpleRational.h"

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
        explicit Entry( float measure, float start, float duration, const Pitch& pitch );
        Entry( const Entry& from );
        virtual ~Entry();
        
        void          serialize( std::ostringstream& stream ) override;
        EventType     type() const override { return Event_Entry; }
        bool          hasNotes() const override;
        float         start() const override;
        void          changeStart( float newTime ) override;
        void          changeDuration( float newTime ) override;
        float         duration() const override;
        bool          addPitch( const Pitch& pitch ) override;
        int           primaryPitchCount() const override { return (int) pitches_.size(); }
        void          addSecondaryPitch( const Pitch& secondaryPitch) override;
        EntryFeatures features() const override { return features_; }
        void          addFeatures( EntryFeatures feature ) override;
        void          removeFeatures( EntryFeatures feature ) override;
        void          tiePitches() override;
        Event*        duplicate() const override;
        bool          isRest() const override;
        void          setAsRest() override;
        
        const std::list<Pitch>& pitches() const;
        std::list<Pitch>&       pitches();
        const std::list<Pitch>& secondaryPitches() const;
        std::list<Pitch>&       secondaryPitches();
        
        bool isTiedTo( Entry* entry );
        bool isNosync() const override { return nosync; }
        void setNosync() override { nosync = true; }

    private:
        std::string formatDuration() const;
        void        serializeNote( std::ostringstream& stream );
        void        serializeChord( std::ostringstream& stream );
        void        serializeFastRepeatedTremolo( std::ostringstream& stream );
        void        serializeTrill( std::ostringstream& stream );
        void        serializeMulti( std::ostringstream& stream );
        int         getHarmonicSoundingPitch( int lowPitch, int writtenHarmonic ) const;
        
    private:
        float               start_;
        float               duration_;
        SimpleRational      rationalDuration_;
        std::list<Pitch>    pitches_;
        std::list<Pitch>    secondaryPitches_; //for alternate tremolos & trills
        EntryFeatures       features_;
        bool                nosync;
    };
}

#endif // _ANTESCOFO_IMPORTER_ENTRY_
