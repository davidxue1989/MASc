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
#include "HandTracker.h"


HandTracker::HandTracker(int inx, int iny, int inp)
{
  int i;
  nx = inx;
  ny = iny;
  num_particles = inp;

  cim = new Color[nx*ny];

  ccspots = new IComp[MAXIMUM_COMPS];
  //sccspots = new IComp[MAXIMUM_COMPS];//Dec. 9, 2010 //dx may 31, 2012: get rid of soap
  tccspots = new IComp[MAXIMUM_COMPS];//This line duplicated for soap
  cc = new int[nx*ny];

  cxo = new int[MAXIMUM_COMPS];
  cyo = new int[MAXIMUM_COMPS];
  tcxo = new int[MAXIMUM_COMPS];
  tcyo = new int[MAXIMUM_COMPS];

  mlhp.setTo(0,ny/2);
  mrhp.setTo(nx,ny/2);

  skin_im = new int[nx*ny];
  towel_im = new int[nx*ny];//This line duplicated for soap
  soap_im = new int[nx*ny];//Modification for soap
  tmpim = new unsigned char[nx*ny*3];

  RGBHSVLookup = buildHSVConverter();

  // objects
  // soap is at (10,100)-(130,205)
  // tap is at (250,35)- (305,95)
  // water is at (240,75)-(315,165)
  // sink is (120,80)-(435,255);

  soapSpace.setTo(10,100,130,205);
  tapSpace.setTo(250,35,305,95);
  waterSpace.setTo(240,75,315,165);
  sinkSpace.setTo(120,80,435,255);
  towelSpace.setTo(480,40,560,160);
  // these should be computed as the centers of the spaces
  soapPos.setTo(60,135);
  LTapPos.setTo(273,54);
  RTapPos.setTo(273,54);
  waterPos.setTo(275,106);

  // data particles
  soapdp = new Speck*[MAX_NUMDATAPARTICLES]; //Modification for soap
  toweldp = new Speck*[MAX_NUMDATAPARTICLES];//This line duplicated for soap
  skindp = new Speck*[MAX_NUMDATAPARTICLES];
  lh_skindp = new Speck*[MAX_NUMDATAPARTICLES];
  rh_skindp = new Speck*[MAX_NUMDATAPARTICLES];

  for (i=0; i<MAX_NUMDATAPARTICLES; i++) {
	soapdp[i] = new Speck();//Modification for soap
	toweldp[i] = new Speck();//This line duplicated for soap
    skindp[i] = new Speck();
    lh_skindp[i] = new Speck();
    rh_skindp[i] = new Speck();
  }

  soapll = new double[MAX_NUMDATAPARTICLES];//Dec. 9, 2010
  towelll = new double[MAX_NUMDATAPARTICLES];//This line duplicated for soap
  skinll = new double[MAX_NUMDATAPARTICLES];
  lh_skinll = new double[MAX_NUMDATAPARTICLES];
  rh_skinll = new double[MAX_NUMDATAPARTICLES];
  
  soapcll = new double[MAX_NUMDATAPARTICLES+1];//Dec. 9, 2010
  towelcll = new double[MAX_NUMDATAPARTICLES+1];//This line duplicated for soap
  skincll = new double[MAX_NUMDATAPARTICLES+1];
  lh_skincll = new double[MAX_NUMDATAPARTICLES+1];
  rh_skincll = new double[MAX_NUMDATAPARTICLES+1];

  accum = new int*[ny];
  for (i=0; i<ny; i++) 
    accum[i] = new int[nx];

  // average color models
  sco.setColor(AVG_TOWEL_HUE,AVG_TOWEL_SAT,AVG_TOWEL_VAL);//Modification for soap
  ssigco.setColor(AVG_TOWEL_HUE_SIG,AVG_TOWEL_SAT_SIG,AVG_TOWEL_VAL_SIG);//Modification for soap
  tco.setColor(AVG_TOWEL_HUE,AVG_TOWEL_SAT,AVG_TOWEL_VAL);//This line duplicated for soap
  tsigco.setColor(AVG_TOWEL_HUE_SIG,AVG_TOWEL_SAT_SIG,AVG_TOWEL_VAL_SIG);//This line duplicated for soap
  lhco.setColor(AVG_HAND_HUE,AVG_HAND_SAT,AVG_HAND_VAL);
  lhsigco.setColor(AVG_HAND_HUE_SIG,AVG_HAND_SAT_SIG,AVG_HAND_VAL_SIG);
  rhco.setColor(AVG_HAND_HUE,AVG_HAND_SAT,AVG_HAND_VAL);
  rhsigco.setColor(AVG_HAND_HUE_SIG,AVG_HAND_SAT_SIG,AVG_HAND_VAL_SIG);

  // the actual particle filter trackers
  bool enableColorDyn(true);
  if (FLOCK_COLOR_WEIGHT == 1.0) {
    enableColorDyn = false;
  } 
//Modification for soap
  spf = new ParticleFilter(num_particles,FLOCKSIZE,//Modification for soap
			   COLL_DIST_SOAP,STRAY_DIST_SOAP,INT_DIST,sco,ssigco,FLOCK_COLOR_WEIGHT,enableColorDyn);//Modification for soap
  tpf = new ParticleFilter(num_particles,FLOCKSIZE,//This line duplicated for soap
			   COLL_DIST_TOWEL,STRAY_DIST_TOWEL,INT_DIST,tco,tsigco,FLOCK_COLOR_WEIGHT,enableColorDyn);//This line duplicated for soap
  lhpf = new ParticleFilter(num_particles,FLOCKSIZE,
			    COLL_DIST_HAND,STRAY_DIST_HAND,INT_DIST,lhco,lhsigco,FLOCK_COLOR_WEIGHT,enableColorDyn);
  rhpf = new ParticleFilter(num_particles,FLOCKSIZE,
			    COLL_DIST_HAND,STRAY_DIST_HAND,INT_DIST,rhco,rhsigco,FLOCK_COLOR_WEIGHT,enableColorDyn);

  // the noise levels - 
  // could have differentones for towel/hand
  loposdyn = 2.0;  
  hiposdyn = 15.0;
  lodepdyn = 1.0;  
  hidepdyn = 15.0;
  locdyn.setColor(0.1,0.05,0.05);  
  hicdyn.setColor(0.5,0.1,0.1);
  
 

  // start randomly
  lhpf->randomizeParticles(nx,ny);
  rhpf->randomizeParticles(nx,ny);
  tpf->randomizeParticles(nx,ny);//This line duplicated for soap
  spf->randomizeParticles(nx,ny);//Modification for soap

  this->resetWeightLimits();
  spf->setWeightLimits(MAX_TOWEL_DATA_WEIGHT,MIN_TOWEL_DATA_WEIGHT,MAX_TOWEL_PRIOR_WEIGHT,MIN_TOWEL_PRIOR_WEIGHT); //Modification for soap
  tpf->setWeightLimits(MAX_TOWEL_DATA_WEIGHT,MIN_TOWEL_DATA_WEIGHT,MAX_TOWEL_PRIOR_WEIGHT,MIN_TOWEL_PRIOR_WEIGHT);//This line duplicated for soap
  lhpf->setWeightLimits(MAX_HAND_DATA_WEIGHT,MIN_HAND_DATA_WEIGHT,MAX_HAND_PRIOR_WEIGHT,MIN_HAND_PRIOR_WEIGHT);
  rhpf->setWeightLimits(MAX_HAND_DATA_WEIGHT,MIN_HAND_DATA_WEIGHT,MAX_HAND_PRIOR_WEIGHT,MIN_HAND_PRIOR_WEIGHT);
  learnweightlimits=0;
 
  history_index = 0;
  hist_length = 20;
  hist_discount = 0.99;

  lh_hist = new IPixel[hist_length];
  rh_hist=  new IPixel[hist_length];
  t_hist = new IPixel[hist_length];
  mlh_hist = new IPixel[hist_length];
  mrh_hist=  new IPixel[hist_length];
  lht_hist = new double[hist_length];
  rht_hist = new double[hist_length];
  lhs_hist = new double[hist_length];
  rhs_hist = new double[hist_length];
  ts_hist = new double[hist_length];

  handspos = -1;
  elapsedFrames = 0;
  //elapsedSlowTime = new Timer(TIMESLOWTHRESH);

  ctlo.setColor(LOWHUE, LOWSAT, LOWVAL);
  tctlo.setColor(LOWTHUE, LOWTSAT, LOWTVAL);//This line duplicated for soap
  sctlo.setColor(LOWTHUE, LOWTSAT, LOWTVAL);//Dec. 9, 2010
  cthi.setColor(HIGHHUE, HIGHSAT, HIGHVAL);
  tcthi.setColor(HIGHTHUE, HIGHTSAT, HIGHTVAL);//This line duplicated for soap
  scthi.setColor(HIGHTHUE, HIGHTSAT, HIGHTVAL);//Dec. 9, 2010

  // set default connected components sizes
  this->hand_size.min = MINHANDSIZE;
  this->hand_size.med = MINHANDSIZE + (MAXHANDSIZE - MINHANDSIZE)/2;
  this->hand_size.max = MAXHANDSIZE;
  this->towel_size.min = MINTOWELSIZE;
  this->towel_size.med = MINTOWELSIZE + (MAXTOWELSIZE - MINTOWELSIZE)/2;
  this->towel_size.max = MAXTOWELSIZE;
}

// destructor (added by Babak)
HandTracker::~HandTracker()
{

	delete [] cim;
	delete [] ccspots;
	delete [] tccspots;//This line duplicated for soap
	//delete [] sccspots;//Dec. 9, 2010 //dx may 31, 2012: get rid of soap

	delete [] cc;
	delete [] cxo;
	delete [] cyo;
	delete [] tcxo;
	delete [] tcyo;

	delete [] skin_im;
	delete [] towel_im;//This line duplicated for soap
	delete [] soap_im;//Modification for soap
	delete [] tmpim;


	for (int i=0; i<MAX_NUMDATAPARTICLES; i++) {
		delete [] soapdp[i];//Modification for soap
		delete [] toweldp[i];//This line duplicated for soap
		delete [] skindp[i];
		delete [] lh_skindp[i];
		delete [] rh_skindp[i];
	}

	delete [] soapdp;//Modification for soap
	delete [] toweldp;//This line duplicated for soap
	delete [] skindp;
	delete [] lh_skindp;
	delete [] rh_skindp;

	//if (soapll!=NULL)
		delete [] soapll;//Dec. 9, 2010
	delete [] towelll;//This line duplicated for soap
	delete [] skinll;
	delete [] lh_skinll;
	delete [] rh_skinll;
	
	//if (soapcll!=NULL)
		delete [] soapcll;//Dec. 9, 2010
	delete [] towelcll;//This line duplicated for soap
	delete [] skincll;
	delete [] lh_skincll;
	delete [] rh_skincll;

	for (int i=0; i<ny; i++) 
	{
		delete [] accum[i];
	}
	delete [] accum;


	delete spf;//Modification for soap
	delete tpf;//This line duplicated for soap
	delete lhpf;
	delete rhpf;

  	delete [] lh_hist;
	delete [] rh_hist;
	delete [] t_hist;
	delete [] mlh_hist;
	delete [] mrh_hist;
	delete [] lht_hist;
	delete [] rht_hist;
	delete [] lhs_hist;
	delete [] rhs_hist;
	delete [] ts_hist;

	////dx may 29, 2012: debug: memory leak issue
	for (int j = 0; j<256; j++)
	{
		for (int i = 0; i<64; i++)
		{
			delete [] RGBHSVLookup[j][i];
		}
		delete [] RGBHSVLookup[j];
	}
	delete [] RGBHSVLookup;
	RGBHSVLookup = NULL;
	
	// these pointers don't need to be deleted, since they arn't newed to begin with
	//delete [] brx;
	//delete [] bry;
	//delete [] tlx;
	//delete [] tly;


}

// checks if h,s,v are skin colored
bool HandTracker::skinSegment(double h, double s, double v)
{
  return (s > ctlo.s && s < cthi.s &&
	  v > ctlo.v && v < cthi.v && 
	  (h > ctlo.h &&  h < cthi.h));
	  //(h > ctlo.h || h < cthi.h));
}

// checks if h,s,v are towel colored
bool HandTracker::towelSegment(double h, double s, double v)//This line duplicated for soap
{
  return (s > tctlo.s && s < tcthi.s &&//This line duplicated for soap
	  v > tctlo.v && v < tcthi.v &&//This line duplicated for soap
	  (h > tctlo.h && h < tcthi.h));//This line duplicated for soap
  //(h > tctlo.h || h < tcthi.h));
}
bool HandTracker::soapSegment(double h, double s, double v)//Modification for soap
{
  return (s > sctlo.s && s < scthi.s &&//Modification for soap
	  v > sctlo.v && v < scthi.v &&//Modification for soap
	  (h > sctlo.h && h < scthi.h));//Modification for soap
  //(h > tctlo.h || h < tcthi.h));
}

