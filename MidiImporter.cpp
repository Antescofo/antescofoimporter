//
// .-. . . .-. .-. .-. .-. .-. .-. .-.
// |-| |\|  |  |-  `-. |   | | |-  | |
// ` ' ' `  '  `-' `-' `-' `-' '   `-'
//
// .-. .  . .-. .-. .-. .-. .-. .-.
//  |  |\/| |-' | | |(   |  |-  |(
// `-' '  ` '   `-' ' '  '  `-' ' '
//
//  MidiImporter.cpp
//
//  Created by Robert Piéchaud on 09/05/15.
//  Copyright (c) 2015 ircam. All rights reserved.
//

#include "MidiImporter.h"
#include "ImporterWrapper.h"
#include "ImportModel.h"
#include "Measure.h"
#include "BeatPerMinute.h"
#include "Event.h"
#include <math.h>
#include <iostream>

using namespace antescofo;
using namespace std;

enum
{
    MIDI_HEADER = 'MThd',
    MIDI_TRACK = 'MTrk'
};

enum
{
    MIDI_NOTE_OFF               = 0x8,
    MIDI_NOTE_ON                = 0x9,
    MIDI_NOTE_AFTERTOUCH        = 0xA,
    MIDI_NOTE_CONTROL_CHANGE    = 0xB,
    MIDI_NOTE_PROGRAM_CHANGE    = 0xC,
    MIDI_NOTE_PRESSURE          = 0xD,
    MIDI_NOTE_PROGRAM_PITCHBEND = 0xE
};

enum
{
    MIDI_META_TRACKNAME     = 0x03,
    MIDI_META_ENDOFTRACK    = 0x2F,
    MIDI_META_TEMPO         = 0x51,
    MIDI_META_TIMESIGNATURE = 0x58,
    MIDI_META_KEYSIGNATURE  = 0x59
};

#define MIDI_MAX_SIZE 10000000

MidiImporter::MidiImporter( ImporterWrapper& wrapper ) :
    wrapper_                ( wrapper ),
    model_                  ( wrapper_.getModel() ),
    onCount_                ( 0 ),
    MidifileType_           ( 0 ),
    tracksCount_            ( 0 ),
    quarterNoteDivision_    ( 1024 ),
    runningStatus_          ( 0 ),
    accumGlobal_            ( 0 ),
    lastNoteOff_            ( 0 ),
    currentMeasure_         ( 0 ),
    currentMeasureDuration_ ( 0 ),
    currentMeasureStart_    ( 0 ),
    newMeasureDuration_     ( 0 ),
    currentQuarterNoteTempo_( 0.0 ),
    currentMetricFactor_    ( 1.0 )
{
    clear();
}

MidiImporter::~MidiImporter()
{
    //NOTHING
}

bool MidiImporter::import()
{
    clear();
    ifstream rawContent;
    rawContent.open( wrapper_.getInputPath(), ifstream::in );
    if ( rawContent.fail() )
        return false;
    if ( !processHeader( rawContent ) )
        return false;
    int processedCount = 0;
    bool trackSelection = ( tracks_.size() > 0 );
    for ( unsigned short t = 1; t <= tracksCount_; ++t )
    {
        if ( trackSelection && processedCount == tracks_.size() )
            break;
        if ( !trackSelection || ( tracks_.size() && find( tracks_.begin(), tracks_.end(), t - 1 ) != tracks_.end() ) )
        {
            if ( wrapper_.isVerbose() )
                cout << "  processing track " << (t - 1) << endl;
            if ( !processTrack( rawContent ) )
            {
                cout << "  MIDI error processing track " << t << " ;-(" << endl;
                return false;
            }
            ++processedCount;
        }
        else    //processing the track without updating the model
        {
            string dummy;
            if ( !processTrackForName( rawContent, dummy) )
            {
                cout << "  MIDI error skipping track " << t << " ;-(" << endl;
                return false;
            }
        }
    }
    tracks_.clear();
    model_.beautify();
    return true;
}

bool MidiImporter::import( const std::vector<int>& tracks )
{
    tracks_ = tracks;
    return import();
}

