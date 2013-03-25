
#ifndef __IMAGERESIZE_IMAGERESIZE_H__
#define __IMAGERESIZE_IMAGERESIZE_H__

// taken from http://www.codeproject.com/KB/GDI-plus/imgresizoutperfgdiplus.aspx?df=100&forumid=202864&exp=0&select=1436637
/*
   Box 	            equivalent to Nearest Neighbor on upsampling, averages pixels on downsampling
   Triangle 	      equivalent to Low; the function can be called Tent function for its shape
   Hermite 	         use of the cubic spline from Hermite interpolation
   Bell 	            attempt to compromise between reducing block artifacts and blurring image
   CubicBSpline      most blurry filter (cubic Bezier spline) - known samples are just "magnets" for this curve
   Lanczos3 	      windowed Sinc function (sin(x)/x) - promising quality, but ringing artifacts can appear
   Mitchell 	      another compromise, but excellent for upsampling
   Cosine 	         an attemp to replace curve of high order polynomial by cosine function (which is even)
   CatmullRom 	      Catmull-Rom curve, used in first image warping algorithm (did you see Terminator II ?)
   Quadratic 	      performance optimized filter - results are like with B-Splines, but using quadratic function only
   QuadraticBSpline 	quadratic Bezier spline modification
   CubicConvolution 	filter used in one example, its weight distribution enhances image edges
   Lanczos8 	      also Sinc function, but with larger window, this function includes largest neighborhood
*/

#include "eastl/types.h"

#include <math.h>

#define M_PI       3.14159265358979323846

struct HermiteFilter {
   static float getDefaultFilterRadius() { return 1.0f; }
   static float getValue(float val) {
      if(val < 0) 
         val = -val;
      if(val < 1)
         return ((2*val-3)*val*val + 1);
      return 0.0f;
   }
};
struct BoxFilter {
   static float getDefaultFilterRadius() { return 0.5f; }
   static float getValue(float val) {
      if(val < 0) 
         val = -val;
      if(val <= 0.5f)
         return 1.0f;
      return 0.0f;
   }
};
struct TriangleFilter {
   static float getDefaultFilterRadius() { return 1.0f; }
   static float getValue(float val) {
      if(val < 0) 
         val = -val;
      if(val < 1)
         return (1-val);
      return 0.0f;
   }
};
struct BellFilter {
   static float getDefaultFilterRadius() { return 1.5f; }
   static float getValue(float val) {
      if(val < 0)
         val = -val;
      if(val < 0.5f)
         return (0.75f-val*val);
      if(val < 1.5f)
         return (0.5f*pow(val-1.5f, 2));
      return 0.0f;
   }
};
struct CubicBSplineFilter {
   static float getDefaultFilterRadius() { return 2.0f; }
   static float getValue(float val) {
      if(val < 0) 
         val = -val;
      if(val < 1.0f) {
         float temp = val*val;
         return (0.5f*temp*val - temp + 2.0f/3.0f);
      }
      if(val < 2.0f) {
         val = 2.0f - val;
         return pow(val, 3)/6.0f;
      }
      return 0.0f;
   }
};
struct Lanczos3Filter {
   static float getDefaultFilterRadius() { return 3.0f; }
   static float sinC(float val) {
      if(val != 0) {
         val *= (float)M_PI;
         return sin(val)/val;
      }
      return 1.0f;
   }
   static float getValue(float val) {
      if(val < 0) 
         val = -val;
      if(val < 3)
         return sinC(val)*sinC(val/3.0f);
      return 0.0f;
   }
};
struct MitchellFilter {
#define cConst (1.0f/3.0f)
   static float getDefaultFilterRadius() { return 2.0f; }
   static float getValue(float val) {
      if(val < 0) 
         val = -val;
      float temp = val*val;
      if(val < 1) {
         val = (((12-9*cConst-6*cConst)*(val*temp))+((-18+12*cConst+6*cConst)*temp)+(6-2*cConst));
         return val/6.0f;
      }
      if(val < 2) {
         val = (((-cConst-6*cConst)*(val*temp))+((6*cConst+30*cConst)*temp)+((-12*cConst-48*cConst)*val)+(8*cConst+24*cConst));
         return val/6.0f;
      }
      return 0.0f;
   }
#undef cConst
};
struct CosineFilter {
   static float getDefaultFilterRadius() { return 1.0f; }
   static float getValue(float val) {
      if((val>=-1) && (val<=1))
         return (float)((cos(val*M_PI)+1)/2.0f);
      return 0.0f;
   }
};
struct CatmullRomFilter {
   static float getDefaultFilterRadius() { return 2.0f; }
   static float getValue(float val) {
      if(val < 0) 
         val = -val;
      float temp = val*val;
      if(val <= 1)
         return (1.5f*temp*val - 2.5f*temp + 1.0f);
      if(val <= 2)
         return (-0.5f*temp*val + 2.5f*temp - 4*val + 2);
      return 0.0f;
   }
};
struct QuadraticFilter {
   static float getDefaultFilterRadius() { return 1.5f; }
   static float getValue(float val) {
      if(val < 0) 
         val = -val;
      if(val <= 0.5f)
         return (-2*val*val + 1);
      if(val <= 1.5f)
         return (val*val - 2.5f*val + 1.5f);
      return 0.0f;
   }
};
struct QuadraticBSplineFilter {
   static float getDefaultFilterRadius() { return 1.5f; }
   static float getValue(float val) {
      if(val < 0) 
         val = -val;
      if(val <= 0.5f)
         return (-val*val+0.75f);
      if(val <= 1.5f)
         return (0.5f*val*val - 1.5f*val + 1.125f);
      return 0.0f;
   }
};
struct CubicConvolutionFilter {
   static float getDefaultFilterRadius() { return 3.0f; }
   static float getValue(float val) {
      if(val < 0) 
         val = -val;
      float temp = val*val;
      if(val <= 1.0f)
         return ((4.0f/3.0f)*temp*val - (7.0f/3.0f)*temp + 1.0f);
      if(val <= 2.0f)
         return (-(7.0f/12.0f)*temp*val + 3*temp - (59.0f/12.0f)*val + 2.5f);
      if(val <= 3.0f)
         return ((1.0f/12.0f)*temp*val - (2.0f/3.0f)*temp + 1.75f*val - 1.5f);
      return 0.0f;
   }
};
struct Lanczos8Filter {
   static float getDefaultFilterRadius() { return 8.0f; }
   static float sinC(float val) {
      if(val != 0) {
         val *= (float)M_PI;
         return sin(val)/val;
      }
      return 1.0f;
   }
   static float getValue(float val) {
      if(val < 0) 
         val = -val;
      if(val < 8)
         return sinC(val)*sinC(val/8.0f);
      return 0.0f;
   }
};


