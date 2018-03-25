//
//  UnitTester.cpp
//  antescofo_converter
//
//  Created by baba on 21/05/15.
//  Copyright (c) 2017 Antescofo. All rights reserved.
//

#include "UnitTester.h"
#include "ImporterWrapper.h"
#include <sys/stat.h>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <dirent.h>
#include <time.h>
#ifdef __APPLE__
#include <mach/clock.h>
#include <mach/mach.h>
#endif
#include <sys/timeb.h>
#include <errno.h>
#include <stdlib.h>

using namespace antescofo;
using namespace std;

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

UnitTester::UnitTester( ImporterWrapper& wrapper, const string& unitaryTestFolder ):
    wrapper_    (wrapper),
    folderPath_ ( unitaryTestFolder )
{
    //NOTHING
}

UnitTester::~UnitTester()
{
    //NOTHING
}

bool UnitTester::run()
{
    struct stat st;
    if ( stat( folderPath_.c_str(), &st ) != 0 )
    {
        cout << "  Error: unit test folder '" << folderPath_ << "' could not be found!" << endl;
        return false;
    }
    string path ( folderPath_ );
    if ( path.back() != '/' )
        path += "/";
    string source( path + "source" );
    if ( stat( source.c_str(), &st ) != 0 )
    {
        cout << "  Error: unit test source folder '" << source << "' could not be found!" << endl;
        return false;
    }
    string reference( path + "rereference" );
    if ( stat( source.c_str(), &st ) != 0 )
    {
        cout << "  Error: unit test rereference folder '" << reference << "' could not be found!" << endl;
        return false;
    }
    string difference( path + "difference/" );
    if ( stat( difference.c_str(), &st ) == 0 )
    {
        std::string command ( "rm -rdf " + difference );
        system( command.c_str() );
        rmdir( difference.c_str() );
    }
    
    DIR* sourceDirectory = opendir( source.c_str() );
    bool success = true;
    if ( sourceDirectory )
    {
        long long t0 = nanoseconds();
        struct dirent* specs;
        int count = 0;
        int wrong = 0;
        while ( ( specs = readdir( sourceDirectory ) ) != nullptr )
        {
            if ( specs->d_name[0] != '.' )
            {
                if ( !processTest( specs->d_name ) )
                {
                    ++wrong;
                    success = false;
                }
                ++count;
            }
        }
        long long t1 = nanoseconds();
        cout << "  => " << count << " unit tests processed in " << (float) (t1 - t0)*1.0e-9 << " sec.";
        if ( wrong > 0 )
            cout << endl << "      ...in which " << wrong << " went wrong :-(";
        cout << endl;
        closedir( sourceDirectory );
    }
    return success;
}

bool UnitTester::processTest( const string& testFile )
{
    string parentPath ( folderPath_ );
    if ( parentPath.back() != '/' )
        parentPath += "/";
    string referencePath = parentPath + "reference/" + testFile.substr( 0, testFile.find_last_of(".") ) + ".asco.txt";
    ifstream reference;
    reference.open( referencePath, std::ifstream::in );
    if ( reference.fail() )
    {
        cout << "  Error: could not open " << referencePath << "reference file!" << endl;
        return false;
    }
    cout << "  Processing " << testFile << "...";
    if ( !wrapper_.import( parentPath + "source/" + testFile ) )
        return false;
    ostringstream& freshResult = wrapper_.getCurrentSerialization();
    bool result = compareFiles( reference, freshResult );
    char green [] = "\033[1;32m";
    char normal [] = "\033[0m";
    if ( !result )
    {
        string difference( parentPath + "difference/");
        struct stat st;
        if ( stat( difference.c_str(), &st ) != 0 )
        {
            mkdir( difference.c_str(), 0777 );
        }
        string ascographFile = testFile.substr( 0, testFile.find_last_of(".") ) + "_diff.asco.txt";
        difference += ascographFile;
        
        FILE* output = fopen ( difference.c_str(), "w" );
        if ( output )
        {
            string content = freshResult.str();
            fprintf( output, "%s", content.c_str() );
            fclose( output );
        }
    }
    else
        cout << green << " ✔︎" << normal << endl;
    
    return result;
}

bool UnitTester::compareFiles( const ifstream& ref, const ostringstream& import ) const
{
    string tmp(std::istreambuf_iterator<char>(ref.rdbuf()), (istreambuf_iterator<char>()));
    stringstream reference ( tmp );
    tmp = import.str();
    stringstream fresh ( tmp );
    
    bool ready = false;
    std::string line;
    int leftLine = 0;
    while ( getline( reference, line ) )
    {
        ++leftLine;
        if ( line == "; start" )
        {
            ready = true;
            break;
        }
    }
    if ( !ready )
        return false;
    int rightLine = 0;
    while ( getline( fresh, line ) )
    {
        ++rightLine;
        if ( line == "; start" )
        {
            break;
        }
    }
    std::string left, right;
    bool identical = true;
    char red [] = "\033[1;31m";
    char normal [] = "\033[0m";
    while ( getline( reference, left ) && getline( fresh, right ) )
    {
        ++rightLine;
        // Remove BOM
        if ((left[0] & 0xFF) == 0xEF &&
            (left[1] & 0xFF) == 0xBB &&
            (left[2] & 0xFF) == 0xBF)
        {
            left.erase(0,3);
        }
        // Remove BOM
        if ((right[0] & 0xFF) == 0xEF &&
            (right[1] & 0xFF) == 0xBB &&
            (right[2] & 0xFF) == 0xBF)
        {
            right.erase(0,3);
        }
        
        if ( left[0] == ';' && right[0] == ';' )
            continue;
        left = left.substr( 0, left.find(" ;") );
        right = right.substr( 0, right.find(" ;") );
        if ( left != right )
        {
            cout << red << " ✘" << normal << endl;
            cout << "    Regression found at line " << rightLine << "!" << endl
                 << "    expected: '" << left << endl
                 << "       found: '" << right << endl;
            identical = false;
            break;
        }
    }
    return identical;
}
