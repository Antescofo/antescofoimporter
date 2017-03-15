//
// .-. . . .-. .-. .-. .-. .-. .-. .-.
// |-| |\|  |  |-  `-. |   | | |-  | |
// ` ' ' `  '  `-' `-' `-' `-' '   `-'
//
// .-. .  . .-. .-. .-. .-. .-. .-.
//  |  |\/| |-' | | |(   |  |-  |(
// `-' '  ` '   `-' ' '  '  `-' ' '
//
//  Measure.cpp
//
//  Created by Robert Piéchaud on 10/05/15.
//  Copyright (c) 2015 ircam. All rights reserved.
//

#include "Measure.h"
#include <iomanip>

using namespace antescofo;

Measure::Measure( float num, float duration, float accum, float metricFactor, const std::string& timeSignature ):
    Event           (),
    beatDuration_   ( duration ),
    accumBeats_     ( accum ),
    metricFactor_   ( metricFactor ),
    timeSignature_  ( timeSignature ),
    keyAccidentals_ ( 0 )
{
    measure_ = num;
}

Measure::Measure( float num, float duration, float accum, float metricFactor, int keyAccidentals, const std::string& timeSignature ):
    Event           (),
    beatDuration_   ( duration ),
    accumBeats_     ( accum ),
    metricFactor_   ( metricFactor ),
    timeSignature_  ( timeSignature ),
    keyAccidentals_ ( keyAccidentals )
{
    measure_ = num;
}

Measure::~Measure()
{
    //NOTHING
}

void Measure::serialize( std::ostringstream& stream )
{
    char buffer [32];
    float duration = accumBeats_;///metricFactor_;
    if ( fabs( round(duration) - duration ) < 0.05 )
        sprintf(buffer, "%ld", (long) duration );
    else
        sprintf(buffer, "%5.1f", duration );
    stream << std::endl;
    bool isPickup = ceilf( measure() ) != measure();
    if ( isPickup )
        stream << "; ----------- pickup to measure " << (int) ceilf( measure() );
    else
        stream << "; ----------- measure " << (int) measure();
    stream << " --- beat " << buffer;
    if ( timeSignature().size() > 0 && !isPickup )
        stream << " --- time signature " << timeSignature();
    stream << std::endl << std::endl;
}

const std::string& Measure::timeSignature() const
{
    return timeSignature_;
}

float Measure::metricFactor() const
{
    return metricFactor_;
}

bool Measure::hasNotes() const
{
    return false;
}

bool Measure::isMeasure() const
{
    return true;
}

float Measure::duration() const
{
    return beatDuration_;
}

void Measure::changeDuration( float newTime )
{
    beatDuration_ = newTime;
}

int Measure::keyAccidentals() const
{
    return keyAccidentals_;
}
