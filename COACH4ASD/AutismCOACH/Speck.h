/*  Author: 
 *    Jesse Hoey
 *    School of Computer Science
 *    University of Dundee
 *  January 7, 2006
 *  This code comes with no guarantee, and is for research purposes only.  
 *  Commercial use strictly forbidden.  
 *  If you use this code, please reference the following paper
 *  Jesse Hoey  "Tracking using Flocks of Features, with Application to Assisted Handwashing", Proceedings of BMVC 2006, Pp 367-376
 */
#ifndef __SPECK
#define __SPECK


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <math.h>
#define PI 3.141592

// side of the speck
#define SPECKSIDE 2
// size of speck
#define SPECKSIZE 4

// prior color weight
#define PRIORCOLWEIGHT 1e-5

inline int roundA(double f) // a simple function for rounding // Babak
{
	if (f >= 0)
		return (int(f+0.5));
	else
		return (int(f-0.5));
}


#ifndef max
#define max(A,B) (((A) > (B)) ? (A) : (B))
#endif
#ifndef min
#define min(A,B) (((A) < (B)) ? (A) : (B))
#endif
#define clamp(A) max(0,min(1,A))

#include "color.h"

double drawSampleFromGaussian();
double drawSampleFromGaussian(double sig);

class Speck {
 public:
  // constructors
  Speck();
  Speck(const Speck & h);
  Speck(int x, int y, double u, double v, double depth, 
	  Color & c, Color & sigc);
  Speck(int x, int y);

  void setTo(Speck *sp);
  void setTo(Speck & sp);
  void setTo(int x, int y, double u, double v, double depth, 
	     Color & c, Color & sigc);
  double getFromImage(int x, int y, Color *cim, 
		      Color & fco, Color & fsigco, 
		      double uxo, double uyo, int nx, int ny);

  void setNoiseDefaults();
  void zero();
  void randomize(int nx, int ny, Color & meancol, Color & sigcol);
  void randomizeColor(Color & meancol, Color & sigcol);
  void randomize(int xo, int yo, int nx, int ny, Color & meancol, Color & sigcol);
  
  bool inside(int ix, int iy);
  double compute_weight(Color *im, Color & flockcol, Color & flocksigcol, 
			int nx, int ny, double flock_weight=1.0, bool logweight=false);
  double projectForward(int nx, int ny);
  double projectForward(double muo, double mvo, int nx, int ny, bool hinoise = false, bool enablecdyn=true);
  double projectForward(double muo, double mvo, int nx, int ny, double, Color &, double, bool enablecdyn=true);

  void print(FILE *fh);
  void scan(FILE *fh);
  void drawOnImage(unsigned char *im, int nx, int ny, unsigned char rval, unsigned char bval, unsigned char gval, int sidesize=SPECKSIDE);
  void drawOnImage(unsigned char *im, int nx, int ny, unsigned char rval, unsigned char bval, unsigned char gval, int pfr, int pto);

  // position of speck
  int xo, yo;
  
  // velocity of speck
  double uo, vo;
  
  // depth of speck
  double depo;
  
  // color and color variance in speck
  Color co, sigco;

  bool usedepth;

};


#endif


