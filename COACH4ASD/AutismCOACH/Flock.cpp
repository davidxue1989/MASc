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
#include "flock.h"

Flock::Flock(int ns, double cdist, double sdist, double idist, Color & fco, Color & fsigco, double fw, bool enablecdyn)
{
  numspecks = ns;
  specks = new Speck*[numspecks];
  for (int i=0; i<numspecks; i++) 
    specks[i] = new Speck();
  colldist = cdist;
  colldist2 = colldist*colldist;
  straydist = sdist;
  straydist2 = straydist*straydist;
  intdist = idist;
  intdist2 = intdist*intdist;
  flock_weight = fw;
  enableColorDynamics = enablecdyn;
  co.setColor(fco);
  sigco.setColor(fsigco);
  gammad = DEFAULT_GAMMAD;
  gammau = DEFAULT_GAMMAU;
  gammac = DEFAULT_GAMMAC;
  gammag = DEFAULT_GAMMAG;
}
Flock::Flock(Flock & fl) 
{
  numspecks = fl.numspecks;
  for (int i=0; i<numspecks; i++) 
    specks[i]->setTo(fl.specks[i]);
  colldist = fl.colldist;
  colldist2 = fl.colldist2;
  straydist = fl.straydist;
  straydist2 = fl.straydist2;
  flock_weight = fl.flock_weight;
  co.setColor(fl.co);
  sigco.setColor(fl.sigco);
  gammad = fl.gammad;
  gammau = fl.gammau;
  gammac = fl.gammac;
  gammag = fl.gammag;
  computeMean();
}

//dx may 30, 2012: memory leak issue
Flock::~Flock()
{
	for (int i=0; i<numspecks; i++)
	{
		delete specks[i];
	}
	delete [] specks;
}


