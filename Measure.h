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
//  Copyright (c) 2017 Antescofo. All rights reserved.
//

#ifndef _ANTESCOFO_IMPORTER_MEASURE_
#define _ANTESCOFO_IMPORTER_MEASURE_

#include "Event.h"
#include "rational.h"
#include <vector>
#include <deque>
#include <string>

namespace antescofo
{
    class Measure: public Event
    {
    public:
        explicit Measure( const std::string& measure, float duration, float accum, float metricFactor, const std::string& timeSignature );
        explicit Measure( const std::string& measure, float duration, float accum, float metricFactor, int keyAccidentals, const std::string& timeSignature );
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
        
        //rational         getPulseSignature() const; //<! infère et retourne la pulse correspondante à la timeSignature
        //static rational  inferePulseSignature(std::string const& timeSignature);
        
        //virtual void queryPulseChange(std::deque<std::pair<float, std::string> >& pulseChangePositions) override;
        //virtual void queryTempoBeatUnitChanges(std::deque<std::pair<float, rational> >&) const override;

    private:
        float               beatDuration_;
        const float         accumBeats_;
        const float         metricFactor_;
        std::string         timeSignature_;
        const int           keyAccidentals_;
    };
}

#endif // _ANTESCOFO_IMPORTER_MEASURE_
