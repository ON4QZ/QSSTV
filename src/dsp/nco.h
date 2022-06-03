/***************************************************************************
 *   Copyright (C) 2004 by Johan Maes - ON4QZ                              *
 *   on4qz@telenet.be                                                      *
 *   http://users.telenet.be/on4qz                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef NCO_H
#define NCO_H
#include <math.h>
//#include "appglobal.h"
#include "appdefs.h"

/**
@author Johan Maes
*/

/** Numerical Controlled Oscillator

	The next value for the sine and/or cosine is calculated each time the function is called
	
*/

class NCO  // numerical controlled oscillator
{
 public:
	/** create an instance of the NCO with a given frequency

	\warning the frequency is the normilized frequency i.e F/samplingrate;
*/
  NCO(double freq=0.5)
    {
      init(freq);
    }
  ~NCO()
    {
    }
	/** initialize the oscilator

			This function is automatically called from the constructor
			\param[in] frequency the frequency the oscillator must be running at
			\warning the frequency is the normilized frequency i.e F/samplingrate;
*/
  void init(double frequency)
    {
      w=(2*frequency*M_PI);
      b=2.*cos(w);
      s1=sin(-w);
      s2=sin(-2.*w);
      c1=sin(M_PI/2.-w);
      c2=sin(M_PI/2.-2.*w);   
    }

	/** get the sine and cosine values
		\param[out] sinVal  sine value
		\param[out] cosVal  cosine value
	*/
  void getSinCos(double &cosVal,double &sinVal)
    {
      sinVal=b*s1-s2;
      s2=s1;
      s1=sinVal;
      cosVal=b*c1-c2;
      c2=c1;
      c1=cosVal;
    }

	/** get the sine value
		\return sine value
	*/
  double getSine()
    {
      double sinVal=b*s1-s2;
      s2=s1;
      return(s1=sinVal);
    }
	/** produce the I & Q values
  \param[out] i the I component (val multiplied by a sine)
  \param[out] q the Q component (val multiplied by a cosine)
	\param[in] val the real value of the sample
 */
  void multiply(double &i, double &q,double val)
		{
      double t=b*s1-s2;
      i=val*t;
			s2=s1;
			s1=t;
			t=b*c1-c2;
			c2=c1;
      q=val*t;
			c1=t;
  	}

 private:
  double w;
  double b;
  double s1,s2;
  double c1,c2;
};




#endif
