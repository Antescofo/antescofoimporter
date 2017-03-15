//
// .-. . . .-. .-. .-. .-. .-. .-. .-.
// |-| |\|  |  |-  `-. |   | | |-  | |
// ` ' ' `  '  `-' `-' `-' `-' '   `-'
//
// .-. .  . .-. .-. .-. .-. .-. .-.
//  |  |\/| |-' | | |(   |  |-  |(
// `-' '  ` '   `-' ' '  '  `-' ' '
//
//  MusicXmlImporter.h
//
//  Created by Robert Piéchaud on 09/05/15.
//  Copyright (c) 2015 ircam. All rights reserved.
//

#ifndef _ANTESCOFO_MIDI_IMPORTER_
#define _ANTESCOFO_MIDI_IMPORTER_

#include "Importer.h"
#include <fstream>

namespace antescofo
{
    class ImporterWrapper;
    class ImportModel;
    
    class MidiImporter: public Importer
    {
    public:
        MidiImporter( ImporterWrapper& wrapper );
        virtual ~MidiImporter();
        
        virtual bool import();
        virtual bool import( const std::vector<int>& tracks );
        virtual void clear();
        virtual bool queryTracks( std::vector<std::string>& tracks );
        
    private:
        void  clearForTrack();
        bool  processHeader( std::ifstream& stream );
        bool  processTrack( std::ifstream& stream );
        bool  processTrackForName( std::ifstream& stream, std::string& name );
        bool  readFourbyte( std::ifstream& stream, unsigned int& value) const;
        bool  readTwobyte( std::ifstream& stream, unsigned short& value ) const;
        bool  readOnebyte( std::ifstream& stream, unsigned char& value ) const;
        short readTimeStamp( std::ifstream& stream, unsigned long& time ) const;
        
        long  handleNote( long time, short midiNote, bool isOn );
        void  handleTempo( long time, long microseconds );
        void  handleTimeSignature( long time, short upper, short lower );
        void  stampMeasure( long delta );
        
        long  quantify( long time ) const;
        void  setMidiNote( int note, long onTime );
        
    private:
        ImporterWrapper&    wrapper_;
        ImportModel&        model_;
        std::vector<int>    tracks_;
        long                midiNotes_ [128];
        int                 onCount_;
        unsigned short      MidifileType_;
        unsigned short      tracksCount_;
        unsigned short      quarterNoteDivision_;
        unsigned char       runningStatus_;
        long                accumGlobal_;
        long                lastNoteOff_;
        long                lastNoteOn_;
        std::string         currentTimeSignature_;
        int                 currentMeasure_;
        long                currentMeasureDuration_;
        long                currentMeasureStart_;
        long                newMeasureDuration_;
        float               currentQuarterNoteTempo_;
        float               currentMetricFactor_;
    };
}

#endif // _ANTESCOFO_MIDI_IMPORTER_
