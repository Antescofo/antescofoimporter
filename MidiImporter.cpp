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
#include "Pitch.h"
#include "ActionGroup.h"
#include "MidiNoteAction.h"
#include "MIDIDefines.h"
#include <math.h>
#include <iostream>
#include <algorithm>

using namespace antescofo;
using namespace std;

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
    lastNoteOn_             ( -1 ),
    restruckNote_           ( -1 ),
    currentMeasure_         ( 0 ),
    currentMeasureDuration_ ( 0 ),
    currentMeasureStart_    ( 0 ),
    newMeasureDuration_     ( 0 ),
    newTimeSignatureMeasure_( 0 ),
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
    for ( int t = 1; t <= tracksCount_; ++t )
    {
        if ( trackSelection && processedCount == tracks_.size() )
            break;
        if ( !trackSelection || ( tracks_.size() && find( tracks_.begin(), tracks_.end(), t ) != tracks_.end() ) )
        {
            cout << endl << "    processing track " << t << "...";
            if ( !processTrack( rawContent ) )
            {
                cout << endl << "  MIDI error processing track " << t << " ;-(" << endl;
                return false;
            }
            ++processedCount;
        }
        else if ( tracks_.size() && find( tracks_.begin(), tracks_.end(), -t ) != tracks_.end() )
        {
            cout << endl << "    processing playback track " << t << "...";
            if ( !processTrackForActionGroup( rawContent, -t ) )
            {
                cout << endl << "  MIDI error processing playback track " << -t << " ;-(" << endl;
                return false;
            }
        }
        else    //processing the track without updating the model
        {
            string dummy;
            if ( !processTrackForName( rawContent, dummy) )
            {
                cout << endl << "  MIDI error skipping track " << t << " ;-(" << endl;
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
        if ( !processTrackForName( rawContent, name ) )
        {
            cout << "  MIDI error processing track " << t << " ;-(" << endl;
            return false;
        }
        tracks.push_back( name );
        if ( wrapper_.isVerbose() )
        {
            cout << "    ...track #" << t << endl;
        }
    }
    return true;
}

bool MidiImporter::queryScoreInfo()
{
    clear();
    ifstream rawContent;
    rawContent.open( wrapper_.getInputPath(), ifstream::in );
    if ( rawContent.fail() )
        return false;
    return processHeader( rawContent );
}

bool MidiImporter::processTrackForName( ifstream& stream, string& name )
{
    bool success = false;
    bool soundFound = false;
    bool hasTimeSignature = false;
    bool hasTempo = false;
    bool hasNotes = false;
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
                short read = readVariableLengthQuantity( stream, delta );
                if ( read == 0 )
                    break;
                unsigned char value = 0;
                if ( !readOnebyte( stream, value ) )
                    break;
                if ( value == 0xFF )    //meta event
                {
                    unsigned char meta = 0;
                    if ( !readOnebyte( stream, meta ) )
                        break;
                    if ( meta == MIDI_META_ENDOFTRACK )
                    {
                        readOnebyte( stream, value );
                        success = true;
                        break;
                    }
                    unsigned long metaSize = 0;
                    if ( readVariableLengthQuantity( stream, metaSize ) == 0 )
                        return false;
                    if ( meta == MIDI_META_TRACKNAME && !name.length() )
                    {
                        char tmp [2];
                        tmp[0] = tmp[1] = 0;
                        for ( unsigned long c = 0; c < metaSize; ++c )
                        {
                            readOnebyte( stream, value );
                            tmp[0] = (char) value;
                            name += tmp;
                        }
                        continue;
                    }
                    else if ( meta == MIDI_META_TEMPO && metaSize == 3 )
                    {
                        hasTempo = true;
                    }
                    else if ( meta == MIDI_META_TIMESIGNATURE && metaSize == 4 )
                    {
                        hasTimeSignature = true;
                    }
                    
                    for ( unsigned long j = 0; j < metaSize; ++j )
                    {
                        readOnebyte( stream, value );
                    }
                    continue;
                }
                unsigned char status = 0;
                unsigned char next = 0;
                unsigned char current = value;
                if ( value < 0x80 ) //running status
                {
                    next = value;
                    status = runningStatus_;
                }
                else
                {
                    status = ( value & 0xF0 ) >> 4;
                    if ( !readOnebyte( stream, next ) )
                        break;
                }
                runningStatus_ = status;
                
                if ( status == MIDI_NOTE_ON || status == MIDI_NOTE_OFF )
                {
                    unsigned char velocity = 0;
                    if ( !readOnebyte( stream, velocity ) )
                        break;
                    hasNotes = true;
                }
                else if ( status == MIDI_NOTE_CONTROL_CHANGE ||
                         status == MIDI_NOTE_PROGRAM_PITCHBEND ||
                         status == MIDI_NOTE_AFTERTOUCH)
                {
                    if ( !readOnebyte( stream, value ) )
                        break;
                }
                else if ( !soundFound && status == MIDI_NOTE_PROGRAM_CHANGE )
                {
                    int p = (int) next;
                    int channel = (int) (current&0x0F);
                    //cout << "  (::queryTracks) prog change: p=" << (int) p << " (" << GMSoundSet[p] << ")" << " cc=" << channel << endl;
                    name += " [GM sound: ";
                    name += GMSoundSet[p];
                    name += ", channel: ";
                    name += to_string( channel );
                    name += "]";
                    soundFound = true;
                }
            }
        }
        //return true;
    }
    if ( success && !hasNotes )
    {
        if ( name.length() )
            name += " ";
        name += "[";
        if ( !hasTempo && !hasTimeSignature )
            name += "display only...";
        if ( hasTempo )
        {
            name += "tempos";
        }
        if ( hasTimeSignature )
        {
            if ( hasTempo )
                name += ", ";
            name += "time signatures";
        }
        name += "]";
    }
    return success;
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
                if ( readVariableLengthQuantity( stream, delta ) == 0 )
                    return false;
                unsigned char value = 0;
                if ( !readOnebyte( stream, value ) )
                    return false;
                if ( value == 0xFF )    //meta event
                {
                    unsigned char meta = 0;
                    if ( !readOnebyte( stream, meta ) )
                        return false;
                    
                    unsigned long metaSize = 0;
                    if ( readVariableLengthQuantity( stream, metaSize ) == 0 )
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
                    
                    for ( unsigned long j = 0; j < metaSize; ++j )
                    {
                        readOnebyte( stream, value );
                    }
                    accumGlobal_ += delta;
                    if ( wrapper_.isVerbose() )
                        cout << "  meta-event " << (int) meta << ", size=" << metaSize << endl;
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
                    handleNote( delta, next, !( status == MIDI_NOTE_OFF || velocity == 0 ) );
                    accumGlobal_ += delta;
                }
                else if ( status == MIDI_NOTE_CONTROL_CHANGE ||
                          status == MIDI_NOTE_PROGRAM_PITCHBEND ||
                          status == MIDI_NOTE_AFTERTOUCH)
                {
                    if ( !readOnebyte( stream, value ) )
                        return false;
                    //unsigned char cc = next;
                    //if ( wrapper_.isVerbose() )
                    //    cout << "  cc: time=" << delta << " cc=" << cc << " value=" << value << endl;
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

bool MidiImporter::processTrackForActionGroup( ifstream& stream, int trackIndex )
{
    bool success = false;
    int GMsound = -1;
    unsigned int fourBytes = 0;
    string label = "playback_track_" + to_string( abs( trackIndex ) );
    ActionGroup* actionGroup = new ActionGroup( label );
    MidiNoteAction noteAction;
    if (  readFourbyte( stream, fourBytes ) && fourBytes == MIDI_TRACK )
    {
        unsigned int size = 0;
        if ( readFourbyte( stream, size ) )
        {
            while ( true )
            {
                unsigned long delta = 0;
                short read = readVariableLengthQuantity( stream, delta );
                if ( read == 0 )
                    break;
                unsigned char value = 0;
                if ( !readOnebyte( stream, value ) )
                    break;
                if ( value == 0xFF )    //meta event
                {
                    unsigned char meta = 0;
                    if ( !readOnebyte( stream, meta ) )
                        break;
                    if ( meta == MIDI_META_ENDOFTRACK )
                    {
                        readOnebyte( stream, value );
                        success = true;
                        break;
                    }
                    unsigned long metaSize = 0;
                    if ( readVariableLengthQuantity( stream, metaSize ) == 0 )
                        return false;
                    for ( unsigned long j = 0; j < metaSize; ++j )
                    {
                        readOnebyte( stream, value );
                    }
                    continue;
                }
                unsigned char status = 0;
                unsigned char next = 0;
                unsigned char current = value;
                if ( value < 0x80 ) //running status
                {
                    next = value;
                    status = runningStatus_;
                }
                else
                {
                    status = ( value & 0xF0 ) >> 4;
                    if ( !readOnebyte( stream, next ) )
                        break;
                }
                runningStatus_ = status;
                
                if ( status == MIDI_NOTE_ON || status == MIDI_NOTE_OFF )
                {
                    unsigned char velocity = 0;
                    if ( !readOnebyte( stream, velocity ) )
                        break;
                    int channel = (int) (current&0x0F);
                    noteAction.clear();
                    noteAction.channel_ = channel;
                    noteAction.velocity_ = velocity;
                    noteAction.delay_ = (float) delta/quarterNoteDivision_;
                    noteAction.note_ = (int) next;
                    actionGroup->appendMidiNote( noteAction );
                }
                else if ( status == MIDI_NOTE_CONTROL_CHANGE ||
                         status == MIDI_NOTE_PROGRAM_PITCHBEND ||
                         status == MIDI_NOTE_AFTERTOUCH)
                {
                    if ( !readOnebyte( stream, value ) )
                        break;
                }
                else if ( GMsound == -1 && status == MIDI_NOTE_PROGRAM_CHANGE )
                {
                    GMsound = (int) next;
                }
            }
        }
    }
    if ( success )
    {
        actionGroup->setGMpatch( GMsound );
        model_.insertFirstEventInMeasure( actionGroup );
    }
    return success;
}

long MidiImporter::handleNote( long delta, short noteIndex, bool isOn )
{
    long unquantified = delta;
    delta = quantify( delta );
    
    //if ( wrapper_.isVerbose() )
    //    cout << "  note=" << noteIndex << " at delta time=" << accumGlobal_ << " on/off=" << (int)isOn << endl;
    if ( isOn )      //NOTEON
    {
        if ( midiNotes_[noteIndex] == -1 )
        {
            long forwardPoint = accumGlobal_ + delta;
            long silence = forwardPoint - lastNoteOff_;
            int measure = currentMeasure_;
            long measureStart = currentMeasureStart_;
            long measureLength = 0;
            Pitch rest ( 0, MidiNote);
            while ( silence > 0 && onCount_ == 0 )
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
                {
                    rest.setFeatures( MidiNote );
                    model_.addNote( measure,
                                    (float)(start - measureStart)/quarterNoteDivision_,
                                    (float)length/quarterNoteDivision_,
                                    rest ); //creating rest
                }
                silence -= length;
                --measure;
                measureStart -= measureLength;
                forwardPoint -= length;
            }
            setMidiNote( noteIndex, accumGlobal_ + unquantified );
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
            long duration = accumGlobal_ + unquantified - noteOn;
            duration = quantify( duration );
            int measure = currentMeasure_;
            long measureStart = currentMeasureStart_;
            Pitch note ( 0, MidiNote );
            while ( duration > 0 )
            {
                long start = noteOn;
                long length = duration;
                float diff = fabs( (float)(measureStart - noteOn)/quarterNoteDivision_ );
                EntryFeatures features = MidiNote;
                if ( noteOn < measureStart )
                {
                    start = measureStart;
                    if ( diff > EPSILON_MIDI )
                    {
                        features |= Tiedbackwards;
                        length = noteOn + duration - start;
                    }
                }
                
                if ( (float) length/quarterNoteDivision_ > EPSILON_MIDI )
                {
                    note.setMidiCents( noteIndex * 100 );
                    note.setFeatures( features );
                    if ( length > 0 )
                        model_.addNote( measure,
                                        (float)(start - measureStart)/quarterNoteDivision_,
                                        (float)length/quarterNoteDivision_,
                                        note ); //creating tied notes
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
            lastNoteOff_ = accumGlobal_ + unquantified;
        }
    }
    return unquantified;
    //return delta;
}

long MidiImporter::quantify( long time ) const
{
    long base2 = quarterNoteDivision_/16;
    long base3 = quarterNoteDivision_/6;
    long t2 = round( (float) time/base2)*base2;
    long t3 = round( (float) time/base3)*base3;
    if ( t2 != time || t3 != time )
    {
        if ( abs( t2 - time) < abs( t3 - time))
            time = t2;
        else
            time = t3;
    }
    return time;
    /*
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
    */
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
    //if ( wrapper_.isVerbose() )
    //    cout << "  tempo=" << BPM << " BPM at time=" << accumGlobal_ << endl;
    
    model_.insertOrReplaceEvent( new BeatPerMinute( currentMeasure_, (accumGlobal_ - currentMeasureStart_)/quarterNoteDivision_, BPM * currentMetricFactor_ ) );
}

void MidiImporter::handleTimeSignature( long delta, short upper, short lower )
{
    string timeSignature = to_string( upper ) + "/" + to_string( lower );
    if ( wrapper_.isVerbose() )
        cout << "  time signature=" << timeSignature.c_str() << " at time=" << accumGlobal_ << endl;
    newTimeSignature_ = timeSignature;
    newMeasureDuration_ = upper * quarterNoteDivision_ * 4 / lower;
    if ( currentMeasureDuration_ == 0 )
        currentMeasureDuration_ = newMeasureDuration_;
    
    int m = currentMeasure_;
    if ( currentMeasureDuration_ > 0 )
        m += ((accumGlobal_+ delta) - currentMeasureStart_)/currentMeasureDuration_;
    if ( m == 0 )
        ++m;
    Event* measure =  model_.findMeasure( m );
    newTimeSignatureMeasure_ = m;
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
                long measureDuration = currentMeasureDuration_;
                string timeSignature = currentTimeSignature_;
                if ( currentMeasure_ >= newTimeSignatureMeasure_ )
                {
                    measureDuration = newMeasureDuration_;
                    timeSignature = newTimeSignature_;
                }
                model_.insertFirstEventInMeasure( new Measure( ( float ) currentMeasure_,
                                                              (float) measureDuration/quarterNoteDivision_,
                                                              (float) currentMeasureStart_/quarterNoteDivision_,
                                                              1.0,
                                                              timeSignature ) );
            }
            else
            {
                newMeasureDuration_ = currentMeasureDuration_ = measure->duration()*quarterNoteDivision_;
                currentTimeSignature_ = ((Measure*) measure)->timeSignature();
            }
        }
    }
    currentMeasureDuration_ = newMeasureDuration_;
    currentTimeSignature_ = newTimeSignature_;
}

