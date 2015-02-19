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
#ifndef __HANDTRACKER
#define __HANDTRACKER
#include <stdlib.h>
#include <stdio.h>
//#include <sys/time.h> // Babak
#include <time.h> // Babak

// ( Babak:
//		http://www.velocityreviews.com/forums/t285536-ltunistdhgt-for-windows-platform.html
//#include <unistd.h>
#include <io.h>
//#include <dosio.h>
// )

#include <sys/types.h>
#include <sys/stat.h>

// ( Babak:
//		http://www.experts-exchange.com/Programming/Languages/CPP/Q_20699524.html
// #include <sys/mman.h> 
//#include <winbase.h>
// )

#include <fcntl.h>
#include <string.h>
#include <math.h>
#include <iostream>
//#include <FL/Fl_Color_Chooser.H> // Babak

#include "particlefilter.h"

#include "flock.h"
//#include "timer.h" // Babak
#include "icomp.h"

#include "PFHTDefines.h"
#include "../VidPlayerDefines.h" // for define (or not define) IMAGE_SIZE_ORIGINAL

enum handtype {LEFT,RIGHT,TOWEL};

#define        ROUND(A)        ((A)<0.0?-((int)(-(A)+0.5)):((int)((A)+0.5)))
/* Babak
#define        max(A,B)        ((A)<(B)?(B):(A))  
#define        min(A,B)        ((A)<(B)?(A):(B))
*/
#define        sign(A,B)        ((A)<0?-(B):(B))
#define        sgn(A)          ((A)<0? (-1) : ( (A) == 0 ? (0) : (1)))
//dx nov 22, 2012: commented out and moved into Flock.h
//#define        Abs(A)          ((A)<0?-(A):(A))

 // Babak
//#define DEBUG 0
#define VERBOSE 0 

#define MEDIANFILTSIZE 9

#ifdef IMAGE_SIZE_ORIGINAL // 640*480 
	// has to be large enough that we dont' get memory overflow. but how large?
	#define MAX_NUMDATAPARTICLES 45000
#else// 320x240
	#define MAX_NUMDATAPARTICLES 30000
#endif

#define MINHANDLEN 10
#define MAXHANDLEN 100
#define MINHANDSIZE 100  // (20x20)
// with short sleeves - this will be larger
#define MAXHANDSIZE 4000 // (40x100)
#define MEDHANDSIZE 2050  // MIN+(MAX-MIN)/2

#define MINTOWELSIZE 1600 //(40x40)
#define MAXTOWELSIZE 20000

// smallest distance between hands before 
// two filters will be on the same hand
#define MIN_HAND_SEPARATION 20  // was 50

// number of specks in the flock
#define FLOCKSIZE 5//3

// weight of flock color model wrt the speck local model
#define FLOCK_COLOR_WEIGHT 0.9

#define STRAY_DIST_SOAP 20//Modification for soap
#define STRAY_DIST_TOWEL 100
#define STRAY_DIST_HAND 40 //40
#define COLL_DIST_SOAP 20//Modification for soap
#define COLL_DIST_TOWEL 70
#define COLL_DIST_HAND 20 //20
#define INT_DIST 100

#define VTHRESH 20
#define VTHRESH2 600
#define TVTHRESH 40
#define TVTHRESH2 1600

// ********************************
// COLOR DEFAULTS
// these average values could be set for hands across a population
// or for a single person's hand
// ********************************
// hue of average hand
#define AVG_HAND_HUE 6.3
// variance over hands in average 
#define AVG_HAND_HUE_STD 6.0

// variance over hands in average 
#define AVG_HAND_SAT 0.2
#define AVG_HAND_SAT_STD 0.06

// variance over hands in average 
#define AVG_HAND_VAL 0.74 
#define AVG_HAND_VAL_STD 0.08

// hue of average towel
#define AVG_TOWEL_HUE -6.0
// variance over towel in average 
#define AVG_TOWEL_HUE_STD 4.0

#define AVG_TOWEL_SAT 0.5
#define AVG_TOWEL_SAT_STD 0.04

#define AVG_TOWEL_VAL 0.6
#define AVG_TOWEL_VAL_STD 0.04

// squared sigmas = actual sigmas

#define AVG_HAND_HUE_SIG AVG_HAND_HUE_STD*AVG_HAND_HUE_STD
#define AVG_HAND_SAT_SIG AVG_HAND_SAT_STD*AVG_HAND_SAT_STD
#define AVG_HAND_VAL_SIG AVG_HAND_VAL_STD*AVG_HAND_VAL_STD
#define AVG_TOWEL_HUE_SIG AVG_TOWEL_HUE_STD*AVG_TOWEL_HUE_STD
#define AVG_TOWEL_SAT_SIG AVG_TOWEL_SAT_STD*AVG_TOWEL_SAT_STD
#define AVG_TOWEL_VAL_SIG AVG_TOWEL_VAL_STD*AVG_TOWEL_VAL_STD