bool HandTracker::skinSegment(Color c)
{
  return skinSegment(c.h,c.s,c.v);
}
bool HandTracker::towelSegment(Color c)//This line duplicated for soap
{
  return towelSegment(c.h,c.s,c.v);//This line duplicated for soap
}
bool HandTracker::soapSegment(Color c)//Modification for soap
{
  return soapSegment(c.h,c.s,c.v);//Modification for soap
}
// performs median filtering on binim
// filter size is nxn  (n should be odd)
// binim is overwritten with result
// accum is a [ny][nx] accumulator array
void medianFilter(int *binim, int **accum, int n, int nx, int ny)
{
  int i,j,bcount;
  // half size
  int m = (n-1)/2;
  int filtthresh =  (int) floor(n*n/2.0);
  int row(nx);
  // accumulator
  for (i=1; i<ny; i++) {
    for (j=1; j<nx; j++) {
      accum[i][j] = accum[i][j-1]+accum[i-1][j]-accum[i-1][j-1]+binim[row+j];
      binim[row+j] = 0;
    }
    row += nx;
  }
  row = 0;
  for (i=0; i<ny; i++) {
    binim[row] = 0;
    row += nx;
  }
  for (j=0; j<nx; j++) 
    binim[j] = 0;
  row = (m+1)*nx;

  for (i=(m+1); i<(ny-m); i++) {
    for (j=(m+1); j<(nx-m); j++) {
      bcount = accum[i+m][j+m]-accum[i-m-1][j+m]-accum[i+m][j-m-1]+accum[i-m-1][j-m-1];
      binim[row+j] = (bcount > filtthresh);
    }
    row += nx;
  }
}
void HandTracker::segmentImage(unsigned char *outskinim)
{
  int i;
  int imsize = nx*ny;
  for (i=0; i<imsize; i++) {
    if (skinSegment(cim[i]))
      skin_im[i] = 1;
    else
      skin_im[i] = 0;
    if (towelSegment(cim[i]))//This line duplicated for soap
      towel_im[i] = 1;		//This line duplicated for soap
    else					//This line duplicated for soap
      towel_im[i] = 0;		//This line duplicated for soap
	if (soapSegment(cim[i]))//Modification for soap
      soap_im[i] = 1;		//Modification for soap
    else					//Modification for soap
      soap_im[i] = 0;		//Modification for soap
  }
  medianFilter(skin_im,accum,MEDIANFILTSIZE,nx,ny);
  medianFilter(towel_im,accum,MEDIANFILTSIZE,nx,ny);//This line duplicated for soap
  medianFilter(soap_im,accum,MEDIANFILTSIZE,nx,ny);//Modification for soap
  if (outskinim != NULL){
    for (i=0; i<imsize; i++) {
      outskinim[i*3] = skin_im[i]*255;
      outskinim[i*3+2] = towel_im[i]*255;//This line duplicated for soap
      outskinim[i*3+1] = soap_im[i]*255;//0;//Modification for soap
    }
    numsp += skin_im[i];
    numtp += towel_im[i];//This line duplicated for soap
	numsoapp += soap_im[i];//Modification for soap
  }
}

void setNoiseLevels(double stren, double & posdyn, Color & cdyn, double & depdyn)
{
  // these are the std. deviations
  double loposdyn = 2.0;  
  double hiposdyn = 15.0;
  double lodepdyn = 1.0;  
  double hidepdyn = 15.0;
  Color locdyn(0.1,0.05,0.05);  
  Color hicdyn(0.5,0.1,0.1);

  // linear relationship bewteen strength and added noise
  double ffac = (10.0-stren)/10.0;
  posdyn = (hiposdyn-loposdyn)*ffac+loposdyn;
  depdyn = (hidepdyn-lodepdyn)*ffac+lodepdyn;
  cdyn.h = (hicdyn.h-locdyn.h)*ffac+locdyn.h;
  cdyn.s = (hicdyn.s-locdyn.s)*ffac+locdyn.s;
  cdyn.v = (hicdyn.v-locdyn.v)*ffac+locdyn.v;
}

void HandTracker::printWeightLimits()//Not called by program
{
  tpf->printWeightLimits();//Not called by program
  lhpf->printWeightLimits();
  rhpf->printWeightLimits();
  fprintf(stderr,"\n");
}

void HandTracker::resetWeightLimits()
{
  spf->resetWeightLimits();//Modification for soap
  tpf->resetWeightLimits();//This line duplicated for soap
  lhpf->resetWeightLimits();
  rhpf->resetWeightLimits();
}
int HandTracker::toggleLearnWeightLimits()//Not called by program
{
  learnweightlimits=(1-learnweightlimits);
 
  // if its the start of a learning episode, 
  // reset the weights
  if (learnweightlimits) {
    resetWeightLimits();
  }

  printWeightLimits();

  tpf->setLearnWeightLimits(learnweightlimits);//Not called by program
  lhpf->setLearnWeightLimits(learnweightlimits);
  rhpf->setLearnWeightLimits(learnweightlimits);
  return learnweightlimits;
}
void HandTracker::updateData(int verbose)
{ 
  double posdyn;
  Color cdyn;
  double depdyn;
  setNoiseLevels(soapStren,posdyn,cdyn,depdyn);//Dec. 9, 2010
  //fprintf(stderr,"noise  %g ",posdyn);
  setNoiseLevels(soapStren,posdyn,cdyn,depdyn);//Dec. 9, 2010
  spf->updateData(cim,nx,ny, soapdp, soapll, soapcll, num_soap_dp, //Dec. 9, 2010
		  alpha_soap, sdw, spw, posdyn, cdyn, depdyn, soapStren, verbose);//Dec. 9, 2010
  setNoiseLevels(towelStren,posdyn,cdyn,depdyn);//This line duplicated for soap
  tpf->updateData(cim,nx,ny, toweldp, towelll, towelcll, num_towel_dp, //This line duplicated for soap
		  alpha_towel, tdw, tpw, posdyn, cdyn, depdyn, towelStren, verbose);//This line duplicated for soap
  setNoiseLevels(lhStren,posdyn,cdyn,depdyn);
  //fprintf(stderr,"noise levels %g ",posdyn);
  lhpf->updateData(cim, nx, ny, lh_skindp, lh_skinll, lh_skincll, num_lh_skin_dp, 
		   alpha_lh, lh_hdw, lh_hpw, posdyn, cdyn, depdyn, lhStren, verbose);
  setNoiseLevels(rhStren,posdyn,cdyn,depdyn);
  //fprintf(stderr," %g\n",posdyn);
  // temporarily here instaed of in estimate Means - should they all be here? 
  lhpf->estimateMean(mlhp);
  rhpf->updateData(cim, nx, ny, rh_skindp, rh_skinll, rh_skincll, num_rh_skin_dp, 
		   alpha_rh, rh_hdw, rh_hpw, posdyn, cdyn, depdyn, rhStren, verbose, &mlhp);

  /*
  if (verbose) 
  fprintf(stderr,"\n");
  */
}

void HandTracker::estimateMeans() {  
  // estimate means
  spf->estimateMean(msp);//Justin - December 8, 2010//Modification for soap
  tpf->estimateMean(mtp);//This line duplicated for soap
  rhpf->estimateMean(mrhp);
  //lhpf->estimateMean(mlhp);
}

void HandTracker::getNewStrengths() {
  // compute strength of new filters
  soapStren = getNewStren(alpha_soap,soapStren,1,sdw,spw,	//Dec, 9 - 2010
			   spf->maxdataweight, spf->mindataweight,
			   spf->maxpriorweight, spf->minpriorweight);
  towelStren = getNewStren(alpha_towel,towelStren,1,tdw,tpw,//This line duplicated for soap
			   tpf->maxdataweight, tpf->mindataweight,//This line duplicated for soap
			   tpf->maxpriorweight, tpf->minpriorweight);//This line duplicated for soap
  //MAX_TOWEL_DATA_WEIGHT,MIN_TOWEL_DATA_WEIGHT,MAX_TOWEL_PRIOR_WEIGHT,MIN_TOWEL_PRIOR_WEIGHT);
  //fprintf(stderr,"towel alpha %g data weight %g prior weight %g ----> stren %d\n",alpha_towel,tdw,tpw,towelStren);
  lhStren = getNewStren(alpha_lh,lhStren,lhcomp,lh_hdw,lh_hpw, 
			lhpf->maxdataweight, lhpf->mindataweight,
			   lhpf->maxpriorweight, lhpf->minpriorweight);
			//MAX_HAND_DATA_WEIGHT,MIN_HAND_DATA_WEIGHT,MAX_HAND_PRIOR_WEIGHT,MIN_HAND_PRIOR_WEIGHT);
  //fprintf(stderr,"left hand alpha %g data weight %g prior weight %g  ----> stren %d\n",alpha_lh,lh_hdw,lh_hpw,lhStren);
  rhStren = getNewStren(alpha_rh,rhStren,rhcomp,rh_hdw,rh_hpw,
			rhpf->maxdataweight, rhpf->mindataweight,
			   rhpf->maxpriorweight, rhpf->minpriorweight);
			//MAX_HAND_DATA_WEIGHT,MIN_HAND_DATA_WEIGHT,MAX_HAND_PRIOR_WEIGHT,MIN_HAND_PRIOR_WEIGHT);
  //fprintf(stderr,"right hand alpha %g data weight %g prior weight %g ----> stren %d\n\n",alpha_rh,rh_hdw,rh_hpw,rhStren);
}

void HandTracker::getAssignComponents(unsigned char *outskinim) 
{
  int i;

  if (componentsComputed)
    return;

  //segment the image into skin colored pixels and towel-colored pixels
  segmentImage(outskinim);

  // get connected components in skin image
  // ccspots[i] is the component of the ith pixel,
  // where ccspots[i] = 0 means its background 
  // compsize[i] is the size of the ith component (but not for i=0
  // cxo, cyo are the centroids, but  numbered from 1 so 
  // cxo[j] is the xo of j+1 st component as labeled in ccspots
  numcc = connectedComponents(skin_im,nx,ny,ccspots,cc);
  
  // now actually figure out where these ccs are (the centroids of them)
  //getCentroids(numcc,ccspots);
  getCentroidsAndBB(numcc,ccspots);

  //Justin - Show Hand Centroid///////////////////////
  //getCentroids(numcc, ccspots, centroidX, centroidY);
  
  // get towel components
  tnumcc = connectedComponents(towel_im,nx,ny,tccspots,cc);//This line duplicated for soap
  //fprintf(stderr,"%d hands components - %d towel components\n",numcc,tnumcc);

  tcomp = 0; //dx nov 28, 2012
  if (tnumcc > 0) {//This line duplicated for soap
    //getCentroids(tnumcc,tccspots);
    getCentroidsAndBB(tnumcc,tccspots);//This line duplicated for soap

	maxtcompsize = 0; //dx nov 28, 2012: seems like they didn't reinitialize it, that's why tcomp is giving weird values
    for (i=0; i<tnumcc; i++) {//This line duplicated for soap
		if (i==0 || tccspots[i].compsize > maxtcompsize) //This line duplicated for soap
		{
			tcomp = i;//This line duplicated for soap
			maxtcompsize = tccspots[i].compsize;//This line duplicated for soap
		}
    }
  }
  
  //dx nov 28, 2012: commented out (i don't understand how the voting here works, but it keeps on swtiching to the wrong blob. so i'll let it just stay as the biggest blob for now
  //// figure out towel component
  //assignTowelComponentsVoting(tccspots,numcc,nx,ny,mtp,towelStren,tcomp,this->towel_size);//This line duplicated for soap
   
  // get soap components
  //snumcc = connectedComponents(soap_im,nx,ny,sccspots,cc);//Dec. 9, 2010 //dx may 31, 2012: get rid of soap
  if (snumcc > 0) {//Dec. 9, 2010
    //getCentroids(tnumcc,tccspots);
    //getCentroidsAndBB(snumcc,sccspots);//Dec. 9, 2010 //dx may 31, 2012: get rid of soap

    for (i=0; i<snumcc; i++) {//Dec. 9, 2010
      //if (i==0 || sccspots[i].compsize > maxscompsize) //Dec. 9, 2010 //dx may 31, 2012: get rid of soap
			scomp = i;//Dec. 9, 2010
      //maxscompsize = sccspots[i].compsize;//Dec. 9, 2010 //dx may 31, 2012: get rid of soap
    }
  }
  // figure out Soap component
  //assignSoapComponentsVoting(sccspots,numcc,nx,ny,msp,soapStren,scomp,this->soap_size); //dx may 31, 2012: get rid of soap

  // figure out which component is the left hand (lhcomp) and which is the right hand (rhcomp)
  // if lhcomp (rhcomp) < 0 then this means there is no appropriate component available for the
  // left hand (right hand)
  lrdist = mlhp.distFrom(mrhp);
  assignHandComponentsVoting(ccspots,numcc,nx,ny,mlhp,mrhp,lrdist,
			     lhStren,rhStren,lhcomp,rhcomp,samecomp, this->hand_size);

  //fprintf(stderr,"lhcomp %d rhcomp %d tcomp %d tstren %d\n",lhcomp,rhcomp,tcomp,towelStren);
  // now, if 
  // mlhp is close to mrhp and 
  // the compsize[lhcomp] is large, then 
  // its likely that the hands are showing up as a single component in the image, 
  // so we'd like to split it up.
  while (((lhcomp < 0 || rhcomp < 0) && samecomp >= 0) && ccspots[samecomp].compsize > this->hand_size.med && 
	 splitComponent(numcc,ccspots[samecomp],ccspots[numcc],mlhp,mrhp)) {
    // old component has changed
    ccspots[samecomp].getCentroid();
    // this is new component
    ccspots[numcc-1].getCentroid();
    assignHandComponentsVoting(ccspots,numcc,nx,ny,mlhp,mrhp,lrdist,
			       lhStren-5,rhStren-5,lhcomp,rhcomp,samecomp, this->hand_size);
  }

  componentsComputed = true;
}
void HandTracker::print(FILE *fh) {//Not called by program
  // other state information first? 
  fprintf(fh,"%d %d %d ",lhStren, rhStren, towelStren);
  fprintf(fh,"%g %g %g ",alpha_lh, alpha_rh, alpha_towel);//Not called by program
  mlhp.print(fh);
  mrhp.print(fh);
  mtp.print(fh);

  lhpf->print(fh);
  rhpf->print(fh);
  tpf->print(fh);//Not called by program
  fprintf(fh,"\n");
}

