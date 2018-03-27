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
//  Copyright (c) 2017 Antescofo. All rights reserved.
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
        
        static rational getBeatDurationFromNoteType(const char*);
        
        class Utils {
        public:
            static int const STEP_NUMBER = 7;
            
            static int stepDistance(char const step1, char const step2) {
                int const unfoldedDistance = abs(step1 - step2);
                return (unfoldedDistance+1 == STEP_NUMBER ? 1 : unfoldedDistance);
            }
            
            static bool oneDegreeBetween(char const step1, char const step2) {
                return (stepDistance(step1, step2) == 1);
            }
            
            static std::string trim(std::string const& s);
            
            static std::string clean(std::string const& s);
        };
        
        struct Dictionary {
            static std::vector<std::string> const aTempo;
            static std::vector<std::string> const tempoPrimo;
            static std::vector<std::string> const waitForNote;
        };
        
        struct Default {
            static int const DIVISIONS = 1;
        };
        
    private:
        bool  retrieveScoreInfo( TiXmlNode* node );
        bool  openDocument( TiXmlDocument& musicXML );
        void  processMeasure( TiXmlNode* measure );
        TiXmlNode* processMeasureAttributes( TiXmlNode* measure );
        void  processDirection( TiXmlNode* node );
        void  appendCurrentTempo(bool generated = false);
        void  appendTempoPrimo();
        bool  processTempo( TiXmlNode* node );
        float processNote( TiXmlNode* node );
        bool  chaseCues( TiXmlNode* measure );
        void  beautifyGraceNotes( TiXmlNode* part, bool beautifyGroups = false );
        bool  isGraceAppoggiatura( TiXmlNode* graceNote, TiXmlNode* isAppoggiatura) const;
        void  handleSingleGraceNote( TiXmlNode* graceNote, TiXmlNode* note );
        void  handleGraceNoteGroup( std::vector<TiXmlNode*>& group, TiXmlNode* entryBefore, TiXmlNode* entryAfter );
        void  handleSingleGraceNoteBeforeTrill( TiXmlNode* graceNote, TiXmlNode* note, int const divisions);
        //TODO: implement this function
        //void  handleGraceNoteGroupTrill( std::vector<TiXmlNode*>& group, TiXmlNode* entryBefore, TiXmlNode* entryAfter );
        float processTimeSignature( TiXmlNode* node, std::string& timeSignature );
        int   getMidiCents( const char diatonic, int octave, float accidental ) const;
        float typeToDuration( const char* type ) const;
        void  improveXml( TiXmlDocument& musicXML );
        
        void  addWaitForNote();        
        
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
