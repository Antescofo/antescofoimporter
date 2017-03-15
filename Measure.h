//
// .-. . . .-. .-. .-. .-. .-. .-. .-.
// |-| |\|  |  |-  `-. |   | | |-  | |
// ` ' ' `  '  `-' `-' `-' `-' '   `-'
//
// .-. .  . .-. .-. .-. .-. .-. .-.
//  |  |\/| |-' | | |(   |  |-  |(
// `-' '  ` '   `-' ' '  '  `-' ' '
//
//  Measure.h
//
//  Created by Robert Piéchaud on 10/05/15.
//  Copyright (c) 2015 ircam. All rights reserved.
//

#ifndef _ANTESCOFO_IMPORTER_MEASURE_
#define _ANTESCOFO_IMPORTER_MEASURE_

#include "Event.h"
#include <vector>
#include <string>

namespace antescofo
{
    class Measure: public Event
    {
    public:
        explicit Measure( int measure, float duration, float accum, float metricFactor, const std::string& timeSignature );
        explicit Measure( int measure, float duration, float accum, float metricFactor, int keyAccidentals, const std::string& timeSignature );
        virtual ~Measure();
        
        virtual void        serialize( std::ostringstream& stream );
        virtual EventType   type() const { return Event_Measure; }
        virtual bool        hasNotes() const;
        virtual bool        isMeasure() const;
        virtual float       duration() const;
        virtual void        changeDuration( float newTime );
        
        const std::string&  timeSignature() const;
        float               metricFactor() const;
        int                 keyAccidentals() const;
        
        void                setTimeSignature( const std::string time ) { timeSignature_ = time; }
        
        
    private:
        float               duration_;
        const float         accumDuration_;
        const float         metricFactor_;
        std::string         timeSignature_;
        const int           keyAccidentals_;
    };
}

#endif // _ANTESCOFO_IMPORTER_MEASURE_