void HandTracker::scan(FILE *fh) {//Not called by program
  // read next frame from fh as printed by print function above
  fscanf_s(fh,"%d %d %d ",&lhStren, &rhStren, &towelStren); // Babak
  fscanf_s(fh,"%lg %lg %lg ",&alpha_lh, &alpha_rh, &alpha_towel); // Babak//Not called by program
  mlhp.scan(fh);
  mrhp.scan(fh);
  mtp.scan(fh);
  
  lhpf->scan(fh);
  rhpf->scan(fh);
  tpf->scan(fh);//Not called by program
}


// scans through ccspots over the bounding box given by tlx,tly-brx,bry and finds the closest pixel i 
// to point such that ccspots[i] == comp
// dxnote nov 28, 2012: i think it's more like scans through each points in ccspots' components (no bounding box involved)
double findClosestDistanceToComponent(IPixel & point, IComp & comp, int nx, int ny)
{
  int i;
  double d, mindist(nx*ny);

  for (i=0; mindist >= 1 && i<comp.compsize; i++) {
    d = point.distFrom(comp.points[i]);
    if (d < mindist) 
      mindist = d;
  } 
  return mindist;
}
////dx nov 28, 2012
//double findClosestDistanceToComponent_multipleLocations(IComp &locations, IComp &comp, int nx, int ny, int closestLocations[])
//{
//	deque<double> mindists;
//	comp.compsize;
//	return 0;
//
//}


double HandTracker::findDistanceToTowel(handtype ht)
{
  return findDistanceToPoint(mtp,ht);
}
double HandTracker::findDistanceToPoint(IPixel & point, handtype ht)
{
  double dist = -1;
  if (componentsComputed) {
    if (ht == LEFT && lhcomp >= 0)
      dist = findClosestDistanceToComponent(point, ccspots[lhcomp], nx, ny);
    if (ht == RIGHT && rhcomp >= 0)
      dist = findClosestDistanceToComponent(point, ccspots[rhcomp], nx, ny);
  } else {
    if (ht == LEFT)
      dist = findClosestDistanceBinToPoint(skin_im, nx, ny, point.x, point.y, mlhp.x, mlhp.y);
    if (ht == RIGHT)
      dist = findClosestDistanceBinToPoint(skin_im, nx, ny, point.x, point.y, mrhp.x, mrhp.y);
  }
  return dist;
}

void HandTracker::constructDataParticles(unsigned char *outskinim) 
{
  // first, get the connected components and assign them 
  getAssignComponents(outskinim);

  // then, set up alpha values based on this
  // towel just gets everything so is based on pf strength only
  alpha_soap = strentoalpha(soapStren,1);//Dec. 9, 2010
  alpha_towel = strentoalpha(towelStren,1);//This line duplicated for soap
  alpha_lh = strentoalpha(lhStren,lhcomp);
  alpha_rh = strentoalpha(rhStren,rhcomp);
  
  num_lh_skin_dp = num_rh_skin_dp = 0;
  num_skin_dp = 0;
  num_towel_dp = 0;//This line duplicated for soap
  num_soap_dp = 0;//Dec. 9, 2010
  num_dp = 0;
  
  //Dec. 9, 2010 - Justin
  if (alpha_soap > 0.0) {//Dec. 9. 2010
    //construct new data-driven particles  from soap images
    //num_soap_dp = constructAllDataParticles(soapdp,MAX_NUMDATAPARTICLES, sco, ssigco,//Dec. 9. 2010 //dx may 31, 2012: get rid of soap
					     //soapll, soapcll, cim, sccspots[scomp], nx, ny);//Dec. 9. 2010 //dx may 31, 2012: get rid of soap
    num_dp += num_soap_dp;//Dec. 9. 2010
    if (num_soap_dp == 0) //Dec. 9. 2010
      alpha_soap = 0;//Dec. 9. 2010
  }
  if (alpha_towel > 0.0) {//This line duplicated for soap
    //construct new data-driven particles  from towel images
    num_towel_dp = constructAllDataParticles(toweldp,MAX_NUMDATAPARTICLES, tco, tsigco,//This line duplicated for soap
					     towelll, towelcll, cim, tccspots[tcomp], nx, ny);//This line duplicated for soap
    num_dp += num_towel_dp;//This line duplicated for soap
    if (num_towel_dp == 0) //This line duplicated for soap
      alpha_towel = 0;//This line duplicated for soap
  }
  if (alpha_lh > 0) {
    num_lh_skin_dp = constructAllDataParticles(lh_skindp,MAX_NUMDATAPARTICLES, lhco, lhsigco,
					       lh_skinll, lh_skincll, cim, ccspots[lhcomp], nx, ny);
    num_dp += num_lh_skin_dp;
    if (num_lh_skin_dp == 0) 
      alpha_lh = 0;
  }
  if (alpha_rh > 0) {
    num_rh_skin_dp = constructAllDataParticles(rh_skindp,MAX_NUMDATAPARTICLES, rhco, rhsigco,
					       rh_skinll, rh_skincll, cim, ccspots[rhcomp], nx, ny);
    num_dp += num_rh_skin_dp;
    if (num_rh_skin_dp == 0) 
      alpha_rh = 0;
  }
}

// Babak:
void HandTracker::ExtractRegionDistances(unsigned char *skinim, double& towelDist, double& soapDist, double& leftTapDist, double& rightTapDist, double& waterDist, double& sinkDist)
{
	int i;

	// initialize with maximum possible distance
	// sqrt(nx + ny) ... no, use ny (maximum distance vertical) (a good heuristics) (doesn't matter so much anyway) 
	towelDist = ny; // initially hands are away from everything (but don't set it to INT_MAX. Because if hands are not in the scene, then the distance will remain INT_MAX and the value is so large that throw off any learning algorithm we might use)
	soapDist = ny;
	leftTapDist = ny;
	rightTapDist = ny;
	waterDist = ny;
	sinkDist = ny;

	// first, get and assign components if not already done
	// get all connected components = full method
	getAssignComponents(skinim);

	for (i=0; i<numcc; i++)
	{
		if (ccspots[i].compsize > this->hand_size.min)
		{
			towelDist		= min(towelDist,		findClosestDistanceToComponent(mtp, ccspots[i],nx,ny));			// distance to towel
			soapDist		= min(soapDist,			findClosestDistanceToComponent(soapPos, ccspots[i], nx, ny));	// distance to soap
			leftTapDist		= min(leftTapDist,		findClosestDistanceToComponent(LTapPos, ccspots[i], nx, ny));	// distance to left tap
			rightTapDist	= min(rightTapDist,		findClosestDistanceToComponent(RTapPos, ccspots[i], nx, ny));	// distance to right tap
			waterDist		= min(waterDist,		findClosestDistanceToComponent(waterPos, ccspots[i], nx, ny));	// distance to water
			sinkDist		= min(sinkDist,			findClosestDistanceToComponent(sinkPos, ccspots[i], nx, ny));	// distance to sink
		}
	}
}

