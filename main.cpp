//
// .-. . . .-. .-. .-. .-. .-. .-. .-.
// |-| |\|  |  |-  `-. |   | | |-  | |
// ` ' ' `  '  `-' `-' `-' `-' '   `-'
//
// .-. .  . .-. .-. .-. .-. .-. .-.
//  |  |\/| |-' | | |(   |  |-  |(
// `-' '  ` '   `-' ' '  '  `-' ' '
//
//      @ Ircam 2015
//      main.ccp (test for antescofo xml & midi conversion)
//      created by Robert Piéchaud
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ImporterWrapper.h"
#include <time.h>
#ifdef __APPLE__
#include <mach/clock.h>
#include <mach/mach.h>
#endif
#include <sys/timeb.h>
#include <sys/stat.h>
#include <iostream>
#include <memory>

using namespace std;
using namespace antescofo;

static long long nanoseconds()
{
    struct timespec now;
#ifdef __APPLE__
    clock_serv_t cclock;
    mach_timespec_t mts;
    host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
    clock_get_time(cclock, &mts);
    mach_port_deallocate(mach_task_self(), cclock);
    now.tv_sec = mts.tv_sec;
    now.tv_nsec = mts.tv_nsec;
#elif defined __linux__
    clock_gettime( CLOCK_REALTIME, &now );
#endif
    return (float) now.tv_nsec + now.tv_sec*1.0e9;
}

void displayHelp()
{
    char bright [] = "\033[1;36m";
    char normal [] = "\033[0m";
    cout << endl
         << "  Normal usage is to pass file path (MusicXML/MIDI) as parameter." << endl << "  Additional options:" << endl << endl
         << bright << "  -midicents" << normal << " displays pitches in cents (for instance C#4 -> 6100)" << endl
         << bright << "  -quarternotetime" << normal << " sets tempos and note durations always relative to the quarter note" << endl
         <<           "     (even for compound times such as 6/8, etc.)" << endl
         << bright << "  -3/8compoundtime" << normal << " consider 3/8 time as compound" << endl
         <<           "     (so as one-beat time, note that the 'quarternotetime' option must be off!)" << endl
         << bright << "  -originalpitches" << normal << " shows pitches & accidentals as specified in the original score" << endl
         <<           "     (otherwise Eb can show up as D# for instance)." << endl
         << bright << "  -querytracks" << normal << " displays the file’s available tracks/staves." << endl
         << bright << "  -tracks=" << normal << "1,4,5,... restricts the conversion to some specific tracks/staves." << endl
         <<           "     (for MIDI files, negative number will mean 'playback track'," << endl
         <<           "      thus converted as 'action group')." << endl
         << bright << "  -outputdirectory" << normal << "=[path] for specific output directory." << endl
         << bright << "  -unittest" << normal << "=[folder path] will process unit tests (for internal use.)" << endl
         << bright << "  -verbose" << normal << " / -v for (too!) many details." << endl
         << bright << "  -help" << normal << " / -h displays these info." << endl
         << endl;
}

