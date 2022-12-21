//
// .-. . . .-. .-. .-. .-. .-. .-. .-.
// |-| |\|  |  |-  `-. |   | | |-  | |
// ` ' ' `  '  `-' `-' `-' `-' '   `-'
//
// .-. .  . .-. .-. .-. .-. .-. .-.
//  |  |\/| |-' | | |(   |  |-  |(
// `-' '  ` '   `-' ' '  '  `-' ' '
//
//  MusicXmlImporter.cpp
//
//  Created by Robert Piéchaud on 09/05/15.
//  Copyright (c) 2017 Antescofo. All rights reserved.
//

#include "MusicXmlImporter.h"
#include "ImporterWrapper.h"
#include "ImportModel.h"
#include "tinyxml.h"
#include "BeatPerMinute.h"
#include "Nosync.h"
#include "Measure.h"
#include "Repeat.h"
#include "Pitch.h"
#include "SimpleRational.h"
#include "extract.h"
#include <math.h>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <stdio.h>
#include "ConvertUTF.h"

using namespace antescofo;
using namespace std;

struct MusicXMLaccidental { const char* s; float a;};
static const MusicXMLaccidental MusicXMLaccidentals [] =
{
    { "sharp", 1. },
    { "natural", 0. },
    { "flat", -1. },
    { "double-sharp", 2. },
    { "sharp-sharp", 2. },
    { "flat-flat", -2. },
    { "natural-sharp", 1. },
    { "natural-flat", -1. },
    { "quarter-flat", -0.5 },
    { "quarter-sharp", 0.5 },
    { "three-quarters-flat", -1.5 },
    { "three-quarters-sharp", -1.5 },
    { "sharp-down", 0.8 },
    { "sharp-up", 1.2 },
    { "natural-down", -0.2 },
    { "natural-up", 0.2 },
    { "flat-down", -1.2 },
    { "flat-up", -0.8 },
    { "triple-sharp", 3. },
    { "triple-flat", -3. },
    { "slash-quarter-sharp", 0.3 },
    { "slash-sharp", 0.3 },
    { "slash-flat", -0.3 },
    { "double-slash-flat", -0.3 },
    { "sharp-1", 1. },
    { "sharp-2", 1. },
    { "sharp-3", 1. },
    { "sharp-5", 1. },
    { "flat-1", -1. },
    { "flat-2", -1. },
    { "flat-3", -1. },
    { "flat-4", -1. },
    { "sori", 0.5 },
    { "koron", -0.5 },
    { nullptr, 0.}
};

std::vector<std::string> const MusicXmlImporter::Dictionary::aTempo = {
    "a tempo"
    , "a  tempo"                    // typo: with two blank spaces
    , "a tempo."                    // typo: with a final dot
    , "sm: reset tempo"             // SmartMusic marker that resets tempo
};

std::vector<std::string> const MusicXmlImporter::Dictionary::tempoPrimo = {
    "tempo i", "tempo i°", "tempo 1", "tempo 1°", "tempo primo",
    "tempo  i", "tempo  i°", "tempo  1", "tempo  1°", "tempo  primo",   // typo: with two blank spaces
    "tempo i.", "tempo i°.", "tempo 1.", "tempo 1°.", "tempo primo.",   // typo: with a final dot
};

std::vector<std::string> const MusicXmlImporter::Dictionary::waitForNote = {
    "sm: wait for note", "sm: resume"               // SmartMusic markers
};

std::vector<std::string> const MusicXmlImporter::Dictionary::noSyncOnNote = {
    "no sync on note"
};

std::vector<std::string> const MusicXmlImporter::Dictionary::noSyncStart = {
    "no sync start"
};

std::vector<std::string> const MusicXmlImporter::Dictionary::noSyncStop = {
    "no sync stop"
};

std::string MusicXmlImporter::Utils::trim(std::string const& s)
{
    auto wsfront=std::find_if_not(s.begin(),s.end(),[](int c){return std::isspace(c);});
    auto wsback=std::find_if_not(s.rbegin(),s.rend(),[](int c){return std::isspace(c);}).base();
    return (wsback<=wsfront ? std::string() : std::string(wsfront,wsback));
}

std::string MusicXmlImporter::Utils::clean(std::string const& s) {
    std::string content = Utils::trim(s);
    std::transform(content.begin(), content.end(), content.begin(), ::tolower);
    return content;
}

MusicXmlImporter::MusicXmlImporter( ImporterWrapper& wrapper ) :
    wrapper_                ( wrapper ),
    model_                  ( wrapper_.getModel() ),
    compressedXML_          ( false ),
    currentMeasure_         ( 0. ),
    currentKeyAccidentals_  ( 0 ),
    currentVoice_           ( 0 ),
    currentTrillVoice_      ( 0 ),
    currentNoteFeatures_    ( None ),
    currentMeasureDuration_ ( 0.0 ),
    accumGlobal_            ( 0.0 ),
    accumLocal_             ( 0.0 ),
    currentChromaticTransposition_  ( 0 ),
    currentDiatonicTransposition_   ( 0 ),
    currentDivision_        ( Default::DIVISIONS ),
    currentRepeatNoteAmount_( 0 ),
    currentMetricFactor_    ( 1.0 ),
    currentIntMetricFactor_ ( 3 ),
    currentQuarterNoteTempo_( 0.0 ),
    currentOriginalBeats_   ( 0.0 ),
    currentOriginalBase_    ( 0.0 ),
    previousDuration_       ( 0.0 )
{
    //NOTHING
}

MusicXmlImporter::~MusicXmlImporter()
{
    //NOTHING
}

void MusicXmlImporter::clear()
{
    compressedXML_ = false;
    currentMeasure_ = 0.;
    currentKeyAccidentals_ = 0;
    currentVoice_ = 0;
    currentTrillVoice_ = 0;
    currentNoteFeatures_ = None;
    currentMeasureDuration_  = 0.0;
    currentChromaticTransposition_ = 0;
    currentDiatonicTransposition_ = 0;
    currentOctaveTransposition_ = 0;
    accumGlobal_ = 0.0;
    accumLocal_ = 0.0;
    currentDivision_ = 1;
    currentRepeatNoteAmount_ = 0;
    currentMetricFactor_ = 1.0;
    currentIntMetricFactor_ = 3;
    currentQuarterNoteTempo_ = 0.0;
    currentOriginalBeats_ = 0.0;
    currentOriginalBase_ = 0.0;
    previousDuration_ = 0.0;
    currentTimeSignature_.clear();
}

bool MusicXmlImporter::openDocument( TiXmlDocument& musicXML )
{
    clear();
    bool loaded = musicXML.LoadFile( TIXML_ENCODING_UTF8 );
    compressedXML_ = false;
    if ( !loaded )  //ok... then perhaps it's UTF-16 ? (from $!*#!! Dolet plug-in) or compressed file ?  (zipped xml = mxl)
    {
        std::string content;
        string inputPath = wrapper_.getInputPath();
        std::ifstream rawContent ( inputPath.c_str(), std::ifstream::binary );
  
        rawContent.seekg ( 0, rawContent.end );
        streamoff size = rawContent.tellg();
        rawContent.seekg ( 0, rawContent.beg );
        unsigned char* buffer = new unsigned char[size];
        rawContent.read ( (char*) buffer, (streamsize) size ) ;
        rawContent.close();
        if ( size > 2 )
        {
          if ( ( buffer[0] == 0xFF && buffer[1] == 0xFE ) || ( buffer[0] == 0xFE && buffer[1] == 0xFF ) ) //UTF-16
          {
            if ( wrapper_.isVerbose() )
              cout << "  (UTF-16 detected and converted to UTF-8...) " << endl;
            if ( ConvertUTF16ToUTF8String( buffer, size, content ) )
            {
              size_t pos = content.find( "UTF-16" );
              if ( pos != std::string::npos )
                content.replace( pos, 6, "UTF-8" );
            }
            else
              content.clear();
          }
        }
        delete [] buffer;
      
        if ( content.size() == 0 )
        {
          string extension;
          size_t index = inputPath.find_last_of(".");
          if ( index > 0 )
              extension = inputPath.substr( index + 1 );
          ostringstream stream;
          if ( extension == "mxl" )
          {
              string containerPath = "META-INF/container.xml";
              if ( !extractOneFile( inputPath.c_str(), containerPath.c_str(), stream ) )
                  return false;
              content = stream.str();
              TiXmlDocument metaInfo;
              if ( !metaInfo.Parse( content.c_str(), nullptr ) )
                  return false;
              TiXmlHandle hDocument ( &metaInfo );
              TiXmlElement* container = hDocument.FirstChildElement( "container" ).Element();
              if ( !container )
                  return false;
              TiXmlNode* rootfiles = container->FirstChildElement( "rootfiles" );
              if ( !rootfiles )
                  return false;
              TiXmlNode* rootfile = rootfiles->FirstChildElement( "rootfile" );
              if ( !rootfile )
                  return false;
              if ( const char* embeddedXml = rootfile->ToElement()->Attribute( "full-path" ) )
              {
                if ( !extractOneFile( inputPath.c_str(), embeddedXml, stream ) )
                  return false;
              }
              else
                return false;
            
              compressedXML_ = true;
          }
          content = stream.str();
        }
        loaded = ( musicXML.Parse( content.c_str(), nullptr ) != nullptr );
    }
    return loaded;
}