// for segmentation only 
// could (should?) be set based on above quantities
// eg mean +- sigma
#define LOWHUE         -10.0
#define HIGHHUE        20.0 
#define LOWSAT           0.1
#define HIGHSAT          0.4
#define LOWVAL           0.6
#define HIGHVAL          1.0

#define LOWTHUE         -40.0
#define HIGHTHUE        20.0 
#define LOWTSAT           0.4
#define HIGHTSAT          0.7
#define LOWTVAL           0.1
#define HIGHTVAL          0.7

//**********************************
// STRENGTH DETERMINATORS
//**********************************
#define MAX_HAND_PRIOR_WEIGHT 1e-1
#define MIN_HAND_PRIOR_WEIGHT 1e-4
#define MAX_HAND_DATA_WEIGHT 1e-3
#define MIN_HAND_DATA_WEIGHT 1e-8

#define MAX_TOWEL_PRIOR_WEIGHT 1e-1
#define MIN_TOWEL_PRIOR_WEIGHT 1e-11
#define MAX_TOWEL_DATA_WEIGHT 1e-3
#define MIN_TOWEL_DATA_WEIGHT 1e-14


// some dimension of the connected components
struct cc_size
{
	int min;
	int med;
	int max;
};

/*
enum
{
	HAND_POS_AWAY = 0,
	HAND_POS_SINK,
	HAND_POS_WATER,
	HAND_POS_TAP,
	HAND_POS_SOAP,
	HAND_POS_TOWEL,
};
*/

// helper functions
int connectedComponents(int *skin, int nx, int ny, IComp *ccspots, int *cc);
//Justin - Oct 22, 2010
void getCentroids(int numcc, IComp *ccspots, int &centroidX, int &centroidY);
void getCentroids(int numcc, IComp *ccspots);
void getCentroidsAndBB(int numcc, IComp *ccspots);

double strentoalpha(int stren, int comp);
bool splitComponent(int & numcc, IComp & ccspots, IComp & newccspot, IPixel & mlhp, IPixel & mrhp);
int votefn(int stren, Speck & mp, int xo, int yo, int nx, int ny, handtype hs);
void assignHandComponentsVoting(IComp * ccspots, int numcomps, 
				int nx, int ny, IPixel & mlhp, IPixel & mrhp, double lrdist, 
				int lhStren, int rhStren, int & lhcomp, int & rhcomp, int & samecomp, cc_size size);
void assignTowelComponentsVoting(IComp * ccspots, int numcomps, //This line duplicated for soap
				 int nx, int ny, IPixel & mtp, int tStren, int & tcomp, cc_size size);//This line duplicated for soap
void assignSoapComponentsVoting(IComp * ccspots, int numcomps, //Dec. 9, 2010
				 int nx, int ny, IPixel & stp, int sStren, int & scomp, cc_size size);//Dec. 9, 2010

double distfrom(IPixel & sp1, Speck & sp2);
double distfrom(Speck & sp1, int x, int y);
     
// finds closest pixel in comp to point
double findClosestDistanceToComponent(IPixel & point, IComp & comp, int nx, int ny);
//double findClosestDistanceToComponent_multipleLocations(IComp &locations, IComp &comp, int nx, int ny, int closestLocations[]); //dx nov 28, 2012

// this takes a binary image bim and looks at all pixels (i,j) for which the speck (dp) with (i,j) at its top left corner 
// is contained within im==1 and computes the likelihood (ll) of the speck given the prior hues (from hue_im) 
// for the specktype  xi, also computes the cumulative likelihood cll
// will only find a maximum of maxndp specks
int constructAllDataParticles(Speck **dp, int maxndp, 
			      Color & fco, Color & fsigco, 
			      double *ll, double *cll, 
			      Color *cim, IComp & bim, int nx, int ny); 


int strenfn(double wgt, double maxw, double minw);
int getNewStren(double alpha, double stren, int comp, 
		double data_wgt, double prior_wgt, 
		double max_data_weight, double min_data_weight, 
		double max_prior_weight, double min_prior_weight);

