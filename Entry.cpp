//
// .-. . . .-. .-. .-. .-. .-. .-. .-.
// |-| |\|  |  |-  `-. |   | | |-  | |
// ` ' ' `  '  `-' `-' `-' `-' '   `-'
//
// .-. .  . .-. .-. .-. .-. .-. .-.
//  |  |\/| |-' | | |(   |  |-  |(
// `-' '  ` '   `-' ' '  '  `-' ' '
//
//  Entry.cpp
//
//  Created by Robert Piéchaud on 09/05/15.
//  Copyright (c) 2015 ircam. All rights reserved.
//

#include "Entry.h"
#include <math.h>

using namespace antescofo;
using namespace std;

Entry::Entry( int measure, float start, float duration, int pitch, bool displayCents, EntryFeatures features /* = None */ ):
    Event         (),
    start_        ( start),
    duration_     ( duration ),
    displayCents_ ( displayCents ),
    features_     ( features )
{
    measure_ = measure;
    if ( features_ & Harmonic )
    {
        features_ |= NaturalHarmonic;
        pitch = getHarmonicSoundingPitch( pitch, pitch );
    }
    if ( features_ & Trill )
    {
        pitch = abs( pitch );
        int trillPitch = (pitch/100)*100;
        int semitones = 1;
        if ( features & WholeToneTrill )
            ++semitones;
        trillPitch += semitones*100;
        secondaryPitches_.push_back( trillPitch );
    }
    else if ( features & AlternateTremolo )
    {
        features_ |= Trill;
    }
    centsPitches_.push_back( pitch );
}

Entry::Entry( const Entry& from ):
    Event             ( from ),
    start_            ( from.start_ ),
    duration_         ( from.duration_ ),
    centsPitches_     ( from.centsPitches_ ),
    secondaryPitches_ ( from.secondaryPitches_ ),
    displayCents_     ( from.displayCents_ ),
    features_         ( from.features_ )
{
    //NOTHING
}

Entry::~Entry()
{
    //NOTHING
}

const std::list<int>& Entry::pitches() const
{
    return centsPitches_;
}

std::list<int>& Entry::pitches()
{
    return centsPitches_;
}

const std::list<int>& Entry::secondaryPitches() const
{
    return secondaryPitches_;
}

std::list<int>& Entry::secondaryPitches()
{
    return secondaryPitches_;
}

bool comparePitches( int a, int b ) { return abs( a ) < abs( b ); }

string Entry::serializePitch( int midiCents ) const
{
    if ( abs( midiCents-( midiCents/100 )*100 ) == 1 )   //pitch marked as trill (ex: 6400-> 6401)
    {
        midiCents = abs( ( midiCents/100 )*100 );
    }
    unsigned int absCents = abs( midiCents );
    if ( displayCents_ || absCents == 0 || absCents%100 != 0 )
        return to_string( midiCents );
    unsigned int relative = (absCents/100)%12;
    char symbolic[3];
    char* ptr = symbolic;
    switch ( relative )
    {
        case 0: *ptr = 'C'; break;
        case 1: *ptr++ = 'C'; *ptr = '#'; break;
        case 2: *ptr = 'D'; break;
        case 3: *ptr++ = 'D'; *ptr = '#'; break;
        case 4: *ptr = 'E'; break;
        case 5: *ptr = 'F'; break;
        case 6: *ptr++ = 'F'; *ptr = '#'; break;
        case 7: *ptr = 'G'; break;
        case 8: *ptr++ = 'G'; *ptr = '#'; break;
        case 9: *ptr = 'A'; break;
        case 10: *ptr++ = 'A'; *ptr = '#'; break;
        case 11: *ptr = 'B'; break;
    }
    *(++ptr) = 0;
    return ( midiCents < 0? string( "-"):string("") ) + string( symbolic ) + to_string( ((int) absCents/1200) - 1 );
}