bool MusicXmlImporter::retrieveScoreInfo( TiXmlNode* root )
{
    if ( !root )
        return false;
    if ( const char* versionAttribute = root->ToElement()->Attribute( "version" ) )
    {
        string version ( versionAttribute );
        version = "MusicXML version " + version;
        if ( compressedXML_ )
            version += " (compressed file)";
        model_.setVersion( version );
    }
    TiXmlNode* identification = root->FirstChildElement( "identification" );
    if ( identification )
    {
        TiXmlNode* encoding = identification->FirstChildElement( "encoding" );
        
        if ( encoding )
        {
            TiXmlNode* software = encoding->FirstChildElement( "software" );
            if ( software )
            {
                string content;
                while ( software )
                {
                    if ( content.size() )
                        content += " ~ ";
                    content += software->ToElement()->GetText();
                    software = encoding->IterateChildren( "software", software );
                }
                if ( content.size() )
                    content = "exported from " + content;
                model_.setFileOrigin( content );
            }
        }
    }
    
    TiXmlNode* credit = root->FirstChildElement( "credit" );
    if ( credit )
    {
        TiXmlNode* creditWords = credit->FirstChildElement( "credit-words" );
        if ( creditWords )
        {
            string content;
            while ( creditWords )
            {
                if ( content.size() )
                    content += " ~ ";
                content += creditWords->ToElement()->GetText();
                creditWords = credit->IterateChildren( "software", creditWords );
            }
            model_.setCredits( content );
        }
    }
    return true;
}

void MusicXmlImporter::improveXml( TiXmlDocument& musicXML )
{
    TiXmlHandle hDocument ( &musicXML );
    TiXmlElement* root = hDocument.FirstChildElement( "score-partwise" ).Element();
    if ( root )
    {
        TiXmlNode* part = root->FirstChildElement( "part" );
        int count = 0;
        while ( part )
        {
            TiXmlNode* measure = part->FirstChildElement( "measure" );
            TiXmlNode* itemBefore = measure->FirstChild();
            bool trillFound = false;
            while ( measure )
            {
                TiXmlNode* item = measure->FirstChild();
                while ( item )
                {
                    if ( strcmp( item->Value(), "direction") == 0 )
                    {
                        TiXmlNode* type = item->FirstChildElement( "direction-type" );
                        if ( type )
                        {
                            TiXmlNode* other = type->FirstChildElement( "other-direction" );
                            if ( other )
                            {
                                TiXmlNode* content = other->FirstChild();
                                if ( content && strncmp( content->Value(), "Tril", 4 ) == 0 )
                                {
                                    trillFound = true;
                                    ++count;
                                    measure->RemoveChild( item );
                                }
                            }
                        }
                    }
                    else if ( strcmp( item->Value(), "note") == 0 )
                    {
                        if ( trillFound )
                        {
                            TiXmlNode* notations = item->FirstChildElement( "notations" );
                            if ( !notations )
                            {
                                TiXmlElement newItem ( "notations" );
                                notations = item->InsertEndChild( newItem );
                            }
                            TiXmlNode* ornaments = notations->FirstChildElement( "ornaments" );
                            if ( !ornaments )
                            {
                                TiXmlElement newItem( "ornaments" );
                                ornaments = notations->InsertEndChild( newItem );
                            }
                            TiXmlElement trill( "trill-mark" );
                            trill.SetAttribute( "placement", "above" );
                            ornaments->InsertEndChild( trill );
                            trillFound = false;
                        }
                    }
                    if ( trillFound )
                    {
                        item = measure->IterateChildren( itemBefore );
                    }
                    else
                    {
                        itemBefore = item;
                        item = measure->IterateChildren( item );
                    }
                }
                measure = part->IterateChildren( "measure", measure );
            }
            part = root->IterateChildren( "part", part );
        }
        if ( count > 0 )
        {
            string newPath = wrapper_.getInputPath();
            newPath = newPath.substr( 0, newPath.find_last_of( "." ) ) + "_improved.xml";
            cout << "  Improving music xml file (" << count << " changes) and saving result as " << newPath << endl;
            musicXML.SaveFile( newPath );
        }
    }
}

bool MusicXmlImporter::import()
{
    TiXmlDocument musicXML ( wrapper_.getInputPath() );
    bool loaded = openDocument( musicXML );
    if ( loaded )
    {
        TiXmlHandle hDocument ( &musicXML );
        TiXmlElement* root = hDocument.FirstChildElement( "score-partwise" ).Element();
        if ( !root )
            return false;
        else
        {
            if ( wrapper_.improveXml() )
                improveXml( musicXML );
            retrieveScoreInfo( root );
            TiXmlNode* part = root->FirstChildElement( "part" );
            int count = 0;
            while ( part )
            {
                bool const
                beautifyAppoggiaturas = wrapper_.appoggiaturas(),
                beautifyGroups = wrapper_.smartGraceNotes();
        
                if (beautifyAppoggiaturas || beautifyGroups)
                    beautifyGraceNotes( part, beautifyGroups );
                if ( !tracks_.size() || ( tracks_.size() && find( tracks_.begin(), tracks_.end(), count + 1 ) != tracks_.end() ) )
                {
                    if ( wrapper_.isVerbose() )
                        cout << "    ...staff #" << count << endl;
                    currentTrillVoice_ = 0;
                    currentChromaticTransposition_ = 0;
                    currentDiatonicTransposition_ = 0;
                    currentOctaveTransposition_ = 0;
                    accumLocal_ = 0.0;
                    TiXmlNode* measure = part->FirstChildElement( "measure" );
                    bool shouldChaseCue = true;
                    int noCueChaseCount = 0;
                    while ( measure )
                    {
                        if ( noCueChaseCount > 1 )
                        {
                            shouldChaseCue = true;
                            noCueChaseCount = 0;
                        }
                        currentVoice_ = 0;
                        currentNoteFeatures_ = None;
                        previousDuration_ = 0.0;
                        if ( wrapper_.chaseCues() && shouldChaseCue )
                            shouldChaseCue = chaseCues( measure );
                        if ( !shouldChaseCue )
                            ++noCueChaseCount;
                        processMeasure( measure );
                        measure = part->IterateChildren( "measure", measure );
                    }
                    currentMeasure_ = 0.;
                }
                ++count;
                part = root->IterateChildren( "part", part );
            }
        }
        tracks_.clear();
        model_.beautify();
        return true;
    }
    tracks_.clear();
    return false;
}

bool MusicXmlImporter::import( const std::vector<int>& tracks )
{
    tracks_ = tracks;
    return import();
}

bool MusicXmlImporter::queryTracks( vector<string>& tracks )
{
    tracks.clear();
    TiXmlDocument musicXML ( wrapper_.getInputPath() );
    bool loaded = openDocument( musicXML );
    if ( loaded )
    {
        TiXmlHandle hDocument ( &musicXML );
        TiXmlElement* root = hDocument.FirstChildElement( "score-partwise" ).Element();
        if ( !root )
            return false;
        else
        {
            TiXmlNode* partList = root->FirstChildElement( "part-list" );
            if ( partList )
            {
                int count = 0;
                TiXmlNode* scorePart = partList->FirstChildElement( "score-part" );
                while ( scorePart )
                {
                    TiXmlNode* partName = scorePart->FirstChildElement( "part-name" );
                    string name;
                    if ( partName )
                    {
                        const char* content = partName->ToElement()->GetText();
                        if ( content )
                            name = content;
                    }
                    if ( name.size() <= 1 )
                    {
                        if ( name.size() == 1 )
                            name += " ";
                        name += "[staff " + to_string( count ) + "]";
                    }
                    tracks.push_back( name );
                    scorePart = partList->IterateChildren( "score-part", scorePart );
                    ++count;
                }
            }
        }
        return true;
    }
    return false;
}

bool MusicXmlImporter::queryScoreInfo()
{
    TiXmlDocument musicXML ( wrapper_.getInputPath() );
    bool good = openDocument( musicXML );
    if ( good )
    {
        TiXmlHandle hDocument ( &musicXML );
        TiXmlElement* root = hDocument.FirstChildElement( "score-partwise" ).Element();
        if ( !root )
            return false;
        else
        {
            retrieveScoreInfo( root );
        }
    }
    return good;
}

