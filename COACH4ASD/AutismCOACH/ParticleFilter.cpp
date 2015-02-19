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
#include "particlefilter.h"

// cdist and sdist are collision and stray flock distances
// fs is the flock size
ParticleFilter::ParticleFilter(int nump, int fs, 
			       double cdist, double sdist, double idist,
			       Color & fco, Color & fsigco, 
			       double fw, bool enablecdyn)
{
  num_particles = nump;

  ////dxdebug nov 28, 2012
  //ptc_old = new Flock *[num_particles];
  //ptc_new = new Flock *[num_particles];  
  //for (int i=0; i<num_particles; i++) {
  //  //default number of specks per flock in flock.h
  //  ptc_old[i] = new Flock(fs,cdist,sdist,idist,fco,fsigco,fw,enablecdyn);
  //  ptc_new[i] = new Flock(fs,cdist,sdist,idist,fco,fsigco,fw,enablecdyn);
  //}

  ptc = new Flock*[num_particles];
  newptc = new Flock*[num_particles];
  ptcweight = new double[num_particles];
  cumdist = new double[num_particles+1];
  int i;
  for (i=0; i<num_particles; i++) {
    //default number of specks per flock in flock.h
    ptc[i] = new Flock(fs,cdist,sdist,idist,fco,fsigco,fw,enablecdyn);
    newptc[i] = new Flock(fs,cdist,sdist,idist,fco,fsigco,fw,enablecdyn);
  }
  learnweightlimits=false;
}

//dx may 29, 2012: memory leak issue
ParticleFilter::~ParticleFilter()
{
	for (int i = 0; i<num_particles; i++)
	{
		delete ptc[i];
		delete newptc[i];
	}
	delete [] ptc;
	delete [] newptc;
	delete [] ptcweight;
	delete [] cumdist;
}


void ParticleFilter::randomizeParticles(int nx, int ny)
{
  int i;
  // initialise weights evenly;
  cumdist[0] = 0.0;
  double wsum = 0.0;
  for (i=0; i<num_particles; i++) {
    ptc[i]->randomize(nx,ny);
    ptcweight[i] = 1.0;
    cumdist[i] = wsum;
    wsum += ptcweight[i];
  }
  cumdist[i] = wsum;
  for (i=0; i<num_particles; i++) {
    cumdist[i] = cumdist[i]/wsum;
    ptcweight[i] = ptcweight[i]/wsum;
  }
  cumdist[i] = cumdist[i]/wsum;

}
// draws a sample according to cumdist
int ParticleFilter::drawSample()
{
  return drawaSample(num_particles,cumdist);
}