//void HandTracker::extractObservationsNewer(unsigned char *skinim, int& lhpos, int& rhpos)
void HandTracker::extractObservationsNewer(unsigned char *skinim, int& ntowel_obs, int& nsoap_obs, int& nsoapspout_obs, int& ntapl_obs, int& ntapr_obs, int& nwater_obs, int& nsink_obs, int& nliquidsoap_obs)//Modification for soap
{

	//Modified December 6, 2010 - Justin  added code for soap spout
	int i;
	int numsoap, nums, numss, numtl, numtr, numw, numo, numk;//Modification for soap
	double *disttr = new double[numcc];
	double *disttl = new double[numcc];
	double *distw = new double[numcc];
	double *disto = new double[numcc];
	double *dists = new double[numcc];
	double *distss = new double[numcc];//Modification for soap
	double *distsoap = new double[numcc];//Modification for soap
	double *distk = new double[numcc];
	int *is_at = new int[numcc]; // what region is this blob in?
	int most=0, second_most=0;
	int most_val=HAND_POS_AWAY, second_most_val=HAND_POS_AWAY;
	
	//Add back - Justin - Aug 19, 2010
	//lhpos = rhpos = HAND_POS_AWAY;

	// first, get and assign components if not already done
	// get all connected components = full method
	getAssignComponents(skinim);  	
	//dx nov 28, 2012:
	// just get the biggest blob as towel blob for now (the voting method (assignTowelComponentsVoting) in getAssignComponents always votes wrong when there's other objects of same colour as towel in scene, don't know why, so bypassing it for now)
	mtp = tccspots[tcomp].center;
	if (tccspots[tcomp].compsize < min_towel_dist*min_towel_dist)//dx dec 03, 2012: a towel not in scene or is smaller than the towel square set in calibration is assigned position 1e+10
	{
		mtp.x = 1e+10;
		mtp.y = 1e+10;
	}

	numsoap = numo = nums = numss = numtl = numtr = numw = numk = 0;//Modification for soap
	for (i=0; i<numcc; i++)
	{
		distsoap[i] = disto[i] = dists[i] = distss[i] = disttr[i] = disttl[i] = distw[i] = distk[i] = 1e+10;//Modification for soap
		is_at[i] = -1;
		if (ccspots[i].compsize > this->hand_size.min)
		{			
			distsoap[i] = findClosestDistanceToComponent(msp, ccspots[i],nx,ny);				// distance to actual soap//Modification for soap
			disto[i] = findClosestDistanceToComponent(mtp, ccspots[i],nx,ny);					// distance to towel//This line duplicated for soap
			//disto[i] = findClosestDistanceToComponent_multipleLocations(			//dx nov 28, 2012
			dists[i] = findClosestDistanceToComponent(soapPos, ccspots[i], nx, ny);				// distance to soap
			distss[i] = findClosestDistanceToComponent(soap_spoutPos, ccspots[i], nx, ny);		// distance to soap spout//Modification for soap
			disttl[i] = findClosestDistanceToComponent(LTapPos, ccspots[i], nx, ny);			// distance to left tap
			disttr[i] = findClosestDistanceToComponent(RTapPos, ccspots[i], nx, ny);			// distance to right tap
			distw[i] = findClosestDistanceToComponent(waterPos, ccspots[i], nx, ny);			// distance to water
			distk[i] = findClosestDistanceToComponent(sinkPos, ccspots[i], nx, ny);				// distance to sink

			if (distsoap[i] < min_soap_dist)//Modification for soap//Feb. 28
			{
				numsoap++;	// real components in soap
			}
			//if (disto[i] < min_towel_dist)
			if (disto[i] < min_towel_dist*1.5) //dx dec 19, 2012: using a little more than the towel region's radius because that threshold is too small sometimes when the hand has an arm (i.e. the centroid of the hand is actually at the wrist or above, thus further to the towel than just the centre of the hand)
			{
				is_at[i] = HAND_POS_TOWEL;
				//if (towelStren > 0)//Feb. 15, 2011
				//{
					numo++;	// real components in towel
				//}
			}
			if (dists[i] < min_soap_dist)
			{
				is_at[i] = HAND_POS_SOAP;
				nums++;	// real components in soap
			}

			if (distss[i] < min_soap_spout_dist)//Modification for soap
			{
				numss++;	// real components in soap
			}
			
			//Edited below - Sept 1 - Justin
			//else if ((disttl[i] < min_ltap_dist) || (disttr[i] < min_rtap_dist))
			//{
			//	is_at[i] = HAND_POS_TAP;
			//	numt++;	// real components in left tap
			//}

			if (disttl[i] < min_ltap_dist)
			{
				is_at[i] = HAND_POS_TAP;
				numtl++;	// real components in left tap
			}

			if (disttr[i] < min_rtap_dist)
			{
				is_at[i] = HAND_POS_TAP;
				numtr++;	// real components in right tap
			}

			if (distw[i] < min_water_dist)
			{
				is_at[i] = HAND_POS_WATER;
				numw++;	// real components in water
				numk++; //Justin - Aug 19, 2010
			}
			if (distk[i] < min_sink_dist)
			{
				is_at[i] = HAND_POS_SINK;
				numk++;
			}
		}
	}


	//Added Aug 19, 2010 - Justin Bimbrahw

	//We want this function to output the number of connected components falling into each of the regions
	//After this info becomes available, we can develop a routine to decide on the hand positions
	//It is expected that the number connected components will not be more than 4 in each region

	ntowel_obs = numo; //dxnote aug 30, 2012: seems like you'd get ntowel_obs being the number of blue blobs that's closer to the centroid of red blobs than the minimum towel detection distance (min_towel_dist)
	//dxnote aug 30, 2012: note that the min_towel_dist is defined as regions[REGION_TOWEL].radius/2 , so calib on how big the region of towel matters
	nsoap_obs = nums;
	nsoapspout_obs = numss;//Modification for soap
	ntapl_obs = numtl;
	ntapr_obs = numtr;
	nwater_obs = numw;
	nsink_obs = numk;
	nliquidsoap_obs = numsoap;//Modification for soap

	delete [] dists; delete [] distss; delete [] distw; delete [] disttl; delete [] disttr; delete [] disto;//Modification for soap
	//dx may 30, 2012: memory leak issue
	delete [] distsoap;	delete [] distk; delete [] is_at;
	
	return;



	//// do the easy case first
	//if (numo >= 2)
	//{
	//	rhpos = lhpos = HAND_POS_TOWEL;
 //   	delete [] dists; delete [] distw; delete [] disttl; delete [] disttr; delete [] disto;
	//	return;
	//}

	//// the other cases...

	//// get the blob in the regions with the highest precedence
	//for (i=0; i<numcc; i++)
	//{
	//	if (is_at[i] >= most_val)
	//	{
	//		most = i; //index of blob in region of highest precedence
	//		most_val = is_at[i];
	//	}
	//}
	//// get the blob in the region with the second highest precedence (if there are 2 blobs)
	//if (numcc > 1)
	//{
	//	for (i=0; i<numcc; i++)
	//	{
	//		if (is_at[i] >= second_most_val && i != most)
	//		{
	//			second_most = i; // index of blob in region of second highest precedence
	//			second_most_val = is_at[i];
	//		}
	//	}
	//}

	////if (DEBUG) // Babak
	//if (VERBOSE) // Babak
	//{
	//	printf ("most[%d]=%d, second[%d]=%d, ", most, most_val, second_most, second_most_val);
	//	printf ("towel %d, soap %d, taps %d, water %d, sink %d\n", numo, nums, numt, numw, numk);
	//}

	//// one hand at towel, assume right hand
	//if (numo == 1)
	//{
	//	rhpos = HAND_POS_TOWEL;
	//	if ((ccspots[most].compsize > this->hand_size.med) || (numcc == 1))
	//	{	// big blob, probably both hand together
	//		lhpos = HAND_POS_TOWEL;
	//	}
	//	else
	//	{
	//		lhpos = second_most_val;
	//	}
 //   	delete [] dists; delete [] distw; delete [] disttl; delete [] disttr; delete [] disto;
	//	return;
	//}

	//// one hand at soap, assume left?
	//if (nums >= 1)
	//{
	//	lhpos = HAND_POS_SOAP;
	//	if ((ccspots[most].compsize > this->hand_size.med) || (numcc == 1))
	//	{	// big blob, probably both hand together
	//		rhpos = HAND_POS_SOAP;
	////if (DEBUG) {printf ("soap blob %d > %d\n", ccspots[most].compsize, this->hand_size.med);} // Babak
	//if (VERBOSE) {printf ("soap blob %d > %d\n", ccspots[most].compsize, this->hand_size.med);}// Babak
	//	}
	//	else
	//	{
	//		rhpos = second_most_val;
	//	}
 //   	delete [] dists; delete [] distw; delete [] disttl; delete [] disttr; delete [] disto;
	//	return;
	//}

	//// one hand at taps, but which one?
	//if (numt >= 1)
	//{	
	////if (DEBUG) {printf ("one at tap, second most? %d\n", second_most_val);} // Babak
	//if (VERBOSE) {printf ("one at tap, second most? %d\n", second_most_val);} // Babak
	//	if (disttl[most] < disttr[most])
	//	{	// closer to left tap, assume left hand at tap
	//		lhpos = HAND_POS_TAP;
	//		rhpos = second_most_val;
	//	}
	//	else
	//	{	// closer to right tap, assume right hand at tap
	//		rhpos = HAND_POS_TAP;
	//		lhpos = second_most_val;
	//	}
 //   	delete [] dists; delete [] distw; delete [] disttl; delete [] disttr; delete [] disto;
	//	return;
	//}

	//// one hand at water, doesn't matter which because if we get to here
	//// the other is at water, sink or away.
	//if (numw >= 1)
	//{
	//	rhpos = HAND_POS_WATER;
	//	if ((ccspots[most].compsize > this->hand_size.med) || (numcc == 1))
	//	{	// big blob, probably both hand together
	//		lhpos = HAND_POS_WATER;
	////if (DEBUG) {printf ("water blob %d > %d\n", ccspots[most].compsize, this->hand_size.med);} // Babak
	//if (VERBOSE) {printf ("water blob %d > %d\n", ccspots[most].compsize, this->hand_size.med);} // Babak
	//	}
	//	else
	//	{
	//		lhpos = second_most_val;
	//	}
 //   	delete [] dists; delete [] distw; delete [] disttl; delete [] disttr; delete [] disto;
	//	return;
	//}

	//// one hand at sink, doesn't matter which because if we get to here
	//// the other is away.
	//if (numk >= 1)
	//{
	//	rhpos = HAND_POS_SINK;
	//	if ((ccspots[most].compsize > this->hand_size.med) || (numcc == 1))
	//	{	// big blob, probably both hand together
	//		lhpos = HAND_POS_SINK;
	////if (DEBUG) {printf ("sink blob %d > %d\n", ccspots[most].compsize, this->hand_size.med);} // Babak
	//if (VERBOSE) {printf ("sink blob %d > %d\n", ccspots[most].compsize, this->hand_size.med);} // Babak
	//	}
	//	else
	//	{
	//		lhpos = second_most_val;
	//	}
 //   	delete [] dists; delete [] distw; delete [] disttl; delete [] disttr; delete [] disto;
	//	return;
	//}

	//// if we get to here, both are away
 //   delete [] dists; delete [] distw; delete [] disttl; delete [] disttr; delete [] disto;
	//return;
}
/******************************************************************************/
// 0 a=away
// 1 s=sink
// 2 e=water
// 3 t=tap
// 4 o=soap
// 5 w=towel
// this function actually figures out the hand position observations for the POMDP
void HandTracker::extractObservationsNew(unsigned char *skinim, int& lhpos, int& rhpos)
{
  int i;
  int nums, numt, numw, numo;
  double *dists = new double[numcc];

  // first, get and assign components if not already done
  // get all connected components = full method
  getAssignComponents(skinim);

  // check for a hand at soap
  nums = 0;
  for (i=0; i<numcc; i++) {
    dists[i] =1e+10; 
    if (ccspots[i].compsize > this->hand_size.min) {
      dists[i] = findClosestDistanceToComponent(soapPos, ccspots[i], nx, ny);
      nums += (dists[i] < min_soap_dist);
    }
  }
  if (nums >= 1) {
    lhpos = HAND_POS_SOAP;
    if (nums >= 2) 
      // report soap
      rhpos = HAND_POS_SOAP;
    else {
      // check if closest component is large
      rhpos = HAND_POS_SINK;
      for (i=0; i<numcc; i++) {
	if (dists[i] < min_soap_dist)
	  if (ccspots[i].compsize > this->hand_size.med) 
	    rhpos = HAND_POS_SOAP;
      }
    }
    delete [] dists; 
    return;
  }

  double *disttr = new double[numcc];
  double *disttl = new double[numcc];
  double *distw = new double[numcc];
  double *disto = new double[numcc];
  
  // check for a hand at tap and/or water
  numt = numw = numo =  0;
  for (i=0; i<numcc; i++) {
    disttr[i] = disttl[i] = distw[i] = disto[i] = 1e+10;
    if (ccspots[i].compsize > MINHANDSIZE) {
      disttl[i] = findClosestDistanceToComponent(LTapPos, ccspots[i], nx, ny);
      disttr[i] = findClosestDistanceToComponent(RTapPos, ccspots[i], nx, ny);
      distw[i] = findClosestDistanceToComponent(waterPos, ccspots[i], nx, ny);
      disto[i] = findClosestDistanceToComponent(mtp,ccspots[i],nx,ny);
      numt += (disttl[i] < min_ltap_dist);
      numt += (disttr[i] < min_rtap_dist);
      numw += (distw[i] < min_water_dist);
      numo += (disto[i] < min_towel_dist);
    }
  }
  if (numt >= 2) {
    // report tap
    lhpos = HAND_POS_TAP;
    rhpos = HAND_POS_TAP;
    delete [] dists;    delete [] distw;    delete [] disttl; delete [] disttr;    delete [] disto;
    return;
  } 
  if (numt >= 1) {
    // report one at tap - where is other? 
    // check for water and towel first
    for (i=0; i<numcc; i++) {
      if (disttl[i] >= min_ltap_dist && disttr[i] >= min_rtap_dist) {
		if (distw[i] < min_water_dist) {
		  // report tapwater
		  lhpos = HAND_POS_TAP;
		  rhpos = HAND_POS_WATER;
		  delete [] dists;    delete [] distw;    delete [] disttl; delete [] disttr;    delete [] disto;
		  return;
		} else if (disto[i] < min_towel_dist) {
		  lhpos = HAND_POS_TAP;
		  rhpos = HAND_POS_TOWEL;
		  delete [] dists;    delete [] distw;    delete [] disttl; delete [] disttr;    delete [] disto;
		  return;
		}
		else {
		  lhpos = HAND_POS_TAP;
		  rhpos = HAND_POS_SINK;
		  delete [] dists;    delete [] distw;    delete [] disttl; delete [] disttr;    delete [] disto;
		  return;
		}
	  }
	}
  }
  
  // check for hand at water
  if (numw >= 2) {
    //report water
    lhpos = HAND_POS_WATER;
    rhpos = HAND_POS_WATER;
    delete [] dists;    delete [] distw;    delete [] disttl; delete [] disttr;    delete [] disto;
    return;
  }

  if (numw >= 1) {
    // one at water - where is the other - check for towel?
    for (i=0; i<numcc; i++) {
      if (disto[i] < min_towel_dist) {
	lhpos = HAND_POS_WATER;
	rhpos = HAND_POS_TOWEL;
	delete [] dists;    delete [] distw;    delete [] disttl; delete [] disttr;    delete [] disto;
	return;
      }
    }
    // must be onewater
    lhpos = HAND_POS_WATER;
    rhpos = HAND_POS_SINK;
    delete [] dists;    delete [] distw;    delete [] disttl; delete [] disttr;    delete [] disto;
    return;
  }
  // ok, one more chance -check towel
  if (numo >= 2) {
    //report towel
    lhpos = HAND_POS_TOWEL;
    rhpos = HAND_POS_TOWEL;
    delete [] dists;    delete [] distw;    delete [] disttl; delete [] disttr;    delete [] disto;
     
    return;
  }
  
  if (numo >= 1) {
    // one at towel - the other must be at sink or away or something - report onetowel
    lhpos = HAND_POS_TOWEL;
    rhpos = HAND_POS_SINK;
    delete [] dists;    delete [] distw;    delete [] disttl; delete [] disttr;    delete [] disto;
    return;
  }  
  // finally, proceed as usual if all else fails
  // don't need to do all this if using continuous observation stuff
  // OH MY GOD
  if (lhcomp > 0) {
    lhdist_towel = disto[lhcomp];
    lhdist_soap = dists[lhcomp];
    lhdist_ltap = disttl[lhcomp];
    lhdist_water =  distw[lhcomp];
    lhdist_sink =  findDistanceToPoint(sinkPos,LEFT);
  } else {
    lhdist_towel = lhdist_soap = lhdist_ltap = lhdist_water = lhdist_sink = -1;
  }
  // OH MY GOD
  if (rhcomp > 0) {
    rhdist_towel = disto[rhcomp];
    rhdist_soap = dists[rhcomp];
    rhdist_rtap = disttr[rhcomp];
    rhdist_water =  distw[rhcomp];
    rhdist_sink =  findDistanceToPoint(sinkPos,RIGHT);
  } else {
    rhdist_towel = rhdist_soap = rhdist_rtap = rhdist_water = rhdist_sink = -1;
  }

  // distances to each object that indicate the hand is at that location
  // OH MY GOD
  if (lhdist_towel >= 0.0 && lhdist_towel < min_towel_dist) {
    lhpos = HAND_POS_TOWEL;
  } else if (lhdist_soap >= 0.0 && lhdist_soap < min_soap_dist) {
    lhpos = HAND_POS_SOAP;
  } else if (lhdist_ltap >= 0.0 && lhdist_ltap < min_ltap_dist) {
    lhpos = HAND_POS_TAP;
  } else if (lhdist_water >= 0.0 && lhdist_water < min_water_dist) {
    lhpos = HAND_POS_WATER;
  //} else if (handAtSink(LEFT)) {
  } else if (lhdist_sink >= 0.0 && lhdist_sink < min_sink_dist) {
    lhpos = HAND_POS_SINK;
  } else {
    lhpos = HAND_POS_AWAY;
  }
  // OH MY GOD
  if (rhdist_towel >= 0.0 && rhdist_towel < min_towel_dist) {
    rhpos = HAND_POS_TOWEL;
  } else if (rhdist_soap >= 0.0 && rhdist_soap < min_soap_dist) {
    rhpos = HAND_POS_SOAP;
  } else if (rhdist_rtap >= 0.0 && rhdist_rtap < min_rtap_dist) {
    rhpos = HAND_POS_TAP;
  } else if (rhdist_water >= 0.0 && rhdist_water < min_water_dist) {
    rhpos = HAND_POS_WATER;
  //} else if (handAtSink(RIGHT)) {
  } else if (rhdist_sink >= 0.0 && rhdist_sink < min_sink_dist) {
    rhpos = HAND_POS_SINK;
  } else {
    rhpos = HAND_POS_AWAY;
  }
  //fprintf(stderr,"%d %d %d %d  %g %g\n",lhcomp,rhcomp,lhpos,rhpos,lhdist_sink,rhdist_sink);
  // cheap and dirty ... don't ask - if 
  /*
  if (rhpos == HAND_POS_AWAY) {
    if (lhpos != HAND_POS_AWAY) {
      rhpos = HAND_POS_SINK;
    } 
  } else if (lhpos == HAND_POS_AWAY) {
    if (rhpos != HAND_POS_AWAY) {
      lhpos = HAND_POS_SINK;
    }
  }
  */
  // sanctioned hack!
  // when the hands come too close together, they sometimes appear to be one.
  // In this case, one flock lock on to both hands while the other runs around
  // looking for the other hand.
  // This helps the situation. Only really have to do it for soap, water and sink.
  if (lhpos == HAND_POS_SOAP && rhpos == HAND_POS_AWAY && rhdist_soap < 0.0 ) {
    rhpos = HAND_POS_SOAP;
  }
  if (lhpos == HAND_POS_WATER && rhpos == HAND_POS_AWAY && rhdist_water < 0.0 ) {
    rhpos = HAND_POS_WATER;
  }
  if (lhpos == HAND_POS_SINK && rhpos == HAND_POS_AWAY && rhdist_sink < 0.0 ) {
    rhpos = HAND_POS_SINK;
  }
  if (lhpos == HAND_POS_TOWEL && rhpos == HAND_POS_AWAY && rhdist_towel < 0.0 ) {
    rhpos = HAND_POS_TOWEL;
  }

  if (rhpos == HAND_POS_SOAP && lhpos == HAND_POS_AWAY && lhdist_soap < 0.0 ) {
    lhpos = HAND_POS_SOAP;
  }
  if (rhpos == HAND_POS_WATER && lhpos == HAND_POS_AWAY && lhdist_water < 0.0 ) {
    lhpos = HAND_POS_WATER;
  }
  if (rhpos == HAND_POS_SINK && lhpos == HAND_POS_AWAY && lhdist_sink < 0.0 ) {
    lhpos = HAND_POS_SINK;
  }
  if (rhpos == HAND_POS_TOWEL && lhpos == HAND_POS_AWAY && lhdist_towel < 0.0 ) {
    lhpos = HAND_POS_TOWEL;
  }

    delete [] dists;    delete [] distw;    delete [] disttl; delete [] disttr;    delete [] disto;
  return;
}
void HandTracker::extractObservations(unsigned char *skinim, int & lhpos, int & rhpos)
{
	// first, get and assign components if not already done
	// get all connected components = full method
	getAssignComponents(skinim);

	// only segment image - line drawing method
	//if (!componentsComputed) 
	//segmentImage();


	

	// don't need to do all this if using continuous observation stuff
	lhdist_towel = findDistanceToTowel(LEFT);
	lhdist_soap = findDistanceToPoint(soapPos,LEFT);
	lhdist_ltap = findDistanceToPoint(LTapPos,LEFT);
	lhdist_rtap = findDistanceToPoint(RTapPos,LEFT);
	lhdist_water =  findDistanceToPoint(waterPos,LEFT);
	lhdist_sink =  findDistanceToPoint(sinkPos,LEFT);

	rhdist_towel = findDistanceToTowel(RIGHT);
	rhdist_soap = findDistanceToPoint(soapPos,RIGHT);
	rhdist_ltap = findDistanceToPoint(LTapPos,RIGHT);
	rhdist_rtap = findDistanceToPoint(RTapPos,RIGHT);
	rhdist_water =  findDistanceToPoint(waterPos,RIGHT);
	rhdist_sink =  findDistanceToPoint(sinkPos,RIGHT);

  // distances to each object that indicate the hand is at that location
  if (lhdist_towel >= 0.0 && lhdist_towel < min_towel_dist) {
    lhpos = HAND_POS_TOWEL;
  } else if (lhdist_soap >= 0.0 && lhdist_soap < min_soap_dist) {
    lhpos = HAND_POS_SOAP;
  } else if (lhdist_ltap >= 0.0 && lhdist_ltap < min_ltap_dist) {
    lhpos = HAND_POS_TAP;
  } else if (lhdist_rtap >= 0.0 && lhdist_rtap < min_rtap_dist) {
    lhpos = HAND_POS_TAP;
  } else if (lhdist_water >= 0.0 && lhdist_water < min_water_dist) {
    lhpos = HAND_POS_WATER;
  //} else if (handAtSink(LEFT)) {
  } else if (lhdist_sink >= 0.0 && lhdist_sink < min_sink_dist) {
    lhpos = HAND_POS_SINK;
  } else {
    lhpos = HAND_POS_AWAY;
  }

  if (rhdist_towel >= 0.0 && rhdist_towel < min_towel_dist) {
    rhpos = HAND_POS_TOWEL;
  } else if (rhdist_soap >= 0.0 && rhdist_soap < min_soap_dist) {
    rhpos = HAND_POS_SOAP;
  } else if (rhdist_ltap >= 0.0 && rhdist_ltap < min_ltap_dist) {
    rhpos = HAND_POS_TAP;
  } else if (rhdist_rtap >= 0.0 && rhdist_rtap < min_rtap_dist) {
    rhpos = HAND_POS_TAP;
  } else if (rhdist_water >= 0.0 && rhdist_water < min_water_dist) {
    rhpos = HAND_POS_WATER;
  //} else if (handAtSink(RIGHT)) {
  } else if (rhdist_sink >= 0.0 && rhdist_sink < min_sink_dist) {
    rhpos = HAND_POS_SINK;
  } else {
    rhpos = HAND_POS_AWAY;
  }

  // sanctioned hack!
  // when the hands come too close together, they sometimes appear to be one.
  // In this case, one flock lock on to both hands while the other runs around
  // looking for the other hand.
  // This helps the situation. Only really have to do it for soap, water and sink.
  if (lhpos == HAND_POS_SOAP && rhpos == HAND_POS_AWAY && rhdist_soap < 0.0 ) {
    rhpos = HAND_POS_SOAP;
  }
  if (lhpos == HAND_POS_WATER && rhpos == HAND_POS_AWAY && rhdist_water < 0.0 ) {
    rhpos = HAND_POS_WATER;
  }
  if (lhpos == HAND_POS_SINK && rhpos == HAND_POS_AWAY && rhdist_sink < 0.0 ) {
    rhpos = HAND_POS_SINK;
  }
  if (lhpos == HAND_POS_TOWEL && rhpos == HAND_POS_AWAY && rhdist_towel < 0.0 ) {
    rhpos = HAND_POS_TOWEL;
  }

  if (rhpos == HAND_POS_SOAP && lhpos == HAND_POS_AWAY && lhdist_soap < 0.0 ) {
    lhpos = HAND_POS_SOAP;
  }
  if (rhpos == HAND_POS_WATER && lhpos == HAND_POS_AWAY && lhdist_water < 0.0 ) {
    lhpos = HAND_POS_WATER;
  }
  if (rhpos == HAND_POS_SINK && lhpos == HAND_POS_AWAY && lhdist_sink < 0.0 ) {
    lhpos = HAND_POS_SINK;
  }
  if (rhpos == HAND_POS_TOWEL && lhpos == HAND_POS_AWAY && lhdist_towel < 0.0 ) {
    lhpos = HAND_POS_TOWEL;
  }

  return;
}
void HandTracker::zeroAlpha()
{
  alpha_soap = 0.0;//Dec. 9, 2010
  alpha_towel = 0.0;//This line duplicated for soap
  alpha_lh = 0.0;
  alpha_rh = 0.0;
}