TiXmlNode* MusicXmlImporter::processMeasureAttributes( TiXmlNode* measure )
{
    TiXmlNode* attributes = measure->FirstChildElement( "attributes" );
    if ( attributes )
    {
        TiXmlNode* divisions = attributes->FirstChildElement( "divisions" );
        if ( divisions )
            currentDivision_ = atoi( divisions->ToElement()->GetText() );
        TiXmlNode* transpose = attributes->FirstChildElement( "transpose" );
        if ( transpose )
        {
            TiXmlNode* chromatic = transpose->FirstChildElement( "chromatic" );
            if ( chromatic )
                currentChromaticTransposition_ = atoi( chromatic->ToElement()->GetText() );
            TiXmlNode* diatonic = transpose->FirstChildElement( "diatonic" );
            if ( diatonic )
                currentDiatonicTransposition_ = atoi( chromatic->ToElement()->GetText() );
            if ( TiXmlNode* octaveChange = transpose->FirstChildElement( "octave-change" ) )
                currentOctaveTransposition_ = atoi( octaveChange->ToElement()->GetText() );
        }
    }
    return attributes;
}

void MusicXmlImporter::processMeasure( TiXmlNode* measure )
{
    float measNumber = 0;
    float previousMeasureBeats = model_.getMeasureDuration( currentMeasure_ );
    measure->ToElement()->QueryValueAttribute( "number", &measNumber );
    if ( measNumber == 0 )
        currentMeasure_ += 0.5; //convention for pickup measure
    else
        currentMeasure_ = measNumber;
    TiXmlNode* attributes = processMeasureAttributes( measure );
    TiXmlNode* barline = measure->FirstChildElement( "barline" );
    while ( barline )
    {
        TiXmlNode* repeat = barline->FirstChildElement( "repeat" );
        if ( repeat )
        {
            const char* attr = repeat->ToElement()->Attribute( "direction" );
            if ( attr )
            {
                std::string direction = attr;
                model_.insertOrReplaceEvent( new Repeat( currentMeasure_, direction == "forward"? 1:-1, 0 ) );
            }
        }
        TiXmlNode* ending = barline->FirstChildElement( "ending" );
        if ( ending )
        {
            const char* attr = ending->ToElement()->Attribute( "number" );
            if ( attr )
            {
                int number = atoi( attr );
                model_.insertOrReplaceEvent( new Repeat( currentMeasure_, 0, number ) );
            }
        }
        barline = measure->IterateChildren( "barline", barline );
    }
    const Event* specs = model_.findMeasure( currentMeasure_ );
    if ( specs == nullptr )
    {
        currentVoice_ = 0;
        currentNoteFeatures_ = None;
        previousDuration_ = 0.0;
        string signature;
        if ( attributes )
        {
            TiXmlNode* time = attributes->FirstChildElement( "time" );
            if ( time )
            {
                currentMeasureDuration_ = processTimeSignature( time, signature );
            }
            TiXmlNode* key = attributes->FirstChildElement( "key" );
            if ( key )
            {
                TiXmlNode* fifths = key->FirstChildElement( "fifths" );
                if ( fifths )
                    currentKeyAccidentals_ = atoi( fifths->ToElement()->GetText() );
            }
        }
        
        if ( signature.size() > 0 )
        {
            currentTimeSignature_ = signature;
        }
        accumGlobal_ += previousMeasureBeats; // * currentMetricFactor_; //currentMeasureDuration_
        model_.insertFirstEventInMeasure( new Measure(  currentMeasure_,
                                                      currentMeasureDuration_,
                                                      accumGlobal_,
                                                      currentMetricFactor_,
                                                      currentKeyAccidentals_,
                                                      currentTimeSignature_ ) );
        
        /* // Error! directions should not be parsed at the beginning of the measure!
         TiXmlNode* direction = measure->FirstChildElement( "direction" );
        while ( direction )
        {
            processDirection( direction );
            direction = measure->IterateChildren( "direction", direction );
        }*/
    }
    else
    {
        currentTimeSignature_ = ((Measure*) specs)->timeSignature();
        currentMetricFactor_ = ((Measure*) specs)->metricFactor();
        currentKeyAccidentals_ = ((Measure*) specs)->keyAccidentals();
        currentMeasureDuration_ = specs->duration();
    }
    accumLocal_ = 0.0;
    TiXmlNode* item = measure->FirstChild();
    bool hasNotes = false;
    while ( item )
    {
        if ( !strcmp( item->Value(), "note") || !strcmp( item->Value(), "forward") || !strcmp( item->Value(), "backup") )
        {
            accumLocal_ += processNote( item );
            hasNotes = true;
        }
        // process directions (e.g. tempo change)
        else if (!strcmp( item->Value(), "direction")) {
            processDirection( item );
        }
        item = measure->IterateChildren( item );
    }
    if ( !hasNotes ) //This to take Sibelius' dialect for multimeasure rests into account...
    {
        Pitch measureRest ( 0, MeasureRest );
        model_.addNote( currentMeasure_, accumLocal_, currentMeasureDuration_*currentMetricFactor_, measureRest );
    }
}

float MusicXmlImporter::typeToDuration( const char* type ) const
{
    if ( type == nullptr )
        return 0.;
    //efficiently parsing music xml "type" visual rhythmic values:
    if (strncmp( type, "q", 1) == 0 )
        return 1.;
    else if (strncmp( type, "e", 1) == 0 )
        return 1./2.;
    else if (strncmp( type, "16", 2) == 0 )
        return 1./4.;
    else if (strncmp( type, "3", 1) == 0 )
        return 1./8.;
    else if (strncmp( type, "h", 1) == 0 )
        return 2.;
    else if (strncmp( type, "w", 1) == 0 )
        return 4.;
    else if (strncmp( type, "6", 1) == 0 )
        return 1./16.;
    else if (strncmp( type, "5", 1) == 0 )
        return 1./128.;
    else if (strncmp( type, "2", 1) == 0 )
        return 1./64.;
    else if (strncmp( type, "12", 2) == 0 )
        return 1./32.;
    else if (strncmp( type, "b", 1) == 0 )
        return 8.;
    else if (strncmp( type, "l", 1) == 0 )
        return 16.;
    else if (strncmp( type, "m", 1) == 0 )
        return 32.;
    else if (strncmp( type, "10", 2) == 0 )
        return 1./256.;
    return 1.;
}

bool MusicXmlImporter::chaseCues( TiXmlNode* measure )
{
    // This algorithm needs a bit of explanation. In order to make a difference between real cues and grace, ornamental grace notes, there are a few assumptions:
    // 1) graces ornamental notes are generally of regular values, general >= 8th notes (ex: a series of 16th little notes)
    // 2) graces ornamental notes are *inside* the music (i.e. a passage of cue notes that immediately follows some normal notes is ornamental).
    // 3) cue notes have mixed rhythms
    // So in one word, the magic happens by observing the mean (E) and standard deviation (S) of cue note groups.
    bool hasCue = false;
    TiXmlNode* cueCheck = measure->FirstChild();
    int cueCount = 0;
    int noteCount = 0;
    vector<float> cueDurations;
    vector<TiXmlNode*> cueNotes;
    while ( cueCheck )
    {
        if ( cueCheck->ValueStr() == "backup" && cueDurations.size() )
            break;
        else if ( cueCheck->ValueStr() == "note" && cueCheck->FirstChildElement( "rest" ) == nullptr )
        {
            bool isCue = false;
            TiXmlNode* type = cueCheck->FirstChildElement( "type" );
            if ( type )
            {
                const char* size = type->ToElement()->Attribute( "size" );
                if ( size && strcmp( size, "cue" ) == 0 )
                    isCue = true;
            }
            if ( !isCue && cueCheck->FirstChildElement( "cue" ) && cueCheck->FirstChildElement( "lyric" ) == nullptr )
                isCue = true;
            if ( isCue && type )
            {
                
                TiXmlNode* content = type->FirstChild();
                if ( content )
                {
                    float duration = typeToDuration( content->Value() );
                    cueDurations.push_back( duration );
                    cueNotes.push_back(cueCheck);
                }
                ++cueCount;
            }
            else
            {
                ++noteCount;
                break;
            }
        }
        cueCheck = measure->IterateChildren( cueCheck );
    }
    if ( cueDurations.size() )
    {
        //mean:
        float E = 0;
        for ( auto x = cueDurations.begin(); x != cueDurations.end(); ++x )
            E += *x;
        E /= cueDurations.size();
        
        //standard deviation:
        float S = 0;
        for ( auto x = cueDurations.begin(); x != cueDurations.end(); ++x )
            S += (E - *x)*(E - *x);
        S /= cueDurations.size();
        S = sqrtf( S );
#ifdef DEBUG
        cout << "meas. " << currentMeasure_ + 1 << ": E = " << E << "  S = " << S << endl;
#endif
        //TODO: perhaps a less simplistic discriminant f(E,S)?..
        if ( E > 0.25 || S > 0.15 )
        {
            for ( auto it = cueNotes.begin(); it != cueNotes.end(); ++it )
            {
                (*it)->ToElement()->SetAttribute( "real-cue", "1" );
            }
            hasCue = true;
#ifdef DEBUG
            cout << "  => ignoring " << cueNotes.size() << " notes found in meas. " << currentMeasure_ + 1 << endl;
#endif
        }
        else
            measure->ToElement()->SetAttribute( "real-notes", "1" );
    }
    else if ( noteCount > 0 )
        measure->ToElement()->SetAttribute( "real-notes", "1" );
    return noteCount == 0;
}

