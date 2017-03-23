/*
  MusicXML Library
  Copyright (C) Grame 2006-2013

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.

  Grame Research Laboratory, 11, cours de Verdun Gensoul 69002 Lyon - France
  research@grame.fr
*/

#ifndef __rational__
#define __rational__

#include <string>
#include <iostream>

/*!
\brief	Rational number representation.
*/
class rational {

   private:
        long int fNumerator;
        long int fDenominator;        

        // Used by rationalise()
        static long int gcd(long int a, long int b);
 
    public:
	
        rational(long int num = 0, long int denom = 1);
        rational(const rational& d);
        rational(const std::string &str);
    
        long int getNumerator()	const		{ return fNumerator; }
        long int getDenominator() const		{ return fDenominator; }
        void setNumerator(long int d)		{ fNumerator = d; }
        void setDenominator(long int d) 	{ fDenominator = (d ? d : 1); }
        void set(long int n, long int d)  { fNumerator = n; fDenominator = (d ? d : 1); }

        // Used to "rationalise" rational.
        void rationalise();
    
        rational operator +(const rational &dur) const;
        //rational operator -(const rational &dur) const;
        //! Useful for notes with dots.
        rational operator *(const rational &dur) const; 
        rational operator /(const rational &dur) const;
        // (i.e. dur * 3/2 or dur * 7/4)
  
        rational operator *(int num) const; 
        rational operator /(int num) const;
  
        rational& operator +=(const rational &dur);
        //rational& operator -=(const rational &dur);
        //! Useful for notes with dots.
        rational& operator *=(const rational &dur); 
        rational& operator /=(const rational &dur);
        // (i.e. dur * 3/2 or dur * 7/4)

        rational& operator *=(long int num) { fNumerator *= num; return *this; }
        rational& operator /=(long int num) { fDenominator *= num; return *this; }
 
        rational& operator =(const rational& dur);
    
        bool operator >(const rational &dur) const;
        bool operator >=(const rational &dur) const 	{return !(*this < dur);}
        bool operator <(const rational &dur) const;
        bool operator <=(const rational &dur) const 	{return !(*this > dur);}
            
        bool operator ==(const rational &dur) const;
        bool operator !=(const rational &dur) const	{return !(*this == dur);}
      
        bool operator >	(double num) const;
        bool operator >=(double num) const;
        bool operator <	(double num) const;
        bool operator <=(double num) const;    
        bool operator ==(double) const;
    
        virtual void show(std::ostream&) const;
        virtual void load(std::istream&);

        operator std::string () const;
		operator double () const;
		operator float () const;
        operator bool () const;
		//operator int () const;

        std::string toString() const;
        double	toDouble() const;
        float	toFloat() const;
        //int		toInt() const;
};

std::ostream& operator<<(std::ostream&, rational const&);

std::istream& operator>>(std::istream&, rational&);

#endif
