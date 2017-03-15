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
        
        bool import() override;
        bool import( const std::vector<int>& tracks ) override;
        void clear() override;
        bool queryTracks( std::vector<std::string>& tracks ) override;
        bool queryScoreInfo() override;
        
    private:
        void  clearForTrack();
        bool  processHeader( std::ifstream& stream );
        bool  processTrack( std::ifstream& stream );
        bool  processTrackForActionGroup( std::ifstream& stream, int trackIndex );
        bool  processTrackForName( std::ifstream& stream, std::string& name );
        bool  readFourbyte( std::ifstream& stream, unsigned int& value) const;
        bool  readTwobyte( std::ifstream& stream, unsigned short& value ) const;
        bool  readOnebyte( std::ifstream& stream, unsigned char& value ) const;
        short readVariableLengthQuantity( std::ifstream& stream, unsigned long& quantity ) const;
        
        long  handleNote( long time, short midiNote, bool isOn );
        void  handleTempo( long time, long microseconds );
        void  handleTimeSignature( long time, short upper, short lower );
        void  stampMeasure( long delta );
        
        long  quantify( long time ) const;
        void  setMidiNote( int note, long onTime );
        
    private:
        ImporterWrapper&    wrapper_;
        ImportModel&        model_;
        std::vector<int>    tracks_;        //tracks are 0-based (should change this to take action groups (negative tracks) into account
        long                midiNotes_ [128];
        int                 onCount_;
        unsigned short      MidifileType_;
        unsigned short      tracksCount_;
        unsigned short      quarterNoteDivision_;
        unsigned char       runningStatus_;
        long                accumGlobal_;
        long                lastNoteOff_;
        long                lastNoteOn_;
        long                restruckNote_;
        std::string         currentTimeSignature_;
        std::string         newTimeSignature_;
        int                 currentMeasure_;
        long                currentMeasureDuration_;
        long                currentMeasureStart_;
        long                newMeasureDuration_;
        long                newTimeSignatureMeasure_;
        float               currentQuarterNoteTempo_;
        float               currentMetricFactor_;
    };
}

#endif // _ANTESCOFO_MIDI_IMPORTER_