void MusicXmlImporter::beautifyGraceNotes(TiXmlNode* part, bool beautifyGroups)
{
    int currentDivision_ = 1;
    TiXmlNode* measure = part->FirstChildElement( "measure" );
    TiXmlNode* entryBefore = nullptr;
    vector<TiXmlNode*> graceNotes;
    while ( measure )
    {
        // flush grace notes before the next measure
        graceNotes.clear();
        
        int currentMeasure = 0;
        processMeasureAttributes( measure );
        measure->ToElement()->QueryValueAttribute( "number", &currentMeasure );
        TiXmlNode* item = measure->FirstChild();
        bool firstEntry = true;
        while ( item )
        {
            if ( item->ValueStr() == "divisions" )
            {
                currentDivision_ = atoi( item->ToElement()->GetText() );
            }
            else if ( item->ValueStr() == "backup" )
            {
                //TODO: multiple layers?..
                //;
            }
            else if ( item->ValueStr() == "note" && item->FirstChildElement( "rest" ) == nullptr )
            {
                TiXmlNode* grace = item->FirstChildElement( "grace" );
                if ( grace )
                {
                    if ( firstEntry && graceNotes.size() )  //grace note(s) in previous measure followed by another grace note
                    {
#ifdef DEBUG
                        cout << "  Group of " << graceNotes.size() << " grace notes found in m." << currentMeasure << endl;
#endif
                        if (beautifyGroups) {
                            handleGraceNoteGroup( graceNotes, entryBefore, item );
                        }
                        graceNotes.clear();
                    }
                    graceNotes.push_back( item );
                }
                else
                {
                    if ( !graceNotes.empty() )
                    {
                        bool isTrill = false;
                        if ( TiXmlNode* notations = item->FirstChildElement( "notations" ) )
                        {
                            if ( TiXmlNode* ornaments = notations->FirstChildElement( "ornaments" ) )
                            {
                                if ( TiXmlNode* trill = ornaments->FirstChildElement( "trill-mark" ) )
                                {
                                    isTrill = true;
                                }
                            }
                        }
                        if ( graceNotes.size() == 1 )
                        {
#ifdef DEBUG
                            cout << "  Single grace note found in m." << currentMeasure;
#endif
                            if (isTrill)
                                handleSingleGraceNoteBeforeTrill(graceNotes[0], item, currentDivision_);
                            else
                                handleSingleGraceNote( graceNotes[0], item );
                        }
                        else if (beautifyGroups)
                        {
#ifdef DEBUG
                            cout << "  Group of " << graceNotes.size() << " grace notes found in m." << currentMeasure << endl;
#endif
                            //TODO: manage trills differently
                            if (!isTrill)
                                handleGraceNoteGroup( graceNotes, entryBefore, item );
                        }
                        graceNotes.clear();
                    }
                    entryBefore = item;
                    entryBefore->ToElement()->SetAttribute( "measure", currentMeasure );
                }
                firstEntry = false;
            }
            item = measure->IterateChildren( item );
        }
        measure = part->IterateChildren( "measure", measure );
    }
}

void MusicXmlImporter::handleGraceNoteGroup( vector<TiXmlNode*>& group, TiXmlNode* entryBefore, TiXmlNode* entryAfter )
{
    TiXmlNode* entry = entryAfter;
    if ( entryBefore )
        entry = entryBefore;
    TiXmlNode* duration = entry->FirstChildElement( "duration" );
    if ( duration )
    {
        char buffer[8];
        float value = atof( duration->ToElement()->GetText() );
        float realValue = (float) currentMetricFactor_ * value / currentDivision_;
        float duraBefore = value;
        float duraGrace = 0;
        if ( (1./8.)*(group.size()+1) < realValue )
        {
            duraGrace = (1./8.)*currentDivision_/currentMetricFactor_;
            duraBefore = value - duraGrace*group.size();
        }
        else
        {
            duraGrace = value / ( group.size() + 1);
            duraBefore = duraGrace;
        }
        sprintf( buffer, "%d", (int) duraBefore );
        duration->ToElement()->FirstChild()->SetValue( buffer );
        sprintf( buffer, "%d", (int) duraGrace );
        
        for ( auto graceNote = group.begin(); graceNote != group.end(); ++graceNote )
        {
            TiXmlElement newItem ( "duration" );
            duration = (*graceNote)->InsertEndChild( newItem );
            duration->InsertEndChild( TiXmlText( buffer ) );
            (*graceNote)->InsertEndChild( *duration );
            TiXmlNode* graceNode = (*graceNote)->FirstChildElement( "grace" );
            (*graceNote)->RemoveChild( graceNode );
        }
    }
}


/*!
 */
bool MusicXmlImporter::isGraceAppoggiatura( TiXmlNode* graceNote, TiXmlNode* note) const {
    bool isAppoggiatura = false;
    
    // exception case: do nothing if note has no duration
    TiXmlNode* duration = note->FirstChildElement( "duration" );
    if ( !duration ) {
        return false;
    }
    
    // Music notation: an appoggiatura cannot be slashed
    TiXmlNode const* graceNode = graceNote->FirstChildElement( "grace" );
    const char* slash = graceNode->ToElement()->Attribute( "slash" );
    if ( slash && !strcmp( slash, "yes" ) ) {
        return false;
    }

    // Musicology: an appoggiatura is always a degree above or below its principal note
    isAppoggiatura = false;
    // Retrieve base note step
    // Retrieve appoggiatura step
    TiXmlNode const* pitch1 = note->FirstChildElement( "pitch" );
    TiXmlNode const* pitch2 = graceNote->FirstChildElement( "pitch" );
        if (pitch1 && pitch2) {
            // Check step
            TiXmlNode const* step1 = pitch1->FirstChildElement( "step" );
            TiXmlNode const* step2 = pitch2->FirstChildElement( "step" );
            if (step1 && step2) {
                char const diatonic1 = step1->ToElement()->GetText()[0];
                char const diatonic2 = step2->ToElement()->GetText()[0];
                isAppoggiatura = Utils::oneDegreeBetween(diatonic1, diatonic2);
                if (isAppoggiatura) {
                    // Check octave
                    TiXmlNode const* octaveNode1 = pitch1->FirstChildElement( "octave" );
                    TiXmlNode const* octaveNode2 = pitch2->FirstChildElement( "octave" );
                    if (octaveNode1 && octaveNode2) {
                        int octave1 = 0, octave2 = 0;
                        octave1 = atoi( octaveNode1->ToElement()->GetText() );
                        octave2 = atoi( octaveNode2->ToElement()->GetText() );
                        isAppoggiatura = (octave1 == octave2)
                        || ((octave2 == octave1 +1) && (diatonic2 == 'C'))  // e.g. B4 to C5
                        || ((octave1 == octave2 +1) && (diatonic1 == 'C'));
                    }
                }
            }
        }
    
    return isAppoggiatura;
}


/*!
 * \brief   Process "human playback" of single grace note before a non-trill single note.
 *
 * Detect if a single note is a an appoggiatura, and if yes, replace it by a "real" note.
 *
 *  \see  https://en.wikisource.org/wiki/A_Dictionary_of_Music_and_Musicians/Appoggiatura for musicological rules about appoggiaturas (aka. long appoggiaturas) and acciaccaturas (aka. short appoggiaturas)
 */
