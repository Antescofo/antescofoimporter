//
//  Pitch.h
//  antescofo_converter
//
//  Created by baba on 17/06/15.
//  Copyright (c) 2015 ircam. All rights reserved.
//

#ifndef _ANTESCOFO_IMPORTER_PITCH_
#define _ANTESCOFO_IMPORTER_PITCH_

#include <stdio.h>
#include "Event.h"

namespace antescofo
{
    class Pitch
    {
    public:
        Pitch();
        explicit Pitch( int pitch );
        explicit Pitch( int pitch, const EntryFeatures feature );
        explicit Pitch( const Pitch& otherPitch );
        virtual ~Pitch();
        
        bool operator==( const Pitch& otherPitch ) const;
        bool operator==( int cents ) const;
        bool operator<( const Pitch& otherPitch ) const;
        Pitch& operator=( const Pitch& otherPitch );
        
        static bool comparePitch( const Pitch& p1, const Pitch& p2 );
        
        std::string serialize() const;
        void setFeatureBits( const EntryFeatures bits );
        bool isRest() const;
        bool isTiedBackwards() const;
        bool isFlat() const;
        bool isSharp() const;
        bool isTrillPitch() const;
        int  midiCents() const { return midiCents_; }
        void setMidiCents( int cents ) { midiCents_ = cents; }
        const EntryFeatures& features() const { return features_; }
        EntryFeatures& features() { return features_; }
        void setFeatures( EntryFeatures features ) { features_ = features; }
        void flagFeatures( EntryFeatures features ) { features_ |= features; }
        void unflagFeatures( EntryFeatures features ) { features_ &= ~features; }
        void setTied( bool status );
        void setNoteSymbol( const char symbol ) { note_ = symbol; }
        void setAccidendal( int accidental ) { accidental_ = accidental; }
        void setOctave( int octave ) { octave_ = octave; }
        void resetNote();
        int  computeMidiCents() const;
        
    private:
        int             midiCents_;
        char            note_;
        int             accidental_;
        int             octave_;
        EntryFeatures   features_;
    };
}

#endif // _ANTESCOFO_IMPORTER_PITCH_
