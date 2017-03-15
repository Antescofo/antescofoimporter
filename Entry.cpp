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
#include <algorithm>

using namespace antescofo;
using namespace std;

Entry::Entry( float measure, float start, float duration, const Pitch& pitch ):
    Event         (),
    start_        ( start),
    duration_     ( duration ),
    features_     ( pitch.features() )
{
    Pitch newPitch ( pitch );
    int cents = pitch.midiCents();
    measure_ = measure;
    if ( features_ & Harmonic )
    {
        features_ |= NaturalHarmonic;
        cents = getHarmonicSoundingPitch( cents, cents );
    }
    if ( features_ & Trill && cents > 0 )
    {
        newPitch.setTied( false );
        Pitch newTrillPitch ( newPitch );
        int trillCents = (cents/100)*100;
        int semitones = 1;
        if ( features_ & WholeToneTrill )
            ++semitones;
        trillCents += semitones*100;
        newTrillPitch.setMidiCents( trillCents );
        newTrillPitch.unflagFeatures( OriginalEnharmony|Trill );
        secondaryPitches_.push_back( newTrillPitch );
    }
    else if ( features_ & AlternateTremolo )
    {
        features_ |= Trill;
        newPitch.setTied( false );
    }
    newPitch.setMidiCents( cents );
    newPitch.setFeatures( features_ );
    pitches_.push_back( newPitch );
}

Entry::Entry( const Entry& from ):
    Event             ( from ),
    start_            ( from.start_ ),
    duration_         ( from.duration_ ),
    pitches_          ( from.pitches_ ),
    secondaryPitches_ ( from.secondaryPitches_ ),
    features_         ( from.features_ )
{
    //NOTHING
}

Entry::~Entry()
{
    //NOTHING
}

const std::list<Pitch>& Entry::pitches() const
{
    return pitches_;
}

std::list<Pitch>& Entry::pitches()
{
    return pitches_;
}

const std::list<Pitch>& Entry::secondaryPitches() const
{
    return secondaryPitches_;
}

std::list<Pitch>& Entry::secondaryPitches()
{
    return secondaryPitches_;
}

bool comparePitches( const Pitch& p1, const Pitch& p2 )
{
    return p1.midiCents() < p2.midiCents();
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
    else if ( features_& (Trill | AlternateTremolo) && !( features_&Trill && pitches_.size() == 1 && !secondaryPitches_.size() ) )
    {
        serializeTrill( stream );
    }
    else if ( pitches_.size() == 1 )
    {
        serializeNote( stream );
    }
    else if ( pitches_.size() > 1 )
    {
        serializeChord( stream );
    }
    stream << formatDuration(); //<< " ; " << duration_;
    bool isPickup = ceilf( measure() ) != measure();
    if ( features_& Staccato )
        stream << " @staccato";
    if ( features_& Fermata )
        stream << " @fermata";
    if ( isFirstInMeasure() && !isPickup && !(features_&MeasureRest) )
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
    << pitches_.begin()->serialize()
    << " ";
}

void Entry::serializeChord( ostringstream& stream )
{
    pitches_.sort( comparePitches );
    stream << "CHORD " << "(";
    for ( auto it = pitches_.begin(); it != pitches_.end(); ++it )
    {
        if ( it != pitches_.begin() )
            stream << " ";
        stream << it->serialize();
    }
    stream << ") ";
}
void Entry::serializeFastRepeatedTremolo( ostringstream& stream )
{
    stream << "TRILL (";
    if ( pitches_.size() == 1 )
    {
        stream << pitches_.begin()->serialize();
    }
    else if ( pitches_.size() > 1 )
    {
        pitches_.sort( comparePitches );
        stream << "(";
        for ( auto it = pitches_.begin(); it != pitches_.end(); ++it )
        {
            if ( it != pitches_.begin() )
                stream << " ";
            stream << it->serialize();
        }
        stream << ")";
    }
    stream << ") ";
}

void Entry::serializeTrill( ostringstream& stream )
{
    pitches_.sort( comparePitches );
    stream << "TRILL " << "(";
    if ( pitches_.size() > 1 )
        stream << "(";
    for ( auto it = pitches_.begin(); it != pitches_.end(); ++it )
    {
        it->setTied( false );
        if ( it != pitches_.begin() )
            stream << " ";
        stream << it->serialize();
    }
    std::list<Pitch> trillPitches;
    if ( !(features_ & AlternateTremolo) )
        trillPitches = pitches_;
    trillPitches.merge( secondaryPitches_ );
    trillPitches.sort( comparePitches );
    if ( pitches_.size() > 1 )
        stream << ") (";
    else
        stream << " ";
    bool started = false;
    for ( auto it = trillPitches.begin(); it != trillPitches.end(); ++it )
    {
        it->setTied( false );
        //int cents = it->midiCents();
        if ( it->features()&Trill )
        //if ( cents-(cents/100)*100 == 1 )   //let us remove the primary trill pitches
            continue;
        if ( started )
            stream << " ";
        started = true;
        stream << it->serialize();
    }
    if ( pitches_.size() > 1 )
        stream << ")";
    stream << ") ";
}

