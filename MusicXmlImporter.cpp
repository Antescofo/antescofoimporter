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
//  Copyright (c) 2015 ircam. All rights reserved.
//

#include "MusicXmlImporter.h"
#include "ImporterWrapper.h"
#include "ImportModel.h"
#include "tinyxml.h"
#include "BeatPerMinute.h"
#include "Measure.h"
#include "Repeat.h"
#include "extract.h"
#include <math.h>
#include <sstream>
#include <fstream>
#include "ConvertUTF.h"

using namespace antescofo;
using namespace std;

MusicXmlImporter::MusicXmlImporter( ImporterWrapper& wrapper ) :
    wrapper_                ( wrapper ),
    model_                  ( wrapper_.getModel() ),
    compressedXML_          ( false ),
    currentMeasure_         ( 0 ),
    currentKeyAccidentals_  ( 0 ),
    currentVoice_           ( 0 ),
    currentTrillVoice_      ( 0 ),
    currentNoteFeatures_    ( None ),
    currentMeasureDuration_ ( 0.0 ),
    accumGlobal_            ( 0.0 ),
    accumLocal_             ( 0.0 ),
    currentDivision_        ( 1 ),
    currentRepeatNoteAmount_( 0 ),
    currentMetricFactor_    ( 1.0 ),
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
    currentMeasure_ = 0;
    currentKeyAccidentals_ = 0;
    currentVoice_ = 0;
    currentTrillVoice_ = 0;
    currentNoteFeatures_ = None;
    currentMeasureDuration_  = 0.0;
    accumGlobal_ = 0.0;
    accumLocal_ = 0.0;
    currentDivision_ = 1;
    currentRepeatNoteAmount_ = 0;
    currentMetricFactor_ = 1.0;
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
    if ( !loaded )  //ok... then perhaps it's UTF-16 ? (from $!*#!! Dolet plug-int) or compressed file ?  (zipped xml = mxl)
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
            TiXmlNode* part = root->FirstChildElement( "part" );
            int count = 0;
            while ( part )
            {
                if ( !tracks_.size() || ( tracks_.size() && find( tracks_.begin(), tracks_.end(), count ) != tracks_.end() ) )
                {
                    if ( wrapper_.isVerbose() )
                        cout << "    ...staff #" << count << endl;
                    currentTrillVoice_ = 0;
                    accumLocal_ = 0.0;
                    TiXmlNode* measure = part->FirstChildElement( "measure" );
                    while ( measure )
                    {
                        currentVoice_ = 0;
                        currentNoteFeatures_ = None;
                        previousDuration_ = 0.0;
                        processMeasure( measure );
                        //accumLocal_ = 0.0;
                        measure = part->IterateChildren( "measure", measure );
                    }
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
                        name = partName->ToElement()->GetText();
                    }
                    if ( !name.size() )
                        name = "[staff " + to_string( count ) + "]";
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

void MusicXmlImporter::processMeasure( TiXmlNode* measure )
{
    measure->ToElement()->QueryValueAttribute( "number", &currentMeasure_ );
    TiXmlNode* attributes = measure->FirstChildElement( "attributes" );
    if ( attributes )
    {
        TiXmlNode* divisions = attributes->FirstChildElement( "divisions" );
        if ( divisions )
            currentDivision_ = atoi( divisions->ToElement()->GetText() );
    }
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
        model_.insertFirstEventInMeasure( new Measure(  currentMeasure_,
                                                      currentMeasureDuration_,
                                                      accumGlobal_,
                                                      currentMetricFactor_,
                                                      currentKeyAccidentals_,
                                                      currentTimeSignature_ ) );
        accumGlobal_ += currentMeasureDuration_ * currentMetricFactor_;
        TiXmlNode* direction = measure->FirstChildElement( "direction" );
        while ( direction )
        {
            processDirection( direction );
            direction = measure->IterateChildren( "direction", direction );
        }
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
    while ( item )
    {
        if ( !strcmp( item->Value(), "note") || !strcmp( item->Value(), "forward") || !strcmp( item->Value(), "backup") )
        {
            accumLocal_ += processNote( item );
        }
        item = measure->IterateChildren( item );
    }
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
    if ( upper.size() == lower.size() == 1 && upper[0] > 3 && lower[0] >= 8 )
    {
        if ( upper[0]%3 == 0 )
        {
            if ( lower[0] == 8 )
                currentMetricFactor_ = (float) 2/3 ;
            else if ( lower[0] == 16 )
                currentMetricFactor_ = (float) 4/3;
            else if ( lower[0] == 32 )
                currentMetricFactor_ = (float) 8/3;
            if ( upper[0] == 3 )
                currentMetricFactor_ = currentMetricFactor_*3;  // NB: 3/8, 3/16, 3/32 times are *not* seen as compound.
        }
    }
    if ( currentQuarterNoteTempo_ != 0.0 && factor != currentMetricFactor_ )
    {
        model_.appendEvent( new BeatPerMinute( currentMeasure_, currentQuarterNoteTempo_ * currentMetricFactor_, currentOriginalBeats_, currentOriginalBase_ ) );
    }
    if ( duration == 0.0 )
        duration = currentMeasureDuration_;
    return duration;
}

void MusicXmlImporter::processDirection( TiXmlNode* direction )
{
    TiXmlNode* directionType = direction->FirstChildElement( "direction-type" );
    while ( directionType )
    {
        TiXmlNode* metronome = directionType->FirstChildElement( "metronome" );
        if ( metronome )
        {
            processTempo( metronome );
            return;
        }
        directionType = direction->IterateChildren( "direction-type", directionType );
    }
    if ( TiXmlNode* sound = direction->FirstChildElement( "sound" ) )
    {
        processTempo( sound );
    }
}

void MusicXmlImporter::processTempo( TiXmlNode* item )
{
    float quarterNoteValue = 0.0;
    float quarterBase = 1.0;
    float beats = 0.0;
    TiXmlNode* beatUnit = item->FirstChildElement( "beat-unit" );
    TiXmlNode* perMinute = item->FirstChildElement( "per-minute" );
    if ( beatUnit && perMinute )
    {
        const char* unit = beatUnit->ToElement()->GetText();
        if ( strcmp(unit, "whole") == 0 )
        {
            quarterBase = 4.0;
        }
        else if ( strcmp(unit, "half") == 0 )
        {
            quarterBase = 2;
        }
        else if ( strcmp(unit, "eighth") == 0 )
        {
            quarterBase = 0.5;
        }
        else if ( strcmp(unit, "16th") == 0 )
        {
            quarterBase = 0.25;
        }
        if ( item->FirstChildElement( "beat-unit-dot" ) != nullptr )
            quarterBase *= 1.5;
        beats = atof( perMinute->ToElement()->GetText() );
        quarterNoteValue = beats * quarterBase;
    }
    else
    {
        item->ToElement()->QueryFloatAttribute( "tempo", &quarterNoteValue );
        beats = quarterNoteValue;
    }
    if ( quarterNoteValue )
    {
        currentQuarterNoteTempo_ = quarterNoteValue;
        model_.appendEvent( new BeatPerMinute( currentMeasure_, quarterNoteValue * currentMetricFactor_, beats, quarterBase ) );
        currentOriginalBeats_ = beats;
        currentOriginalBase_ = quarterBase;
    }
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
    if ( !strcmp( note->Value(), "backup") )
    {
        TiXmlNode* durationNode = note->FirstChildElement( "duration" );
        if ( !durationNode )    //should not happen, but better safe than sorry!
            return 0.0;
        duration = (float) currentMetricFactor_ * atoi( durationNode->ToElement()->GetText() ) / currentDivision_;
        return -duration;
    }
    EntryFeatures features = None;
    TiXmlNode* grace = note->FirstChildElement( "grace" );
    if ( !grace )
    {
        TiXmlNode* durationNode = note->FirstChildElement( "duration" );
        if ( !durationNode )    //should not happen, but better safe than sorry!
            return 0.0;
        duration = (float) currentMetricFactor_ * atoi( durationNode->ToElement()->GetText() ) / currentDivision_;
    }
    else
         features |= GraceNote;
    TiXmlNode* rest = note->FirstChildElement( "rest" );
    if ( rest )
    {
        const char* attr = rest->ToElement()->Attribute( "measure" );
        if ( attr != nullptr && strlen( attr ) > 0)
            return 0.0;
    }
    TiXmlNode* pitch = note->FirstChildElement( "pitch" );
    float midiCents = 0.0;
    int diatonicStep = -1;
    float accidental = 0.0;
    if ( pitch )
    {
        TiXmlNode* tie = note->FirstChildElement( "tie" );
        TiXmlNode* step = pitch->FirstChildElement( "step" );
        TiXmlNode* octave = pitch->FirstChildElement( "octave" );
        TiXmlNode* alter = pitch->FirstChildElement( "alter" );
        if ( step && octave )
        {
            const char diatonic = step->ToElement()->GetText()[0];
            diatonicStep = diatonic - 'A';
            int oct = atoi( octave->ToElement()->GetText() );
            if ( alter )
            {
                accidental = atof( alter->ToElement()->GetText() );
            }
            midiCents = getMidiCents( diatonic, oct, accidental );
            if ( tie )
            {
              const char* tieType = tie->ToElement()->Attribute( "type" );
              if ( tieType && strcmp( tieType, "stop" ) == 0 )
              {
                  midiCents = -midiCents;
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
    if ( baseDegree == 1 || baseDegree == 4 )
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
    if ( midiCents != 0 && currentTrillVoice_ > 0 && currentTrillVoice_ == currentVoice_ && duration > 0.5 )
    {
        features |= Trill;
        if ( semitoneAfter == 2 )
            features |= WholeToneTrill;
        if ( midiCents > 0 )
            ++midiCents;
        else
            --midiCents;
    }
    TiXmlNode* chord = note->FirstChildElement( "chord" );
    TiXmlNode* notations = note->FirstChildElement( "notations" );
    if ( notations )
    {
        if ( notations->FirstChildElement( "fermata" ) )
            features |= Fermata;
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
                features |= GlissandoStart;
              }
              if ( type == "stop")
              {
                features |= GlissandoEnd;
              }
            }
        }
        if ( TiXmlNode* articulations = notations->FirstChildElement( "articulations" ) )
        {
            if ( articulations->FirstChildElement( "staccato" ) != nullptr )
                features |= Staccato;
        }
        TiXmlNode* ornaments = notations->FirstChildElement( "ornaments" );
        if ( duration > 0 && ornaments )
        {
            TiXmlNode* tremolo = ornaments->FirstChildElement( "tremolo" );
            TiXmlNode* trill = ornaments->FirstChildElement( "trill-mark" );
            if ( tremolo )
            {
                if ( const char* tremoloType = tremolo->ToElement()->Attribute( "type" ) )
                {
                  string type ( tremoloType );
                  
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
                      return model_.addRepeatedNotes( currentMeasure_, accumLocal_, duration, noteDivision, midiCents );
                    }
                    features |= FastRepeatedTremolo;
                  }
                  else
                  {
                    int strokes = atoi( tremolo->ToElement()->GetText() );
                    if ( ( duration*2 > 0.5 && strokes <= 2 ) || ( duration*2 <= 0.5 && strokes == 1 ))
                    {
                      int noteDivision = 2;
                      if ( duration > 0.5 )
                        noteDivision = duration * strokes * 2;
                      if ( actualNotes != 1 )
                        noteDivision = actualNotes;
                      //currentRepeatNoteAmount_ = noteDivision;
                      //TODO: measured repeated notes...
                    }
                    else    //unmeasured tremolo
                    {
                      if ( type == "start" ) //alternate tremolo start
                      {
                        if ( !chord && currentNoteFeatures_&AlternateTremolo )    //Sibelius xml export bug ('start' instead of 'stop'...)
                        {
                          currentNoteFeatures_ = TremoloEnd;
                          features |= TremoloEnd;
                          accumLocal_ -= duration*2;
                        }
                        else
                        {
                          currentNoteFeatures_ = AlternateTremolo;
                          features |= AlternateTremolo;
                        }
                        duration *= 2;
                      }
                      else if ( type == "stop" && currentNoteFeatures_&AlternateTremolo ) //alternate tremolo end
                      {
                        currentNoteFeatures_ = TremoloEnd;
                        features |= TremoloEnd;
                        accumLocal_ -= duration*2;
                        duration *= 2;  // ex: 𝅗𝅥 ≣ 𝅗𝅥 is 2 x quarter note
                      }
                    }
                  }
                }
            }
            else if ( trill && midiCents > 0 )
            {
                if ( !(features & Trill) )
                    ++midiCents;    //by this trick, we mark the primary pitch as a trill (ex: 6200 -> 6201)
                TiXmlNode* accidentalMark = ornaments->FirstChildElement( "accidental-mark" );
                if ( accidentalMark )
                {
                    int displayAccidental = 0;
                    string content = accidentalMark->ToElement()->GetText();
                    if ( content == "sharp" )
                        displayAccidental = 1;
                    else if ( content == "double-sharp" )
                        displayAccidental = 2;
                    else if ( content == "natural" )
                        displayAccidental = 0;
                    else if ( content == "flat" )
                        displayAccidental = -1;
                    else if ( content == "double-flat" )
                        displayAccidental = -2;
                    if ( ( accidental - displayAccidental == -1 ) || ( accidental == -1 && displayAccidental == -1 ))
                        semitoneAfter = 2;
                    else if ( ( accidental - displayAccidental == 1 ) ||
                         ( semitoneAfter == 2 && displayAccidental == -1 ) )
                        semitoneAfter = 1;
                }
                features |= Trill;
                if ( semitoneAfter == 2 )
                    features |= WholeToneTrill;
            }
            
            TiXmlNode* wavy = ornaments->FirstChildElement( "wavy-line" );
            if ( wavy )
            {
                string type = wavy->ToElement()->Attribute( "type" );
                bool start = ( type == "start" || type == "continue" );
                bool stop = ( type == "stop" );
                if ( start )
                    currentTrillVoice_ = currentVoice_;
                if ( stop && !start)
                    currentTrillVoice_ = 0;
            }
        }
    }
    if ( !chord && features == None && duration >= 1 && normalNotes >= 4*actualNotes ) //buggy Finale "piano" tremolo...
    {
        if ( currentNoteFeatures_ & AlternateTremolo )
        {
            currentNoteFeatures_ = TremoloEnd;
            features |= TremoloEnd;
            accumLocal_ -= duration*2;
        }
        else
        {
            currentNoteFeatures_ = AlternateTremolo;
            features |= AlternateTremolo;
        }
        duration *= 2;
    }
    TiXmlNode* notehead = note->FirstChildElement( "notehead" );
    if ( notehead )
    {
        if ( strcmp( notehead->ToElement()->GetText(), "diamond" ) == 0 )
            features |= Harmonic;
        else if ( strcmp( notehead->ToElement()->GetText(), "square" ) == 0 )
            features |= SquareNotehead;
    }
    if ( chord )
    {
        features |= Chord;
        if ( currentNoteFeatures_ & AlternateTremolo || currentNoteFeatures_ & TremoloEnd )
        {
            features |= AlternateTremolo;
        }
        if ( currentRepeatNoteAmount_ == 0 )
            duration = previousDuration_;
        if ( currentNoteFeatures_ & TremoloEnd )
        {
            features |= TremoloEnd;
        }
        if ( currentRepeatNoteAmount_ > 0 )
            model_.addRepeatedNotes( currentMeasure_, accumLocal_ - duration, duration, currentRepeatNoteAmount_, midiCents );
        else
            model_.addNote( currentMeasure_, accumLocal_ - duration, duration, midiCents, features );
        return 0.0;
    }
    else
    {
        previousDuration_ = duration;
        currentRepeatNoteAmount_ = 0;
        if ( features == None )
            currentNoteFeatures_ &= ~(TremoloEnd|AlternateTremolo);
    }
    duration = model_.addNote( currentMeasure_, accumLocal_, duration, midiCents, features );
    return duration;
}

int MusicXmlImporter::getMidiCents( const char diatonic, int octave, float accidental ) const
{
    int midiCents = midiCents = ( octave + 1 )*12;
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