void HandTracker::zeroHComp() {
  rhcomp = lhcomp = 0;
}
void HandTracker::zeroDP() {//Not called by program
  num_lh_skin_dp = num_rh_skin_dp = 0;
  num_skin_dp = 0;
  num_towel_dp = 0;//Not called by program//This line duplicated for soap
  num_soap_dp = 0;//Dec. 9, 2010
  num_dp = 0;
}

void HandTracker::swapLRPF()
{
  ParticleFilter *tmppf;
  int tmpStren,tmp;
  double alpha_tmp;

  tmppf = lhpf;
  lhpf = rhpf;
  rhpf = tmppf;
  
  tmpStren = lhStren;
  lhStren = rhStren;
  rhStren = tmpStren;
  
  alpha_tmp = alpha_lh;
  alpha_lh = alpha_rh;
  alpha_rh = alpha_tmp;
  
  tmp = mlhp.x;
  mlhp.x = mrhp.x;
  mrhp.x = tmp;
  
  tmp = mlhp.y;
  mlhp.y = mrhp.y;
  mrhp.y = tmp;
}
bool outRegion(IPixel px, int nx, int ny)
{
  return (px.x > 540);
}

//is a single hand at distance d from the towel using the towel? 
bool HandTracker::handUsingTowel(double d, double td)
{
  return (d < td);
}
bool HandTracker::handUsingSoap(IPixel & mhp, double td) 
{
  // soap is at (10,100)-(130,205)
  //return mhp.in(10,100,130,205);
  return mhp.in(soapSpace);
}
bool HandTracker::handUsingTap(IPixel & mhp, double td)
{
  // tap is at (250,35)- (305,95)
  //return mhp.in(250,35,305,95);
  return mhp.in(tapSpace);
}

bool HandTracker::handAtWater(IPixel & mhp, double td)
{
  // water is at (240,75)-(315,165)
  //return mhp.in(240,75,315,165);
  return mhp.in(waterSpace);
}
bool HandTracker::handAtSink(handtype ht)
{
  if (ht == LEFT)
    return handAtSink(mlhp);
  else
    return handAtSink(mrhp);
}
bool HandTracker::handAtSink(IPixel & mhp, double td)
{
  // sink is (120,80)-(435,255);
  //return mhp.in(120,80,435,255);
  return mhp.in(sinkSpace);
}

int HandTracker::getHandPos(IPixel & avgh, IPixel & avgminh, double avght, double tapd, double soapd, double waterd, double toweld, double sinkd) 
{
  //(observations	(handpos away sink water tap soap towel))
  // away = 0
  // sink = 1;
  // water = 2;
  // tap  = 3;
  // soap = 4;
  // towel = 5;
  if (handUsingTap(avgminh,tapd))
    return 3;
  else if (handUsingSoap(avgminh,soapd))
    return 4;
  else if (handAtWater(avgminh,waterd))
    return 2;
  else if (handUsingTowel(avght,toweld))
    return 5;
  else if (handAtSink(avgh,sinkd))
    return 1;
  return 0;
}
int HandTracker::getLHPos(int & lhpos, int & rhpos, IPixel & avglh, IPixel & avgrh, IPixel & avgminlh, IPixel & avgminrh, double avglht, double avgrht)
{
  // left hand
  lhpos = getHandPos(avglh,avgminlh,avglht,40,30,40,100,100);
  rhpos = getHandPos(avgrh,avgminrh,avgrht,40,50,40,100,100);
  if (lhpos == 3 || rhpos == 3) 
    return 3;
  else if (lhpos == 4 && rhpos == 4)
    return 4;
  else if (lhpos == 2 || rhpos == 2)
    return 2;
  else if (lhpos == 5 || rhpos == 5)
    return 5;
  else if (lhpos == 1 || rhpos == 1)
    return 1;
  return 0;
}

