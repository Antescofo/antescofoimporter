//
// .-. . . .-. .-. .-. .-. .-. .-. .-.
// |-| |\|  |  |-  `-. |   | | |-  | |
// ` ' ' `  '  `-' `-' `-' `-' '   `-'
//
// .-. .  . .-. .-. .-. .-. .-. .-.
//  |  |\/| |-' | | |(   |  |-  |(
// `-' '  ` '   `-' ' '  '  `-' ' '
//
//  UnitTester.h
//
//  Created by Robert Piéchaud on 21/05/15.
//  Copyright (c) 2015 ircam. All rights reserved.
//

#ifndef _ANTESCOFO_IMPORTER_UNITTESTER_
#define _ANTESCOFO_IMPORTER_UNITTESTER_

#include <stdio.h>
#include <string>
#include <fstream>

namespace antescofo
{
    class ImporterWrapper;
    
    class UnitTester
    {
    public:
        UnitTester( ImporterWrapper& wrapper, const std::string& unitaryTestFolder );
        virtual ~UnitTester();
        
        bool run();
        
    private:
        bool processTest( const std::string& path );
        bool compareFiles( const std::ifstream& reference, const std::ostringstream& import ) const;
        
    private:
        ImporterWrapper&    wrapper_;
        const std::string   folderPath_;
    };
}

#endif // _ANTESCOFO_IMPORTER_UNITTESTER_