void Flock::setTo(Flock *fl)
{
  numspecks = fl->numspecks;
  for (int i=0; i<numspecks; i++) 
    specks[i]->setTo(fl->specks[i]);
  colldist = fl->colldist;
  colldist2 = fl->colldist2;
  straydist = fl->straydist;
  straydist2 = fl->straydist2;
  co.setColor(fl->co);
  sigco.setColor(fl->sigco);
  gammad = fl->gammad;
  gammau = fl->gammau;
  gammac = fl->gammac;
  gammag = fl->gammag;
  computeMean();
}
// randomize the flock by 
// first picking random location
// then generating specks according to (rough) flocking characteristics
// straydist and colldist
void Flock::randomize(int nx, int ny)  
{
  double sdist = straydist;
  int isdist = (int) sdist;
  // this will not be the final mean, just a starting point
  xo = ((int) floor(sdist+(nx-2*sdist)*(rand()/(RAND_MAX+1.0))));
  yo = ((int) floor(sdist+(ny-2*sdist)*(rand()/(RAND_MAX+1.0))));
  
  for (int i=0; i<numspecks; i++) 
    specks[i]->randomize(xo, yo, 2*isdist, 2*isdist, co, sigco);
  computeMean();
}
double Flock::compute_likelihood(Color *im, int nx, int ny)
{
  double wgt = 0.0;
  for (int i=0; i<numspecks; i++) 
    wgt += specks[i]->compute_weight(im,co,sigco,nx,ny,flock_weight,true);
  return exp(-wgt/numspecks);
}
double Flock::compute_weight(double proposal_weight, Color  *im, int nx, int ny, IPixel * otherflockmean)
{
  double wd, wu, wc,wg;
  // compute likelihood
  ll = compute_likelihood(im,nx,ny);

  //fprintf(stdout,"%g ",log(ll));
  // compute dynamics probability D'
  // already computed in projectForward
  // dynll is 0.0 if never computed (if this is a data-driven particle)
  wd = exp(-gammad*dynll);

  // compute flocking penalties, pu and pc
  wu = exp(-gammau*strayPenalty());
  wc = exp(-gammac*collisionPenalty());

  
  // add small penalty for being further up in the image
  // since we are really interested in tracking the top-most point
  // of each region
  //wg = exp(-gammag*topPenalty(nx,ny));
  
  if (otherflockmean != NULL) {
    wg = exp(-gammag*interactionPenalty(otherflockmean));
  } else {
    wg = 1.0;
  }
	 
  // multiply all four to get final weight
  // ll*D'*pu*pc/proposal_weight
  // where proposal_weight = D' in the case of a prior sample
  // what is this HACK?? is it right? 
  if (proposal_weight == 0)
    proposal_weight = 1.0;
  return (ll*wd*wu*wc*wg/proposal_weight);
}
void Flock::getMinY(int & minx, int & miny, int ny)
{
  miny = ny+1;
  for (int i=0; i<numspecks; i++) {
    if (specks[i]->yo < miny) {
      miny = specks[i]->yo;
      minx = specks[i]->xo;
    }
  }
}
// penalty for being too far from top
// for hands only
double Flock::topPenalty(int nx, int ny)
{
  int miny = ny;
  int minx;
  double pen(0.0);
  getMinY(minx,miny,ny);
  pen = TOP_PEN*(((double) miny)/ny);
  return pen;
}
// interaction with another flock penalty
double Flock::interactionPenalty(IPixel * otherflockmean)
{
  double dd;
  double pen(0.0);
  for (int i=0; i<numspecks; i++) {
    // this is g_g
    dd = (specks[i]->xo-otherflockmean->x)*(specks[i]->xo-otherflockmean->x) + 
      (specks[i]->yo-otherflockmean->y)*(specks[i]->yo-otherflockmean->y);
    if (dd > intdist2) 
      pen += INT_PEN*sigmoid(-0.01,dd,intdist2);
  }
  return pen/numspecks;
}
// penalty for straying too far from center of flock
double Flock::strayPenalty()
{
  double pen(0.0);
  double dd,sdist2;
  
  sdist2 = straydist2;

  for (int i=0; i<numspecks; i++) {
    // this is g_u
    dd = (specks[i]->xo-xo)*(specks[i]->xo-xo) + (specks[i]->yo-yo)*(specks[i]->yo-yo);
    if (dd > sdist2) 
      pen += STRAY_PEN*sigmoid(0.01,dd,sdist2);
  }
  return pen/numspecks;
}
double sigmoid(double a, double d, double x)
{
  return (1.0/(1.0+exp(-a*(d-x))));
}
// penalty for moving too close to a neighbor
double Flock::collisionPenalty()
{
  double pen(0.0);
  double dd,cdist2;
  cdist2 = colldist2;
  int numpen = 0;
  for (int i=0; i<numspecks; i++) {
    for (int j=i+1; j<numspecks; j++) {
      dd = (specks[i]->xo-specks[j]->xo)*(specks[i]->xo-specks[j]->xo) + 
	(specks[i]->yo-specks[j]->yo)*(specks[i]->yo-specks[j]->yo);
      pen += COLL_PEN*sigmoid(-0.01,dd,cdist2);
      numpen++;
    }
  }
  if (numpen > 0) 
    pen = pen/numpen;

  //fprintf(stderr,"pen %g\n",pen);
  return pen;
}