void Entry::serialize( ostringstream& stream )
{
    
    if ( features_& GlissandoEnd && duration_ == 0 )
    {
        return;
    }
    else if ( features_& GlissandoStart )
    {
        serializeMulti( stream );
    }
    else if ( features_& FastRepeatedTremolo )
    {
        serializeFastRepeatedTremolo( stream );
    }
    else if ( features_& (Trill | AlternateTremolo) )
    {
        serializeTrill( stream );
    }
    else if ( centsPitches_.size() == 1 )
    {
        serializeNote( stream );
    }
    else if ( centsPitches_.size() > 1 )
    {
        serializeChord( stream );
    }
    stream << formatDuration();
    if ( features_& Staccato )
        stream << " @staccato";
    if ( features_& Fermata )
        stream << " @fermata";
    if ( isFirstInMeasure() )
        stream << " measure" << measure();
    if ( features_ & Harmonic )
        stream << " ; harmonics";
    if ( features_ & SquareNotehead )
        stream << " ; square notehead";
    stream << endl;
}

void Entry::serializeNote( ostringstream& stream )
{
    stream << "NOTE "
    << serializePitch( *centsPitches_.begin() )
    << " ";
}

void Entry::serializeChord( ostringstream& stream )
{
    centsPitches_.sort( comparePitches );
    stream << "CHORD " << "(";
    for ( auto it = centsPitches_.begin(); it != centsPitches_.end(); ++it )
    {
        if ( it != centsPitches_.begin() )
            stream << " ";
        stream << serializePitch( *it );
    }
    stream << ") ";
}
void Entry::serializeFastRepeatedTremolo( ostringstream& stream )
{
    stream << "TRILL (";
    if ( centsPitches_.size() == 1 )
    {
        stream << serializePitch( *centsPitches_.begin() );
    }
    else if ( centsPitches_.size() > 1 )
    {
        centsPitches_.sort( comparePitches );
        stream << "(";
        for ( auto it = centsPitches_.begin(); it != centsPitches_.end(); ++it )
        {
            if ( it != centsPitches_.begin() )
                stream << " ";
            stream << serializePitch( *it );
        }
        stream << ")";
    }
    stream << ") ";
}

void Entry::serializeTrill( ostringstream& stream )
{
    centsPitches_.sort( comparePitches );
    stream << "TRILL " << "(";
    if ( centsPitches_.size() > 1 )
        stream << "(";
    for ( auto it = centsPitches_.begin(); it != centsPitches_.end(); ++it )
    {
        if ( it != centsPitches_.begin() )
            stream << " ";
        stream << serializePitch( abs( *it ) );
    }
    std::list<int> trillPitches;
    if ( !(features_ & AlternateTremolo) )
        trillPitches = centsPitches_;
    trillPitches.merge( secondaryPitches_ );
    trillPitches.sort( comparePitches );
    if ( centsPitches_.size() > 1 )
        stream << ") (";
    else
        stream << " ";
    bool started = false;
    for ( auto it = trillPitches.begin(); it != trillPitches.end(); ++it )
    {
        int cents = *it;
        if ( abs( cents-(cents/100)*100 ) == 1 )   //let us remove the primary trill pitches
            continue;
        if ( started )
            stream << " ";
        started = true;
        stream << serializePitch( abs( *it ) );
    }
    if ( centsPitches_.size() > 1 )
        stream << ")";
    stream << ") ";
}

void Entry::serializeMulti( ostringstream& stream )
{
    std::string tremolo = ( features_& FastRepeatedTremolo )? "'":"";
    centsPitches_.sort( comparePitches );
    stream << "MULTI " << "(";
    if ( centsPitches_.size() > 1 )
        stream << "(";
    for ( auto it = centsPitches_.begin(); it != centsPitches_.end(); ++it )
    {
        if ( it != centsPitches_.begin() )
            stream << " ";
        stream << serializePitch( abs( *it ) );
    }
    if ( centsPitches_.size() > 1 )
        stream << ")" << tremolo << " -> (";
    else
        stream << tremolo << " -> ";
    bool started = false;
    for ( auto it = secondaryPitches_.begin(); it != secondaryPitches_.end(); ++it )
    {
        if ( started )
            stream << " ";
        started = true;
        stream << serializePitch( abs( *it ) );
    }
    if ( centsPitches_.size() > 1 )
        stream << ")";
    stream << tremolo;
    stream << ") ";
}

