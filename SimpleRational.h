//
// .-. . . .-. .-. .-. .-. .-. .-. .-.
// |-| |\|  |  |-  `-. |   | | |-  | |
// ` ' ' `  '  `-' `-' `-' `-' '   `-'
//
// .-. .  . .-. .-. .-. .-. .-. .-.
//  |  |\/| |-' | | |(   |  |-  |(
// `-' '  ` '   `-' ' '  '  `-' ' '
//
//  SimpleRational.h
//
//  Created by Robert Piéchaud on 07/10/15.
//  Copyright (c) 2017 Antescofo. All rights reserved.
//

#ifndef __SIMPLE_RATIONAL__
#define __SIMPLE_RATIONAL__

#include <stdio.h>
#include <iostream>

class SimpleRational
{
public:
    SimpleRational( int n = 0, int d = 1);
    SimpleRational( SimpleRational& r );
    ~SimpleRational();
    
    SimpleRational& operator*=( const SimpleRational& r );
    SimpleRational& operator+=( const SimpleRational& r );
    SimpleRational& operator-=( const SimpleRational& r );
    
    SimpleRational& add(const SimpleRational& r);
    SimpleRational& subtract(const SimpleRational& r);
    SimpleRational& multiply(const SimpleRational& r);
    SimpleRational& divide( const SimpleRational& r );
    float toFloat() const;
    
private:
    void reduce();
    
private:
    int n_;
    int d_;
};

#endif // __SIMPLE_RATIONAL__
