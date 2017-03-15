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
        Pitch( int pitch );
        Pitch( int pitch, const EntryFeatures feature );
        explicit Pitch( const Pitch& otherPitch );
        virtual ~Pitch();
        
        bool operator==( const Pitch& otherPitch ) const;
        bool operator<( const Pitch& otherPitch ) const;
        Pitch& operator=( const Pitch& otherPitch );
        
        static bool comparePitch( const Pitch& p1, const Pitch& p2 );
        
        std::string serialize() const;
        void setFeature( const EntryFeatures bits );
        bool isRest() const;
        bool isTiedBackwards() const;
        bool isFlat() const;
        bool isSharp() const;
        bool isTrillPitch() const;
        
    private:
        int             pitch_;
        EntryFeatures   features_;
    };
}

#endif // _ANTESCOFO_IMPORTER_PITCH_
