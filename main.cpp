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
#include <mach/clock.h>
#include <mach/mach.h>
#include <sys/timeb.h>
#include <sys/stat.h>
#include <iostream>

using namespace std;
using namespace antescofo;

static long long nanoseconds()
{
    struct timespec now;
    clock_serv_t cclock;
    mach_timespec_t mts;
    host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
    clock_get_time(cclock, &mts);
    mach_port_deallocate(mach_task_self(), cclock);
    now.tv_sec = mts.tv_sec;
    now.tv_nsec = mts.tv_nsec;
    return (float) now.tv_nsec + now.tv_sec*1.0e9;
}

void displayHelp()
{
    char bright [] = "\033[1;36m";
    char normal [] = "\033[0m";
    cout << endl
         << "  Normal usage is to pass file path (MusicXML/MIDI) as parameter." << endl << "  Additional options:" << endl << endl
         << bright << "  -midicents" << normal << " displays pitches in cents (for instance C#4 -> 6100)" << endl
        << bright << "  -quarternotetime" << normal << " sets tempos and note durations always relative to the quarter note." << endl
        << bright << "  -querytracks" << normal << " displays the file’s available tracks/staves." << endl
        << bright << "  -tracks=" << normal << "0,3,4,... restricts the import to some specific tracks/staves." << endl
         << "   (even for compound times such as 6/8, etc.)" << endl
         << bright << "  -unittest" << normal << "=[folder path] will process unit tests (for internal use.)" << endl
         << bright << "  -verbose" << normal << " / -v for (too!) many details." << endl
         << bright << "  -help" << normal << " / -h displays these info." << endl
         << endl;
}

int main( int argc, char **argv )
{
    char* filePath = nullptr;
    char* unitaryTestFolderPath = nullptr;
    string tracks;
    bool displayMidiCents = false;
    bool quarternotetime = false;
    int success = 0;
    bool verbose = false;
    bool queryTracks = false;
    
    cout << endl;
    cout << "Antescofo musicXML and MIDI importer" << endl;
    cout << "Command line utility" << endl;
    cout << "Version 0.1.22" << endl;
    cout << "©2015 IRCAM" << endl;

    for (int i = 1; i < argc; ++i)
    {
        if ( strncmp(argv[i], "-unittest=", 10 ) == 0 )
        {
            unitaryTestFolderPath = argv[i] + 10;
            cout << "  Unit tests..." << endl;
        }
        else if ( strncmp(argv[i], "-midicents", 10 ) == 0 )
        {
            displayMidiCents = true;
            cout << "  MIDI cents representation on..." << endl;
        }
        else if ( strncmp(argv[i], "-querytracks", 12 ) == 0 )
        {
            queryTracks = true;
        }
        else if ( strncmp(argv[i], "-tracks=", 8 ) == 0 )
        {
            tracks = argv[i] + 8;
        }
        else if ( strncmp(argv[i], "-quarternotetime", 16 ) == 0 )
        {
            quarternotetime = true;
            cout << "  Quarter note division on..." << endl;
        }
        else if ( strncmp(argv[i], "-help", 5 ) == 0 || strncmp(argv[i], "-h", 2 ) == 0 || strncmp(argv[i], "--help", 6 ) == 0 )
        {
            unitaryTestFolderPath = argv[i] + 13;
            displayHelp();
            return 0;
        }
        else if ( strncmp(argv[i], "-verbose", 8 ) == 0 || strncmp(argv[i], "-v", 2 ) == 0 )
        {
            verbose = true;
        }
        else
        {
            filePath = argv[i];
        }
    }
    std::string input;
    if ( filePath == nullptr && !unitaryTestFolderPath )
    {
        displayHelp();
        std::cout << "Please enter the path (relative or absolute) to a MusicXML or MIDI file: " << std::endl << "> ";
        std::cin >> input;
        if ( input.length() )
            filePath = (char *) input.c_str();
    }
    ImporterWrapper* importer = new ImporterWrapper();
    importer->setPitchesAsMidiCents( displayMidiCents );
    importer->setVerbose( verbose );
    char red [] = "\033[1;31m";
    char green [] = "\033[1;32m";
    char normal [] = "\033[0m";
    if ( filePath != nullptr )
    {
        cout << "  File passed: '" << filePath << "'" << endl;
        string out ( filePath );
        out = out.substr( 0, out.find_last_of( "." ) ) + ".asco.txt";
        long long t0 = nanoseconds();
        if ( queryTracks )
        {
            vector<string> tracks;
            if ( importer->queryTracks( filePath, tracks ) )
            {
                cout << "There " << ( tracks.size() == 1? "is ": "are " ) << tracks.size() << " track" << ( tracks.size() == 1? "": "s" ) << " in this file:" << endl;
                int count = 0;
                for ( auto it = tracks.begin(); it != tracks.end(); ++it )
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
            if ( tracks.length() )
            {
                vector<int> trackList;
                bool listOk = true;
                for ( int i = 0; i < tracks.length(); ++i)
                {
                    if ( tracks[i] != ',' && !isdigit(tracks[i]))
                    {
                        cout << "  Bad track list syntax (good example: -tracks=0,2,3)" << endl;
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
                    cout << "  Processing tracks: ";
                    for ( int i = 0; i < trackList.size(); ++i )
                        cout << trackList[i] << " ";
                    cout << endl;
                    imported = importer->import( filePath, trackList );
                }
            }
            else
            {
                imported = importer->import( filePath );
            }
            if ( imported )
            {
                if ( importer->save( out ) )
                {
                    cout << "  saved as '" << out << "'" << endl;
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
    delete importer;
    return success;
}