// updates the average hand positions avg* (averaged over time), distance from hands to towel avg*t
// and variances avg*v of these quantities given new data mhlp, mrhp, mtp, minlhp. minrhp
void updateAvgPositions(IPixel & mlhp, IPixel & mrhp, IPixel & mtp, IPixel & minlhp, IPixel & minrhp,//Not called by program
			int lhStren, int rhStren, int towelStren,
			IPixel *lh_hist, IPixel *rh_hist, IPixel *t_hist, 
			IPixel *mlh_hist, IPixel *mrh_hist, 
			double *lht_hist, double *rht_hist,
			double *lhs_hist, double *rhs_hist, double *ts_hist,
			int & history_index, double hist_discount, int hist_length,
			IPixel & avglh, IPixel & avgrh, IPixel & avgt, IPixel & avgminlh, IPixel & avgminrh,
			double  & avglht, double & avgrht, 
			IPixel & avglhv, IPixel & avgrhv, IPixel & avgtv, 
			double & avglhtv, double & avgrhtv);
  
// gets the hue image from im into hue_im, and segments it and puts the result in skin_im
void getHSV(Color ***clook, unsigned char *im, Color *cim, int nx, int ny);
bool outIRegion(int xo, int yo, int nx, int ny);
double findClosestDistanceBinToPoint(int *cc, int nx, int ny, int x0, int y0, int x1, int y1);



class HandTracker
{
 public:
  // constructor
  HandTracker(int inx, int iny, int inp);

  //Justin Oct22, 2010
  int centroidX, centroidY;

  // destructor (added by Babak)
  ~HandTracker();

  void setColorThresholds(Color & tlo, Color & thi);
  
  void updateData(int verbose=0);
  void estimateMeans();
  void getNewStrengths();

  void swapLRPF();
  void getAssignComponents(unsigned char *);
  double findDistanceToPoint(IPixel & point, handtype ht);
  double findDistanceToTowel(handtype ht);

  void constructDataParticles(unsigned char *);
  
  void zeroAlpha();
  void zeroHComp();
  void zeroDP();

  int update(unsigned char *im, int frame,  bool updateAvgPos, unsigned char *finalim = NULL, unsigned char *outskinim = NULL, unsigned char *bestim = NULL);

  // resets the three filters by setting strengths to 0 
  void reset();
  
  // checks if h,s,v are skin colored
  bool skinSegment(double h, double s, double v);
  
  // checks if h,s,v are towel colored
  bool towelSegment(double h, double s, double v);//This line duplicated for soap

  // checks if h,s,v are soap colored
  bool soapSegment(double h, double s, double v);//Modification for soap

  bool skinSegment(Color c);
  
  bool towelSegment(Color c);//This line duplicated for soap

  bool soapSegment(Color c);//Modification for soap
  
  void segmentImage(unsigned char *outskinim=NULL);

  void resetWeightLimits();
  void printWeightLimits();
  int toggleLearnWeightLimits();

  bool handUsingTowel(double d, double td);
  bool handUsingSoap(IPixel & mhp, double td); 
  bool handUsingTap(IPixel & mhp, double td); 
  bool handAtWater(IPixel & mhp, double td); 
  bool handAtSink(handtype ht); 
  bool handAtSink(IPixel & mhp, double td=0); 
  int getHandPos(IPixel & avgh, IPixel & avgminh, double avght, double tapd, double soapd, double waterd, double toweld, double sinkd) ;
  int getLHPos(int & lhpos, int & rhpos, IPixel & avglh, IPixel & avgrh, IPixel & avgminlh, IPixel & avgminrh, double avglht, double avgrht);

  int getAvgPos(int frame);

  // figure out the observations of lh and rh positions for POMDP
  void extractObservations(unsigned char *skinim, int & lhpos, int & rhpos);
  void extractObservationsNew(unsigned char *skinim, int & lhpos, int & rhpos);
  //void extractObservationsNewer(unsigned char *skinim, int & lhpos, int & rhpos);
  void extractObservationsNewer(unsigned char *skinim, int& ntowel_obs, int& nsoap_obs, int& nsoapspout_obs, int& ntapl_obs, int& ntapr_obs, int& nwater_obs, int& nsink_obs, int& nliquidsoap_obs);//Modification for soap

  // extract the hand distance to each region (Babak)
  void ExtractRegionDistances(unsigned char *skinim, double& towelDist, double& soapDist, double& leftTapDist, double& rightTapDist, double& waterDist, double& sinkDist);


  // lighten the region corresponding to handspos
  //(observations	(handpos away sink water tap soap towel))
  void drawIRegion(unsigned char *im, unsigned char *outim, int nx, int ny, int handspos);
  void drawOnImage(unsigned char *im, unsigned char *finalim);

  void drawDemoImage(unsigned char *im,unsigned char *demoim,int lhpos, int rhpos);
  void drawCompOnImage(unsigned char*im, IComp * comp, int col=0);
  void brightenRegion(unsigned char * im, IPixel & pos, double radius);