// fills lhpos and rhpos with current hand positions 
// and returns if the hand positions are estimated to have changed
int HandTracker::getAvgPos(int frame)
{
  // gotNewPos = 0 means no new position
  // = 1 means new position from hands
  // = 2 means new position due to timeout
  int gotNewPos(0);

  // now estimate min y positions for each hand and 
  // figure out if a significant change has occurred but the
  // hands have stopped moving ? - signal 
  // the change if so.
  rhpf->estimateMinY(minrhp, ny);
  lhpf->estimateMinY(minlhp, ny);
  updateAvgPositions(mlhp, mrhp, mtp, minlhp, minrhp,
		     lhStren, rhStren, towelStren,
		     lh_hist, rh_hist,t_hist,
		     mlh_hist, mrh_hist, 
		     lht_hist, rht_hist,
		     lhs_hist, rhs_hist, ts_hist, 
		     history_index, hist_discount, hist_length,
		     avglh, avgrh, avgt, avgminlh, avgminrh,
		     avglht, avgrht, 
		     avglhv, avgrhv, avgtv, avglhtv, avgrhtv);
  
  int newhandspos, newlhpos, newrhpos;
  newhandspos = getLHPos(newlhpos, newrhpos, avglh, avgrh, avgminlh, avgminrh, avglht, avgrht);
  elapsedFrames++;
  return gotNewPos;
}
void HandTracker::reset()
{
  lhStren = rhStren = towelStren = soapStren = 0;//Dec. 9, 2010
  alpha_lh = alpha_rh = alpha_towel = alpha_soap = 1.0;//Dec. 9, 2010
}
int HandTracker::update(unsigned char *im, int frame, bool updateAvgPos, unsigned char *finalim, unsigned char *outskinim, unsigned char *bestpim)
{
  numsp = numtp = numsoapp = 0;//Modification for soap
  getHSV(RGBHSVLookup,im,cim,nx,ny);

  int gotNewPos(0);

  componentsComputed = false;


  //getAssignComponents(outskinim);

  //return 0;

  constructDataParticles(outskinim);

  //fprintf(stderr,"updating particle filter with alpha_towel %g alpha_lh %g alpha_rh %g ",alpha_towel,alpha_lh, alpha_rh);
  //fprintf(stdout,"%d %d %d %d %g %g %g ",frame,towelStren,lhStren,rhStren,alpha_towel,alpha_lh,alpha_rh);

  updateData(0);

  estimateMeans();

  getNewStrengths();

  // add penalty if the two hands are close together and both are strong
  // this is the only interaction between the two hand filters. 
  double dist = mrhp.distFrom(mlhp);
  if (dist < MIN_HAND_SEPARATION && lhStren > 6 && rhStren > 6) {
    lhStren = rhStren = 0;
  }
  //fprintf(stderr,"towel strength %d lh stren %d rh stren %d\n",towelStren,lhStren,rhStren);
  
  // if left is to the right of right and they are not too close together, swap the hands
  if (mrhp.x < mlhp.x && dist > 50) 
    swapLRPF();

  if (updateAvgPos) 
    gotNewPos = getAvgPos(frame);


  if (finalim != NULL) {
    //drawRegion(im,tmpim,nx,ny,handspos);
    lhpf->drawParticles(im,finalim,nx,ny,255,0,0,50);
    rhpf->drawParticles(finalim,tmpim,nx,ny,0,0,255,50);
    tpf->drawParticles(tmpim,finalim,nx,ny,255,255,0,50);//This line duplicated for soap
	spf->drawParticles(tmpim,finalim,nx,ny,255,255,0,50);//Dec. 9, 2010
    if (soapStren > 0)  //Dec. 9, 2010
      msp.drawOnImage(finalim,nx,ny,0,0,0,-5,5);//Dec. 9, 2010
	if (towelStren > 0) //This line duplicated for soap 
      mtp.drawOnImage(finalim,nx,ny,0,0,0,-5,5);//This line duplicated for soap
    if (rhStren > 0) 
      mrhp.drawOnImage(finalim,nx,ny,255,0,255,-5,5);
    if (lhStren > 0) 
      mlhp.drawOnImage(finalim,nx,ny,0,0,255,-5,5);
  }
  if (bestpim  != NULL) {
    lhpf->drawBestParticle(im,bestpim,nx,ny,255,0,0,2);
    rhpf->drawBestParticle(bestpim,tmpim,nx,ny,0,0,255,2);
    tpf->drawBestParticle(tmpim,bestpim,nx,ny,255,255,0,2);//This line duplicated for soap
	spf->drawBestParticle(tmpim,bestpim,nx,ny,255,255,0,2);//Dec. 9, 2010
  }
  return gotNewPos;
}
void HandTracker::drawDemoImage(unsigned char *im,unsigned char *demoim, int lhpos, int rhpos)//Not called by program
{
  // int i,j,k; // Babak
  //copy  im to demoim
  memcpy(demoim,im,3*nx*ny*sizeof(unsigned char));
  // draw on the components corresponding to each hand
  drawCompOnImage(demoim,ccspots+lhcomp,0);
  drawCompOnImage(demoim,ccspots+rhcomp,1);
  drawCompOnImage(demoim,tccspots+tcomp,2);//Not called by program

  // draw on the region that each hand is assigned to (lhpos, rhpos)
  switch (lhpos) {
  case HAND_POS_SINK:
    brightenRegion(demoim,sinkPos,min_sink_dist);
    break;
  case HAND_POS_WATER:
    brightenRegion(demoim,waterPos,min_water_dist);
    break;
  case HAND_POS_TAP:
    brightenRegion(demoim,RTapPos,min_rtap_dist);
    brightenRegion(demoim,LTapPos,min_ltap_dist);
    break;
  case HAND_POS_SOAP:
    brightenRegion(demoim,soapPos,min_soap_dist);
    break;
  case HAND_POS_TOWEL:
    brightenRegion(demoim,tccspots[tcomp].center,min_towel_dist);//Not called by program
    break;
  }

  
}
void HandTracker::brightenRegion(unsigned char * im, IPixel & pos, double radius)
{
  int index;
  //for (int i=max(0,pos.y-radius); i<min(ny,pos.y+radius); i++) { // Babak
  for (int i=max(0,int(pos.y-radius)); i<min(ny,int(pos.y+radius)); i++) { // Babak
    //for (int j=max(0,pos.x-radius); j<min(nx,pos.x+radius); j++) { // Babak
	for (int j=max(0,int(pos.x-radius)); j<min(nx,int(pos.x+radius)); j++) { // Babak
      index = 3*(i*nx+j);
      im[index] = min(255,im[index]+100);
      im[index+1] = min(255,im[index+1]+100);
      im[index+2] = min(255,im[index+2]+100);
    }
  }
}
void HandTracker::drawCompOnImage(unsigned char*im, IComp * comp, int col)
{
  int index;
  for (int i=0; i<comp->compsize; i++) {
    index = (comp->points[i].y)*nx+(comp->points[i].x);
    if (col == 0) {
      im[index*3] = min(255,im[index*3]+100);
    } else if (col == 1) {
      im[index*3+1] = min(255,im[index*3+1]+100);
    } else {
      im[index*3+2] = min(255,im[index*3+2]+100);
    }
  }
}
void HandTracker::drawOnImage(unsigned char *im, unsigned char *finalim)//Not called by program
{
  // set to 50 for normal drawing
    int numtodraw = 50;
    lhpf->drawParticles(im,finalim,nx,ny,255,0,0,numtodraw);
    if (numtodraw >0) {
      rhpf->drawParticles(finalim,tmpim,nx,ny,0,0,255,numtodraw);
      tpf->drawParticles(tmpim,finalim,nx,ny,255,255,0,numtodraw);//Not called by program
    }
    if (towelStren > 0)  
      mtp.drawOnImage(finalim,nx,ny,255,255,0,-5,5);
    if (rhStren > 0) 
      mrhp.drawOnImage(finalim,nx,ny,255,0,255,-5,5);
    if (lhStren > 0) 
      mlhp.drawOnImage(finalim,nx,ny,0,0,255,-5,5);
}

// gets the hue image from im into hue_im, and segments it and puts the result in skin_im
// clook is a lookup table clook[r][g][b] = hsv color
void getHSV(Color ***clook, unsigned char *im, Color *cim, int nx, int ny)
{
  int i,j,index,index3(0);
  int row(0);
  for (i=0; i<ny; i++) {
    for (j=0; j<nx; j++) {
      index = row+j;


      cim[index] = clook[im[index3]][im[index3+1]/4][im[index3+2]/4];

      index3 += 3;
    }
    row += nx;
  }
}  
// searches et to find which rows (uc, lc) contain labels (ul, ll) resp.
void findcomps(int ul, int ll, int *uc, int *lc, int * equivtable) {
  int i,j,ui,ufound,lfound;
  ufound = lfound = 0;
  for (i=0; i<MAXIMUM_COMPS; i++) {
    ui = i*MAXIMUM_COMPS;
    for (j=1; j<=equivtable[ui]; j++) {
      if (ufound == 0 && equivtable[ui+j] == ul) {
	*uc = i;
	ufound = 1;
      }
      if (lfound == 0 && equivtable[ui+j] == ll) {
	*lc = i;
	lfound = 1;
      }
      if (lfound == 1 && ufound == 1) 
	return;
    }
  }
}
int getClass(int ul, int nc, int*equivtable) {
  int i,ui;
  for (i=0; i< MAXIMUM_COMPS; i++) {
    ui = i*MAXIMUM_COMPS;
    if (equivtable[ui] > 0) 
      for (int j=1; j<=equivtable[ui]; j++) 
	if (equivtable[ui+j] == ul) 
	  return i;
  }
  return -1;
}
void printTable(int *equivtable)
{
  int k,i;
  // print the equiv table
  for (k=0; k<MAXIMUM_COMPS; k++) {
    if (equivtable[k*MAXIMUM_COMPS] > 0) {
      fprintf(stderr,"%d ",k);
      for (i=0; i<equivtable[k*MAXIMUM_COMPS]+1; i++) 
	fprintf(stderr,"%d ",equivtable[k*MAXIMUM_COMPS+i]);
      fprintf(stderr,"\n");
    }
  }
}

// equivtable is a MAXIMUM_COMPS by MAXIMUM_COMPS matrix, where
// each row is an equivalent class of labels in the 
// connected components algorithm
// the first element of equivtable in every row 
// is the number of labels in that set
// the remainder of the elements in the row are the labels in that set.
// a row can have first element 0
// returns the connected components image in cc labeled appropriately,
// and a vector compsize of component sizes (some may be zero). The return
// value of this function is the number of elements in compsize
int equivtable[MAXIMUM_COMPS*MAXIMUM_COMPS];
int etablemap[MAXIMUM_COMPS];
int connectedComponents(int *skin, int nx, int ny, IComp *conc, int *cc) {
  int index,i,j,k;
  int labelnum=1;
  int ul,ll,uc,lc,lci,uci,lcnum,ucnum;
//  int tmpuc, tmplc;
  
  for (i=0; i<MAXIMUM_COMPS; i++)
    equivtable[i*MAXIMUM_COMPS] = 0;

  for (i=0; i<ny; i++)
    cc[i*nx] = 0;
  for (j=0; j<nx; j++)
    cc[j] = 0;
  for (i=1; i<ny; i++) {
    for (j=1; j<nx; j++) {
      index = i*nx+j;
      cc[index] = 0;
      if (skin[index] > 0) {
	ul = cc[(i-1)*nx+j];
	ll = cc[i*nx+j-1];
	if (ul > 0 && ll == 0) 
	  cc[index] = ul;
	else if (ll > 0 && ul == 0)
	  cc[index] = ll;
	else if (ul > 0 && ll > 0) {
	    cc[index] = ul;
	    // if labels are different, we update the equivalence table
	    if (ul != ll) {
	      //findcomps(ul,ll,&tmpuc,&tmplc,equivtable);
	      uc = etablemap[ul];
	      lc = etablemap[ll];
	      // uc, lc are the classes in equivtable to which ul, ll labels belong
	      if (uc != lc) {
		// merge uc into lc
		lci = lc*MAXIMUM_COMPS; uci = uc*MAXIMUM_COMPS;
		lcnum = equivtable[lci]; ucnum = equivtable[uci];
		for (k=1; k<ucnum+1; k++) {
		  if (lci+lcnum+1 >= MAXIMUM_COMPS*MAXIMUM_COMPS || uci+k >= MAXIMUM_COMPS*MAXIMUM_COMPS) 
		    fprintf(stderr,"whoops!");
		  *(equivtable+lci+lcnum+1) = *(equivtable+uci+k);
		  etablemap[equivtable[uci+k]] = lc;
		  lcnum++;
		}
		if (lcnum > MAXIMUM_COMPS) {
		  fprintf(stderr,"too many components (%d) - increase MAXIMUM_COMPS!!\n", lcnum); 
		  exit(1);
		}
		equivtable[lci] = lcnum;
		equivtable[uci] = 0;
	      }
	    }
	} else {
	  // new entry in equivalence table
	  // find first 0 entry 
	  k=0;
	  while (k < MAXIMUM_COMPS && equivtable[k*MAXIMUM_COMPS] > 0)
	    k++;
	  if (k>=MAXIMUM_COMPS) {
	    fprintf(stderr,"too many components (%d) - increase MAXIMUM_COMPS!\n",k); 
	    exit(1);
	  }
	  equivtable[k*MAXIMUM_COMPS] = 1;
	  equivtable[k*MAXIMUM_COMPS+1] = labelnum;
	  etablemap[labelnum] = k;
	  cc[index] = labelnum;
	  labelnum++;
	}
      }
    }
  }
  
  //figure out number of components and  remove all zero
  // build maptable maps individual small components to large merged ones
  // the etablemap here is different than above because we remove
  // the zero-size components which are present in the equivalence table
  int tcomps = 0;
  for (k=0; k<MAXIMUM_COMPS; k++) {
    if (equivtable[k*MAXIMUM_COMPS] > 0) {
      for (j=1; j<=equivtable[k*MAXIMUM_COMPS]; j++) {
	etablemap[equivtable[k*MAXIMUM_COMPS+j]] = tcomps;
      }
      tcomps++;
    }
    conc[k].compsize = 0;
  }
  // scan the picture one more time to re-assign labels
  // also count the number of pixels in each component
  for (i=1; i<ny; i++) {
    for (j=1; j<nx; j++) {
      index = i*nx+j;
      if (cc[index] > 0) {
	// get connected component index for cc[index] from etablemap
	k = etablemap[cc[index]];
	if (k >= 0) {
	  cc[index] = k+1;
	  conc[k].points[conc[k].compsize].x = j;
	  conc[k].points[conc[k].compsize].y = i;
	  conc[k].compsize++;
	} else {
	  // means this component is is background - set to 0
	  cc[index] = 0;
	}
      }
    }
  }
  int maxtcomps = tcomps;
  // finally, remove too small or too large components
  k = 0;
  while (k < maxtcomps) {
    if (conc[k].compsize < MIN_COMP_SIZE || conc[k].compsize > MAX_COMP_SIZE) {
      // replace this component with the last one
      conc[k].setTo(conc[maxtcomps-1]);
      maxtcomps--;
    } else {
      k++;
    }
  }
  return maxtcomps;
}