int sortcompare(const void *v1, const void *v2) 
{
  return ((vcastd(v1) < vcastd(v2)) ? -1 : ((vcastd(v1) > vcastd(v2)) ? 1 : 0));
}
int searchcompare(const void *key, const void *arr)
{
  double dk = vcastd(key);
  double *da = ((double *) arr);
  if (dk <= da[0])
    return -1;
  if (dk > da[1]) 
    return 1;
  return 0;
}
// sets this flock to be a sample from cll, which is a (cumulative, normalised) nx*ny distribution
// over positions of each speck in the flock
// the specks are given in speckdata (numdata) of them and the raw likelihoods of each speck are in ll
// if getcolor = true (default is false), then also compute mean color of the flock from the sampled specks/
// otherwise, use the pre-existing color
// returns the likelihood of the sample
double Flock::setToSampleFrom(Speck **speckdata, double *ll, double *cll, int numdata, int nx, int ny, bool getcolor)
{
  int i,k;
  double totll(-1.0);
  double *rands = new double[numspecks];
  double *tv;
  //double lnd = log(numdata); // Babak
  double lnd = log(double(numdata)); // Babak
  while (totll <= 0.0) {
    totll = 1.0;
    dynll = 0.0;
    for (i=0; i<numspecks; i++) 
      //rands[i] = drand48(); // Babak
	  //rands[i] = (double(rand()) / RAND_MAX); // Babak
	  //rands[i] = (double(rand()+1) / (RAND_MAX+2)); // Babak 
	  rands[i] = drand48_open_interval(); // Babak 
    // order the random numbers - ordering of specks does not matter
    qsort(rands, numspecks, sizeof(double),&sortcompare);
    // binary search method
    for (k=0; k<numspecks; k++) {
      tv = (double *) (bsearch(&(rands[k]),cll,numdata,sizeof(double),&searchcompare));
      // figure out index from tihs
      i = (tv-cll);
      specks[k]->setTo(speckdata[i]);
      totll *= ll[i];
      dynll += lnd;
    }
  }
  delete [] rands;

  // get mean of all specks
  computeMean();
  if (getcolor)
    computeMeanCol();
  return totll;
}
// project with noise levels passed in 
double Flock::projectForward(int nx, int ny, double posdyn, Color cdyn, double depdyn)
{
  // a flock has the property that all its specks
  // have the same velocity. Since we guarantee that this
  // condition holds, each speck can be projected independently
  dynll = 0.0;
  for (int i=0; i<numspecks; i++) 
    dynll += specks[i]->projectForward(uo,vo,nx,ny,posdyn,cdyn,depdyn,enableColorDynamics);
  dynll = dynll/numspecks;
  computeMean();
  return exp(-dynll);
}
// checks if x, y is inside the flock
int Flock::inside(int x, int y)
{
  int i;
  bool above(false),below(false),left(false),right(false);
  for (i=0; i<numspecks; i++) {
    if (specks[i]->xo < x)
      right = true;
    else 
      left = true;
    if (specks[i]->yo < y)
      below = true;
    else
      above = true;
  }
  return (above && below && left && right);
}
// computes mean position, velocity and depth
void Flock::computeMean()
{
  double dxo(0), dyo(0);
  uo = vo = depo = 0.0;
  for (int i=0; i<numspecks; i++) {
    dxo += specks[i]->xo;
    dyo += specks[i]->yo;
    uo += specks[i]->uo;
    vo += specks[i]->vo;
    depo += specks[i]->depo;
  }
  xo = (int) floor(dxo/numspecks);
  yo = (int) floor(dyo/numspecks);
  uo = uo/numspecks;
  vo = vo/numspecks;
  depo = depo/numspecks;
}
// also computes mean color
void Flock::computeMeanCol()
{
  double ho(0.0), sigho(0.0);
	double hsin(0.0), hcos(0.0);
  double so(0.0), sigso(0.0);
  double fvo(0.0), sigfvo(0.0);
	double dDegToRadRatio = 3.14159265 / 180.0;
  
  for (int i=0; i<numspecks; i++) {
		hsin += sin(specks[i]->co.h * dDegToRadRatio);
		hcos += cos(specks[i]->co.h * dDegToRadRatio);
    //sigho += (specks[i]->co.h*specks[i]->co.h);
    so += specks[i]->co.s;
    sigso += (specks[i]->co.s*specks[i]->co.s);
    fvo += specks[i]->co.v;
    sigfvo += (specks[i]->co.v*specks[i]->co.v);
  }
  ho = atan2(hsin, hcos) / dDegToRadRatio;  //ho/numspecks;
  so = so/numspecks;
  fvo = fvo/numspecks;

  sigho = sqrt( (hsin*hsin) + (hcos*hcos) ) / numspecks; //sigho/numspecks-ho*ho;
	sigho = 2.0 * (1.0 - sigho) / (dDegToRadRatio*dDegToRadRatio);
  sigso = sigso/numspecks-so*so;
  sigfvo = sigfvo/numspecks-vo*vo;
  co.setColor(ho,so,fvo);
  sigco.setColor(sigho,sigso,sigfvo);
}
void Flock::print(FILE *fh)
{
  fprintf(fh,"%d %d %g %g ",xo,yo,uo,vo);
  co.print(fh);
  sigco.print(fh);
  for (int i=0; i<numspecks; i++) 
    specks[i]->print(fh);
}
void Flock::scan(FILE *fh)
{
  fscanf_s(fh,"%d %d %lg %lg ",&xo,&yo,&uo,&vo); // Babak: _s
  co.scan(fh);
  sigco.scan(fh);
  //fprintf(stderr,"number of specks scanning %d\n",numspecks);
  for (int i=0; i<numspecks; i++) {
    specks[i]->scan(fh);
    //specks[i]->print(stderr);
    //fprintf(stderr,"\n");
  } 
}
void Flock::printParticle(FILE *fh)
{
  fprintf(fh,"%d\t%d\t%g\t%g\t",xo,yo,uo,vo);
  co.print(fh);
  sigco.print(fh);
  for (int i=0; i<numspecks; i++) 
    specks[i]->print(fh);
}
void Flock::scanParticle(FILE *fh)
{
  fscanf_s(fh,"%d\t%d\t%lg\t%lg\t",&xo,&yo,&uo,&vo); // Babak: _s
}
void Flock::drawOnImage(unsigned char *outim, int nx, int ny, unsigned char rval, unsigned char gval, unsigned char bval, int thick)
{
  thick = thick-1;
  for (int i=0; i<numspecks; i++) {
    // draw the specks
    specks[i]->drawOnImage(outim,nx,ny,rval,bval,gval);
    // draw lines emanating from the mean to the speck
    for (int j=-thick; j<=thick; j++) 
      for (int k=-thick; k<=thick; k++) 
		drawlineBresenham(outim,nx,ny,xo+j,yo+k,specks[i]->xo+j,specks[i]->yo+k,rval,gval,bval);
  }
}
void swap(int & x, int &y) 
{
  int tmp = x;
  x = y;
  y = tmp;
}
void setcval(unsigned char *s, unsigned char rval, unsigned char gval, unsigned char bval)
{
  *s = rval;
  *(s+1) = gval;
  *(s+2) = bval;
}