void MusicXmlImporter::handleSingleGraceNote( TiXmlNode* graceNote, TiXmlNode* note )
{
    // infere if graceNote is an appoggiatura or an acciaccatura
    bool isAppoggiatura = this->isGraceAppoggiatura( graceNote, note);
    
    // retrieve 'duration' node of principal note
    TiXmlNode* duration = note->FirstChildElement( "duration" );
    if ( !duration ) {
        isAppoggiatura = false;
    }
    
    // retrieve 'grace' node of grace note
    TiXmlNode* graceNode = graceNote->FirstChildElement( "grace" );
    if ( !graceNode ) {
        isAppoggiatura = false;
    }
    
    if ( isAppoggiatura )
    {
        // Retrieve full duration
        int fullDuration = 0;
        fullDuration = atoi( duration->ToElement()->GetText() );
        
        // Compute grace note duration
        // Rule 1. 'Whenever it is possible to divide the principal note into two equal parts, the appoggiatura receives one half' (Ex. 5 in ref.)
        int graceDuration = fullDuration / 2;
        //TODO: Rule 2. 'When the principal note is dotted the appoggiatura receives two-thirds and the principal note one' (Ex. 6)
        //TODO: Rule 3.If the principal note is tied to another shorter note, the appogiatura receives the whole value of the principal note' (Ex. 7)
        
        // Compute principal note duration
        int noteDuration = fullDuration - graceDuration;

        // Alter duration of principal note
        char noteDurationStr[16]; sprintf( noteDurationStr, "%d", noteDuration );
        duration->ToElement()->FirstChild()->SetValue( noteDurationStr );
        // Add duration of grace note
        char graceDurationStr[16]; sprintf( graceDurationStr, "%d", graceDuration );
        TiXmlElement newItem( "duration" );
        duration = graceNote->InsertEndChild( newItem );
        duration->InsertEndChild( TiXmlText( graceDurationStr ) );
        graceNote->InsertEndChild( *duration );
        // Convert grace note into real note
        graceNote->RemoveChild( graceNode );
#ifdef DEBUG
        cout << "  => real appoggiatura (adjusted)" << endl;
#endif
    }
    else
    {
#ifdef DEBUG
        cout << "  => accacciatura (nothing done)" << endl;
#endif
    }
}


/*!
 * \brief   Process "human playback" of single grace note before a single trill note.
 *
 * Detect if a single note is a an appoggiatura, and if yes, replace it by a "real" note.
 */
//TODO: finish this function: infer graceValueDuration correctly
void MusicXmlImporter::handleSingleGraceNoteBeforeTrill( TiXmlNode* graceNote, TiXmlNode* note, int const divisions )
{
    // infere if graceNote is an appoggiatura or an acciaccatura
    bool isAppoggiatura = this->isGraceAppoggiatura( graceNote, note);
    // retrieve 'duration' node of principal note
    TiXmlNode* duration = note->FirstChildElement( "duration" );
    if ( !duration ) {
        isAppoggiatura = false;
    }
    /*
    // retrieve 'grace' node of grace note
    TiXmlNode* graceNode = graceNote->FirstChildElement( "grace" );
    if ( !graceNode ) {
        isAppoggiatura = false;
    }
    
    if ( isAppoggiatura )
    {
        // Retrieve full duration
        int fullDuration = 0;
        fullDuration = atoi( duration->ToElement()->GetText() );
        
        // Compute grace note duration corresponding to its value
        int graceValueDuration = fullDuration / 2 ; //TODO: compute it from MusicXML elements: value, time-modification, dot, etc.
        
        // Compute duration of 'converted' grace note
        // Rule: Give to the grace note its full value duration if possible, else half-duration of principal note
        int graceDuration = (graceValueDuration < fullDuration ?  graceValueDuration : fullDuration / 2 );
        
        // Compute principal note duration
        int noteDuration = fullDuration - graceDuration;
        
        // Alter duration of principal note
        char noteDurationStr[16]; sprintf( noteDurationStr, "%d", noteDuration );
        duration->ToElement()->FirstChild()->SetValue( noteDurationStr );
        // Add duration of grace note
        char graceDurationStr[16]; sprintf( graceDurationStr, "%d", graceDuration );
        TiXmlElement newItem( "duration" );
        duration = graceNote->InsertEndChild( newItem );
        duration->InsertEndChild( TiXmlText( graceDurationStr ) );
        graceNote->InsertEndChild( *duration );
        // Convert grace note into real note
        graceNote->RemoveChild( graceNode );
#ifdef DEBUG
        cout << "  => real appoggiatura (adjusted)" << endl;
#endif
    }
    else
    {
        // - if single slashed note is on a neighbor degree, then it determines the trill prefix
        //TODO: if on the SAME or one degree ABOVE, then simply remove it! Indeed, those degree are already in the trill pitches
        //TODO: if one degree
#ifdef DEBUG
        cout << "  => accacciatura (nothing done)" << endl;
#endif
    }*/
}

float MusicXmlImporter::processTimeSignature( TiXmlNode* time, string& timeSignature )
{
    vector<int> upper;
    vector<int> lower;
    float duration = 0.0;
    TiXmlNode* beats = time->FirstChildElement( "beats" );
    while ( beats )
    {
        upper.push_back( atoi( beats->ToElement()->GetText() ) );
        beats = time->IterateChildren( "beats", beats );
    }
    TiXmlNode* beatType = time->FirstChildElement( "beat-type" );
    while ( beatType )
    {
        lower.push_back( atoi( beatType->ToElement()->GetText() ) );
        beatType = time->IterateChildren( "beat-type", beatType );
    }
    timeSignature.clear();
    if ( upper.size() == lower.size() )
    {
        for ( int i = 0; i < upper.size(); ++i )
        {
            if ( i > 0 )
                timeSignature += "+";
            timeSignature += to_string(upper[i]) + "/" + to_string(lower[i]);
            duration += (float) upper[i] * 4 / lower[i];
        }
    }
    float factor = currentMetricFactor_;
    currentMetricFactor_ = 1.0;
    currentIntMetricFactor_ = 3;
    if ( !wrapper_.hasQuarterNoteTempo() && upper.size() == lower.size() == 1 && ( ( upper[0] == 3 && wrapper_.is3_8_compound() ) || upper[0] > 3 ) && lower[0] >= 8 )
    {
        if ( upper[0]%3 == 0 )
        {
            if ( lower[0] == 8 )
            {
                currentMetricFactor_ = (float) 2/3 ;
                currentIntMetricFactor_ = 2;
            }
            else if ( lower[0] == 16 )
            {
                currentMetricFactor_ = (float) 4/3;
                currentIntMetricFactor_ = 4;
            }
            else if ( lower[0] == 32 )
            {
                currentMetricFactor_ = (float) 8/3;
                currentIntMetricFactor_ = 3;
            }
        }
    }
    if ( currentQuarterNoteTempo_ != 0.0 && factor != currentMetricFactor_ )
    {
        appendCurrentTempo(true);
    }
    if ( duration == 0.0 )
        duration = currentMeasureDuration_;
    return duration;
}

/*!
 Imported elements:
 - 'metronome' elements
 - 'sound' elements
 - SmartMusic markers: see Dictionary::waitForNote
 */
void MusicXmlImporter::processDirection( TiXmlNode* direction )
{
    //// A. Tempo
    
    // 1. Look for 'metronome' element
    if (TiXmlNode* directionType = direction->FirstChildElement( "direction-type" )) {
        do
        {
            if ( TiXmlNode* metronome = directionType->FirstChildElement( "metronome" ) )
            {
                if ( processTempo( metronome ) )
                    return;
            }
            // Find an 'other-direction' or 'words' element that contains an 'a tempo'-like text
        } while ( (directionType = direction->IterateChildren( "direction-type", directionType )) );
    }
    
    // 2. Look for 'sound' element
    if ( TiXmlNode* sound = direction->FirstChildElement( "sound" ) )
    {
        processTempo( sound );
    }
    
    // 3. Look for 'a tempo' or 'tempo primo' text element
    else if (TiXmlNode* directionType = direction->FirstChildElement( "direction-type" )) {
        do
        {
            // Find an 'other-direction' or 'words' element that contains an 'a tempo'-like text
            TiXmlNode* otherDirection = directionType->FirstChildElement( "other-direction" );
            if (!otherDirection) {
                otherDirection = directionType->FirstChildElement( "words" );
            }
            if (otherDirection)
            {
                // Look for a 'tempo primo'-compatible text
                string content = otherDirection->ToElement()->GetText();
                content = Utils::clean(content);
                // Look for 'tempo primo'-compatible text
                vector<string> const& tempoPrimo = Dictionary::tempoPrimo;
                if (std::find(tempoPrimo.begin(), tempoPrimo.end(), content) != tempoPrimo.end()) {
                    appendTempoPrimo();
                }
                // Look for 'a tempo'-compatible text
                else
                {
                    vector<string> const& aTempo = Dictionary::aTempo;
                    if (std::find(aTempo.begin(), aTempo.end(), content) != aTempo.end()) {
                        appendCurrentTempo(false);
                    }
                }
            }
        } while ( (directionType = direction->IterateChildren( "direction-type", directionType )) );
    }
    
    
    ///// B. Other SmartMusic markers
    if (TiXmlNode* directionType = direction->FirstChildElement( "direction-type" )) {
        do
        {
            if ( TiXmlNode* otherDirection = directionType->FirstChildElement( "other-direction" ) )
            {
                string content = otherDirection->ToElement()->GetText();
                content = Utils::clean(content);
                // Look for "wait for note" compatible events
                vector<string> const& dict = Dictionary::waitForNote;
                if (std::find(dict.begin(), dict.end(), content) != dict.end()) {
                    addWaitForNote();
                }
                
                // Look for "noSyncOnNote" compatible events
                vector<string> const& nosyncOnNoteDict = Dictionary::noSyncOnNote;
                if (std::find(nosyncOnNoteDict.begin(), nosyncOnNoteDict.end(), content) != nosyncOnNoteDict.end()) {
                    model_.appendEvent(new NosyncOnNote());
                }
                // Look for "noSyncStart" compatible events
                vector<string> const& nosyncStartDict = Dictionary::noSyncStart;
                if (std::find(nosyncStartDict.begin(), nosyncStartDict.end(), content) != nosyncStartDict.end()) {
                    model_.appendEvent(new NosyncStart());
                }
                // Look for "noSyncStart" compatible events
                vector<string> const& nosyncStopDict = Dictionary::noSyncStop;
                if (std::find(nosyncStopDict.begin(), nosyncStopDict.end(), content) != nosyncStopDict.end()) {
                    model_.appendEvent(new NosyncStop());
                }
                 
            }
            // Find an 'other-direction' or 'words' element that contains an 'a tempo'-like text
        } while ( (directionType = direction->IterateChildren( "direction-type", directionType )) );
    }
}