//Justin - Oc 22, 2010
void getCentroids(int numcc, IComp *ccspots, int &centroidX, int &centroidY)
{
  int k;
  for (k=0; k<numcc; k++) 
    ccspots[k].getCentroid(centroidX, centroidY);
}

void getCentroids(int numcc, IComp *ccspots)
{
  int k;
  for (k=0; k<numcc; k++) 
    ccspots[k].getCentroid();
}
// also gets bounding box, top left (tlx, tly) bottom right (brx, bry)
void getCentroidsAndBB(int numcc, IComp *ccspots)
{
  int k;
  for (k=0; k<numcc; k++) 
    ccspots[k].getCentroidAndBB();
}
// converts a strengh value 0-10 to an alpha value
double strentoalpha(int stren, int comp)
{
  double a;
  // a = (1-(stren > 0)*min(1.0,stren/8.0))*(comp>=0);
  if (stren == 0) {
    if (comp < 0) 
      a = 0.0;
    else
      a = 1.0;
  } else {
    if (comp < 0) {
      a = 0.0;
    } else {
      //a = 1.0-min(1.0,stren/10.0);
      a = 1.0-(stren+1.0)/13.0;
    }
  }
  return a;
}

int votefn(int stren, IPixel & mp, IPixel & center, int nx, int ny, handtype hs)
{
  int vote = (int) (floor((10.0-stren)/8.0));
  double dist;
  if (stren > 0) {
    dist = mp.distFrom(center);
    dist = max(1.0,dist);
    vote += 5*min(10,(int) floor(50.0/dist));
  } 
  if (hs == LEFT && center.x < nx/2.0) {
    vote += 1;
  }
  if (hs == RIGHT && center.x > nx/2.0) {
    vote += 1;
  }
  return vote;
}
//euclidean distance betweeen vectors v1 and v2
// assumes v1 and v2 are same length
double eDist(double *v1, double *v2, int len) {
  int i;
  double sum = 0;
  for (i=0; i<len; i++) 
    sum += (v1[i]-v2[i])*(v1[i]-v2[i]);
  return sqrt(sum);
}

//calculates the mean vector of all the vectors (rows) in v1
// which have an equivalent index in ind which matches index 
void mean(double *mean, double **v1, int len, int fvdim, int *ind, int index) {
  int i;
  for (i=0; i<fvdim; i++)
    mean[i] = 0.0;
  int counts=0;
  for (i=0; i<len; i++) 
    if (ind[i] == index) {
      counts++;
      for (int j=0; j<fvdim; j++) 
	mean[j] += v1[i][j];
    }
  if (counts > 0)
    for (int j=0; j<fvdim; j++)
      mean[j] = mean[j]/counts;
  else 
    for (int j=0; j<fvdim; j++)
      mean[j] = 0.0;
}
// does K-means on dat where K is defined by the
// length of the first dimension of mns, which are
// the initial means -
// returns the groupings
// dat assumed correct dimensions for model (featureset not needed)
void Kmeans(double **dat, int numdat, int fvdim, double **mns,int xst,
	    int *groupings, double tolerance) {
  int i,j,mini;
  double temp, mindist;
  int converged = 0;

  double *tempmean = new double[fvdim];
  int numits(0);
  while (!converged) {
    //classify the vectors
    for (i=0; i<numdat; i++) {
      mindist = eDist(dat[i],mns[0],fvdim);
      mini = 0;
      for (j=1;j<xst;j++) {
	temp = eDist(dat[i],mns[j],fvdim);
	if (temp < mindist) {
	  mindist = temp;
	  mini=j;
	}
      }
      //vector i classified as mini
      groupings[i] = mini;
    }

    // get new means and calculate distance from old means at same time
    temp = 0;
    for (j=0; j<xst; j++) {
      mean(tempmean,dat,numdat,fvdim,groupings,j);
      temp += eDist(tempmean,mns[j],fvdim);
      for (i=0; i<fvdim; i++)
	mns[j][i] = tempmean[i];
    }
    //fprintf(stderr," curr: %f tol: %f\n",temp,tolerance);
    converged = (temp < tolerance || numits > 100);
    numits++;
  }
  //dx may 30, 2012: memory leak issue
  delete [] tempmean;
}

// splits the component comp into two pieces
// with initial means mlhp, mrhp
bool splitComponent(int & numcc, IComp & ccspots, IComp & newccspot, IPixel & mlhp, IPixel & mrhp)
{
  int i,j,k;
  bool success;
  double **pts;
  double *mns[2];
  int *gps;
  double tol(0.001);
  k=0;
  int numpts = ccspots.compsize;
  pts = new double*[numpts];
  gps = new int[numpts];
  for (j=0; j<numpts; j++) 
    pts[j] = new double[2];
  mns[0] = new double[2];
  mns[1] = new double[2];
  mns[0][0] = mlhp.x;
  mns[0][1] = mlhp.y;
  mns[1][0] = mrhp.x;
  mns[1][1] = mrhp.y;
  
  for (i=0; i<numpts; i++) {
    pts[k][0] = ccspots.points[i].x;
    pts[k][1] = ccspots.points[i].y;
  }
  // split it up
  Kmeans(pts,k,2,mns,2,gps,tol);

  numcc++;
  int oldcsize(0);
  newccspot.zero();
  // reassign the components
  while (oldcsize < ccspots.compsize) {
    if (gps[oldcsize] == 1) {
      newccspot.points[newccspot.compsize].setTo(ccspots.points[oldcsize]);
      ccspots.points[oldcsize].setTo(ccspots.points[ccspots.compsize-1]);
      gps[oldcsize] = gps[ccspots.compsize-1];
      ccspots.compsize--;
      newccspot.compsize++;
    } else {
      oldcsize++;
    }
  }
  if (oldcsize == 0) {
    // whoops!  - everything is new component
    // copy back over
    success = false;
    ccspots.compsize = newccspot.compsize;
    for (i=0; i<newccspot.compsize; i++) 
      ccspots.points[i].setTo(newccspot.points[i]);
  } else if (newccspot.compsize == 0) {
    // whoops - nothing got copied over!
    success = false;
  } else {
    // great - everything worked - component got split
    numcc++;
    success = true;
  }
  delete [] gps;
  for (j=0; j<numpts; j++) 
    delete [] pts[j];
  delete [] pts;
  delete [] mns[0];
  delete [] mns[1];
  return success;
}


// new version  - based on voting of each hand on each component
void assignHandComponentsVoting(IComp * ccspots, int numcomps, 
			  int nx, int ny, IPixel & mlhp, IPixel & mrhp, double lrdist, 
			  int lhStren, int rhStren, int & lhcomp, int & rhcomp, int & samecomp, cc_size size)
{
  int i,j;
  lhcomp = 0;
  rhcomp = 0;
  samecomp = 0;
  
//printf("min %d, max %d, comps %d\n",size.min, size.max, ccspots[0].compsize);

  if (numcomps == 0) 
    return;

  int *lhvotes = new int[numcomps];
  int *rhvotes = new int[numcomps];
  for (i=0; i<numcomps; i++) 
    lhvotes[i] = rhvotes[i] = 0;
 

  for (i=0; i<numcomps; i++) {
    if (outRegion(ccspots[i].center,nx,ny) ||
	ccspots[i].compsize <  size.min || 
	ccspots[i].compsize > size.max) {
      lhvotes[i] = rhvotes[i] = 0;
    } else {
      lhvotes[i] += votefn(lhStren,mlhp,ccspots[i].center,nx,ny,LEFT);
      rhvotes[i] += votefn(rhStren,mrhp,ccspots[i].center,nx,ny,RIGHT);
    }
  }

  // now pick best association based on votes
  int bestlh(-1), bestrh(-1), bestsame(-1);
  int totvotes, maxvotes = 0, maxsamevotes = 0;
  if (numcomps == 1) {
    bestsame = 0;
    if (lhvotes[0] > rhvotes[0]) {
      bestlh = 0;
    } else {
      bestrh = 0;
    }
  } else {
    for (i=0; i<numcomps; i++) {
      for (j=0; j<numcomps; j++) {
	totvotes = lhvotes[i]+rhvotes[j];
	if (i != j && totvotes > maxvotes) {
	  maxvotes = totvotes;
	  bestlh = i;
	  bestrh = j;
	}
	if (i == j && totvotes > maxsamevotes) {
	  maxsamevotes = totvotes;
	  bestsame = i;
	}
      }
    }
  }
  samecomp = lhcomp = rhcomp = -1;
  if (bestlh >=0 && lhvotes[bestlh] > 0)
    lhcomp = bestlh;
  if (bestrh >= 0 && rhvotes[bestrh] > 0)
    rhcomp = bestrh;
  if (bestsame >=0)
    samecomp = bestsame;
  delete [] lhvotes;
  delete [] rhvotes;
}

// new version  - based on voting of each hand on each component
void assignTowelComponentsVoting(IComp * ccspots, int numcomps, //This line duplicated for soap
			  int nx, int ny, IPixel & mtp, int tStren, int & tcomp, cc_size size)//This line duplicated for soap
{
  // int i,j;// Babak
  int i;
  tcomp = 0;

  if (numcomps == 0) 
    return;

  int *tvotes = new int[numcomps];
  for (i=0; i<numcomps; i++) 
    tvotes[i] = 0;
 

  for (i=0; i<numcomps; i++) {
    if (outRegion(ccspots[i].center,nx,ny) ||
	ccspots[i].compsize <  size.min || 
	ccspots[i].compsize > size.max) {
      tvotes[i] = 0;
    } else {
      tvotes[i] += votefn(tStren,mtp,ccspots[i].center,nx,ny,TOWEL);
    }
    //mtp.print(stderr);
    //ccspots[i].center.print(stderr);
    //fprintf(stderr,"%d %d\n",ccspots[i].compsize,tvotes[i]);
  }

  // now pick best association based on votes
  int bestt(-1);
  int totvotes, maxvotes = 0, maxsamevotes = 0;
  if (numcomps == 1) {
    bestt = 0;
  } else {
    for (i=0; i<numcomps; i++) {
      totvotes = tvotes[i];
      if (totvotes > maxvotes) {
	maxvotes = totvotes;
	bestt= i;
      }
    }
  }

  tcomp = -1;
  if (bestt >=0 && tvotes[bestt] > 0)
    tcomp = bestt;
  delete [] tvotes;
}

