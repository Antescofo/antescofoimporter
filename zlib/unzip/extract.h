//
//  extract.h
//  antescofo_converter
//
//  Created by baba on 24/05/15.
//  Copyright (c) 2017 Antescofo. All rights reserved.
//

#ifndef antescofo_converter_extract_h
#define antescofo_converter_extract_h

#include <sstream>

bool extractOneFile( const char* zipFileName, const char* filenameToExtract, std::ostringstream& stream, bool silent = true );

#endif