/*
// added by Babak
void drawPoint(unsigned char *s, int nx, int ny, int x, int y, unsigned char rval, unsigned char gval, unsigned char bval)
{
	if (x > nx)
		return;
	if (y > ny)
		return;
	if (x < 0)
		return;
	if (y < 0)
		return;
	int index = (y*nx+x)*3;
	setcval(s+index, rval, gval, bval);
}
*/


// Bresenham's algorithm for line drawing - taken and adapted from
// http://en.wikipedia.org/wiki/Bresenham's_line_algorithm
void drawlineBresenham(unsigned char *s, int nx, int ny, int x0, int y0, int x1, int y1, unsigned char rval, unsigned char gval, unsigned char bval) 
{

	/*
    unsigned short steep;
    steep = Abs(y1 - y0) > Abs(x1 - x0);
    if (steep) {
        swap(x0, y0);
        swap(x1, y1);
    }
    if (x0 > x1) {
        swap(x0, x1);
        swap(y0, y1);
    }
    {
        int deltax = x1 - x0;
        int deltay = Abs(y1 - y0);
        int error = deltax / 2;
        int ystep;
        int y = y0;
        int x;
        if (y0 < y1) ystep = 1; else ystep = -1;
        for (x = x0; x <= x1; ++x) {
            if (steep)
			{
				drawPoint(s, nx, ny, y, x, rval, gval, bval);
			}
			else 
			{
				drawPoint(s, nx, ny, y, x, rval, gval, bval);
			}
            error = error - deltay;
            if (error < 0) {
                y = y + ystep;
                error = error + deltax;
            }
        }
    }

*/

  int total_size = nx*ny*3; // added by Babak

  bool steep = Abs(y1 - y0) > Abs(x1 - x0);
  if (steep) {
    swap(x0, y0);
    swap(x1, y1);
  }
  int deltax = Abs(x1 - x0);
  int deltay = Abs(y1 - y0);
  int error = 0;
  int deltaerr = deltay;
  int x(x0), y(y0);
  int index;
  int xstep(-1),ystep(-1);
  int xstep3, ystep3;
  if (x0 < x1) 
    xstep = 1;
  if (y0 < y1)
    ystep = 1;
  if (steep) {
    xstep3 = xstep*nx*3;
    ystep3 = ystep*3;
    index = (x*nx+y)*3;
  } else {
    xstep3 = xstep*3;
    ystep3 = ystep*nx*3;
    index = (y*nx+x)*3;
  }
  if ( (index>=0) && (index<total_size) ) // added by Babak
  { setcval(s+index, rval, gval, bval); }

  while (x != x1) {
    x = x + xstep;
    index = index + xstep3;
    error = error + deltaerr;
    if (2*error > deltax) {
      y = y + ystep;
      index = index + ystep3;
      error = error - deltax;
    }
	if ( (index>=0) && (index<total_size) ) // added by Babak
	{ setcval(s+index, rval, gval, bval); }
  } 

  
}