void Entry::serializeMulti( ostringstream& stream )
{
    std::string tremolo = ( features_& FastRepeatedTremolo )? "'":"";
    pitches_.sort( comparePitches );
    stream << "MULTI " << "(";
    if ( pitches_.size() > 1 || ( pitches_.size() == 1 && secondaryPitches_.size() > 1 ) )
        stream << "(";
    for ( auto it = pitches_.begin(); it != pitches_.end(); ++it )
    {
        if ( it != pitches_.begin() )
            stream << " ";
        stream << it->serialize();
    }
    if ( pitches_.size() == 1 && secondaryPitches_.size() > 1 )
    {
        for ( int i = 1; i < secondaryPitches_.size(); ++i )
            stream << " " << pitches_.begin()->serialize();
    }
    if ( pitches_.size() > 1 || ( pitches_.size() == 1 && secondaryPitches_.size() > 1 ) )
        stream << ")";;
    stream << tremolo << " -> ";
    bool started = false;
    if ( secondaryPitches_.size() > 1 || ( secondaryPitches_.size() == 1 && pitches_.size() > 1 ) )
         stream << "(";
    for ( auto it = secondaryPitches_.begin(); it != secondaryPitches_.end(); ++it )
    {
        if ( started )
            stream << " ";
        started = true;
        stream << it->serialize();
    }
    if ( secondaryPitches_.size() == 1 && pitches_.size() > 1 )
    {
        for ( int i = 1; i < pitches_.size(); ++i )
            stream << " " << secondaryPitches_.begin()->serialize();
    }
    if ( secondaryPitches_.size() > 1 || ( secondaryPitches_.size() == 1 && pitches_.size() > 1 ) )
        stream << ")";
    stream << tremolo;
    stream << ") ";
}

string Entry::formatDuration() const
{
    string displayDuration = to_string( duration_ );
    if ( duration_ == 0.40000 )
        displayDuration = displayDuration;
    unsigned short limit = features_& MidiNote? 16:64;
    float theta = features_& MidiNote? THETA_MIDI:THETA;
    float epsilon = features_& MidiNote? EPSILON_MIDI:EPSILON;
    // some particular case...
    if ( !(features_& MidiNote) && duration_ < (float)1/12 )
        theta = duration_*3.3;
    if ( features_& MidiNote )
    {
        if ( fabs( duration_ - 0.23333 ) < EPSILON_MIDI || fabs( duration_ - 0.26666 ) < EPSILON_MIDI )
            return "1/4";
        else if ( fabs( duration_ - 0.4) < EPSILON_MIDI/2 )
            return "5/12";
        else if ( fabs( duration_ - 0.083) < EPSILON_MIDI/2 )
            return "1/12";
    }
    for ( int i = 1; i <= limit; ++i )
    {
        // let us keep it simple with MIDI files:
        if ( features_ & MidiNote && i != 1 && i != 2 && i != 3 && i != 4 && i != 6 && i != 8 && i != 12 && i != 16 )
            continue;
        // otherwise (musicxml) filtering some undesired 'far away' prime numbers:
        if ( i == 23 || i == 29 || i == 31 || i == 37 || i == 41 || i == 43 || i == 47 || i == 53  || i == 59  || i == 61 )
            continue;
        if ( fabs( round( duration_*i ) - duration_*i ) < epsilon * (1 + i*theta) )
        {
            displayDuration = to_string( (int) round( duration_ * i ) );
            if ( i > 1 )
                displayDuration += "/" + to_string( i );
            break;
        }
    }
    return displayDuration;
}

