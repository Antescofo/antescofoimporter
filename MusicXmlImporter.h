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

#ifndef _ANTESCOFO_MUSICXML_IMPORTER_
#define _ANTESCOFO_MUSICXML_IMPORTER_

#include "Importer.h"
#include "Event.h"
#include <string>

class TiXmlNode;
class TiXmlDocument;

namespace antescofo
{
    class ImporterWrapper;
    class ImportModel;
    
    class MusicXmlImporter : public Importer
    {
    public:
        MusicXmlImporter( ImporterWrapper& wrapper );
        virtual ~MusicXmlImporter();
        
        bool import() override;
        bool import( const std::vector<int>& tracks ) override;
        void clear() override;
        bool queryTracks( std::vector<std::string>& tracks ) override;
        bool queryScoreInfo() override;
        
    private:
        bool  retrieveScoreInfo( TiXmlNode* node );
        bool  openDocument( TiXmlDocument& musicXML );
        void  processMeasure( TiXmlNode* node );
        void  processDirection( TiXmlNode* node );
        bool  processTempo( TiXmlNode* node );
        float processNote( TiXmlNode* node );
        float processTimeSignature( TiXmlNode* node, std::string& timeSignature );
        int   getMidiCents( const char diatonic, int octave, float accidental ) const;
        
    private:
        ImporterWrapper& wrapper_;
        ImportModel&     model_;
        std::vector<int> tracks_;
        bool             compressedXML_;
        float            currentMeasure_;       //can be fractional: 0.5 means pickup to meas 1, 14.5 is pickup to meas 15 etc.
        std::string      currentTimeSignature_;
        int              currentKeyAccidentals_; // -2 -> 2 flats, +3 -> 3 sharps etc.
        int              currentVoice_;
        int              currentTrillVoice_;
        EntryFeatures    currentNoteFeatures_;
        float            currentMeasureDuration_;
        float            accumGlobal_;
        float            accumLocal_;
        int              currentChromaticTransposition_;
        int              currentDiatonicTransposition_;
        int              currentDivision_;
        int              currentRepeatNoteAmount_;
        float            currentQuarterNoteTempo_;
        float            currentMetricFactor_;
        float            currentIntMetricFactor_;
        float            currentOriginalBeats_;
        float            currentOriginalBase_;
        float            previousDuration_;     //a safeguard for Sibelius xml export bug...
    };
}

#endif // _ANTESCOFO_MUSICXML_IMPORTER_