rational MusicXmlImporter::getBeatDurationFromNoteType(const char* unit)
{
    if ( unit )
    {
        if ( strcmp(unit, "maxima") == 0 )
        {
            return 32;
        }
        else if ( strcmp(unit, "long") == 0 )
        {
            return 16;
        }
        else if ( strcmp(unit, "breve") == 0 )
        {
            return 8;
        }
        else if ( strcmp(unit, "whole") == 0 )
        {
            return 4;
        }
        else if ( strcmp(unit, "half") == 0 )
        {
            return 2;
        }
        else if ( strcmp(unit, "eighth") == 0 )
        {
            return rational(1,2);
        }
        else if ( strcmp(unit, "16th") == 0 )
        {
            return rational(1,4);
        }
        else if ( strcmp(unit, "32nd") == 0 )
        {
            return rational(1,8);
        }
        else if ( strcmp(unit, "64th") == 0 )
        {
            return rational(1,16);
        }
        else if ( strcmp(unit, "128th") == 0 )
        {
            return rational(1,32);
        }
        else if ( strcmp(unit, "256th") == 0 )
        {
            return rational(1,64);
        }
        else if ( strcmp(unit, "512th") == 0 )
        {
            return rational(1,128);
        }
        else if ( strcmp(unit, "1024th") == 0 )
        {
            return rational(1,256);
        }
        else
            return 1;
    }
    
    return 1;
}


/*! \short  Add a new Tempo event that copies the previous event
 */
void MusicXmlImporter::appendCurrentTempo(bool generated)
{
    float currentTempo = currentQuarterNoteTempo_;
    if (!currentTempo)
        currentTempo = Importer::Default::beatPerMinute;
    model_.appendEvent( new BeatPerMinute( currentMeasure_, accumLocal_, currentTempo * currentMetricFactor_, currentOriginalBeats_, currentOriginalBase_, generated) );
}


/*! \short  Add a new Tempo event that copies the first Tempo event of the score
 *
 *  If no Tempo Event is found, then a new one with current score attributes is created.
 */
void MusicXmlImporter::appendTempoPrimo()
{
    if (Event *event = model_.findFirstBeatPerMinute()) {
        BeatPerMinute *bpm = (BeatPerMinute*) event;
        model_.appendEvent(BeatPerMinute::copyAt(currentMeasure_, accumLocal_, *bpm));
    }
    else {
        appendCurrentTempo(false);
    }
}


/* Handle 3 cases:
 * - 1.     tempo Mark : quarter = 90 BPM
 * - 2.     tempo modulation : quarter = eighth
 * - 1 bis. tempo Mark using "tempo" attribute
 */
bool MusicXmlImporter::processTempo( TiXmlNode* item )
{
    float quarterNoteValue = 0.0;
    float quarterBase = 1.0;
    float beats = -1.0;
    bool tempoMark = false, tempoModulation = false;    // indicate
    float quarterBaseBeforeModulation = 1.0;
    unsigned short dotCount = 0.;
    
    if (TiXmlNode* beatUnit = item->FirstChildElement( "beat-unit" ))
    {
        // Read quarter time duration of beat unit
        const char* unit = beatUnit->ToElement()->GetText();
        if ( unit )
            quarterBase = getBeatDurationFromNoteType(unit);
        
        // Process other children
        TiXmlNode* child = item->IterateChildren(beatUnit);
        while ( child )
        {
            // Case 1 : tempo MARK
            if (!strcmp( child->Value(), "per-minute"))
            {
                tempoMark = true;
                // Read tempo value
                //// First try: convert string as float
                beats = atof( child->ToElement()->GetText() );
                //// Second try: extract first float in string
                if (beats <= 0.0)
                {
                    std::sscanf(child->ToElement()->GetText(), "%*[^0123456789]%f", &beats);
                }
                //std::cerr << "BPM " << beats << std::endl;
                if ( beats <= 0.0 )
                    return false;
                // Apply dots on beat unit
                if (dotCount)
                    quarterBase *= 2. - powf(2.,-dotCount);
                dotCount = 0;
                // Convert tempo Mark to quarter note
                quarterNoteValue = beats * quarterBase;
                break;  // parsing of Tempo Mark is over, so we can exit loop
            }
            // Case 2 : tempo MODULATION
            else if (!strcmp( child->Value(), "beat-unit"))
            {
                tempoModulation = true;
                // Apply dots on beat unit
                if (dotCount)
                    quarterBase *= 2. - powf(2.,-dotCount);
                dotCount = 0;
                // Store PREVIOUS beat unit
                quarterBaseBeforeModulation = quarterBase;
                // Reinit dot counter;
                // Read quarter time duration of SECOND beat unit
                const char* unit2 = child->ToElement()->GetText();
                if ( unit2 )
                    quarterBase = getBeatDurationFromNoteType(unit2);
            }
            else if (!strcmp( child->Value(), "beat-unit-dot"))
            {
                ++dotCount;
            }
            
            child = item->IterateChildren(child);
        }
        
        // Apply dots on beat unit
        if (dotCount)
            quarterBase *= 2. - powf(2.,-dotCount);
        dotCount = 0;
    }
    if (!tempoMark && !tempoModulation) // Case 1 bis. simple tempo indication
    {
        if ( item->ToElement()->QueryFloatAttribute( "tempo", &quarterNoteValue ) == TIXML_SUCCESS )
            beats = quarterNoteValue;
    }
    // Case of metric modulation
    else if ( tempoModulation )
    {
        if (quarterBase != quarterBaseBeforeModulation) // do nothing if this is not really a modulation
        {
            currentOriginalBase_ = quarterBase;
            // Compute new tempo using règle de trois
            currentQuarterNoteTempo_ *= (quarterBase / quarterBaseBeforeModulation);
            currentOriginalBeats_ *= (quarterBase / quarterBaseBeforeModulation);
            // Insert tempo mark as a "generated" one (@modulate)
            model_.insertOrReplaceEvent( new BeatPerMinute( currentMeasure_, accumLocal_, currentQuarterNoteTempo_ * currentMetricFactor_, currentOriginalBeats_, currentOriginalBase_, true ) );
        }
        return true;
    }
    
    // Case of tempo mark
    if ( quarterNoteValue > 0.0 )
    {
        currentOriginalBase_ = quarterBase;
        currentQuarterNoteTempo_ = quarterNoteValue;
        currentOriginalBeats_ = beats;
        model_.insertOrReplaceEvent( new BeatPerMinute( currentMeasure_, accumLocal_, currentQuarterNoteTempo_ * currentMetricFactor_, currentOriginalBeats_, currentOriginalBase_ ) );
        return true;
    }
    return false;
}