bool MidiImporter::queryTracks( std::vector<std::string>& tracks )
{
    clear();
    tracks_.clear();
    ifstream rawContent;
    rawContent.open( wrapper_.getInputPath(), ifstream::in );
    if ( rawContent.fail() )
        return false;
    if ( !processHeader( rawContent ) )
        return false;
    string name;
    for ( unsigned short t = 1; t <= tracksCount_; ++t )
    {
        if ( !tracks_.size() || ( tracks_.size() && find( tracks_.begin(), tracks_.end(), t - 1 ) != tracks_.end() ) )
        {
            if ( !processTrackForName( rawContent, name ) )
            {
                cout << "  MIDI error processing track " << t << " ;-(" << endl;
                return false;
            }
            tracks.push_back( name );
            if ( wrapper_.isVerbose() )
            {
                cout << "    ...track #" << ( t - 1 ) << endl;
            }
        }
    }
    return true;
}

bool MidiImporter::processTrackForName( ifstream& stream, string& name )
{
    name.clear();
    unsigned int fourBytes = 0;
    if (  readFourbyte( stream, fourBytes ) && fourBytes == MIDI_TRACK )
    {
        unsigned int size = 0;
        if ( readFourbyte( stream, size ) )
        {
            while ( true )
            {
                unsigned long delta = 0;
                short read = readTimeStamp( stream, delta );
                if ( read == 0 )
                    return false;
                unsigned char value = 0;
                if ( !readOnebyte( stream, value ) )
                    return false;
                if ( value == 0xFF )    //meta event
                {
                    unsigned char meta = 0;
                    if ( !readOnebyte( stream, meta ) )
                        return false;
                    if ( meta == MIDI_META_ENDOFTRACK )
                    {
                        readOnebyte( stream, value );
                        return true;
                    }
                    unsigned char metaSize = 0;
                    if ( !readOnebyte( stream, metaSize ) )
                        return false;
                    if ( meta == MIDI_META_TRACKNAME && !name.length() )
                    {
                        char tmp [2];
                        tmp[0] = tmp[1] = 0;
                        for ( int c = 0; c < metaSize; ++c )
                        {
                            readOnebyte( stream, value );
                            tmp[0] = (char) value;
                            name += tmp;
                        }
                        continue;
                    }
                    
                    for ( unsigned char j = 0; j < metaSize; ++j )
                    {
                        readOnebyte( stream, value );
                    }
                    continue;
                }
                unsigned char status = 0;
                unsigned char next = 0;
                if ( value < 0x80 ) //running status
                {
                    next = value;
                    status = runningStatus_;
                }
                else
                {
                    status = ( value & 0xF0 ) >> 4;
                    if ( !readOnebyte( stream, next ) )
                        return false;
                }
                runningStatus_ = status;
                
                if ( status == MIDI_NOTE_ON || status == MIDI_NOTE_OFF )
                {
                    unsigned char velocity = 0;
                    if ( !readOnebyte( stream, velocity ) )
                        return false;
                }
                else if ( status == MIDI_NOTE_CONTROL_CHANGE ||
                         status == MIDI_NOTE_PROGRAM_PITCHBEND ||
                         status == MIDI_NOTE_AFTERTOUCH)
                {
                    if ( !readOnebyte( stream, value ) )
                        return false;
                }
            }
        }
        return true;
    }
    return false;
}

bool MidiImporter::processHeader( ifstream& stream )
{
    unsigned int fourBytes = 0;
    if (  readFourbyte( stream, fourBytes ) )
    {
        if ( fourBytes != MIDI_HEADER )
        {
#ifdef DEBUG
            cout << "  MIDI error: unrecognized header" << endl;
#endif
            return false;
        }
        
        unsigned int size = 0;
        if ( readFourbyte( stream, size ) )
        {
            if ( size != 6 )
            {
                if ( wrapper_.isVerbose() )
                    cout << "  MIDI error: unexpected header size = " << size << endl;
                return false;
            }
        }
        else
            return false;
        if ( readTwobyte( stream, MidifileType_ ) )
        {
            if ( MidifileType_ > 1 )
            {
                cout << "  MIDI error: unsupported type = " << MidifileType_ << endl;
                return false;
            }
            string origin = "MIDI file type 0 (single track)";
            if ( MidifileType_ == 1 )
                origin = "MIDI file type 1 (multiple tracks)";
            model_.setFileOrigin( origin );
            if ( wrapper_.isVerbose() )
                cout << "  MIDI file type: " << MidifileType_ << endl;
            
            if ( readTwobyte( stream, tracksCount_ ) )
            {
                if ( wrapper_.isVerbose() )
                    cout << "  MIDI tracks count: " << tracksCount_ << endl;
                
                if ( readTwobyte( stream, quarterNoteDivision_ ) )
                {
                    if ( wrapper_.isVerbose() )
                        cout << "  tick/quarter note: " << quarterNoteDivision_ << endl;
                    return true;
                }
                return false;
            }
            return false;
            
        }
    }
    return false;
}

