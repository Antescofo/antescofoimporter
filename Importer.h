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
//  Created by Robert Piéchaud on 09/05/15.
//  Copyright (c) 2017 Antescofo. All rights reserved.
//

#ifndef _ANTESCOFO_IMPORTER_
#define _ANTESCOFO_IMPORTER_

#include <stdio.h>
#include <vector>
#include <string>

namespace antescofo
{
    class Importer
    {
    public:
        Importer() {}
        virtual ~Importer() {}
        
        virtual bool import() = 0;
        virtual bool import( const std::vector<int>& tracks ) = 0;
        virtual void clear() = 0;
        virtual bool queryTracks( std::vector<std::string>& tracks ) = 0;
        virtual bool queryScoreInfo() = 0;
    };
}

#endif //_ANTESCOFO_IMPORTER_
