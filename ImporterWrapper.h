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
//  Copyright (c) 2015 ircam. All rights reserved.
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
        
        bool import( const std::string& path );
        bool import( const std::string& path, const std::vector<int>& tracks );
        bool save( const std::string& path ) const;
        bool save() const;
        bool runUnitaryTests( const std::string& unitaryTestFolder );
        void setPitchesAsMidiCents( bool status );
        bool queryTracks( const std::string& path, std::vector<std::string>& tracks );
        
        std::ostringstream& getCurrentSerialization();
        
        const std::string&  getInputPath() const;
        ImportModel&        getModel();
        void                setVerbose( bool status );
        bool                isVerbose() const;
        
    private:
        void setInputPath( const std::string& path );
        void addImporter( Importer* importer );
        
    private:
        std::vector<Importer*>  importers_;
        ImportModel*            model_;
        bool                    verbose_;
        std::string             inputPath_;
    };
}

#endif //_ANTESCOFO_IMPORTER_WRAPPER_