float MusicXmlImporter::processNote( TiXmlNode* note )
{
    TiXmlNode* voiceNode = note->FirstChildElement( "voice" );
    if ( voiceNode )
    {
        int voice = atoi( voiceNode->ToElement()->GetText() );
        if ( currentVoice_ != voice )
        {
            currentVoice_ = voice;
        }
    }
    float duration = 0.0;
    int intDuration = 0;
    if ( !strcmp( note->Value(), "backup") )
    {
        TiXmlNode* durationNode = note->FirstChildElement( "duration" );
        if ( !durationNode )    //should not happen, but better safe than sorry!
            return 0.0;
        intDuration = atoi( durationNode->ToElement()->GetText() );
        duration = (float) currentMetricFactor_ * intDuration / currentDivision_;
        return -duration;
    }
    EntryFeatures features = None;
    TiXmlNode* grace = note->FirstChildElement( "grace" );
    if ( grace )
    {
        features |= Feature::GraceNote;
        const char* slash = grace->ToElement()->Attribute( "slash" );
        if ( slash && strcmp( slash, "yes" ) == 0 )
        {
            features |= Feature::SlashedGraceNote;
        }
    }
    else
    {
        TiXmlNode* durationNode = note->FirstChildElement( "duration" );
        if ( !durationNode )    //should not happen, but better safe than sorry!
            return 0.0;
        intDuration = atoi( durationNode->ToElement()->GetText() );
        duration = (float) currentMetricFactor_ * intDuration / currentDivision_;
    }
    
    TiXmlNode* rest = note->FirstChildElement( "rest" );
    if ( rest )
    {
        if ( rest->ToElement()->Attribute( "measure" ) != nullptr )
        {
            features |= Feature::MeasureRest;
        }
    }

    TiXmlNode* pitch = note->FirstChildElement( "pitch" );
    const char* realCue = note->ToElement()->Attribute( "real-cue" );
    bool isMuted = realCue != nullptr;
    TiXmlNode* staffElement = note->FirstChildElement( "staff" );
    if ( staffElement ) {
        int staff = atoi( staffElement->ToElement()->GetText() );
        std::vector<int> staffList = wrapper_.staffList();
        if (!staffList.empty() && (std::find(staffList.begin(), staffList.end(), staff) == staffList.end()))
        {
            isMuted = true;
            if (pitch) {
                note->RemoveChild(pitch);
                pitch = nullptr;
            }
        }
    }
    
    float midiCents = 0.0;
    int diatonicStep = -1;
    float displayedAccidental = 0.0;
    Pitch newNote( 0, features );
                                             
    if ( pitch && !isMuted )
    {
        TiXmlNode* tie = note->FirstChildElement( "tie" );
        TiXmlNode* accidental = note->FirstChildElement( "accidental" );
        TiXmlNode* step = pitch->FirstChildElement( "step" );
        TiXmlNode* octave = pitch->FirstChildElement( "octave" );
        TiXmlNode* alter = pitch->FirstChildElement( "alter" );
        if ( step && octave )
        {
            const char diatonic = step->ToElement()->GetText()[0];
            newNote.setNoteSymbol( diatonic );
            diatonicStep = diatonic - 'A';
            int oct = atoi( octave->ToElement()->GetText() );
            newNote.setOctave( oct );
            if ( alter )
            {
                displayedAccidental = atof( alter->ToElement()->GetText() );
                newNote.setAccidendal( displayedAccidental );
            }
            if ( displayedAccidental == 0. && accidental )
            {
                const char* textAcc = accidental->ToElement()->GetText();
                int index = 0;
                while ( MusicXMLaccidentals[index].s != nullptr )
                {
                    if ( strcmp(textAcc, MusicXMLaccidentals[index].s) == 0 )
                    {
                        displayedAccidental = MusicXMLaccidentals[index].a;
                        newNote.resetNote();
                        break;
                    }
                    ++index;
                }
            }
            midiCents = getMidiCents( diatonic, oct, displayedAccidental );
            if ( tie )
            {
              const char* tieType = tie->ToElement()->Attribute( "type" );
              if ( tieType && strcmp( tieType, "stop" ) == 0 )
              {
                  features |= Feature::Tiedbackwards;
              }
            }
        }
    }
    int baseDegree = diatonicStep;
    if ( currentKeyAccidentals_ > 0 )
        baseDegree += 3*currentKeyAccidentals_;  //degree/A see in 'A minor' so for instance B in D major => A
    else
        baseDegree -= 4*currentKeyAccidentals_; // same for key with flats
    baseDegree = baseDegree%7;
    int semitoneAfter = 2;
    bool hasTrueAccidental = false;
    for ( int i = 1; i <= currentKeyAccidentals_; ++i )
    {
        int degree = (diatonicStep - 4*i)%7;
        if ( degree == 1 )
        {
            hasTrueAccidental = true;
            break;
        }
    }
    if ( baseDegree == 1 || baseDegree == 4 )
        semitoneAfter = 1;
    else if ( displayedAccidental == 1 && !hasTrueAccidental )
        semitoneAfter = 1;
    TiXmlNode* timeModification = note->FirstChildElement( "time-modification" );
    int actualNotes = 1;
    int normalNotes = 1;
    if ( timeModification )
    {
        TiXmlNode* actual_notes = timeModification->FirstChildElement( "actual-notes" );
        if ( actual_notes )
            actualNotes = atoi( actual_notes->ToElement()->GetText() );
        TiXmlNode* normal_notes = timeModification->FirstChildElement( "normal-notes" );
        if ( normal_notes )
            normalNotes = atoi( normal_notes->ToElement()->GetText() );
    }
    if ( midiCents > 0 && currentTrillVoice_ > 0 && currentTrillVoice_ == currentVoice_ && duration > 0.5 )
    {
        features |= Feature::Trill;
        if ( semitoneAfter == 2 )
            features |= Feature::WholeToneTrill;
        ++midiCents;
    }
    TiXmlNode* chord = note->FirstChildElement( "chord" );
    TiXmlNode* notations = note->FirstChildElement( "notations" );
    if ( notations )
    {
        if ( notations->FirstChildElement( "fermata" ) )
            features |= Feature::Fermata;
        TiXmlNode* glissando = notations->FirstChildElement( "slide" );
        if ( !glissando )
            glissando = notations->FirstChildElement( "glissando" );
        if ( glissando )
        {
            if ( const char* glissType = glissando->ToElement()->Attribute( "type" ) )
            {
              string type = glissType;
              if ( type == "start")
              {
                features |= Feature::GlissandoStart;
              }
              if ( type == "stop")
              {
                features |= Feature::GlissandoEnd;
              }
            }
        }
        if ( TiXmlNode* articulations = notations->FirstChildElement( "articulations" ) )
        {
            if ( articulations->FirstChildElement( "staccato" ) != nullptr )
                features |= Feature::Staccato;
        }
        TiXmlNode* ornaments = notations->FirstChildElement( "ornaments" );
        if ( duration > 0 && ornaments )
        {
            TiXmlNode* tremolo = ornaments->FirstChildElement( "tremolo" );
            TiXmlNode* trill = ornaments->FirstChildElement( "trill-mark" );
            TiXmlNode* wavy = ornaments->FirstChildElement( "wavy-line" );
            if ( tremolo )
            {
                string type = string( "single" ); // owing to MusicXML standard, default type of tremolo is "single".
                if ( const char* tremoloType = tremolo->ToElement()->Attribute( "type" ) )
                {
                    type = string( tremoloType );
                }
                  
                  if ( type == "single" ) //repeated note or chord
                  {
                    int strokes = atoi( tremolo->ToElement()->GetText() );
                    if ( ( duration > 0.5 && strokes <= 2 ) || ( duration >= 0.33 && strokes == 1 ))
                    {
                        int noteDivision = 2;
                        if ( duration > 0.5 )
                            noteDivision = duration * strokes * 2;
                        if ( actualNotes != 1 )
                            noteDivision = actualNotes;
                        currentRepeatNoteAmount_ = noteDivision;
                        newNote.setFeatures( features );
                        newNote.setMidiCents( midiCents );
                        return model_.addRepeatedNotes( currentMeasure_, accumLocal_, duration, noteDivision, newNote );
                    }
                    else {
                        features |= Feature::FastRepeatedTremolo;
                    }
                  }
                  else
                  {
                    int strokes = atoi( tremolo->ToElement()->GetText() );
                      
                    bool isMeasuredTremolo = false;
                      bool isHalfNote = false;
                      TiXmlNode* noteType = note->FirstChildElement( "type" );
                      if ( noteType )
                      {
                          if ( const char* typeDuration = noteType->ToElement()->GetText() )
                          {
                              if ( !strcmp(typeDuration, "half") )
                              {
                                  isHalfNote = true;
                              }
                          }
                      }
                      if ( isHalfNote )
                      {
                          // Music notation rules: exclude all possible notations of unmeasured half note tremolos.
                          if ( strokes == 1 || ( strokes == 2 && !note->FirstChildElement( "beam" ) ) )
                          {
                              isMeasuredTremolo = true;
                          }
                      }
                      else if ( ( duration*2 > 0.5 && strokes <= 2 ) || ( duration*2 <= 0.5 && strokes == 1 ) ) // old heuristic
                      {
                          isMeasuredTremolo = true;
                      }
                      
                    if ( isMeasuredTremolo )
                    {
                      //int noteDivision = 2;
                      //if ( duration > 0.5 )
                      //  noteDivision = duration * strokes * 2;
                      //if ( actualNotes != 1 )
                      //  noteDivision = actualNotes;
                      //currentRepeatNoteAmount_ = noteDivision;
                      //TODO: measured repeated notes...
                    }
                    else    //unmeasured tremolo
                    {
                      if ( type == "start" ) //alternate tremolo start
                      {
                        if ( !chord && currentNoteFeatures_ & Feature::AlternateTremolo )    //Sibelius xml export bug ('start' instead of 'stop'...)
                        {
                          currentNoteFeatures_ = Feature::TremoloEnd;
                          features |= Feature::TremoloEnd;
                          accumLocal_ -= duration*2;
                        }
                        else
                        {
                          currentNoteFeatures_ = Feature::AlternateTremolo;
                          features |= Feature::AlternateTremolo;
                        }
                        duration *= 2;
                      }
                      else if ( type == "stop" && currentNoteFeatures_ & Feature::AlternateTremolo ) //alternate tremolo end
                      {
                        currentNoteFeatures_ = TremoloEnd;
                        features |= Feature::TremoloEnd;
                        accumLocal_ -= duration*2;
                        duration *= 2;  // ex: 𝅗𝅥 ≣ 𝅗𝅥 is 2 x quarter note
                      }
                    }
                  }
            }
            else
            {
                bool wavyStart = false;
                bool wavyStop = false;
                if ( wavy )
                {
                    string type = wavy->ToElement()->Attribute( "type" );
                    wavyStart = ( type == "start" /*|| type == "continue"*/ );
                    wavyStop = ( type == "stop" );
                    
                    bool wavyStoppingWithinScore = false;
                    
                    if ( wavyStart )
                    {
                        // AIMP-2 & AIMP-13 : Check if there's a closing wavy within the same scope! Though it doesn't make sense, it can happen in some MXMLs.. Go figure..
                        TiXmlNode* anotherWavy=wavy->NextSibling("wavy-line");
                        if (anotherWavy)
                        {
                            string lastWayType = anotherWavy->ToElement()->Attribute( "type" );
                            wavyStoppingWithinScore = (lastWayType == "stop");
                        }
                        if (wavyStoppingWithinScore)
                        {
                            currentTrillVoice_ = 0;
                            wavyStop = true;
                        }
                        else
                            currentTrillVoice_ = currentVoice_;
                    }
                    
                    
                    // Dealing with Wavy Stop after assigning Trill now (AIMP-14)
                }
                
//                if ( trill || (wavy && !wavyStop && !wavyStart && !trill) )
                if ( trill || (wavy && !wavyStart && !trill) )  // wavyStop SHOULD generate Trill! (AIMP-14)
                {
                    if ( !(features & Trill) && midiCents > 0 )
                        ++midiCents;    //by this trick, we mark the primary pitch as a trill (ex: 6200 -> 6201)
                    
                    TiXmlNode* accidentalMark = ornaments->FirstChildElement( "accidental-mark" );
                    if ( accidentalMark )
                    {
                        int displayedTrillAccidental = 0;
                        string content = accidentalMark->ToElement()->GetText();
                        if ( content == "sharp" )
                            displayedTrillAccidental = 1;
                        else if ( content == "double-sharp" )
                            displayedTrillAccidental = 2;
                        else if ( content == "natural" )
                            displayedTrillAccidental = 0;
                        else if ( content == "flat" )
                            displayedTrillAccidental = -1;
                        else if ( content == "double-flat" )
                            displayedTrillAccidental = -2;
                        if ( ( displayedAccidental - displayedTrillAccidental == -1 ) ||
                            ( displayedAccidental == -1 && displayedTrillAccidental == -1 && baseDegree != 1 && baseDegree != 4 )  ||
                            ( displayedAccidental == 1 && displayedTrillAccidental == 1 && baseDegree != 1 && baseDegree != 4 ) )
                            semitoneAfter = 2;
                        else if ( ( displayedAccidental - displayedTrillAccidental == 1 ) ||
                                 ( semitoneAfter == 2 && displayedTrillAccidental == -1 ) ||
                                 ( displayedAccidental == -1 && displayedTrillAccidental == -1 && ( baseDegree == 1 || baseDegree == 4 ) ) )
                            semitoneAfter = 1;
                    }
                    features |= Feature::Trill;
                    if ( semitoneAfter == 2 )
                        features |= Feature::WholeToneTrill;
                }
                
                if ( wavyStop && !wavyStart)
                    currentTrillVoice_ = 0;
            }
        }
    }
    // Start: Workaround for Finale MusicXML export bug.
    // For half note tremolos, Finale usually does not export the "tremolo" attribute.
    bool is32ndOrShorterType = false; // Music notation rule: un-measured tremolos are written with 32nd or shorter note types.
    TiXmlNode* type = note->FirstChildElement( "type" );
    if ( type )
    {
        if (const char* typeDuration = type->ToElement()->GetText())
        {
            rational rationalTypeDurational = getBeatDurationFromNoteType(typeDuration);
            {
                if ( rationalTypeDurational <= getBeatDurationFromNoteType("32nd") )
                {
                    is32ndOrShorterType = true;
                }
            }
        }
    }
    bool isHalfNoteTremolo = is32ndOrShorterType && normalNotes >= 4 * actualNotes; // Ad hoc rule to identify half note tremolos.
    if ( !chord && features == None && isHalfNoteTremolo )
    {
        if ( currentNoteFeatures_ & Feature::AlternateTremolo ) //Sibelius xml export bug ('start' instead of 'stop'...)
        {
            currentNoteFeatures_ = Feature::TremoloEnd;
            features |= Feature::TremoloEnd;
            accumLocal_ -= duration*2;
        }
        else
        {
            currentNoteFeatures_ = Feature::AlternateTremolo;
            features |= Feature::AlternateTremolo;
        }
        duration *= 2;
    }
    // End: Workaround for Finale MusicXML export bug.
    TiXmlNode* notehead = note->FirstChildElement( "notehead" );
    if ( notehead )
    {
        if ( strcmp( notehead->ToElement()->GetText(), "diamond" ) == 0 )
            features |= Feature::Harmonic;
        else if ( strcmp( notehead->ToElement()->GetText(), "square" ) == 0 )
            features |= Feature::SquareNotehead;
    }
    if ( chord )
    {
        features |= Feature::Chord;
        if ( currentNoteFeatures_ & AlternateTremolo || currentNoteFeatures_ & TremoloEnd )
        {
            features |= Feature::AlternateTremolo;
        }
        if ( currentRepeatNoteAmount_ == 0 )
            duration = previousDuration_;
        if ( currentNoteFeatures_ & Feature::TremoloEnd )
        {
            features |= Feature::TremoloEnd;
        }
        newNote.setFeatures( features );
        newNote.setMidiCents( midiCents );
        if ( currentRepeatNoteAmount_ > 0 )
            model_.addRepeatedNotes( currentMeasure_, accumLocal_ - duration, duration, currentRepeatNoteAmount_, newNote );
        else
            model_.addNote( currentMeasure_, accumLocal_ - duration, duration, newNote );
        return 0.0;
    }
    else
    {
        previousDuration_ = duration;
        currentRepeatNoteAmount_ = 0;
        if ( features == None )
            currentNoteFeatures_ &= ~(Feature::TremoloEnd|Feature::AlternateTremolo);
    }
    newNote.setFeatures( features );
    newNote.setMidiCents( midiCents );
    duration = model_.addNote( currentMeasure_, accumLocal_, duration, newNote );
    return duration;
}

int MusicXmlImporter::getMidiCents( const char diatonic, int octave, float accidental ) const
{
    int midiCents = currentChromaticTransposition_ + ( currentOctaveTransposition_ + octave + 1 ) * 12;
    switch ( diatonic )
    {
        case 'D':
            midiCents += 2;
            break;
            
        case 'E':
            midiCents += 4;
            break;
            
        case 'F':
            midiCents += 5;
            break;
            
        case 'G':
            midiCents += 7;
            break;
            
        case 'A':
            midiCents += 9;
            break;
            
        case 'B':
            midiCents += 11;
            break;
            
        default:
            break;
    }
    midiCents *= 100;
    midiCents += accidental*100;
    return midiCents;
}


void MusicXmlImporter::addWaitForNote() {
    int const measureNumber = (int)currentMeasure_;
    float const measurePosition = accumLocal_;
    std::ostringstream stringStream;
    if (measurePosition)
        stringStream << "(($measure" << measureNumber << ")+(" << measurePosition << "))";
    else
        stringStream << "($measure" << measureNumber << ")";
    std::string const position = stringStream.str();
    if (!position.empty())
        model_.addWaitForNote(position);
}

