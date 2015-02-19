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
#ifndef __ICOMP
#define __ICOMP

#include "../VidPlayerDefines.h" // for define (or not define) IMAGE_SIZE_ORIGINAL

#ifdef IMAGE_SIZE_ORIGINAL // 640*480 
	// has to be large enough that we dont' get memory overflow. but how large?
	#define MIN_COMP_SIZE 120
	#define MAX_COMP_SIZE 45000
#else// 320x240
	#define MIN_COMP_SIZE 60
	#define MAX_COMP_SIZE 30000
#endif

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "PFHTDefines.h"
#include "../VidPlayerDefines.h"

#ifdef IMAGE_SIZE_ORIGINAL // 640*480 
	// has to be large enough that we dont' get memory overflow. but how large?
	#define MAXIMUM_COMPS 2500
#else //320*240
	#define MAXIMUM_COMPS 400
#endif

class IRegion;

class IPixel
{
 public:
  IPixel() { x = y = 0; };
  IPixel(int xx, int yy) { setTo(xx,yy);};
  double distFrom(IPixel & c) { return sqrt(((double) ((c.x-x)*(c.x-x) + (c.y-y)*(c.y-y))));};
  void zero() { x = y = 0; };
  void setTo(IPixel & c) { x = c.x; y = c.y;};
  void setTo(int xx, int yy) { x = xx; y = yy;};
  bool isEqualTo(IPixel & c) { return ((x == c.x) && (y == c.y)); };
  bool in(int tlx, int tly, int brx, int bry) { return (x > tlx && x < brx && y > tly && y < bry);};
  bool in(IPixel & tl, IPixel & br) { return (in(tl.x,tl.y,br.x,br.y));};
  bool in(IRegion & ir); 
  void drawOnImage(unsigned char *outim, int nx, int ny, 
		   unsigned char rval, unsigned char bval, unsigned char gval, 
		   int pfr, int pto);
  void print(FILE *fp);
  void scan(FILE *fp);
  int x,y;
};
class IRegion
{
 public:
  IRegion() { tl.x = tl.y = br.x = br.y = 0;};
  IRegion(IPixel & tll, IPixel & brr) { setTo(tll,brr); };
  IRegion(int tlx, int tly, int brx, int bry) { tl.setTo(tlx,tly); br.setTo(brx,bry);};
  void setTo(IPixel & tll, IPixel & brr) { tl.setTo(tll);  br.setTo(brr); };
  void setTo(int tlx, int tly, int brx, int bry) { tl.setTo(tlx,tly); br.setTo(brx,bry); };
  bool in(IPixel & pt) { return pt.in(tl,br); };
  void zero() {tl.zero(); br.zero(); };
  int getBBArea() { int area=(br.x-tl.x)*(br.y-tl.y); assert(area>=0); return area; }; //dx nov 28, 2012
  IPixel tl, br;
};

class IComp
{
 public:
  IComp(int cps=MAX_COMP_SIZE);
  ~IComp();
  void getCentroid();
  //Justin - Oct 22, 2010
  void getCentroid(int &centroidX, int &centroidY);
  void getCentroidAndBB();
  void setTo(IComp & tc);
  void zero() {compsize = 0; center.zero(); bbox.zero();}
  IPixel * points;
  int compsize;
  IPixel center;
  IRegion bbox;
};
#endif
