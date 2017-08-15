//
// .-. . . .-. .-. .-. .-. .-. .-. .-.
// |-| |\|  |  |-  `-. |   | | |-  | |
// ` ' ' `  '  `-' `-' `-' `-' '   `-'
//
// .-. .  . .-. .-. .-. .-. .-. .-.
//  |  |\/| |-' | | |(   |  |-  |(
// `-' '  ` '   `-' ' '  '  `-' ' '
//
//  SimpleRational.cpp
//
//  Created by Robert Piéchaud on 07/10/15.
//  Copyright (c) 2017 Antescofo. All rights reserved.
//

#include "SimpleRational.h"

SimpleRational::SimpleRational( int n, int d )
{
    n_ = d < 0 ? -n : n;
    d_ = d < 0 ? -d : d;
    reduce();
}

SimpleRational::SimpleRational( SimpleRational& r ):
    n_ ( r.n_ ),
    d_ ( r.d_ )
{
    //NOTHING
}

SimpleRational::~SimpleRational()
{
    //NOTHING
}

SimpleRational& SimpleRational::add(const SimpleRational& a)
{
    n_ = a.n_ * d_ + a.d_ * n_;
    d_ = a.d_ * d_;
    reduce();
    return *this;
}

SimpleRational& SimpleRational::subtract(const SimpleRational& s)
{
    n_ = s.d_ * n_ - d_ * s.n_;
    d_ = s.d_ * d_;
    reduce();
    return *this;
}

SimpleRational& SimpleRational::multiply( const SimpleRational& m )
{
    n_ = m.n_ * n_;
    d_ = m.d_ * d_;
    reduce();
    return *this;
}

SimpleRational& SimpleRational::divide(const SimpleRational& v)
{
    n_ = v.d_ * n_;
    d_ = d_ * v.n_;
    reduce();
    return *this;
}

void SimpleRational::reduce()
{
    int n = n_ < 0 ? -n_ : n_;
    int d = d_;
    int largest = n > d ? n : d;
    
    int gcd = 0;                                 // greatest common divisor
    
    for ( int loop = largest; loop >= 2; loop-- )
        if (n_ % loop == 0 && d_ % loop == 0)
        {
            gcd = loop;
            break;
        }
    
    if (gcd != 0)
    {
        n_ /= gcd;
        d_ /= gcd;
    }
}

float SimpleRational::toFloat() const
{
    return (float) n_/d_;
}

SimpleRational& SimpleRational::operator*=( const SimpleRational& r )
{
    return this->multiply( r );
}

SimpleRational& SimpleRational::operator+=( const SimpleRational& r )
{
    return this->add( r );
}

SimpleRational& SimpleRational::operator-=( const SimpleRational& r )
{
    return this->subtract( r );
}
