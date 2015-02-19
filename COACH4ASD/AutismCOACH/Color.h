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
#ifndef __COLOR
#define __COLOR
#include <stdio.h>
#include "math.h"

#define _CP  1e-20
#define NEGLOGCP (-log(_CP))
#define UNDEFINED_HUE     -1000000

#ifndef min
#define min(A,B) ((A)<(B) ? (A) : (B))
#endif
#define        max3(A,B,C)      ((A)<(B)? ((B) < (C) ? (C) : (B)) : ((A) < (C) ? (C) : (A)))
#define        min3(A,B,C)      ((A)>(B)? ((B) > (C) ? (C) : (B)) : ((A) > (C) ? (C) : (A)))
#define        max4(A,B,C,D)    max(A,max3(B,C,D))
#define        min4(A,B,C,D)    min(A,min3(B,C,D))    

class Color
{
 public:
  Color();
  Color(unsigned char r, unsigned char g, unsigned char b);
  Color(double h, double s, double v);
  Color(Color *c);

  void zero();
  void setColor(Color &c);
  void setColor(double, double, double);
  double colll(Color &co, Color &sigco);
  double colllp(Color &co, Color &sigco, double neglogprior=NEGLOGCP);
  void print(FILE *fp);
  void scan(FILE *fp);
  double h,s,v;
};
Color ***buildHSVConverter();
void rgb2hsv(double r, double g, double b, double *h, double *s, double *v);
#endif

