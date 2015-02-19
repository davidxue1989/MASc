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
#include "color.h"

Color::Color()
{
  h = s = v = 0.0;
}
Color::Color(unsigned char r, unsigned char g, unsigned char b)
{
  rgb2hsv(r,g,b,&h,&s,&v);
}
Color::Color(double ih, double is, double iv)
{
  setColor(ih,is,iv);
}
Color::Color(Color *c)
{
  setColor(c->h,c->s,c->v);
}

void Color::setColor(Color & c)
{
  h = c.h;
  v = c.v;
  s = c.s;
}
void Color::setColor(double ih, double is, double iv)
{
  h = ih;
  v = iv;
  s = is;
}
void Color::zero()
{
  h = v = s = 0;
}
// compute the distance from this color to co scaled by sigco
// with prior heavy tail value nlogcp
double Color::colllp(Color & co, Color & sigco, double nlogcp)
{
  double weight = colll(co,sigco);
  /*
  if (nlogcp < weight) 
    fprintf(stderr,"*");
  */
  return (min(weight,nlogcp));
  //return weight;
}
// compute the distance from this color to co scaled by sigco
// no prior heavy tail
double Color::colll(Color & co, Color & sigco)
{
  double weight = fabs(h-co.h);   
	if (weight > 180)	// check for angle "wraparound" because H channel is in units of degrees
		weight = (360 - weight);

	weight = weight*weight/(sigco.h);		
  weight += (s-co.s)*(s-co.s)/(sigco.s);
  weight += (v-co.v)*(v-co.v)/(sigco.v);
  return (0.5*weight);
}

void Color::print(FILE *fp)
{
  fprintf(fp,"%g %g %g ",h,s,v);
}
void Color::scan(FILE *fp)
{
  fscanf_s(fp,"%lg %lg %lg ",&h,&s,&v); // Babak: _s
}
void rgb2hsv(double r, double g, double b, double  *h, double *s, double *v) {
  double max = max3(r,g,b);
  double min = min3(r,g,b);
  *v = max/255.0;
  *s = (max != 0.0) ? ((max-min)/max):0.0;
  if (*s == 0.0) {
    *h = UNDEFINED_HUE;   // actually is UNDEFINED
  } else {
    double delta = max - min;
    if (r == max) {
      *h = (g-b)/delta;
	}
    else if (g == max) {
      *h = 2.0 + (b-r)/delta;
	}
    else if (b == max) {
      *h = 4.0 + (r-g)/delta;
	}

    *h *= 60.0;

    if (*h > 180) {
      *h = *h-360;
	}
  }
}

Color ***buildHSVConverter()
{
	Color ***lt;
	double h,s,v,r,g,b;
	int ir,ig,ib;
	lt = new Color**[256];
	for (ir=0; ir<256; ir++)
	{
		lt[ir] = new Color*[64];
		for (ig=0; ig<64; ig++)
		{
			lt[ir][ig] = new Color[64];
			for (ib=0; ib<64; ib++)
			{
				r = (double) ir;
				g = (double) ig*4;
				b = (double) ib*4;
				rgb2hsv(r,g,b,&h,&s,&v);
				lt[ir][ig][ib].h = h;
				lt[ir][ig][ib].s = s;
				lt[ir][ig][ib].v = v;
			}
		}
	}
	return lt;
}