bool Entry::addPitch( const Pitch& pitch )
{
    int midiCents = pitch.midiCents();
    auto it = find( pitches_.begin(), pitches_.end(), pitch );
    if ( it == pitches_.end() )
    {
        if ( pitches_.size() > 0 && pitch == 0 )  // rest + note = note !
            return false;
        if ( pitches_.size() == 1 && *pitches_.begin() == 0 ) // existing rest
        {
            Pitch& existingPitch = *pitches_.begin();
            EntryFeatures existingFeatures = existingPitch.features();
            existingPitch = pitch;
            features_ &= ~MeasureRest;
            if ( existingFeatures&Trill )       //particular case of buggy Sibelius export... :-(
            {
                existingPitch.setTied( false );
                existingPitch.flagFeatures( Trill );
                Pitch newTrillPitch ( pitch );
                int trillCents = (midiCents/100)*100;
                int semitones = 1;
                trillCents += semitones*100;
                newTrillPitch.setMidiCents( trillCents );
                newTrillPitch.unflagFeatures( OriginalEnharmony|Trill);
                secondaryPitches_.push_back( newTrillPitch );
            }
        }
        else
        {
            int soundingPitch = 0;
            if ( pitch.features() & Harmonic )
            {
                if ( pitches_.size() == 1 )
                {
                    if ( !( features_ & Harmonic ) )
                    {
                        soundingPitch = getHarmonicSoundingPitch( pitches_.begin()->midiCents(), midiCents );
                        if ( soundingPitch != 0 )
                        {
                            (*pitches_.begin()).setMidiCents( soundingPitch );
                            (*pitches_.begin()).unflagFeatures( OriginalEnharmony );
                            (*pitches_.begin()).setTied( pitch.isTiedBackwards() );
                        }
                    }
                    else
                    {
                        midiCents = getHarmonicSoundingPitch( midiCents, midiCents );
                    }
                }
                else if ( pitches_.size() >= 2 )
                {
                    soundingPitch = getHarmonicSoundingPitch( (--pitches_.end())->midiCents(), midiCents );
                    if ( soundingPitch != 0 )
                    {
                        (*--pitches_.end()).setMidiCents( soundingPitch );
                        (*--pitches_.end()).unflagFeatures( OriginalEnharmony );
                        (*--pitches_.end()).setTied( pitch.isTiedBackwards() );
                    }
                    if ( features_ & Harmonic )
                        features_ |= DoubleHarmonic;
                }
            }
            if ( soundingPitch == 0 )
            {
                pitches_.push_back( pitch );
            }
        }
        features_ |= pitch.features();
    }
    if ( ( features_&AlternateTremolo && !(pitch.features() & (AlternateTremolo|TremoloEnd )))
        || (features_ & GlissandoStart && !(pitch.features()&GlissandoStart) ) )  //pitch needs to be added to secondary group if coming from other voice
    {
        auto it = find (secondaryPitches_.begin(), secondaryPitches_.end(), pitch );
        if ( it == secondaryPitches_.end() )
        {
            if ( secondaryPitches_.size() > 0 && midiCents == 0 )  // rest + note = note !
                return false;
            if ( secondaryPitches_.size() == 1 && *pitches_.begin() == 0 ) // existing rest
            {
                (*secondaryPitches_.begin()).setMidiCents( midiCents );
                (*secondaryPitches_.begin()).setFeatures( pitch.features() );
            }
            else
            {
                secondaryPitches_.push_back( pitch );
            }
        }
    }
    return true;
}

int Entry::getHarmonicSoundingPitch( int lowPitch, int writtenHarmonic ) const
{
    int interval = writtenHarmonic - lowPitch;
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

void Entry::addSecondaryPitch( const Pitch& pitch )
{
    int midiCents = pitch.midiCents();
    auto it = find( secondaryPitches_.begin(), secondaryPitches_.end(), pitch );
    if ( it == secondaryPitches_.end() )
    {
        if ( secondaryPitches_.size() > 0 && midiCents == 0 )  // rest + note = note !
            return;
        if ( secondaryPitches_.size() == 1 && *secondaryPitches_.begin() == 0 ) // existing rest
        {
            (*secondaryPitches_.begin()).setMidiCents( midiCents );
            (*secondaryPitches_.begin()).setFeatures( pitch.features() );
        }
        else
        {
            int soundingPitch = 0;
            if ( pitch.features() & Harmonic )
            {
                if ( secondaryPitches_.size() == 1 )
                {
                    soundingPitch = getHarmonicSoundingPitch( secondaryPitches_.begin()->midiCents(), midiCents );
                    if ( soundingPitch != 0 )
                    {
                        (*secondaryPitches_.begin()).setMidiCents( soundingPitch );
                        (*secondaryPitches_.begin()).unflagFeatures( OriginalEnharmony );
                    }
                }
                else if ( secondaryPitches_.size() >= 2 )
                {
                    soundingPitch = getHarmonicSoundingPitch( (--secondaryPitches_.end())->midiCents(), midiCents );
                    if ( soundingPitch != 0 )
                    {
                        (*--secondaryPitches_.end()).setMidiCents( soundingPitch );
                        (*--secondaryPitches_.end()).unflagFeatures( OriginalEnharmony );
                    }
                    if ( features_ & Harmonic )
                        features_ |= DoubleHarmonic;
                }
            }
            if ( soundingPitch == 0 )
            {
                Pitch newPitch( pitch );
                newPitch.setMidiCents( midiCents );
                secondaryPitches_.push_back( newPitch );
            }
        }
    }
}

void Entry::tiePitches()
{
    for ( auto it = pitches_.begin(); it != pitches_.end(); ++it )
    {
        it->setTied( !(features_&Trill) );
    }
}

bool Entry::isTiedTo( Entry* next )
{
    if ( next == nullptr ||
         duration_ == 0.0 ||
         type() != Event_Entry || next->type() != Event_Entry ||
         (pitches_.size() != next->pitches_.size() ) ||
         measure_ != next->measure_ ||
         !isEqual( start_ + duration_, next->start() ) )
        return false;
    pitches_.sort(comparePitches);
    next->pitches_.sort(comparePitches);
    auto it = pitches_.begin(), itNext = next->pitches_.begin();
    while ( it != pitches_.end() )
    {
        if ( itNext->midiCents() > 0 || itNext->midiCents() != it->midiCents() )
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

void Entry::removeFeatures( EntryFeatures feature )
{
    features_ &= ~feature;
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
    return pitches_.size() > 0 && *pitches_.begin() == 0;
}