class ImageResize {
   typedef struct {
      int pixelOffset;
      float weight;
   } Contributor;
   typedef struct {
      int number;
      Contributor *p;
      float wsum;
   } ContributorEntry;

public:
   template<class filter> static uint32_t *resample(uint32_t inputSizeX, uint32_t inputSizeY, uint32_t *input, 
                     uint32_t outputSizeX, uint32_t outputSizeY);
};

template<class filter> uint32_t *ImageResize::resample(uint32_t inputSizeX, uint32_t inputSizeY, uint32_t *input, 
                                                       uint32_t outputSizeX, uint32_t outputSizeY) {
    uint32_t *work = new uint32_t[outputSizeX * inputSizeY];
    uint32_t *output = new uint32_t[outputSizeX * outputSizeY];

    float scaleX = (float)outputSizeX / (float)inputSizeX;
    float scaleY = (float)outputSizeY / (float)inputSizeY;

    size_t contributorSize = eastl::max(outputSizeX, outputSizeY);
    ContributorEntry *contributors = new ContributorEntry[contributorSize];

    if(scaleX < 1.0f) {
       //scales from bigger to smaller width
       float wdth = filter::getDefaultFilterRadius() / scaleX;

       for(unsigned int i=0; i<outputSizeX; ++i) {
          contributors[i].number = 0;
          contributors[i].p = new Contributor[(int)floor(2*wdth+1)];
          contributors[i].wsum = 0;
          float center = (i+0.5f)/scaleX;
          int left = (int)(center-wdth);
          int right = (int)(center+wdth);

          for(int j=left; j<=right; ++j) {
             float weight = filter::getValue((center-j-0.5f)*scaleX);
             if((weight == 0) || (j < 0) || (j >= (signed int)inputSizeX))
                continue;
             contributors[i].p[contributors[i].number].pixelOffset = j;
             contributors[i].p[contributors[i].number].weight = weight;
             contributors[i].wsum += weight;
             contributors[i].number++;
          }
       }
    } else {
       //scales from smaller to bigger width
       for(unsigned int i=0; i<outputSizeX; ++i) {
          contributors[i].number = 0;
          contributors[i].p = new Contributor[(int)floor(2*filter::getDefaultFilterRadius()+1)];
          contributors[i].wsum = 0;
          float center = (i+0.5f)/scaleX;
          int left = (int)floor(center-filter::getDefaultFilterRadius());
          int right = (int)ceil(center+filter::getDefaultFilterRadius());

          for(int j=left; j<=right; ++j) {
             float weight = filter::getValue(center-j-0.5f);
             if((weight == 0) || (j < 0) || (j >= (signed int)inputSizeX))
                continue;
             contributors[i].p[contributors[i].number].pixelOffset = j;
             contributors[i].p[contributors[i].number].weight = weight;
             contributors[i].wsum += weight;
             contributors[i].number++;
          }
       }
    }

    //filter horizontally from input to work
    for(unsigned int k=0; k<inputSizeY; ++k) {
       for(unsigned int i=0; i<outputSizeX; ++i) {
          float intensityR = 0;
          float intensityG = 0;
          float intensityB = 0;
          for(int j=0; j<contributors[i].number; ++j) {
             float weight = contributors[i].p[j].weight;
             //            intensity += input[contributors[i].p[j].pixelOffset + inputSizeX*4*k]*weight;
             uint32_t sourcePixel = input[contributors[i].p[j].pixelOffset + inputSizeX*k];
             intensityR += ((sourcePixel&0x00ff0000) >> 16) * weight;
             intensityG += ((sourcePixel&0x0000ff00) >> 8) * weight;
             intensityB +=  (sourcePixel&0x000000ff) * weight;
          }
          intensityR /= contributors[i].wsum;
          intensityG /= contributors[i].wsum;
          intensityB /= contributors[i].wsum;
          if(intensityR < 0) intensityR = 0;
          if(intensityR > 255) intensityR = 255;
          if(intensityG < 0) intensityG = 0;
          if(intensityG > 255) intensityG = 255;
          if(intensityB < 0) intensityB = 0;
          if(intensityB > 255) intensityB = 255;
          //         work[i,k] = min(max(intensity/contributors[i].wsum, minValue), MaxValue);
          work[i+k*outputSizeX] = (((int)intensityR)<<16) | (((int)intensityG)<<8) | ((int)intensityB);
       }
    }

    for(unsigned int i=0; i<outputSizeX; ++i) {
       delete[] contributors[i].p;
    }

    //pre-calculate filter contributions for a column
    if(scaleY < 1.0f) {
       //scales from bigger to smaller height
       float wdth = filter::getDefaultFilterRadius() / scaleY;
       for(unsigned int i=0; i<outputSizeY; ++i) {
          contributors[i].number = 0;
          contributors[i].p = new Contributor[(int)floor(2*wdth+1)];
          contributors[i].wsum = 0;
          float center = (i+0.5f)/scaleY;
          int left = (int)(center-wdth);
          int right = (int)(center+wdth);
          for(int j=left; j<=right; ++j) {
             float weight = filter::getValue((center-j-0.5f)*scaleY);
             if((weight==0) || (j<0) || (j>=(signed int)inputSizeY))
                continue;
             contributors[i].p[contributors[i].number].pixelOffset = j;
             contributors[i].p[contributors[i].number].weight = weight;
             contributors[i].wsum += weight;
             contributors[i].number++;
          }
       }
    } else {
       //vertical upsampling
       for(unsigned int i=0; i<outputSizeY; ++i) {
          contributors[i].number = 0;
          contributors[i].p = new Contributor[(int)floor(2*filter::getDefaultFilterRadius()+1)];
          contributors[i].wsum = 0;
          float center = (i+0.5f)/scaleY;
          int left = (int)(center-filter::getDefaultFilterRadius());
          int right = (int)(center+filter::getDefaultFilterRadius());
          for(int j=left; j<=right; ++j) {
             float weight = filter::getValue(center-j-0.5f);
             if((weight==0) || (j<0) || (j>=(signed int)inputSizeY))
                continue;
             contributors[i].p[contributors[i].number].pixelOffset = j;
             contributors[i].p[contributors[i].number].weight = weight;
             contributors[i].wsum += weight;
             contributors[i].number++;
          }
       }
    }

    //filter vertically from work to output
    for(unsigned int k=0; k<outputSizeX; ++k) {
       for(unsigned int i=0; i<outputSizeY; ++i) {
          float intensityR = 0;
          float intensityG = 0;
          float intensityB = 0;
          for(int j=0; j<contributors[i].number; ++j) {
             float weight = contributors[i].p[j].weight;
             //            intensity += work[k, contributors[i].p[j].pixelOffset]*weight;

             uint32_t sourcePixel = work[contributors[i].p[j].pixelOffset*outputSizeX + k];
             intensityR += ((sourcePixel&0x00ff0000) >> 16) * weight;
             intensityG += ((sourcePixel&0x0000ff00) >> 8) * weight;
             intensityB +=  (sourcePixel&0x000000ff) * weight;
          }
          //         output[k,i] = min(max(intensity/contributors[i].wsum, minValue), MaxValue);
          intensityR /= contributors[i].wsum;
          intensityG /= contributors[i].wsum;
          intensityB /= contributors[i].wsum;
          if(intensityR < 0) intensityR = 0;
          if(intensityR > 255) intensityR = 255;
          if(intensityG < 0) intensityG = 0;
          if(intensityG > 255) intensityG = 255;
          if(intensityB < 0) intensityB = 0;
          if(intensityB > 255) intensityB = 255;
          output[k+i*outputSizeX] = (((int)intensityR)<<16) | (((int)intensityG)<<8) | ((int)intensityB);
       }
    }

    for(unsigned int i=0; i<outputSizeY; ++i) {
       delete[] contributors[i].p;
    }
    delete[] contributors;

    delete[] work;
    return output;
}



#endif