  void print(FILE *fh);
  void scan(FILE *fh);

  // member variables

  // strengths
  int lhStren, rhStren, towelStren, soapStren;//Dec. 9, 2010

  // alphas
  double alpha_lh, alpha_rh, alpha_towel, alpha_soap;//Dec. 9, 2010
  
  // particle filter trackers
  //soap
  ParticleFilter *spf; //Modification for soap
  // towel
  ParticleFilter *tpf; //This line duplicated for soap
  // left hand
  ParticleFilter *lhpf;
  // right hand
  ParticleFilter *rhpf;


  int learnweightlimits;

  // color models
  // towel
  Color tco,tsigco;//This line duplicated for soap
  //soap
  Color sco,ssigco;//Modification for soap
  // left hand
  Color lhco, lhsigco;
  // right hand
  Color rhco, rhsigco;

  int num_particles;

  bool componentsComputed;
  // prior and data weights
  double hdw, hpw, tdw, tpw, sdw, spw;//Dec. 9, 2010
  double lh_hdw, lh_hpw;
  double rh_hdw, rh_hpw;
  

  int nx, ny; // Babak: image width and height (what a strange notation!)
  Color *cim;
  int *skin_im, *towel_im, *soap_im;//Modification for soap
  int **accum;

  int numsp,numtp, numsoapp, numcc, tnumcc, snumcc;//Modification for soap
  unsigned char *tmpim;

  // color thresholds
  Color ctlo, cthi;
  Color tctlo, tcthi;//This line duplicated for soap
  Color sctlo, scthi;//Modification for soap


  // locations of objects in image
  // will have to be set in the user interface somewhere
  IRegion soapSpace, sinkSpace, tapSpace, waterSpace, towelSpace;
  IPixel soapPos, soap_spoutPos, LTapPos, RTapPos, waterPos, sinkPos;//Modification for soap

  // distances of hands to each object
  double lhdist_soap, lhdist_ltap, lhdist_rtap, lhdist_water, lhdist_towel, lhdist_sink;
  double rhdist_soap, rhdist_ltap, rhdist_rtap, rhdist_water, rhdist_towel, rhdist_sink;

  // area of region  
  double min_towel_dist;
  double min_soap_dist;
  double min_soap_spout_dist;//Modification for soap
  double min_ltap_dist;
  double min_rtap_dist;
  double min_water_dist;
  double min_sink_dist;

  // connected components stuff
  IComp *ccspots, *tccspots, *sccspots;//Dec. 9, 2010
  int *cc;
  int *cxo, *cyo;
  int *brx, *bry, *tlx, *tly;
  int *tcxo, *tcyo;
  //int *tbrx, *tbry, *ttlx, *ttly;

  // data particles
  Speck **soapdp;//Modification for soap
  Speck **toweldp;//This line duplicated for soap
  Speck **skindp;
  Speck **lh_skindp;
  Speck **rh_skindp;

  // likelihood maps and cumulative ll maps
  double *soapll;//Dec. 9, 2010
  double *towelll;//This line duplicated for soap
  double *skinll; 
  double *lh_skinll;
  double *rh_skinll;
  
  double *soapcll;//Dec. 9, 2010
  double *towelcll;//This line duplicated for soap
  double *skincll; 
  double *lh_skincll;
  double *rh_skincll;

  int num_dp, num_skin_dp, num_towel_dp, num_soap_dp;//Dec. 9, 2010
  int num_lh_skin_dp, num_rh_skin_dp;

  int lhcomp, rhcomp, samecomp, tcomp, scomp;//Dec. 9, 2010
  int maxtcompsize, maxscompsize;//Dec. 9, 2010

  IPixel msp, mtp, mrhp, mlhp;//Modification for soap
  IPixel minrhp, minlhp;
  double lrdist;

  // connected components sizes
  cc_size soap_size, towel_size, hand_size;//Modification for soap

  // dynamics noise params
  double loposdyn, hiposdyn;
  Color locdyn, hicdyn;
  double hidepdyn, lodepdyn;
  
  // bunch of stuff for temporal averaging 
  IPixel *lh_hist, *rh_hist, *t_hist;
  IPixel  *mlh_hist, *mrh_hist;
  double *lht_hist, *rht_hist;
  double *lhs_hist, *rhs_hist, *ts_hist;
  int history_index, hist_length;
  double hist_discount;
  IPixel avglh, avgrh, avgt, avgminlh, avgminrh;
  double avglht, avgrht;
  IPixel avglhv, avgrhv, avgtv;
  double avglhtv, avgrhtv;
  int handspos, elapsedFrames;

  Color ***RGBHSVLookup;
};
#endif