int main( int argc, char **argv )
{
    int success = 0;
    
    cout << endl;
    cout << "Antescofo musicXML and MIDI importer ~ designed by Robert Piéchaud for IRCAM" << endl;
    cout << "Command line utility" << endl;
    cout << ImporterWrapper::getVersion() << endl;
    cout << "©2015 IRCAM" << endl;
    
    char* unitaryTestFolderPath = nullptr;
    vector<string> arguments;
    for ( int i = 1; i < argc; ++i )
    {
        if ( strncmp(argv[i], "-unittest=", 10 ) == 0 )
        {
            unitaryTestFolderPath = argv[i] + 10;
        }
        else if ( strncmp(argv[i], "-help", 5 ) == 0 || strncmp(argv[i], "-h", 2 ) == 0 || strncmp(argv[i], "--help", 6 ) == 0 )
        {
            unitaryTestFolderPath = argv[i] + 13;
            displayHelp();
            return 0;
        }
        arguments.push_back( argv[i] );
    }
    std::unique_ptr<ImporterWrapper> importer( new ImporterWrapper() );
    int parseResult = importer->parseArguments( arguments );
    string filePath = importer->getInputPath();
    std::string input;
    if ( !parseResult )
    {
        displayHelp();
        std::cout << "Please enter the path (relative or absolute) to a MusicXML or MIDI file: " << std::endl << "> ";
        std::cin >> input;
        if ( input.length() )
            filePath = (char *) input.c_str();
    }
    
    char red [] = "\033[1;31m";
    char green [] = "\033[1;32m";
    char normal [] = "\033[0m";
    if ( filePath.length() )
    {
        cout << "  File passed: '" << filePath << "'" << endl;
        long long t0 = nanoseconds();
        if ( importer->trackListQuery() )
        {
            vector<string> trackList;
            if ( importer->queryTracklist( trackList ) )
            {
                cout << "There " << ( trackList.size() == 1? "is ": "are " ) << trackList.size() << " track" << ( trackList.size() == 1? "": "s" ) << " in this file:" << endl;
                int count = 1;
                for ( auto it = trackList.begin(); it != trackList.end(); ++it )
                {
                    cout << "(" << count << ")  " << *it << endl;
                    ++count;
                }
            }
            else
            {
                cout << red << "  Error: tracks query failed for some reason!" << normal << endl;
                success = -1;
            }
        }
        else
        {
            bool imported = false;
            string tracks = importer->rawTrackSelection();
            if ( tracks.length() )
            {
                vector<int> trackList;
                bool listOk = true;
                for ( int i = 0; i < tracks.length(); ++i)
                {
                    if ( tracks[i] != ',' && tracks[i] != '-' && !isdigit(tracks[i]))
                    {
                        cout << "  Bad track list syntax (good example: -tracks=1,3,4)" << endl;
                        listOk = false;
                        break;
                    }
                }
                if ( listOk )
                {
                    string sub;
                    size_t pos = string::npos;
                    do
                    {
                        pos = tracks.find(',');
                        if ( pos == string::npos )
                        {
                            sub = tracks;
                            tracks = "";
                        }
                        else
                        {
                            sub = tracks.substr( 0, pos );
                            tracks = tracks.substr( pos + 1 );
                        }
                        trackList.push_back( atoi( sub.c_str()));
                    }
                    while ( tracks.length() > 0 );
                }
                if ( trackList.size() )
                {
                    if ( trackList.size() > 1 )
                    {
                        cout << "  Processing tracks: ";
                        for ( int i = 0; i < trackList.size(); ++i )
                            cout << abs( trackList[i] ) << " ";
                        cout << endl;
                    }
                    imported = importer->import( filePath, trackList );
                }
            }
            else
            {
                imported = importer->import( filePath );
            }
            if ( imported )
            {
                if ( importer->save() )
                {
                    cout << endl << "  saved as '" << importer->outputPath() << "'" << endl;
                    cout << green << "  Conversion to Antescofo successful! :-)" << normal << endl;
                }
                else
                {
                    cout << red << "  Could not saved file! :-(" << normal << endl;
                }
                long long delta = nanoseconds() - t0;
                cout << "  (file processed in " << (float) delta*1.0e-9 << " sec.)" << endl;
            }
            else
                success = -1;
        }
    }
    else if ( unitaryTestFolderPath )
    {
        if ( !importer->runUnitaryTests( unitaryTestFolderPath ) )
        {
            cout << red << "  Error: unit test run failed!" << normal << endl;
            success = -1;
        }
        else
            cout << green << "  Unit test campaign successful! :-)" << normal << endl;
    }
    else
    {
        cout << "  Missing or bad parameters..." << endl;
        displayHelp();
    }
    if ( success == -1 && !unitaryTestFolderPath )
    {
        cout << red << "  Error: nothing could be processed!" << normal << endl;
    }
    cout << endl;
    return success;
}
