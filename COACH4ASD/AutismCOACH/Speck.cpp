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
#include "stdafx.h"
#include "speck.h"
//#include <math.h> // Babak

//default constructor
Speck::Speck()
{
  zero();
}
// copy constructor
Speck::Speck(const Speck & h)
{
  xo = h.xo; yo = h.yo;
  uo = h.uo; vo = h.vo;
  depo = h.depo;
  co = h.co; 
  sigco = h.sigco;
  usedepth = h.usedepth;
}
Speck::Speck(int x, int y,  
	     double u, double v, double depth, 
	     Color & c, Color & sigc)
{
  setTo(x,y,u,v,depth,c,sigc);
}
Speck::Speck(int x, int y) 
{
  Color c, sigc;
  setTo(x,y,0,0,0,c,sigc);
}
  
void Speck::setTo(Speck *hp)
{
  xo = hp->xo; yo = hp->yo;
  uo = hp->uo; vo = hp->vo;
  depo = hp->depo;
  co = hp->co; sigco = hp->sigco;

  usedepth = hp->usedepth;
}

void Speck::setTo(Speck  & hp)
{
  xo = hp.xo; yo = hp.yo;
  uo = hp.uo; vo = hp.vo;
  depo = hp.depo;
  co = hp.co; sigco = hp.sigco;

  usedepth = hp.usedepth;
}

void Speck::setTo(int x, int y,  
	     double u, double v, double depth, 
	     Color & c, Color & sigc)
{
  xo = x; yo = y;
  uo = u;  vo = v;
  depo = depth;
  co = c; sigco = sigc;
  usedepth = false;
}
void Speck::zero()
{
  xo = yo = 0;
  uo = vo = 0;
  depo = 0.0;
  co.zero();
  usedepth = false;
}
// randomly assign non-moving particles
// with a-priori depth and color models
void Speck::randomize(int nx, int ny, Color & meancol, Color & sigcol) 
{
  xo = (int) floor((nx-SPECKSIDE+1)*(rand()/(RAND_MAX+1.0)));
  yo = (int) floor((ny-SPECKSIDE+1)*(rand()/(RAND_MAX+1.0)));
  uo = 0; vo = 0;
  depo = 0;
  randomizeColor(meancol,sigcol);
}
void Speck::randomizeColor(Color & meancol, Color & sigcol)
{
	//Note: no need to use circular stats for H channel here because we are just "jittering" an H degree value +/- a bit
	//(and not calculating a mean or variance of pixels)
  co.h = meancol.h + drawSampleFromGaussian(sqrt(sigcol.h));
  sigco.h = sigcol.h;
  co.v = clamp(meancol.v + drawSampleFromGaussian(sqrt(sigcol.v)));
  sigco.v = sigcol.v;
  co.s = clamp(meancol.s + drawSampleFromGaussian(sqrt(sigcol.s)));
  sigco.s = sigcol.s;
}
//  assigns random speck in nx by ny region centered at mxo, myo with color drawn from  meancol,sigcol normal
void Speck::randomize(int mxo, int myo, int nx, int ny, Color & meancol, Color & sigcol)
{
  xo = ((int) floor((nx-SPECKSIDE+1)*(rand()/(RAND_MAX+1.0))))-nx/2+mxo;
  yo = ((int) floor((ny-SPECKSIDE+1)*(rand()/(RAND_MAX+1.0))))-ny/2+myo;
  uo = vo = 0;
  depo = 0;
  randomizeColor(meancol,sigcol);
}
// checks if (x,y) is inside this Speck's area
bool Speck::inside(int ix, int iy)
{
  return ((xo == ix || xo == ix-1) &&
	  (yo == iy || yo == iy-1));
}

// computes the speck at x,y from image cim (size nx by ny)
//  with generic color model (e.g. from a flock) fco, fsigco
// and velocity uxo, uyo
double Speck::getFromImage(int x, int y, Color *cim, 
			   Color & fco, Color & fsigco, 
			   double uxo, double uyo, int nx, int ny)
{
  int ii,jj,index;
  double hll(0.0), hsin(0.0), hcos(0.0);
	double dDegToRadRatio = 3.14159265 / 180.0;

  xo = x;
  yo = y;

  uo = uxo;
  vo = uyo;

  co.zero();
  sigco.zero();
  
  for (ii=0; ii<SPECKSIDE; ii++) {
    for (jj=0; jj<SPECKSIDE; jj++) {
      index = min(ny,(y+ii))*nx+min(nx,x+jj);
      //co.h += cim[index].h;
			hsin += sin(cim[index].h * dDegToRadRatio);
			hcos += cos(cim[index].h * dDegToRadRatio);
      co.s += cim[index].s;
      co.v += cim[index].v;
      /*
      sigco.h += (cim[index].h)*(cim[index].h);
      sigco.s += (cim[index].s)*(cim[index].s);
      sigco.v += (cim[index].v)*(cim[index].v);
      */
      hll += cim[index].colll(fco,fsigco);
    }
  }
  co.h = atan2(hsin, hcos) / dDegToRadRatio;  //co.h/SPECKSIZE;
  co.s = co.s/SPECKSIZE;
  co.v = co.v/SPECKSIZE;
  // these are variances - no need to take square root
  /*
  sigco.h = sigco.h/SPECKSIZE-co.h*co.h+PRIORCOLWEIGHT;
  sigco.s = sigco.s/SPECKSIZE-co.s*co.s+PRIORCOLWEIGHT;
  sigco.v = sigco.v/SPECKSIZE-co.v*co.v+PRIORCOLWEIGHT;
  */
  sigco = fsigco;
  return hll;
}

