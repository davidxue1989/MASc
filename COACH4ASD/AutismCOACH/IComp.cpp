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
#include "icomp.h"
bool IPixel::in(IRegion & ir)
{
 return in(ir.tl,ir.br); 
}

void IPixel::drawOnImage(unsigned char *outim, int nx, int ny, 
			 unsigned char rval, unsigned char bval, unsigned char gval, 
			 int pfr, int pto)
{
  int i,j,index;
  for (i=y+pfr; i<y+pto; i++) {
    for (j=x+pfr; j<x+pto; j++) {
      if (i >=0 &&  i<ny && j>=0 && j<nx) {
	index = (i*nx+j)*3;
	outim[index] = rval;
	outim[index+1] = gval;
	outim[index+2] = bval;
      }
    }  
  }
}

void IPixel::print(FILE *outf)
{
  fprintf(outf,"%d %d ",x,y);
}
void IPixel::scan(FILE *inf)
{
  fscanf_s(inf,"%d %d ",&x,&y); // Babak: _s
}
IComp::IComp(int cps)
{
  compsize = cps;
  // but this is fixed for my purposes ... 
  // see e.g. setTo below where points array is not re-allocated
  points = new IPixel[compsize];
}
IComp::~IComp()
{
  delete [] points;
}
void IComp::setTo(IComp & tc) 
{
  compsize = tc.compsize;
  for (int k=0; k<compsize; k++) {
    points[k].x = tc.points[k].x;
    points[k].y = tc.points[k].y;
  }
}
void IComp::getCentroid()
{
  center.x = center.y = 0;
  for (int k=0; k<compsize; k++) {
    center.x += points[k].x;
    center.y += points[k].y;
  }
  center.x = (int) floor(((double) center.x)/((double) compsize));
  center.y = (int) floor(((double) center.y)/((double) compsize));
}

//Justin - Oct 22, 2010
void IComp::getCentroid(int &centroidX, int &centroidY)
{
  center.x = center.y = 0;
  for (int k=0; k<compsize; k++) {
    center.x += points[k].x;
    center.y += points[k].y;
  }
  center.x = (int) floor(((double) center.x)/((double) compsize));
  center.y = (int) floor(((double) center.y)/((double) compsize));

  centroidX = center.x;
  centroidY = center.y;
}

void IComp::getCentroidAndBB()
{
  int i,j;
  center.x = center.y = 0;
  bbox.tl.x = bbox.tl.y = 10000;
  bbox.br.x = bbox.br.y = 0;
  for (int k=0; k<compsize; k++) {
    i = points[k].x;
    j = points[k].y;
    center.x += points[k].x;
    center.y += points[k].y;
    bbox.tl.x = ((i < bbox.tl.x) ? i : bbox.tl.x);
    bbox.tl.y = ((j < bbox.tl.y) ? j : bbox.tl.y);
    bbox.br.x = ((i > bbox.br.x) ? i : bbox.br.x);
    bbox.br.y = ((j > bbox.br.y) ? j : bbox.br.y);
  }
  center.x = (int) floor(((double) center.x)/((double) compsize));
  center.y = (int) floor(((double) center.y)/((double) compsize));
}  
