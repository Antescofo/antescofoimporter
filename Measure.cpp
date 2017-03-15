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

Measure::Measure( int num, float duration, float accum, float metricFactor, const std::string& timeSignature ):
    Event           (),
    duration_       ( duration ),
    accumDuration_  ( accum ),
    metricFactor_   ( metricFactor ),
    timeSignature_  ( timeSignature ),
    keyAccidentals_ ( 0 )
{
    measure_ = num;
}

Measure::Measure( int num, float duration, float accum, float metricFactor, int keyAccidentals, const std::string& timeSignature ):
    Event           (),
    duration_       ( duration ),
    accumDuration_  ( accum ),
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
    if ( round(accumDuration_) == accumDuration_ )
        sprintf(buffer, "%ld", (long) accumDuration_ );
    else
        sprintf(buffer, "%5.1f", accumDuration_ );
    stream << std::endl
           << "; ----------- measure " << measure()
           << " --- beat " << buffer;
    if ( timeSignature().size() > 0 )
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
    return duration_;
}

void Measure::changeDuration( float newTime )
{
    duration_ = newTime;
}

int Measure::keyAccidentals() const
{
    return keyAccidentals_;
}
