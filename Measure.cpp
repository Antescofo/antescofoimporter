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
#include "rational.h"
#include <iomanip>
#include <iostream>

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

/*
rational Measure::getPulseSignature() const
{
    return inferePulseSignature(timeSignature());
}
*/
/*
rational Measure::inferePulseSignature(std::string const& timeSignature)
{
    // if there is an error, returns 1 = 1/1
    rational pulse(1, 1);
    // Parse timeSignature as rational number
    rational r(timeSignature);
    if (r.getNumerator() == 0)    // parsing error
        return pulse;
    long int const denom = r.getDenominator();
    long int const num = r.getNumerator();

    // Inférence de la pulse à partir de la time-signature
    // see: http://www2.siba.fi/muste1/index.php?id=98&la=en
    
    if (denom == 4)
        pulse.set(1, 1);        // TODO: traiter le cas de la pulsation à la blanche pointée
    else if (denom == 3)
        pulse.set(4, 3);
    else if (denom == 2)
        pulse.set(2, 1);
    else if (denom == 1)
        pulse.set(4, 1);
    else if (denom == 8)    // cas particulier: 6/8, 9/8, 12/8, etc. --> pulse = 1.5 (3/2)  // sinon, pulse = 0.5
    {
        if ((num % 3 == 0) && (num > 3))
            pulse.set(3,2);     // doted quarter note
        else
            pulse.set(1,2);           // eight note
    }
    else if (denom == 16)    // cas particulier: 3/16, 6/16, 9/16, 12/16, etc. --> pulse = 3/4 // sinon, pulse = 1/4
    {
        if ((num % 3 == 0)) //&& (num > 3)
            pulse.set(3,4);     // doted eight note
        else
            pulse.set(1,4);           // sixteenth note
    }
    else    // normalement, on ne devrait pas tomber sur ce cas
    {
        // Change numerator to quarter note
        pulse.setNumerator(4);
        // (optional) Reduce
        pulse.rationalise();
    }
    
    return pulse;
}*/

/*
void Measure::queryPulseChange(std::deque<std::pair<float, std::string> > & pulseChangePositions) // by default, do nothing
{
    std::string const& lastPulse = (pulseChangePositions.empty() ? "" : pulseChangePositions.back().second);
    
    std::string const& currentPulse = this->getPulseSignature().toString();
    
    // filter only CHANGES of pulse
    if (currentPulse != lastPulse)
    {
        // get beat position of last change
        float const lastPosition = (pulseChangePositions.empty() ? 0.f : pulseChangePositions.back().first);
        // get current position
        float const currentPosition = this->accumBeats();
        // should not happen!
        if (currentPosition == lastPosition)
        {
            if (!pulseChangePositions.empty())
            {
                std::cerr << "Warning, simultaneous pulse changes at position " << currentPosition << ", current pulse=" << currentPulse << " and last pulse=" << lastPulse << std::endl;
                pulseChangePositions.pop_back();
            }
        }
        else if (currentPosition < lastPosition) // should NOT happen at all !
            return;
        // get current position
        pulseChangePositions.emplace_back(currentPosition, currentPulse);
    }
}*/

/*
void Measure::queryTempoBeatUnitChanges(std::deque<std::pair<float, rational> >& beatUnitChanges) const
{
    // Get last beat unit
    rational const& lastBeatUnit = (beatUnitChanges.empty() ? rational(0, 1) : beatUnitChanges.back().second);
    // Get beat unit of current measure
    rational const& currentBeatUnit = this->getPulseSignature();
}*/