string Entry::formatDuration() const
{
    string displayDuration = to_string( duration_ );
    unsigned short limit = features_& MidiNote? 24:64;
    float epsilon = features_& MidiNote? THETA:EPSILON;
    for ( int i = 1; i <= limit; ++i )
    {
        if ( features_ && MidiNote && ( i == 7 || i == 10 || i == 11 || i == 13 ) )
            continue;
        if ( fabs( round( duration_*i ) - duration_*i ) < epsilon*(1+i*0.2) )
        {
            displayDuration = to_string( (int) round( duration_ * i ) );
            if ( i > 1 )
                displayDuration += "/" + to_string( i );
            break;
        }
    }
    return displayDuration;
}

bool Entry::addPitch( int midiCents, EntryFeatures noteFeature )
{
    auto it = find (centsPitches_.begin(), centsPitches_.end(), midiCents);
    if ( it == centsPitches_.end() )
        it = find (centsPitches_.begin(), centsPitches_.end(), -midiCents);
    if ( it == centsPitches_.end() )
    {
        if ( centsPitches_.size() > 0 && midiCents == 0 )  // rest + note = note !
            return false;
        if ( centsPitches_.size() == 1 && *centsPitches_.begin() == 0 ) // existing rest
            *centsPitches_.begin() = midiCents;
        else
        {
            int soundingPitch = 0;
            if ( noteFeature & Harmonic )
            {
                if ( centsPitches_.size() == 1 )
                {
                    if ( !( features_ & Harmonic ) )
                    {
                        soundingPitch = getHarmonicSoundingPitch( *centsPitches_.begin(), midiCents );
                        if ( soundingPitch != 0 )
                            *centsPitches_.begin() = soundingPitch;
                    }
                    else
                    {
                        midiCents = getHarmonicSoundingPitch( midiCents, midiCents );
                    }
                }
                else if ( centsPitches_.size() >= 2 )
                {
                    soundingPitch = getHarmonicSoundingPitch( *--centsPitches_.end(), midiCents );
                    if ( soundingPitch != 0 )
                        *--centsPitches_.end() = soundingPitch;
                    if ( features_ & Harmonic )
                        features_ |= DoubleHarmonic;
                }
            }
            if ( soundingPitch == 0 )
                centsPitches_.push_back( midiCents );
        }
        features_ |= noteFeature;
    }
    else if ( midiCents > 0 )
    {
        *it = abs( *it );
    }
    if ( ( features_&AlternateTremolo && !(noteFeature & (AlternateTremolo|TremoloEnd )))
        || (features_ & GlissandoStart && !(noteFeature&GlissandoStart) ) )  //pitch needs to be added to secondary group if coming from other voice
    {
        auto it = find (secondaryPitches_.begin(), secondaryPitches_.end(), midiCents);
        if ( it == secondaryPitches_.end() )
            it = find (secondaryPitches_.begin(), secondaryPitches_.end(), -midiCents);
        if ( it == secondaryPitches_.end() )
        {
            if ( secondaryPitches_.size() > 0 && midiCents == 0 )  // rest + note = note !
                return false;
            if ( secondaryPitches_.size() == 1 && *centsPitches_.begin() == 0 ) // existing rest
                *secondaryPitches_.begin() = midiCents;
            else
                secondaryPitches_.push_back( midiCents );
        }
        else if ( midiCents > 0 )
        {
            *it = abs( *it );
        }
    }
    return true;
}