int drawaSample(int nump, double *cdist)
{
  // draw new sample set
  //double rands =  drand48(); // Babak
  //double rands = (double(rand()) / RAND_MAX); // Babak
  double rands = drand48_open_interval(); // Babak
  int j=1;
  while (j<=nump && cdist[j] < rands) 
    j++;
  return j-1;
}
void ParticleFilter::printWeightLimits()
{
  fprintf(stderr,"%g %g %g %g\n",mindataweight,maxdataweight,minpriorweight,maxpriorweight);
}
void ParticleFilter::resetWeightLimits()
{
  maxdataweight = -(INFINITY);
  mindataweight = +(INFINITY);
  
  maxpriorweight = -(INFINITY);
  minpriorweight = +(INFINITY);
    
}
void ParticleFilter::setWeightLimits(double maxd,double mind,double maxp,double minp)
{
  maxdataweight = maxd;
  mindataweight = mind;
  maxpriorweight = maxp;
  minpriorweight = minp;
}
// perform an update with data-driven specks 
// with likelihood distribution ll1
// and cumulative likelihood distribution cll1
// alpha1 is probability of using data particles from set ll
// 1-alpha1
void ParticleFilter::updateData(Color *cim, int nx, int ny, 
				Speck **data1, double *ll1, double *cll1, 
				int numdata1, double alpha1,
				double & dataweight, double & priorweight,
				double posdyn, Color & cdyn, double depdyn,
				double stren, int verbose, IPixel * mean_otherhandp)
{
  int i, thesample;
  double pweight, rands;
  int priorsample, nprior(0), ndata(0);
  double opartweight;
  dataweight = priorweight = 0.0;
  for (i=0; i<num_particles; i++) {
    priorsample = 0;
    // choose whether to sample from dynamics proposal or data proposal
    //rands =  drand48(); // Babak
	//rands =  (double(rand()) / RAND_MAX); // Babak
	rands =  drand48_open_interval(); // Babak
    if (rands < alpha1) {
      pweight = newptc[i]->setToSampleFrom(data1,ll1,cll1,numdata1,nx,ny);
      opartweight = 1.0/(alpha1*num_particles);
      priorsample = 1;
    } else {
      thesample = i; //drawSample();
      newptc[i]->setTo(ptc[thesample]);
      pweight = newptc[i]->projectForward(nx, ny, posdyn, cdyn, depdyn);
      opartweight = ptcweight[i];
    }
    ptcweight[i] = opartweight*(newptc[i]->compute_weight(pweight,cim,nx,ny,mean_otherhandp));
    if (priorsample) {
      dataweight += ptcweight[i];
      ndata++;
    } else {
      priorweight += ptcweight[i];
      nprior++;
    }
    if (verbose>1) {
      newptc[i]->print(stderr);
      fprintf(stderr,"proposal weight %g final weight %g\n",pweight,ptcweight[i]);
      fprintf(stderr,"%d %d %g %g\n",i,priorsample,pweight,ptcweight[i]);
    }
  }
  //fprintf(stdout,"\n");
  dataweight = ndata*dataweight/(ndata+nprior);
  priorweight = nprior*priorweight/(ndata+nprior);
  
  if (learnweightlimits) {
    if (ndata > 0) {
      maxdataweight = max(maxdataweight,dataweight);
      mindataweight = min(mindataweight,dataweight);
    }
    if (nprior > 0) {
      maxpriorweight = max(maxpriorweight,priorweight);
      minpriorweight = min(minpriorweight,priorweight);
    }
  }


  if (verbose > 0) {
    fprintf(stderr,"total data weight %d %g prior weight %d %g\n",ndata,dataweight,nprior,priorweight);
    fprintf(stdout,"%d %g %d %g ",ndata,dataweight,nprior,priorweight);
  }
  normaliseResample();
}
void ParticleFilter::setLearnWeightLimits(int lwl)
{
  learnweightlimits=lwl;
}
void ParticleFilter::normaliseResample()
{
  int j;
  int thesample;
  // normalise the weights for each component separately
  double neff;
  double wsum;
  neff = normaliseWeights(wsum,true);

  // possible resample
  if (neff <= num_particles) {
    for (j=0; j<num_particles; j++) {
      thesample = drawSample();
      newptc[j]->setTo(ptc[thesample]);
      ptcweight[j] = 1.0/num_particles;
      if (thesample == best_particle) 
	best_particle = j;
    }
    neff = normaliseWeights(wsum);
  }
}
double ParticleFilter::normaliseWeights(double & wsum, bool getbest) 
{
  int j;
  wsum = 0.0;
  for (j=0; j<num_particles; j++) {
    cumdist[j] = wsum;
    wsum += ptcweight[j];
    if (getbest && (j==0 || ptcweight[j] > best_particle_weight)) {
      best_particle_weight = ptcweight[j];
      best_particle = j;
    }
  }
  if (wsum == 0.0) {
    // if they are all zero, make them all even
    for (j=0; j<num_particles; j++) {
      cumdist[j] = wsum;
      ptcweight[j] = 1.0/num_particles;
      wsum += ptcweight[j];
    }
  }
  cumdist[j] = 1.0;
  double neff = 0.0;
  // normalize and compute cumdist
  for (j=0; j<num_particles; j++) {

	  ////dxdeug nov 28, 2012:
	  //ptc_old[j]->setTo(ptc[j]);
	  //ptc_new[j]->setTo(newptc[j]);

    ptc[j]->setTo(newptc[j]);
    ptcweight[j] = ptcweight[j]/wsum;
    cumdist[j] = cumdist[j]/wsum;
    neff += ptcweight[j]*ptcweight[j];
  }
  return (1.0/neff);
}

void assignto(Speck & hp, double *mn)
{
  hp.xo = (int) (mn[0]);
  hp.yo = (int) (mn[1]);
}
double computedistance(Speck & hp, double *mn) 
{
  return ((hp.xo-mn[0])*(hp.xo-mn[0])+(hp.yo-mn[1])*(hp.yo-mn[1]));
}
// returns the order of the indices of x sorted in descending order (does not change x)
void selsort(int *x, int n, int *ix)
{
  int *checked = new int[n];
  for (int i=0; i<n; i++)
    checked[i] = x[i];
  int m = n;
  int d(0), mi(0), k(0);
  while (m > 0) {
    for (int i=0; i<n; i++) {
      if (i==0 || checked[i] > d) {
	d = checked[i];
	mi = i;
      }
    }
    ix[k] = mi;
    checked[mi] = -1;
    
    m--;
    k++;
  }
  
  delete [] checked;
  
}
// figures out if position (x,y) is "inside" the distribution of flocks
// (x,y) is inside if there is some speck above, below, to the left and to the right
// do this for ns samples only
int ParticleFilter::inside(int x, int y, int ns)
{
  int i,k;
  int inside=0;
  for (i=0; i<ns && !inside; i++) {
    k = drawSample();
    inside = inside + (ptc[k]->inside(x,y));
  }
  return inside;
}
// eastimates variance - mean passed in
void ParticleFilter::estimateVariance(IPixel & meanp, double & sigxo, double  & sigyo)
{
  sigxo = sigyo = 0.0;
  for (int i=0; i<num_particles; i++) {
    sigxo += ptcweight[i]*(ptc[i]->xo-meanp.x)*(ptc[i]->xo-meanp.x);
    sigyo += ptcweight[i]*(ptc[i]->yo-meanp.y)*(ptc[i]->yo-meanp.y);
  }
  sigxo = sigxo/(num_particles-1);
  sigyo = sigyo/(num_particles-1);
}
// eastimates mean
void ParticleFilter::estimateMean(IPixel & meanp)
{
  double xo,yo,theweight;
  xo = yo = 0.0;
  for (int i=0; i<num_particles; i++) {
    theweight = ptcweight[i];
    xo += theweight*ptc[i]->xo;
    yo += theweight*ptc[i]->yo;
  }
  //meanp.setTo(((int) round(xo)),((int) round(yo))); // Babak
  meanp.setTo(((int) roundA(xo)),((int) roundA(yo))); // Babak
}