void MidiImporter::clear()
{
    MidifileType_ = 0;
    tracksCount_ = 0;
    quarterNoteDivision_ = 1024;
    clearForTrack();
    currentMeasureDuration_ = 0;
    newMeasureDuration_ = 0;
    newTimeSignatureMeasure_ = 0;
    newTimeSignatureMeasure_ = 0;
    currentTimeSignature_ = "";
    newTimeSignature_ = "";
}

void MidiImporter::clearForTrack()
{
    runningStatus_ = 0;
    accumGlobal_ = 0;
    lastNoteOff_ = 0;
    lastNoteOn_ = -1;
    restruckNote_ = -1;
    currentMeasure_ = 0;
    newMeasureDuration_ = 4*quarterNoteDivision_;
    newTimeSignatureMeasure_ = 0;
    currentMeasureDuration_ = 0;
    currentMeasureStart_ = 0;
    for ( int i = 0; i < 128; ++i )
        midiNotes_[i] = -1;
    onCount_ = 0;
}

short MidiImporter::readVariableLengthQuantity( ifstream& stream, unsigned long& time ) const
{
    short readCount = 0;
    unsigned char byte = 0;
    if ( readOnebyte( stream, byte ) )
    {
        time = byte;
        ++readCount;
        if ( time & 0x80 )
        {
            time &= 0x7F;
            do
            {
                time = (time << 7) + ( ( byte = stream.get() ) & 0x7F);
                ++readCount;
            }
            while ( byte & 0x80 );
        }
    }
    return readCount;
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