double Speck::compute_weight(Color *cim, Color & flockcol, Color & flocksigcol, 
			     int nx, int ny, double flock_weight, bool logweight)
{
  // compute weight of this particle given the hue image
  int i,j,index;
  double weight(0.0);
  int row;
  if (xo > 0 && xo < nx-SPECKSIDE+1-5 && yo > 0 && yo < ny-SPECKSIDE+1-5) {
    // weight due to local color model
    if (flock_weight < 1.0) {
      row = yo*nx;
      for (i=yo; i<yo+SPECKSIDE; i++) {
	for (j=xo; j<xo+SPECKSIDE; j++) {
	  index = row+j;
	  weight += (1.0-flock_weight)*cim[index].colllp(co,sigco);
	}
	row += nx;
      }
    }
    // weight due to flock color model
    row = yo*nx;
    for (i=yo; i<yo+SPECKSIDE; i++) {
      for (j=xo; j<xo+SPECKSIDE; j++) {
	index = row+j;
	weight += flock_weight*cim[index].colllp(flockcol,flocksigcol);
      }
      row += nx;
    }
    // also add depth weight? 
    // ...
    if (logweight)
      return weight/(SPECKSIZE);
    else
      return exp(-weight/(SPECKSIZE));
  } else {
    if (logweight) 
      return 10000.0;
    else
      return 0.0;
  }
}

double Speck::projectForward(double muo, double mvo, int nx, int ny, 
			     double dynn, Color & cdynn, double depdynn, bool enableColorDynamics)
{
  double ll, ndepo;
  Color nco;
  Color cdynnvar;

  //int ix = (int) round(xo+muo+drawSampleFromGaussian(dynn)); // Babak
  //int iy = (int) round(yo+mvo+drawSampleFromGaussian(dynn)); // Babak
  int ix = (int) roundA(xo+muo+drawSampleFromGaussian(dynn)); // Babak
  int iy = (int) roundA(yo+mvo+drawSampleFromGaussian(dynn)); // Babak
  int nxo = max(0,min(ix,nx-SPECKSIZE+1));
  int nyo = max(0,min(iy,ny-SPECKSIZE+1));
  ll = 0.5*(((nxo-xo-muo)*(nxo-xo-muo)+ (nyo-yo-mvo)*(nyo-yo-mvo))/(dynn*dynn));
  
  // cdynn is the std. dev
  // cdynnvar is the variance
  if (enableColorDynamics) {
		//Note: no need to use circular stats for H channel here because we are just "jittering" an H degree value +/- a bit
		//(and not calculating a mean or variance of pixels)
    cdynnvar.setColor(cdynn.h*cdynn.h,cdynn.s*cdynn.s,cdynn.v*cdynn.v);
    nco.h = co.h + drawSampleFromGaussian(cdynn.h);
    nco.v = clamp(co.v + drawSampleFromGaussian(cdynn.v));
    nco.s = clamp(co.s + drawSampleFromGaussian(cdynn.s));
    ll = ll + nco.colll(co,cdynnvar);
    co = nco;
  }


  uo = nxo-xo;
  vo = nyo-yo;
  xo = nxo;  yo = nyo;
  if (usedepth) {
    ndepo = depo + drawSampleFromGaussian(depdynn);
    ll = ll + 0.5*(ndepo-depo)*(ndepo-depo)/(depdynn*depdynn);
    depo = ndepo;
  }
  return ll;
}

void Speck::print(FILE *fh)
{
  //fprintf(stderr,"mean %d %d vel  %g %g\n",xo,yo,uo,vo);
  fprintf(fh,"%d %d %g %g ",xo,yo,uo,vo);
  co.print(fh);
  sigco.print(fh);
}
void Speck::scan(FILE *fh)
{
  fscanf_s(fh,"%d %d %lg %lg ",&xo,&yo,&uo,&vo); // Babak: _s
  co.scan(fh);
  sigco.scan(fh);
}

void Speck::drawOnImage(unsigned char *outim, int nx, int ny, unsigned char rval, unsigned char bval, unsigned char gval, int sidesize)
{
  int i,j,index;
  for (i=yo; i<yo+sidesize; i++) {
    for (j=xo; j<xo+sidesize; j++) {
      if (i >=0 &&  i<ny && j>=0 && j<nx) {
	index = (i*nx+j)*3;
	outim[index] = rval;
	outim[index+1] = gval;
	outim[index+2] = bval;
      }
    }  
  }
}
void Speck::drawOnImage(unsigned char *outim, int nx, int ny, unsigned char rval, unsigned char bval, unsigned char gval, int pfr, int pto)
{
  int i,j,index;
  for (i=yo+pfr; i<yo+pto; i++) {
    for (j=xo+pfr; j<xo+pto; j++) {
      if (i >=0 &&  i<ny && j>=0 && j<nx) {
	index = (i*nx+j)*3;
	outim[index] = rval;
	outim[index+1] = gval;
	outim[index+2] = bval;
      }
    }  
  }
}

// draws a gaussian sample using the box Muller method
double drawSampleFromGaussian(double sig) {
  return sig*drawSampleFromGaussian();
}
// draws gaussian sample mean 0 with std. dev 1
double drawSampleFromGaussian() {
   double rn1(0);
   while (rn1 == 0) 
     rn1 = rand()/(RAND_MAX+1.0);
   double rn2 = rand()/(RAND_MAX+1.0);
   double x1 = sin(2*3.141592*rn2);
   double x2 = sqrt(-2*log(rn1));
   return (x1*x2);
}
