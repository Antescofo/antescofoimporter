/*
  MusicXML Library
  Copyright (C) Grame 2006-2013

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Grame Research Laboratory, 11, cours de Verdun Gensoul 69002 Lyon - France
  research@grame.fr
*/

#include "rational.h"
#include <sstream>
#include <cmath>
#include <stdlib.h>
#include <string.h>

using namespace std;

std::ostream& operator<<(std::ostream& o, rational const& number)
{
    number.show(o);
    return o;
}


// Read 3 or 5/2
std::istream& operator>>(std::istream& i, rational& number)
{
    number.load(i);
    return i;
}

void rational::show(std::ostream& o) const
{
    o << fNumerator;
    if ((fDenominator != 1) && (fNumerator != 0))
        o << "/" << fDenominator;
}

void rational::load(std::istream& i)
{
    if (!i)
        return;
    
    long int num = 0, denom = 1;
    
    if(!(i >> num))
        return;
    
    if (i.peek() == '/')
    {
        // there's an /
        i.ignore(1); // or is.get() to skip over it
        
        if(!(i >> denom))  //TODO: en fait, il faudrait ne parser que l'entier
            return;
    }
    
    this->set(num, denom);
}

string rational::toString() const
{
    ostringstream res;
    res << (*this);
    return res.str();
}

rational::operator string() const
{
	return toString();
}

rational::operator double() const
{ 
	return toDouble(); 
}

rational::operator float() const
{ 
	return toFloat(); 
}

/*rational::operator int() const
{ 
	const double x = toDouble();
	return ((int)floor(x + 0.5f));
}*/

// Read 3, 3/4 or 3/4+2+5/7
rational::rational(const string &str)
{
    // convert string as stringstream;
    std::stringstream sstr; sstr << str;
    // read first rational
    sstr >> (*this);
    // read +2+5/7 (char '+' and further rational)
    if (sstr)
    {
        rational r1;
        while (sstr && (sstr.peek() == '+'))
        {
            sstr.ignore(1); // ignore +
            // (try to) read r1
            sstr >> r1;
            if (sstr)
                 (*this) += r1;
        }
    }
}

rational::rational(long int num, long int denom) : fNumerator(num), fDenominator(denom ? denom : 1) // don't allow zero denominators!
{
    /*rationalise();*/
}

rational::rational(const rational& d)
{
    fNumerator = d.fNumerator;
    fDenominator = d.fDenominator;
}

rational rational::operator +(const rational &dur) const {
    return rational(fNumerator * dur.fDenominator + dur.fNumerator * fDenominator, fDenominator * dur.fDenominator);
}

/*
rational rational::operator -(const rational &dur) const {
    return rational(fNumerator * dur.fDenominator - dur.fNumerator * fDenominator, fDenominator * dur.fDenominator);
}*/

rational rational::operator *(const rational &dur) const {
    return rational(fNumerator * dur.fNumerator, fDenominator * dur.fDenominator);
}

rational rational::operator /(const rational &dur) const {
    return rational(fNumerator * dur.fDenominator, fDenominator * dur.fNumerator);
}

rational rational::operator *(int num) const {
	return rational(fNumerator * num, fDenominator);
}

rational rational::operator /(int num) const {
	return rational(fNumerator, fDenominator * num);
}

rational& rational::operator +=(const rational &dur)
{
    if(fDenominator == dur.fDenominator) {
		fNumerator += dur.fNumerator;
	}
    else
    {
        // Si un dénominateur est un multiple de l'autre, alors on prend le plus petit
        if (fDenominator % dur.fDenominator == 0)
        {
            fNumerator += dur.fNumerator * (fDenominator / dur.fDenominator);
        }
        else if (dur.fDenominator % fDenominator == 0)
        {
            // Change fraction denominator
            fDenominator = dur.fDenominator;
            fNumerator *= (dur.fDenominator / fDenominator);
            // Add other numerator
            fNumerator += dur.fNumerator;
        }
        else
        {
            fNumerator = fNumerator * dur.fDenominator + dur.fNumerator * fDenominator;
            fDenominator *= dur.fDenominator;
            rationalise();
        }
	}
    /*rationalise();*/
    return (*this);
}

/*
rational& rational::operator -=(const rational &dur)
{
	if(fDenominator == dur.fDenominator) {
		fNumerator -= dur.fNumerator;
	} else {
		fNumerator = fNumerator * dur.fDenominator - dur.fNumerator * fDenominator;
		fDenominator *= dur.fDenominator;
    }
	return (*this);
}*/

rational& rational::operator *=(const rational &dur)
{
    fNumerator   *= dur.fNumerator;
    fDenominator *= dur.fDenominator;
    /*rationalise();*/
    return (*this);
}

rational& rational::operator /=(const rational &dur)
{
    fNumerator   *= dur.fDenominator;
    fDenominator *= dur.fNumerator;
    return (*this);
}

rational& rational::operator =(const rational& dur) {
    fNumerator   = dur.fNumerator;
    fDenominator = dur.fDenominator;
    /*rationalise();*/
    return (*this);
}

bool rational::operator >(const rational &dur) const
{
    // a/b > c/d if and only if a * d > b * c.
    return ((fNumerator * dur.fDenominator) > (fDenominator * dur.fNumerator));
}

bool rational::operator <(const rational &dur) const
{
    // a/b < c/d if and only if a * d < b * c.
    return ((fNumerator * dur.fDenominator) < (fDenominator * dur.fNumerator));
}

bool rational::operator ==(const rational &dur) const
{
    // a/b < c/d if and only if a * d < b * c.
    return ((fNumerator * dur.fDenominator) == (fDenominator * dur.fNumerator));
}

bool rational::operator >(double num) const
{	
	return (toDouble() > num);
}

bool rational::operator >= (double num) const
{
	return (toDouble() >= num);
}

bool rational::operator <(double num) const
{
	return (toDouble() < num);
}

bool rational::operator <=(double num) const
{
	return (toDouble() <= num);
}

bool rational::operator ==(double num) const
{
	return (toDouble() == num);
}

// gcd(a, b) calculates the gcd of a and b using Euclid's algorithm.
long int rational::gcd(long int a1, long int b1)
{
    long int r;

    long int a = abs(a1);
    long int b = abs(b1);

    if (!(a == 0) || (b == 0)){
        while (b > 0){
            r = a % b;
            a = b;
            b = r;
        }
        return a;
    }
    return 1;
}

void rational::rationalise()
{
    if (fNumerator == 0)
        fDenominator = 1;
    else
    {
        long int const g = gcd(fNumerator, fDenominator);
        fNumerator /= g;
        fDenominator /= g;
    }
}

double rational::toDouble() const
{
    return (fDenominator != 0) ? ((double)fNumerator/(double)fDenominator) : 0;
}

float rational::toFloat() const
{
    return (fDenominator != 0) ? ((float)fNumerator/(float)fDenominator) : 0;
}
