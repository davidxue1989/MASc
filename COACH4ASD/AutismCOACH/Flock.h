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
#ifndef __FLOCK
#define __FLOCK

// Babak:
// a rand function that generates numbers between 0 and 1 (but not 0 and 1) (0,1) (not [0,1])
#define drand48_open_interval() ((double(rand()+1)/(RAND_MAX+2)))


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <math.h>

#define        Abs(A)          ((A)<0?-(A):(A)) //dx nov 22, 2012


#define STRAY_PEN 1


#define COLL_PEN 1


#define DEFAULT_GAMMAD 1.0
#define DEFAULT_GAMMAU 60.0
#define DEFAULT_GAMMAC 20.0
#define DEFAULT_GAMMAG 20.0

#define TOP_PEN 1
#define INT_PEN 1

#include "speck.h"
#include "color.h"
#include "icomp.h"


#define vcastd(A) (* ((double *) A))
double sigmoid(double a, double d, double x);

void swap(int & x, int &y);

void drawlineBresenham(unsigned char *s, int nx, int ny, int x0, int y0, int x1, int y1, unsigned char rval, unsigned char gval, unsigned char bval);
//void drawPoint(unsigned char *s, int nx, int ny, int x, int y, unsigned char rval, unsigned char gval, unsigned char bval); // added by Babak

class Flock {
 public:
  // constructors
  Flock(int numspecks, double cdist, double sdist, double idist, Color & fco, Color & fsigco, double fw=1.0, bool enablecdyn=true);
  Flock(Flock & h);
  ~Flock();//dx may 30, 2012: memory leak issue

  void setTo(Flock *fl);
  void randomize(int nx, int ny);

  double projectForward(int nx, int ny, double posdynn, Color cdyn, double depdyn);
  double compute_likelihood(Color *im, int nx, int ny);
  double compute_weight(double proposal_weight, Color *im, int nx, int ny, IPixel * otherflockmean=NULL);
  double strayPenalty();
  double topPenalty(int, int);
  double interactionPenalty(IPixel * otheflockmean);
  double collisionPenalty();
  double setToSampleFrom(Speck **speckdata, double *ll, double *cll, int numdata, 
			 int nx, int ny, bool getcolor=false);
  void computeMean();
  void computeMeanCol();
  void getMinY(int & minx, int & miny, int ny);
  int inside(int x, int y);
  bool isHand();
  bool isTowel();
  void print(FILE *);
  void scan(FILE *);
  void printParticle(FILE *);
  void scanParticle(FILE *);
  void drawOnImage(unsigned char *outim, int nx, int ny, 
		   unsigned char rval, unsigned char bval, unsigned char gval, int thick=1);
  

  // some Specks
  Speck **specks;
  int numspecks;
  // mean position
  int xo, yo;
  // mean velocity
  double uo, vo;
  // mean depth
  double depo;
  //mean color
  Color co, sigco;
  // variable to hold dynamics likelihood
  // computed during projection
  double dynll;

  // relative weight of flock color model vs. speck local color model
  // a number [0,1] where 1 means local color model is not used
  double flock_weight;

  // if true, the specks have dynamics in the color space
  bool enableColorDynamics;
  // collision and stray distances and interaction distance
  double colldist, straydist, intdist;
  // squared versions
  double colldist2, straydist2, intdist2;

  // weights for different terms in dynamics
  double gammad,gammau,gammac,gammag;

  // likelihood
  double ll;
};


#endif

