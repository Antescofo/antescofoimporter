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
        explicit Measure( float measure, float duration, float accum, float metricFactor, const std::string& timeSignature );
        explicit Measure( float measure, float duration, float accum, float metricFactor, int keyAccidentals, const std::string& timeSignature );
        virtual ~Measure();
        
        void        serialize( std::ostringstream& stream ) override;
        EventType   type()  const override { return Event_Measure; }
        bool        hasNotes() const override;
        bool        isMeasure() const override;
        float       duration() const override;
        void        changeDuration( float newTime ) override;
        
        const std::string&  timeSignature() const;
        float               metricFactor() const;
        int                 keyAccidentals() const;
        void                setTimeSignature( const std::string time ) { timeSignature_ = time; }
        float               accumBeats() const { return accumBeats_; }
        
        
    private:
        float               beatDuration_;
        const float         accumBeats_;
        const float         metricFactor_;
        std::string         timeSignature_;
        const int           keyAccidentals_;
    };
}

#endif // _ANTESCOFO_IMPORTER_MEASURE_
