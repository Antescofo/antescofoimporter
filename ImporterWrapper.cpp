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
//  Copyright (c) 2017 Antescofo. All rights reserved.
//

#include "ImporterWrapper.h"
#include "ImportModel.h"
#include "MusicXmlImporter.h"
#include "MidiImporter.h"
#include "UnitTester.h"
#include <sys/stat.h>
#include <iostream>

using namespace antescofo;
using namespace std;

ImporterWrapper::ImporterWrapper() :
    model_                  ( new ImportModel( *this ) ),
    verbose_                ( false ),
    trackListQuery_         ( false ),
    quarterNoteTempo_       ( false ),
    displayMidiCents_       ( false ),
    treat_3_8_as_compound_  ( false ),
    originalAccidentals_    ( false ),
    displayMetadata_        ( false ),
    displayQueriesOnly_     ( false ),
    improveXml_             ( false ),
    chaseCues_              ( false ),
    smartGraceNotes_        ( false )
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


std::string ImporterWrapper::getVersion()
{
    return "version 0.2.3";
}

bool ImporterWrapper::parseArguments( vector<string>& args )
{
    string filePath;
    string unitaryTestFolderPath;
    string tracks;
    
    for ( int i = 0; i < args.size(); ++i )
    {
        if ( args[i].substr( 0, 10 ) == "-unittest=" )
        {
            unitaryTestFolderPath = args[i].substr( 10 );
            cout << "  Unit tests..." << endl;
        }
        else if ( args[i].substr( 0, 17 ) ==  "-outputdirectory=" )
        {
            outputDirectory_ = args[i].substr( 17 );
            cout << "  ✔︎ Output directory set to " << outputDirectory_ << endl;
        }
        else if ( args[i].substr( 0, 12 ) ==  "-outputfile=" )
        {
            outputFilePath_ = args[i].substr( 12 );
            cout << "  ✔︎ Output file path set to " << outputFilePath_ << endl;
        }
        else if ( args[i] == "-midicents" )
        {
            displayMidiCents_ = true;
            cout << "  ✔︎ MIDI cents representation" << endl;
        }
        else if ( args[i] == "-querytracks" )
        {
            trackListQuery_ = true;
            cout << "  ✔︎ Tracks query" << endl;
        }
        else if ( args[i].substr( 0, 8 ) == "-tracks=" )
        {
            trackSelection_ = args[i].substr( 8 );
            cout << "  ✔︎ Track selection" << endl;
        }
        else if (( args[i] == "-quarternotetime" ) || ( args[i] == "-quarternotetimes" ))
        {
            quarterNoteTempo_ = true;
            cout << "  ✔︎ Quarter note division mode" << endl;
        }
        else if ( args[i] == "-originalpitches" )
        {
            originalAccidentals_ = true;
            cout << "  ✔︎ Original pitches display" << endl;
        }
        else if ( args[i] == "-3/8compoundtime" )
        {
            treat_3_8_as_compound_ = true;
            cout << "  ✔︎ Treat 3/8 time as compound" << endl;
        }
        else if ( args[i] == "-verbose" || args[i] == "-v" )
        {
            verbose_ = true;
        }
        else if ( args[i] == "-metadata" )
        {
            displayMetadata_ = true;
            cout << "  ✔︎ Add metadata in score" << endl;
        }
        else if ( args[i] == "-querymetadata" )
        {
            displayMetadata_ = displayQueriesOnly_ = true;
            cout << "  ✔︎ Replace metadata in score" << endl;
        }
        else if ( args[i] == "-improvexml" )
        {
            improveXml_ = true;
        }
        else if ( args[i] == "-chasecues" )
        {
            chaseCues_ = true;
        }
        else if ( args[i] == "-smartgracenotes" )
        {
            smartGraceNotes_ = true;
        }
        else
        {
            struct stat st;
            if ( stat( args[i].c_str(), &st ) == 0 )   //check if this is an existing file
                filePath = args[i];
        }
    }
    if ( filePath.length() == 0 && unitaryTestFolderPath.length() == 0 )
        return false;
    
    if ( improveXml_ )
        cout << "  ✔︎ Improve source music xml files" << endl;
    if ( chaseCues_ )
        cout << "  ✔︎ Chasing cues" << endl;
    if ( smartGraceNotes_ )
        cout << "  ✔︎ Smart grace notes" << endl;
    
    if ( filePath.length() > 0 )
        setInputPath( filePath );
    return true;
}

void ImporterWrapper::setVerbose( bool status )
{
    verbose_ = status;
}

bool ImporterWrapper::isVerbose() const
{
    return verbose_;
}

void ImporterWrapper::setInputPath( const string& path )
{
    inputPath_ = path;
}

void ImporterWrapper::setOutputDirectory( const string& directory )
{
    outputDirectory_ = directory;
    if ( outputDirectory_.length() &&  outputDirectory_.at( outputDirectory_.length() - 1 ) != '/' )
        outputDirectory_ += "/";
}

void  ImporterWrapper::setOutputFile( const std::string& output )
{
    outputFilePath_ = output;
}

const string& ImporterWrapper::getInputPath() const
{
    return inputPath_;
}

bool ImporterWrapper::inputIsMIDI() const
{
    return inputPath_.find( ".xml") == string::npos &&
           inputPath_.find( ".musicxml") == string::npos &&
           inputPath_.find( ".mxl") == string::npos;
}

ImportModel& ImporterWrapper::getModel()
{
    return *model_;
}