void ParticleFilter::estimateMean(Speck & meanp)
{
  int i;
	double dDegToRadRatio = 3.14159265 / 180.0;
  double xo,yo,uo,vo,depo, ho, hsin, hcos, sigho, vao, sigvo, so, sigso, theweight;
  xo = yo = uo = vo =depo = ho = hsin = hcos = sigho = vao = sigvo = so = sigso = 0.0;
  for (i=0; i<num_particles; i++) {
    theweight = ptcweight[i];
    xo += theweight*ptc[i]->xo;
    yo += theweight*ptc[i]->yo;
    uo += theweight*ptc[i]->uo;
    vo += theweight*ptc[i]->vo;

		// calc ho using circular stats
    hsin += theweight * sin(ptc[i]->co.h * dDegToRadRatio);
		hcos += theweight * cos(ptc[i]->co.h * dDegToRadRatio);
		ho = atan2(hsin,hcos) / dDegToRadRatio;
		//note: weighted avg of H variances below is ok to do with regular stats instead of circular
		//stats because variances should all be positive angle values >= 0 deg.
    sigho += theweight*ptc[i]->sigco.h;	
    
		vao += theweight*ptc[i]->co.v;
    sigvo += theweight*ptc[i]->sigco.v;
    so += theweight*ptc[i]->co.s;
    sigso += theweight*ptc[i]->sigco.s;
    depo += theweight*ptc[i]->depo;
  }
  Color meanc(ho,so,vao);
  Color meansigc(sigho,sigso,sigvo);
  meanp.setTo(((int) floor(xo)),((int) floor(yo)),uo,vo,depo,meanc,meansigc);
}
void ParticleFilter::estimateMinY(Speck & minp, int ny)
{
  IPixel pminp;
  estimateMinY(pminp,ny);
  minp.xo = pminp.x;
  minp.yo = pminp.y;
}
void ParticleFilter::estimateMinY(IPixel & minp, int ny)
{
  int i;
  double dminy(0.0),dminx(0.0);
  int tdx,tdy;
  for (i=0; i<num_particles; i++) {
    ptc[i]->getMinY(tdx,tdy,ny);
    dminx += ptcweight[i]*tdx;
    dminy += ptcweight[i]*tdy;
  }
  minp.setTo(((int) floor(dminx)),((int) floor(dminy)));
}
void ParticleFilter::print(FILE *fh) {
  for (int k=0; k<num_particles; k++) {
    fprintf(fh,"%g ",ptcweight[k]);
    ptc[k]->printParticle(fh);
  }
}
void ParticleFilter::scan(FILE *fh) {
  for (int k=0; k<num_particles; k++) {
    fscanf_s(fh,"%lg ",ptcweight+k); // Babak

    ptc[k]->scan(fh);
  }
}
void ParticleFilter::printParticles(FILE *fh) {
  for (int k=0; k<num_particles; k++) {
    fprintf(stderr,"%d\t%g\t",k,ptcweight[k]);
    ptc[k]->printParticle(fh);
    fprintf(stderr,"\n");
  }
}

void ParticleFilter::drawParticles(unsigned char *im, unsigned char *outim, int nx, int ny, unsigned char rval, unsigned char gval, unsigned char bval, int np)
{
  int k;
  int thesample;
  if (np < 0) 
    np = num_particles;
  memcpy(outim,im,nx*ny*3*sizeof(unsigned char));
  for (k=0; k<np; k++) {
    // since the particles are a weighted sample, we must
    // draw an unweighted set to print them out
    thesample = drawSample();
    ptc[thesample]->drawOnImage(outim,nx,ny,rval,gval,bval);
  }
}

void ParticleFilter::drawBestParticle(unsigned char *im, unsigned char *outim, int nx, int ny, unsigned char rval, unsigned char gval, unsigned char bval, int thick)
{
  memcpy(outim,im,nx*ny*3*sizeof(unsigned char));
  // since the particles are a weighted sample, we must
  // draw an unweighted set to print them out
  ptc[best_particle]->drawOnImage(outim,nx,ny,rval,gval,bval,thick);
}