void assignSoapComponentsVoting(IComp * ccspots, int numcomps, //Dec. 9, 2010
			  int nx, int ny, IPixel & stp, int sStren, int & scomp, cc_size size)//Dec. 9, 2010
{
  // int i,j;// Babak
  int i;
  scomp = 0;

  if (numcomps == 0) 
    return;

  int *tvotes = new int[numcomps];
  for (i=0; i<numcomps; i++) 
    tvotes[i] = 0;
 

  for (i=0; i<numcomps; i++) {
    if (outRegion(ccspots[i].center,nx,ny) ||
	ccspots[i].compsize <  size.min || 
	ccspots[i].compsize > size.max) {
      tvotes[i] = 0;
    } else {
      tvotes[i] += votefn(sStren,stp,ccspots[i].center,nx,ny,TOWEL);
    }
    //stp.print(stderr);
    //ccspots[i].center.print(stderr);
    //fprintf(stderr,"%d %d\n",ccspots[i].compsize,tvotes[i]);
  }

  // now pick best association based on votes
  int bestt(-1);
  int totvotes, maxvotes = 0, maxsamevotes = 0;
  if (numcomps == 1) {
    bestt = 0;
  } else {
    for (i=0; i<numcomps; i++) {
      totvotes = tvotes[i];
      if (totvotes > maxvotes) {
	maxvotes = totvotes;
	bestt= i;
      }
    }
  }
  scomp = -1;
  if (bestt >=0 && tvotes[bestt] > 0)
    scomp = bestt;
  delete [] tvotes;
}

double distfrom(IPixel & sp1, Speck & sp2)
{
  return distfrom(sp2,sp1.x,sp1.y);
}
double distfrom(Speck & sp1, int x, int y)
{
  double dist = (sp1.xo-x)*(sp1.xo-x);
  dist += (sp1.yo-y)*(sp1.yo-y);
  return sqrt(dist);
}
int constructAllDataParticles(Speck **dp, int maxndp, 
			      Color & fco, Color & fsigco, 
			      double *ll, double *cll, 
			      Color *cim, IComp & bim, int nx, int ny)
{
  int i,k,ndp(0);
  double hll(0.0),uxo,uyo;
  
  // go over ccim once and assign all new particles
  // now the pixels on the edges of the connected component 
  // will have a speck that "spans" the edge - but 
  // this additional variance will not hurt the particle filters
  int pixstep = 2;
  for (k=0; k < bim.compsize; k+=pixstep) {
    // randomize the velocity - could get this from optical flow
    // make this a little bigger, since the flows 
    // are averaged over sets of 5 specks
    uxo = 5.0*(rand()/(RAND_MAX+1.0))-0.5;
    uyo = 5.0*(rand()/(RAND_MAX+1.0))-0.5;

    // add a new speck here
    hll = dp[ndp]->getFromImage(bim.points[k].x,bim.points[k].y,cim,fco,fsigco,uxo,uyo,nx,ny);

    ll[ndp] = exp(-hll);
    ndp++;
  }
  //fprintf(stderr,"number of components added %d\n",ndp);
  // compute cumulative ll now
  hll = 0.0;
  for (i=0; i<ndp; i++) {
    cll[i] = hll;
    hll += ll[i];
  }
  cll[i] = hll;
  for (i=0; i<ndp; i++) {
    ll[i] = ll[i]/hll;
    cll[i] = cll[i]/hll;
  }
  cll[i] = cll[i]/hll;

  return ndp;
}
// converts wgt to a strength
int strenfn(double wgt, double maxw, double minw)
{
  return max(min(10,((int) floor(10*((log(wgt)-log(minw))/(log(maxw)-log(minw)))))),0);
}
// computes the strenght of the particle filter
// based on data and prior weights of the particle filter
// if alpha = 0, only prior weight is used
// if alpha = 1, only data weight is used
// else, both are used
int getNewStren(double alpha, double stren, int comp, double data_wgt, double prior_wgt, double max_data_weight, double min_data_weight, double max_prior_weight, double min_prior_weight)
{
  int newstren;
  if (stren == 0 && comp < 0) {
    newstren = 0;
  } else if (alpha == 0.0) {
    newstren =  strenfn(prior_wgt, max_prior_weight, min_prior_weight);
  } else if (alpha == 1.0) {
    newstren =  strenfn(data_wgt, max_data_weight, min_data_weight);
  } else {
    newstren =  strenfn(prior_wgt+data_wgt, max(max_prior_weight,max_data_weight), min(min_prior_weight,min_data_weight));
  }    
  return newstren;
}
// updates the average hand positions avg* (averaged over time), distance from hands to towel avg*t
// and variances avg*v of these quantities given new data mhlp, mrhp, mtp, minlhp. minrhp
void updateAvgPositions(IPixel & mlhp, IPixel & mrhp, IPixel & mtp, IPixel & minlhp, IPixel & minrhp,
			int lhStren, int rhStren, int towelStren,
			IPixel *lh_hist, IPixel *rh_hist, IPixel *t_hist, 
			IPixel *mlh_hist, IPixel *mrh_hist, 
			double *lht_hist, double *rht_hist,
			double *lhs_hist, double *rhs_hist, double *ts_hist,
			int & history_index, double hist_discount, int hist_length,
			IPixel & avglh, IPixel & avgrh, IPixel & avgt, IPixel & avgminlh, IPixel & avgminrh,
			double  & avglht, double & avgrht, 
			IPixel & avglhv, IPixel & avgrhv, IPixel & avgtv, 
			double & avglhtv, double & avgrhtv)
  
{
  int i=history_index;
  bool done(false);
  // compute new distances from towel to each hand
  double lhtd = mlhp.distFrom(mtp);
  double rhtd = mrhp.distFrom(mtp);

  // insert new values into history lists
  lh_hist[i].setTo(mlhp);
  rh_hist[i].setTo(mrhp);
  t_hist[i].setTo(mtp);

  mlh_hist[i].setTo(minlhp);
  mrh_hist[i].setTo(minrhp);

  lht_hist[i] = lhtd;
  rht_hist[i]=  rhtd;

  lhs_hist[i] = exp(-2.0/lhStren);
  rhs_hist[i] = exp(-2.0/rhStren);
  ts_hist[i] = exp(-2.0/towelStren);
  
  double dfact = 1.0;
  double mfact, lhfact(0.0), rhfact(0.0), tfact(0.0);
  double lhtfact(0.0), rhtfact(0.0);
  avglh.zero();   avgrh.zero();  avgt.zero();
  avgminlh.zero();  avgminrh.zero();
  avglht = 0.0;  avgrht = 0.0;
  avglhv.zero();  avgrhv.zero();  avgtv.zero();
  avglhtv = 0.0;  avgrhtv = 0.0;
  
  while (!done) {
    
    // left hand
    mfact = lhs_hist[i]*dfact+0.001;
    avglh.x = avglh.x + ((int) floor(lh_hist[i].x*mfact));
    avglh.y = avglh.y + ((int) floor(lh_hist[i].y*mfact));
    avglhv.x = avglhv.x + ((int) floor(lh_hist[i].x*lh_hist[i].x*mfact));
    avglhv.y = avglhv.y + ((int) floor(lh_hist[i].y*lh_hist[i].y*mfact));
    avgminlh.x = avgminlh.x + ((int) floor(mlh_hist[i].x*mfact));
    avgminlh.y = avgminlh.y + ((int) floor(mlh_hist[i].y*mfact));
    lhfact = lhfact + mfact;

    // right hand
    mfact = rhs_hist[i]*dfact+0.001;
    avgrh.x = avgrh.x + ((int) floor(rh_hist[i].x*mfact));
    avgrh.y = avgrh.y + ((int) floor(rh_hist[i].y*mfact));
    avgrhv.x = avgrhv.x + ((int) floor(rh_hist[i].x*rh_hist[i].x*mfact));
    avgrhv.y = avgrhv.y + ((int) floor(rh_hist[i].y*rh_hist[i].y*mfact));
    avgminrh.x = avgminrh.x + ((int) floor(mrh_hist[i].x*mfact));
    avgminrh.y = avgminrh.y + ((int) floor(mrh_hist[i].y*mfact));
    rhfact = rhfact + mfact;

    // towel
    mfact = ts_hist[i]*dfact+0.001;
    avgt.x = avgt.x + ((int) floor(t_hist[i].x*mfact));
    avgt.y = avgt.y + ((int) floor(t_hist[i].y*mfact));
    avgtv.x = avgtv.x + ((int) floor(t_hist[i].x*t_hist[i].x*mfact));
    avgtv.y = avgtv.y + ((int) floor(t_hist[i].y*t_hist[i].y*mfact));
    tfact = tfact + mfact;

    // left hand to towel  dist
    mfact = ts_hist[i]*lhs_hist[i]*dfact+0.001;
    avglht = avglht +  lht_hist[i]*mfact;
    avglhtv = avglhtv +  lht_hist[i]*lht_hist[i]*mfact;
    lhtfact += mfact;

    // right hand to towel  dist
    mfact = ts_hist[i]*rhs_hist[i]*dfact+0.001;
    avgrht = avgrht +  rht_hist[i]*mfact;
    avgrhtv = avgrhtv +  rht_hist[i]*rht_hist[i]*mfact;
    rhtfact += mfact;

    i = (i-1);
    if (i < 0) 
      i = hist_length-1;
    done = (i==history_index);
    dfact = dfact * hist_discount;
  }
  
  avglh.x = ((int) floor(avglh.x/lhfact));  avglh.y = ((int) floor(avglh.y/lhfact));
  avgminlh.x = ((int) floor(avgminlh.x/lhfact));  avgminlh.y = ((int) floor(avgminlh.y/lhfact));
  avgrh.x = ((int) floor(avgrh.x/rhfact));  avgrh.y = ((int) floor(avgrh.y/rhfact));
  avgminrh.x = ((int) floor(avgminrh.x/rhfact));  avgminrh.y = ((int) floor(avgminrh.y/rhfact));
  avgt.x = ((int) floor(avgt.x/tfact));  avgt.y = ((int) floor(avgt.y/tfact));
  avglht = avglht/lhtfact;  avgrht = avgrht/rhtfact;
  
  avglhv.x = ((int) floor(avglhv.x/lhfact)) - avglh.x*avglh.x;
  avglhv.y = ((int) floor(avglhv.y/lhfact)) - avglh.y*avglh.y;

  avgrhv.x = ((int) floor(avgrhv.x/rhfact)) - avgrh.x*avgrh.x;
  avgrhv.y = ((int) floor(avgrhv.y/rhfact)) - avgrh.y*avgrh.y;

  avgtv.x = ((int) floor(avgtv.x/tfact)) - avgt.x*avgt.x;
  avgtv.y = ((int) floor(avgtv.y/tfact)) - avgt.y*avgt.y;

  avglhtv = avglhtv/lhtfact - avglht*avglht;
  avgrhtv = avgrhtv/rhtfact - avgrht*avgrht;
  
  history_index = (history_index+1)%hist_length;
}
// lighten the region corresponding to handspos
//(observations	(handpos away sink water tap soap towel))
void HandTracker::drawIRegion(unsigned char *im, unsigned char *outim, int nx, int ny, int handspos)
{
  int xl, xr, yt, yb;
  memcpy(outim,im,nx*ny*3*sizeof(unsigned char));
  IRegion theregion;
  switch (handspos) {
  case (1):
    //sink
    theregion = sinkSpace;
    break;
  case (2):
    //water
    theregion = waterSpace;
    break;
  case (3):
    //tap
    theregion = tapSpace;
    break;
  case (4):
    //soap
    theregion = soapSpace;
    xl = 35; xr = 120;
    yt = 106; yb = 180;
    break;
  case (5):
    //towel
    theregion = towelSpace;
    break;
  default:
    // away - do nothing
    return;
  };
  xl = theregion.tl.x; xr = theregion.br.x;
  yt = theregion.tl.y; yb = theregion.br.y;
  int index;
  for (int i=yt; i<yb; i++) {
    for (int j=xl; j<xr; j++) {
      index = i*nx+j;
      outim[index*3] = min(255,outim[index*3]+40);
      outim[index*3+1] = min(255,outim[index*3+1]+40);
      outim[index*3+2] = min(255,outim[index*3+2]+40);
    }
  }
}

// search from origin point xo,yo along a line to point x1,y1 and find the furthest point (x,y) along this line for which cc[y*nx+x] = 1
// without a break in between - uses the bresenham algorithm
double findClosestDistanceBinToPoint(int *cc, int nx, int ny, int x0, int y0, int x1, int y1)
{
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
  double dist, mindist(nx*ny*nx*ny);
  if (steep) {
    xstep3 = xstep*nx;
    ystep3 = ystep;
    index = (x*nx+y);
  } else {
    xstep3 = xstep;
    ystep3 = ystep*nx;
    index = (y*nx+x);
  }
  if (cc[index]) {
    dist = (x-x1)*(x-x1)+(y-y1)*(y-y1);
    if (dist < mindist) 
      mindist = dist;
  } else {
    return mindist;
  }
  while (x != x1) {
    x = x + xstep;
    index = index + xstep3;
    error = error + deltaerr;
    if (2*error > deltax) {
      y = y + ystep;
      index = index + ystep3;
      error = error - deltax;
    }
    if (cc[index]) {
      dist = (x-x1)*(x-x1)+(y-y1)*(y-y1);
      if (dist < mindist) 
	mindist = dist;
    } else {
      return mindist;
    }
  }
  return mindist;
}
