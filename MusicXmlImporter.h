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
        
        virtual bool import();
        virtual bool import( const std::vector<int>& tracks );
        virtual void clear();
        virtual bool queryTracks( std::vector<std::string>& tracks );
        
    private:
        bool  openDocument( TiXmlDocument& musicXML );
        void  processMeasure( TiXmlNode* node );
        void  processDirection( TiXmlNode* node );
        void  processTempo( TiXmlNode* node );
        float processNote( TiXmlNode* node );
        float processTimeSignature( TiXmlNode* node, std::string& timeSignature );
        int   getMidiCents( const char diatonic, int octave, float accidental ) const;
        
    private:
        ImporterWrapper& wrapper_;
        ImportModel&     model_;
        std::vector<int> tracks_;
        bool             compressedXML_;
        int              currentMeasure_;
        std::string      currentTimeSignature_;
        int              currentKeyAccidentals_; // -2 -> 2 flats, +3 -> 3 sharps etc.
        int              currentVoice_;
        int              currentTrillVoice_;
        EntryFeatures    currentNoteFeatures_;
        float            currentMeasureDuration_;
        float            accumGlobal_;
        float            accumLocal_;
        int              currentDivision_;
        int              currentRepeatNoteAmount_;
        float            currentQuarterNoteTempo_;
        float            currentMetricFactor_;
        float            currentOriginalBeats_;
        float            currentOriginalBase_;
        float            previousDuration_;     //a safeguard for Sibelius xml export bug...
    };
}

#endif // _ANTESCOFO_MUSICXML_IMPORTER_