bool MidiImporter::processTrack( ifstream& stream )
{
    unsigned int fourBytes = 0;
    if (  readFourbyte( stream, fourBytes ) && fourBytes == MIDI_TRACK )
    {
        unsigned int size = 0;
        if ( readFourbyte( stream, size ) )
        {
            clearForTrack();
            while ( true )
            {
                unsigned long delta = 0;
                short read = readTimeStamp( stream, delta );
                if ( read == 0 )
                    return false;
                unsigned char value = 0;
                if ( !readOnebyte( stream, value ) )
                    return false;
                if ( value == 0xFF )    //meta event
                {
                    unsigned char meta = 0;
                    if ( !readOnebyte( stream, meta ) )
                        return false;
                    
                    unsigned char metaSize = 0;
                    if ( !readOnebyte( stream, metaSize ) )
                        return false;
                    if ( meta == MIDI_META_ENDOFTRACK )
                    {
                        return true;
                    }
                    else if ( meta == MIDI_META_TEMPO && metaSize == 3 )
                    {
                        long microseconds = 0;
                        for ( int k = 2; k >= 0; --k )
                        {
                            readOnebyte( stream, value );
                            microseconds += value * pow( 256, k ) ;
                        }
                        stampMeasure( delta );
                        handleTempo( delta, microseconds );
                        continue;
                    }
                    else if ( meta == MIDI_META_TIMESIGNATURE && metaSize == 4 )
                    {
                        readOnebyte( stream, value );
                        short upper = (short) value;
                        readOnebyte( stream, value );
                        short lower = pow( 2, value );
                        readOnebyte( stream, value );
                        readOnebyte( stream, value );
                        handleTimeSignature( delta, upper, lower );
                        continue;
                    }
                    
                    for ( unsigned char j = 0; j < metaSize; ++j )
                    {
                        readOnebyte( stream, value );
                    }
                    accumGlobal_ += delta;
                    if ( wrapper_.isVerbose() )
                        cout << "  meta-event " << (int) meta << ", size=" << (int) metaSize << endl;
                    continue;
                }
                stampMeasure( delta );
                unsigned char status = 0;
                unsigned char current = value;
                unsigned char next = 0;
                if ( value < 0x80 ) //running status
                {
                    next = value;
                    status = runningStatus_;
                }
                else
                {
                    status = ( value & 0xF0 ) >> 4;
                    if ( !readOnebyte( stream, next ) )
                        return false;
                }
                runningStatus_ = status;
                
                if ( status == MIDI_NOTE_ON || status == MIDI_NOTE_OFF )
                {
                    unsigned char velocity = 0;
                    if ( !readOnebyte( stream, velocity ) )
                        return false;
                    accumGlobal_ += handleNote( delta, next, !( status == MIDI_NOTE_OFF || velocity == 0 ) );
                }
                else if ( status == MIDI_NOTE_CONTROL_CHANGE ||
                          status == MIDI_NOTE_PROGRAM_PITCHBEND ||
                          status == MIDI_NOTE_AFTERTOUCH)
                {
                    unsigned char cc = next;
                    if ( !readOnebyte( stream, value ) )
                        return false;
                    if ( wrapper_.isVerbose() )
                        cout << "  cc: time=" << delta << " cc=" << cc << " value=" << value << endl;
                    accumGlobal_ += delta;
                }
                else if ( status == MIDI_NOTE_PROGRAM_CHANGE ||
                          status == MIDI_NOTE_PRESSURE )
                {
                    unsigned char p = next;
                    if ( wrapper_.isVerbose() )
                        cout << "  prog change/pressure: time=" << delta << " cc=" << (current&0x0F) << " p=" << p << endl;
                    accumGlobal_ += delta;
                }
            }
        }
        return true;
    }
    return false;
}