int Entry::getHarmonicSoundingPitch( int lowPitch, int writtenHarmonic ) const
{
    lowPitch = abs( lowPitch );
    int interval = abs( writtenHarmonic ) - lowPitch;
    int soundingPitch = 0;
    switch ( interval )
    {
        case Unison:
        {
            soundingPitch = lowPitch; //+ Octave;
            break;
        }
        case MinorThird:
        {
            soundingPitch = lowPitch + Octave*2 + Fifth;
            break;
        }
            
        case MajorThird:
        case MajorSixth:
        case MajorTenth:
        {
            soundingPitch = lowPitch + Octave*2 + MajorThird;
            break;
        }
            
        case Forth:
        {
            soundingPitch = lowPitch + Octave*2;
            break;
        }
            
        case Fifth:
        {
            soundingPitch = lowPitch + Octave + Fifth;
            break;
        }
            
        case Octave:
        {
            soundingPitch = lowPitch + Octave ;
            break;
        }
            
        default:
        {
            soundingPitch = 0;
            break;
        }
    }
    if ( writtenHarmonic < 0 ) // tied note
        soundingPitch = -soundingPitch;
    return soundingPitch;
}

void Entry::addSecondaryPitch( int midiCents, EntryFeatures feature )
{
    auto it = find (secondaryPitches_.begin(), secondaryPitches_.end(), midiCents);
    if ( it == secondaryPitches_.end() )
        it = find (secondaryPitches_.begin(), secondaryPitches_.end(), -midiCents);
    if ( it == secondaryPitches_.end() )
    {
        if ( secondaryPitches_.size() > 0 && midiCents == 0 )  // rest + note = note !
            return;
        if ( secondaryPitches_.size() == 1 && *secondaryPitches_.begin() == 0 ) // existing rest
            *secondaryPitches_.begin() = midiCents;
        else
        {
            int soundingPitch = 0;
            if ( feature & Harmonic )
            {
                if ( secondaryPitches_.size() == 1 )
                {
                    soundingPitch = getHarmonicSoundingPitch( *secondaryPitches_.begin(), midiCents );
                    if ( soundingPitch != 0 )
                        *secondaryPitches_.begin() = soundingPitch;
                }
                else if ( secondaryPitches_.size() >= 2 )
                {
                    soundingPitch = getHarmonicSoundingPitch( *--secondaryPitches_.end(), midiCents );
                    if ( soundingPitch != 0 )
                        *--secondaryPitches_.end() = soundingPitch;
                    if ( features_ & Harmonic )
                        features_ |= DoubleHarmonic;
                }
            }
            if ( soundingPitch == 0 )
                secondaryPitches_.push_back( midiCents );
        }
    }
    else if ( midiCents > 0 )
    {
        *it = abs( *it );
    }
}

void Entry::tiePitches()
{
  for ( auto it = centsPitches_.begin(); it != centsPitches_.end(); ++it )
  {
    (*it) = - abs( (*it) );
  }
}

bool Entry::isTiedTo( Entry* next )
{
    if ( next == nullptr ||
         duration_ == 0.0 ||
         type() != Event_Entry || next->type() != Event_Entry ||
         (centsPitches_.size() != next->centsPitches_.size() ) ||
         measure_ != next->measure_ ||
         !isEqual( start_ + duration_, next->start() ) )
        return false;
    centsPitches_.sort(comparePitches);
    next->centsPitches_.sort(comparePitches);
    auto it = centsPitches_.begin(), itNext = next->centsPitches_.begin();
    while ( it != centsPitches_.end() )
    {
        if ( *itNext > 0 || abs( *itNext ) != abs( *it ) )
            return false;
        ++it;
        ++itNext;
    }
    return true;
}


void Entry::addFeatures( EntryFeatures feature )
{
    features_ |= feature;
}

bool Entry::hasNotes() const  // true even for a rest...
{
    return true;
}

float Entry::start() const
{
    return start_;
}

float Entry::duration() const
{
    return duration_;
}

void Entry::changeStart( float newTime )
{
    start_ = newTime;
}

void Entry::changeDuration( float newTime )
{
    duration_ = newTime;
}

Event* Entry::duplicate() const
{
    return new Entry( *this );
}


bool Entry::isRest() const
{
    return centsPitches_.size() > 0 && *centsPitches_.begin() == 0;
}

