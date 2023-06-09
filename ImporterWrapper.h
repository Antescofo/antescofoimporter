//
// .-. . . .-. .-. .-. .-. .-. .-. .-.
// |-| |\|  |  |-  `-. |   | | |-  | |
// ` ' ' `  '  `-' `-' `-' `-' '   `-'
//
// .-. .  . .-. .-. .-. .-. .-. .-.
//  |  |\/| |-' | | |(   |  |-  |(
// `-' '  ` '   `-' ' '  '  `-' ' '
//
//  Importer.h
//
//  Created by Robert Piéchaud on 29/04/15.
//  Copyright (c) 2017 Antescofo. All rights reserved.
//

#ifndef _ANTESCOFO_IMPORTER_WRAPPER_
#define _ANTESCOFO_IMPORTER_WRAPPER_

#include <stdio.h>
#include <string>
#include <vector>

namespace antescofo
{
    class ImportModel;
    class Importer;
    
    class ImporterWrapper
    {
    public:
        ImporterWrapper();
        virtual ~ImporterWrapper();
        
        bool  parseArguments( std::vector<std::string>& args );
        bool  import();
        bool  import( const std::vector<int>& tracks );
        bool  import( const std::string& path );
        bool  import( const std::string& path, const std::vector<int>& tracks );
        bool  save( const std::string& path );
        bool  save();
        void  setInputPath( const std::string& path );
        std::string outputPath() { return outputFilePath_; }
        void  setOutputDirectory( const std::string& directory );
        void  setOutputFile( const std::string& output );
        std::string outputDirectory() { return outputDirectory_; }
        bool  runUnitaryTests( const std::string& unitaryTestFolder );
        bool  trackListQuery() const { return trackListQuery_; }
        std::string rawTrackSelection() { return trackSelection_; }
        bool  queryTracklist( const std::string& path, std::vector<std::string>& tracks );
        bool  queryTracklist( std::vector<std::string>& tracks );
        bool  queryScoreInfo( std::string& scoreInfo );
        /// Staves that should be imported (empty means all staves should be).
        std::vector<int> staffList() const { return staffSelection_; }
        /// Parse an array of staves out of a string.
        std::vector<int> parseStaffList( std::string staffSelection ) const;
        float getInitialTempo() const;
        void  clear();
        bool  inputIsMIDI() const;
        
        static std::string getVersion();
        
        std::ostringstream& getCurrentSerialization();
        
        const std::string&  getInputPath() const;
        ImportModel&        getModel();
        void                setVerbose( bool status );
        bool                isVerbose() const;
        void                setPitchesAsMidiCents( bool status );
        bool                pitchesAsMidiCents() const;
        void                setQuarterNoteTempo( bool status );
        bool                hasQuarterNoteTempo() const;
        void                setOriginalPitches( bool status );
        bool                hasOriginalPitches() const;
        void                set3_8_compound( bool status );
        bool                is3_8_compound() const;
        bool                displayMetadata() const;
        bool                displayQueriesOnly() const;
        void                setImproveXml( bool status ) { improveXml_ = status; }
        bool                improveXml() const { return improveXml_; }
        void                setChaseCues( bool status ) { chaseCues_ = status; }
        bool                chaseCues() const { return chaseCues_; }
        void                setSmartGraceNotes( bool status ) { smartGraceNotes_ = status; }
        bool                smartGraceNotes() const { return smartGraceNotes_; }
        bool                appoggiaturas() const { return (appoggiaturas_ || smartGraceNotes()); }
        void                setAppoggiaturas( bool status ) { appoggiaturas_ = status; }
    private:
        void addImporter( Importer* importer );
        
    private:
        std::vector<Importer*>  importers_;
        ImportModel*            model_;
        bool                    verbose_;
        bool                    trackListQuery_;
        std::string             trackSelection_;
        std::vector<int>        staffSelection_;
        std::string             inputPath_;
        std::string             outputFilePath_;
        std::string             outputDirectory_;
        bool                    displayMidiCents_;
        bool                    quarterNoteTempo_;
        bool                    treat_3_8_as_compound_;
        bool                    originalAccidentals_;
        bool                    displayMetadata_;
        bool                    displayQueriesOnly_;
        bool                    improveXml_;
        bool                    chaseCues_;
        bool                    smartGraceNotes_;
        bool                    appoggiaturas_;
    };
}

#endif //_ANTESCOFO_IMPORTER_WRAPPER_
