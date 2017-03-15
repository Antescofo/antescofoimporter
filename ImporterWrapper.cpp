//
// .-. . . .-. .-. .-. .-. .-. .-. .-.
// |-| |\|  |  |-  `-. |   | | |-  | |
// ` ' ' `  '  `-' `-' `-' `-' '   `-'
//
// .-. .  . .-. .-. .-. .-. .-. .-.
//  |  |\/| |-' | | |(   |  |-  |(
// `-' '  ` '   `-' ' '  '  `-' ' '
//
//  Converter.cpp
//
//  Created by Robert Piéchaud on 29/04/15.
//  Copyright (c) 2015 ircam. All rights reserved.
//

#include "ImporterWrapper.h"
#include "ImportModel.h"
#include "MusicXmlImporter.h"
#include "MidiImporter.h"
#include "UnitTester.h"
#include <sys/stat.h>
#include <iostream>

using namespace antescofo;

ImporterWrapper::ImporterWrapper() :
    model_  ( new ImportModel( *this ) ),
    verbose_( false )
{
    addImporter( new MusicXmlImporter( *this ) );
    addImporter( new MidiImporter( *this ) );
}

ImporterWrapper::~ImporterWrapper()
{
    auto it = importers_.begin();
    while ( it != importers_.end() )
        it = importers_.erase( it );
    delete model_;
}


void ImporterWrapper::setVerbose( bool status )
{
    verbose_ = status;
}

bool ImporterWrapper::isVerbose() const
{
    return verbose_;
}

void ImporterWrapper::setInputPath( const std::string& path )
{
    inputPath_ = path;
}

const std::string& ImporterWrapper::getInputPath() const
{
    return inputPath_;
}

ImportModel& ImporterWrapper::getModel()
{
    return *model_;
}

void ImporterWrapper::addImporter( Importer* importer )
{
    importers_.push_back( importer );
}

bool ImporterWrapper::import( const std::string& path )
{
    struct stat st;
    if ( stat( path.c_str(), &st ) != 0 )
    {
        std::cout << "  Error: file '" << path << "' could not be found!" << std::endl;
        return false;
    }
    setInputPath( path );
    model_->clear();
    for ( std::vector<Importer*>::iterator it = importers_.begin(); it != importers_.end(); ++it)
    {
        if ( (*it)->import() )
        {
            return true;
        }
    }
    std::cout << "  Import error: no suitable importer was found to import " << inputPath_ << std::endl;
    return false;
}


bool ImporterWrapper::import( const std::string& path, const std::vector<int>& tracks )
{
    struct stat st;
    if ( stat( path.c_str(), &st ) != 0 )
    {
        std::cout << "  Error: file '" << path << "' could not be found!" << std::endl;
        return false;
    }
    setInputPath( path );
    model_->clear();
    for ( std::vector<Importer*>::iterator it = importers_.begin(); it != importers_.end(); ++it)
    {
        if ( (*it)->import( tracks ) )
        {
            return true;
        }
    }
    std::cout << "  Import error: no suitable importer was found to import " << inputPath_ << std::endl;
    return false;
}

bool ImporterWrapper::queryTracks( const std::string& path, std::vector<std::string>& tracks )
{
    struct stat st;
    if ( stat( path.c_str(), &st ) != 0 )
    {
        std::cout << "  Error when querying tracks: file '" << path << "' could not be found!" << std::endl;
        return false;
    }
    setInputPath( path );
    model_->clear();
    for ( std::vector<Importer*>::iterator it = importers_.begin(); it != importers_.end(); ++it)
    {
        if ( (*it)->queryTracks( tracks ) )
        {
            return true;
        }
    }
    std::cout << "  Tracks query error: no suitable importer could answer the query!" << inputPath_ << std::endl;
    return false;
}

bool ImporterWrapper::save( const std::string& path ) const
{
    return model_->save( path );
}

bool ImporterWrapper::save() const
{
    return false;
    //TODO
}

std::ostringstream& ImporterWrapper::getCurrentSerialization()
{
    return model_->getSerialization();
}

void ImporterWrapper::setPitchesAsMidiCents( bool status )
{
    model_->setPitchesAsMidiCents( status );
}

bool ImporterWrapper::runUnitaryTests( const std::string& unitaryTestFolder )
{
    UnitTester* tester = new UnitTester( *this, unitaryTestFolder );
    bool result = tester->run();
    delete tester;
    return result;
}