long MidiImporter::handleNote( long delta, short noteIndex, bool isOn )
{
    delta = quantify( delta );
    
    if ( wrapper_.isVerbose() )
        cout << "  note=" << noteIndex << " at delta time=" << accumGlobal_ << " on/off=" << (int)isOn << endl;
    if ( isOn )      //NOTEON
    {
        if ( midiNotes_[noteIndex] == -1 )
        {
            long forwardPoint = accumGlobal_ + delta;
            long silence = quantify( forwardPoint - lastNoteOff_ );
            int measure = currentMeasure_;
            long measureStart = currentMeasureStart_;
            long measureLength = 0;
            while ( silence > 0 && onCount_ == 0 /*&& lastNoteOn_ != -1*/ )
            {
                const Event* specs = model_.findMeasure( measure );
                if ( specs )
                {
                    measureLength = specs->duration() * quarterNoteDivision_;
                }
                else break;
                long start = lastNoteOff_;
                long length = silence;
                if ( forwardPoint - measureStart < silence )
                {
                    start = measureStart;
                    length = min( forwardPoint - measureStart, measureLength );
                }
                length = quantify( length );
                if ( length > 0 )
                    model_.addNote( measure,
                                   (float)(start - measureStart)/quarterNoteDivision_,
                                   (float)length/quarterNoteDivision_,
                                   0,
                                   MidiNote ); //creating rest
                silence -= length;
                --measure;
                measureStart -= measureLength;
                forwardPoint -= length;
            }
            setMidiNote( noteIndex, accumGlobal_ + delta );
            if ( lastNoteOn_ != noteIndex )
            {
                lastNoteOn_ = noteIndex;
            }
        }
    }
    else if ( !isOn )    //NOTEOFF
    {
        if ( midiNotes_[noteIndex] != -1 )
        {
            float noteOn = midiNotes_[noteIndex];
            long duration = accumGlobal_ + delta - noteOn;
            int measure = currentMeasure_;
            long measureStart = currentMeasureStart_;
            int sign = 1;
            while ( duration > 0 )
            {
                long start = noteOn;
                long length = duration;
                float diff = fabs( (float)(measureStart - noteOn)/quarterNoteDivision_ );
                if ( noteOn < measureStart )
                {
                    start = measureStart;
                    if ( diff > ALPHA )
                    {
                        sign = -1;
                        length = noteOn + duration - start;
                    }
                    else
                    {
                        sign = 1;
                    }
                }
                else
                {
                    sign = 1;
                }
                
                if ( (float) length/quarterNoteDivision_ > ALPHA )
                {
                    model_.addNote( measure,
                                   (float)(start - measureStart)/quarterNoteDivision_,
                                   (float)length/quarterNoteDivision_,
                                   noteIndex * 100 * sign,
                                   MidiNote ); //creating tied notes
                }
                duration -= length;
                --measure;
                const Event* specs = model_.findMeasure( measure );
                if ( specs )
                {
                    long measureLength = specs->duration() * quarterNoteDivision_;
                    measureStart -= measureLength;
                }
                else break;
            }
            setMidiNote( noteIndex, -1 );
            lastNoteOff_ = accumGlobal_ + delta;
        }
    }
    return delta;
}

long MidiImporter::quantify( long time ) const
{
    float quantified = time;
    
    for ( int i = 1; i <= 24; ++i )
    {
        if ( fabs( round( (float)time*i/quarterNoteDivision_ ) - (float) time*i/quarterNoteDivision_ ) < EPSILON*(1+i*0.2) )
        {
            quantified = (round( (float)time*i/quarterNoteDivision_ )*quarterNoteDivision_)/i;
            break;
        }
    }
    
    return (long) quantified;
}


void MidiImporter::setMidiNote( int note, long onTime )
{
    long current = midiNotes_[note];
    midiNotes_[note] = onTime;
    if ( current != onTime )
    {
        if ( onTime == -1 )
            --onCount_;
        else
            ++onCount_;
    }
}

void MidiImporter::handleTempo( long time, long microseconds )
{
    accumGlobal_ += time;
    float BPM = roundf( (float) 60000000 / microseconds );
    if ( wrapper_.isVerbose() )
        cout << "  tempo=" << BPM << " BPM at time=" << accumGlobal_ << endl;
    
    model_.insertOrReplaceEvent( new BeatPerMinute( currentMeasure_, (accumGlobal_ - currentMeasureStart_)/quarterNoteDivision_, BPM * currentMetricFactor_ ) );
    /*
    float factor = currentMetricFactor_;
    if ( currentQuarterNoteTempo_ != 0.0 && ( upper.size() == lower.size() == 1 ) && upper[0]%3 == 0 )
    {
        if ( lower[0] == 8 )
            currentMetricFactor_ = (float) 2/3 ;
        else if ( lower[0] == 16 )
            currentMetricFactor_ = (float) 4/3;
        else if ( lower[0] == 32 )
            currentMetricFactor_ = (float) 8/3;
    }
    else
        currentMetricFactor_ = 1.0;
    if ( factor != currentMetricFactor_ )
    {
        model_.insertOrReplaceEvent( new BeatPerMinute( currentMeasure_, currentQuarterNoteTempo_ * currentMetricFactor_, currentOriginalBeats_, currentOriginalBase_ ) );
    }
     */
}

