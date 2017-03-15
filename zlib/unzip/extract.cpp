/*
   miniunz.c
   Version 1.01h, December 28th, 2009

   Copyright (C) 1998-2009 Gilles Vollant
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include "extract.h"

#if 1 //def unix
# include <unistd.h>
# include <utime.h>
#else
# include <direct.h>
# include <io.h>
#endif

#include "unzip.h"

#define CASESENSITIVITY (0)
#define WRITEBUFFERSIZE (8192)
#define MAXFILENAME (256)

#ifdef WIN32
#define USEWIN32IOAPI
#include "iowin32.h"
#endif

using namespace std;

int do_extract_currentfile( unzFile uf, const int* popt_extract_without_path, int* popt_overwrite, ostringstream& stream, bool silent  )
{
    char filename_inzip[256];
    char* filename_withoutpath;
    char* p;
    int result = UNZ_OK;
    void* buf;
    uInt size_buf;

    unz_file_info file_info;
    result = unzGetCurrentFileInfo( uf, &file_info, filename_inzip, sizeof(filename_inzip), NULL, 0, NULL, 0 );

    if ( result != UNZ_OK )
    {
        if ( !silent )
            printf("error %d with zipfile in unzGetCurrentFileInfo\n", result );
        return result;
    }

    size_buf = WRITEBUFFERSIZE;
    buf = (void*)malloc(size_buf);
    if (buf==NULL)
    {
        if ( !silent )
            printf("Error allocating memory\n");
        return UNZ_INTERNALERROR;
    }

    p = filename_withoutpath = filename_inzip;
    while ((*p) != '\0')
    {
        if (((*p)=='/') || ((*p)=='\\'))
            filename_withoutpath = p+1;
        p++;
    }

    const char* write_filename;

    if ((*popt_extract_without_path)==0)
        write_filename = filename_inzip;
    else
        write_filename = filename_withoutpath;

    result = unzOpenCurrentFilePassword( uf, NULL );
    if ( result != UNZ_OK )
    {
        if ( !silent )
            printf("error %d with zipfile in unzOpenCurrentFilePassword\n", result );
    }
    
    if ( !silent )
        printf(" extracting: %s\n", write_filename );

    do
    {
        result = unzReadCurrentFile( uf, buf, size_buf );
        if ( result < 0 )
        {
            if ( !silent )
                printf("error %d with zipfile in unzReadCurrentFile\n", result );
            break;
        }
        if ( result > 0 )
        {
            stream.write( (const char*) buf, result);
        }
    }
    while ( result > 0 );

    if ( result == UNZ_OK )
    {
        result = unzCloseCurrentFile ( uf );
        if ( result != UNZ_OK )
        {
            if ( !silent )
                printf( "error %d with zipfile in unzCloseCurrentFile\n", result );
        }
    }
    else
        unzCloseCurrentFile( uf );

    free(buf);
    return result;
}

int do_extract_onefile( unzFile uf, const char* filenameToExtract, int opt_extract_without_path, int opt_overwrite, ostringstream& stream, bool silent )
{
    if ( unzLocateFile( uf, filenameToExtract, CASESENSITIVITY ) != UNZ_OK )
    {
        if ( !silent )
            printf( "file %s not found in the zipfile\n", filenameToExtract );
        return 2;
    }

    if ( do_extract_currentfile( uf, &opt_extract_without_path, &opt_overwrite, stream, silent ) == UNZ_OK )
        return 0;
    return 1;
}

bool extractOneFile( const char* zipFileName, const char* filenameToExtract, ostringstream& stream, bool silent )
{
    unzFile uf = NULL;
    stream.clear();
    char filenameTry [MAXFILENAME+16] = "";
    if ( zipFileName != NULL)
    {
        
#ifdef USEWIN32IOAPI
        zlib_filefunc_def ffunc;
#endif
    
        strncpy( filenameTry, zipFileName, MAXFILENAME - 1 );
        filenameTry[ MAXFILENAME ] = '\0';
        
#ifdef USEWIN32IOAPI
        fill_win32_filefunc( &ffunc );
        uf = unzOpen2( zipFileName, &ffunc );
#else
        uf = unzOpen( zipFileName );
#endif
        if ( uf == NULL )
        {
            strcat( filenameTry, ".zip" );
#ifdef USEWIN32IOAPI
            uf = unzOpen2( filenameTry, &ffunc );
#else
            uf = unzOpen( filenameTry );
#endif
        }
        if ( uf != NULL )
            return ( do_extract_onefile( uf, filenameToExtract, 0, 1, stream, silent ) == UNZ_OK );
    }
    return false;
}