void ImporterWrapper::addImporter( Importer* importer )
{
    importers_.push_back( importer );
}

bool ImporterWrapper::import()
{
    if ( inputPath_.length() == 0 )
        return false;
    model_->clear();
    for ( vector<Importer*>::iterator it = importers_.begin(); it != importers_.end(); ++it)
    {
        if ( (*it)->import() )
        {
            return true;
        }
    }
    cout << "  Import error: no suitable importer was found to import " << inputPath_ << endl;
    return false;
}

bool ImporterWrapper::import( const string& path )
{
    struct stat st;
    if ( stat( path.c_str(), &st ) != 0 )
    {
        cout << "  Error: file '" << path << "' could not be found!" << endl;
        return false;
    }
    setInputPath( path );
    return import();
}

bool ImporterWrapper::import( const vector<int>& tracks )
{
    if ( tracks.size() == 0 )
        return import();
    model_->clear();
    for ( vector<Importer*>::iterator it = importers_.begin(); it != importers_.end(); ++it)
    {
        if ( (*it)->import( tracks ) )
        {
            return true;
        }
    }
    cout << "  Import error: no suitable importer was found to import " << inputPath_ << endl;
    return false;
}

bool ImporterWrapper::import( const string& path, const vector<int>& tracks )
{
    struct stat st;
    if ( stat( path.c_str(), &st ) != 0 )
    {
        cout << "  Error: file '" << path << "' could not be found!" << endl;
        return false;
    }
    setInputPath( path );
    return import( tracks );
}

bool ImporterWrapper::queryTracklist( vector<string>& tracks )
{
    if ( inputPath_.length() == 0 )
        return false;
    struct stat st;
    if ( stat( inputPath_.c_str(), &st ) != 0 )
    {
        cout << "  Error when querying tracks: file '" << inputPath_ << "' could not be found!" << endl;
        return false;
    }
    model_->clear();
    for ( vector<Importer*>::iterator it = importers_.begin(); it != importers_.end(); ++it)
    {
        if ( (*it)->queryTracks( tracks ) )
        {
            return true;
        }
    }
    cout << "  Tracks query error: no suitable importer could answer the query!" << inputPath_ << endl;
    return false;
}

bool ImporterWrapper::queryTracklist( const string& path, vector<string>& tracks )
{
    setInputPath( path );
    return queryTracklist( tracks );
}

bool ImporterWrapper::queryScoreInfo( string& scoreInfo )
{
    if ( inputPath_.length() == 0 )
        return false;
    struct stat st;
    if ( stat( inputPath_.c_str(), &st ) != 0 )
    {
        cout << "  Error when querying score info: file '" << inputPath_ << "' could not be found!" << endl;
        return false;
    }
    model_->clear();
    for ( vector<Importer*>::iterator it = importers_.begin(); it != importers_.end(); ++it)
    {
        if ( (*it)->queryScoreInfo() )
        {
            scoreInfo = model_->displayScoreInfo();
            return true;
        }
    }
    cout << "  Score info query error: no suitable importer could retrieve the info!" << inputPath_ << endl;
    return false;
}

bool ImporterWrapper::save( const string& path )
{
    return model_->save( path );
}

bool ImporterWrapper::save()
{
    string output;
    if ( outputFilePath_.length() )
    {
        output = outputFilePath_;
    }
    else
    {
        output = inputPath_;
        output = output.substr( 0, output.find_last_of( "." ) ) + ".asco.txt";
        if ( outputDirectory_.length() > 0 )
        {
            output = output.substr( output.find_last_of( "/" ) + 1 );
        }
    }
    outputFilePath_ = output;
    return model_->save( output );
}

float ImporterWrapper::getInitialTempo() const
{
    //TODO!
    return 120.;
}

ostringstream& ImporterWrapper::getCurrentSerialization()
{
    return model_->getSerialization();
}

void ImporterWrapper::setPitchesAsMidiCents( bool status )
{
    displayMidiCents_ = status;
}

bool ImporterWrapper::pitchesAsMidiCents() const
{
    return displayMidiCents_;
}

bool ImporterWrapper::runUnitaryTests( const string& unitaryTestFolder )
{
    UnitTester* tester = new UnitTester( *this, unitaryTestFolder );
    bool result = tester->run();
    delete tester;
    return result;
}

void ImporterWrapper::clear()
{
    verbose_ = false;
    trackListQuery_ = false;
    model_->clear();
    inputPath_.clear();
    outputFilePath_.clear();
    trackSelection_.clear();
    for ( vector<Importer*>::iterator it = importers_.begin(); it != importers_.end(); ++it)
    {
        (*it)->clear();
    }
}

void ImporterWrapper::setQuarterNoteTempo( bool status )
{
    quarterNoteTempo_ = status;
}

bool ImporterWrapper::hasQuarterNoteTempo() const
{
    return quarterNoteTempo_;
}

void ImporterWrapper::setOriginalPitches( bool status )
{
    originalAccidentals_ = status;
}

bool ImporterWrapper::hasOriginalPitches() const
{
    return originalAccidentals_;
}

void ImporterWrapper::set3_8_compound( bool status )
{
    treat_3_8_as_compound_ = status;
}

bool ImporterWrapper::is3_8_compound() const
{
    return treat_3_8_as_compound_;
}

bool ImporterWrapper::displayMetadata() const
{
    return displayMetadata_;
}

bool ImporterWrapper::displayQueriesOnly() const
{
    return displayQueriesOnly_;
}