void MidiImporter::handleTimeSignature( long delta, short upper, short lower )
{
    string timeSignature = to_string( upper ) + "/" + to_string( lower );
    if ( wrapper_.isVerbose() )
        cout << "  time signature=" << timeSignature.c_str() << " at time=" << accumGlobal_ << endl;
    currentTimeSignature_ = timeSignature;
    newMeasureDuration_ = upper * quarterNoteDivision_ * 4 / lower;
    if ( currentMeasureDuration_ == 0 )
        currentMeasureDuration_ = newMeasureDuration_;
    
    int m = currentMeasure_;
    if ( m > 0 && accumGlobal_ + delta >= currentMeasureStart_+currentMeasureDuration_ )
        ++m;
    Event* measure =  model_.findMeasure( m );
    if ( measure )
    {
        measure->changeDuration( newMeasureDuration_/quarterNoteDivision_ );
        ( (Measure*) measure)->setTimeSignature( timeSignature );
        currentMeasureDuration_ = newMeasureDuration_;
    }
    accumGlobal_ += delta;
    stampMeasure( 0 );
}

void MidiImporter::stampMeasure( long delta )
{
    long time = accumGlobal_ + delta;
    if ( currentMeasureDuration_ > 0 )
    {
        while ( currentMeasure_ == 0 || ( time - currentMeasureStart_ ) >= currentMeasureDuration_ )
        {
            if ( currentMeasure_ > 0 )
                currentMeasureStart_ += currentMeasureDuration_;
            ++currentMeasure_;
            const Event* measure =  model_.findMeasure( currentMeasure_ );
            if ( measure == nullptr )
            {
                model_.insertFirstEventInMeasure( new Measure( currentMeasure_,
                                                              (float) newMeasureDuration_/quarterNoteDivision_,
                                                              (float) currentMeasureStart_/quarterNoteDivision_,
                                                              1.0,
                                                              currentTimeSignature_ ) );
            }
            else
            {
                newMeasureDuration_ = currentMeasureDuration_ = measure->duration()*quarterNoteDivision_;
                currentTimeSignature_ = ((Measure*) measure)->timeSignature();
            }
        }
    }
    currentMeasureDuration_ = newMeasureDuration_;
}

void MidiImporter::clear()
{
    MidifileType_ = 0;
    tracksCount_ = 0;
    quarterNoteDivision_ = 1024;
    clearForTrack();
    currentMeasureDuration_ = 0;
    newMeasureDuration_ = 0;
    currentTimeSignature_ = "";
}

void MidiImporter::clearForTrack()
{
    runningStatus_ = 0;
    accumGlobal_ = 0;
    lastNoteOff_ = 0;
    lastNoteOn_ = -1;
    currentMeasure_ = 0;
    newMeasureDuration_ = 4*quarterNoteDivision_;
    currentMeasureDuration_ = 0;
    currentMeasureStart_ = 0;
    for ( int i = 0; i < 128; ++i )
        midiNotes_[i] = -1;
    onCount_ = 0;
}

short MidiImporter::readTimeStamp( ifstream& stream, unsigned long& time ) const
{
    short read = 0;
    unsigned char byte = 0;
    if ( readOnebyte( stream, byte ) )
    {
        time = byte;
        ++read;
        if ( time & 0x80 )
        {
            time &= 0x7F;
            do
            {
                time = (time << 7) + ( ( byte = stream.get() ) & 0x7F);
                ++read;
            }
            while ( byte & 0x80 );
        }
    }
    return read;
}

bool MidiImporter::readFourbyte( ifstream& stream, unsigned int& value ) const
{
    if ( !stream.read( (char*) &value, 4 ) )
        return false;
    char tmp = ((char*)&value)[0];
    ((char*)&value)[0] = ((char*)&value)[3];
    ((char*)&value)[3] = tmp;
    tmp = ((char*)&value)[2];
    ((char*)&value)[2] = ((char*)&value)[1];
    ((char*)&value)[1] = tmp;
    return true;
}

bool MidiImporter::readTwobyte( ifstream& stream, unsigned short& value ) const
{
    if ( !stream.read( (char*) &value, 2 ) )
        return false;
    char tmp = ((char*)&value)[0];
    ((char*)&value)[0] = ((char*)&value)[1];
    ((char*)&value)[1] = tmp;
    return true;
}

bool MidiImporter::readOnebyte( ifstream& stream, unsigned char& value ) const
{
    return ( bool ) stream.read( (char*) &value, 1 );
}

