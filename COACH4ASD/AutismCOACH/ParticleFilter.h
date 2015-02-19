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
#ifndef __PFILTER
#define __PFILTER

#define MAXK 4

#define INFINITY 100000000.0

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <math.h>

#include "flock.h"
#include "icomp.h"

int drawaSample(int nump, double *cdist);
void find_closest_handpos(int modelorder, int * hassoc, double **gpmeans, int *numgppts, Speck & meanlhp, Speck & meanrhp, bool & lhpres, bool &rhpres, int nx, int ny);
void selsort(int *x, int n, int *ix);
double computedistance(Speck & hp, double *mn);
void assignto(Speck & hp, double *mn);


class ParticleFilter {
 public:
  ParticleFilter(int nump, int fs, double cdist, double sdist, double idist, Color & fco, Color & fsigco,double fw=1,bool enablecdyn=false);
  ~ParticleFilter();//dx may 30, 2012: memory leak issue
  void randomizeParticles(int nx, int ny);
  int drawSample();
  void update(Color *im, int nx, int ny);
  double normaliseWeights(double &wsum, bool getbest=false);
  void normaliseResample();
  int inside(int x, int y, int ns);
  void updateData(Color *cim, int nx, int ny, 
		  Speck **data1, double *ll1, double *cll1, 
		  int numdata1, double alpha1,
		  double & dataweight, double & priorweight, 
		  double posdyn, Color & cdyn, double depdyn,
		  double stren=10, int verbose=0, IPixel * mop=NULL);
  
  void estimateMeans(bool & tpres, bool & lhpres, bool & rhpres, Speck & meantp, Speck & meanlhp, Speck  & meanrhp, int nx, int ny);
  void estimateMean(Speck & meanp);
  void estimateMean(IPixel & meanp);
  void estimateVariance(IPixel & meanp, double & sigxo, double  & sigyo);

  void estimateMinY(Speck & minp, int ny);
  void estimateMinY(IPixel & minp, int ny);

  void printWeightLimits();
  void resetWeightLimits();
  void setWeightLimits(double maxd,double mind,double maxp,double minp);
  void setLearnWeightLimits(int lwl=0);

  void drawParticles(unsigned char *im, unsigned char *outim, int nx, int ny, unsigned char rval, unsigned char gval, unsigned char bval, int np=-1);
  void drawBestParticle(unsigned char *im, unsigned char *outim, int nx, int ny, unsigned char rval, unsigned char gval, unsigned char bval, int thick=1);

  void printParticles(FILE *fh);

  void print(FILE *fh);
  void scan(FILE *fh);
  

  int num_particles;
  int best_particle;
  double best_particle_weight;
  double maxdataweight,mindataweight;
  double maxpriorweight,minpriorweight;
  int learnweightlimits;

  Flock **ptc, **newptc;
  double *ptcweight;
  double *cumdist;

  ////dxdebug nov 28, 2012
  //Flock **ptc_old, **ptc_new;
};

#endif
