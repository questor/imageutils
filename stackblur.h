
#ifndef __IMAGEUTILS_STACKBLUR_H__
#define __IMAGEUTILS_STACKBLUR_H__

#include "eastl/types.h"

class Stackblur {
   // Stack Blur Algorithm by Mario Klingemann <mario@quasimondo.com>

   // This is a compromise between Gaussian Blur and Box blur
   // It creates much better looking blurs than Box Blur, but is
   // 7x faster than my Gaussian Blur implementation.
   //
   // I called it Stack Blur because this describes best how this
   // filter works internally: it creates a kind of moving stack
   // of colors whilst scanning through the image. Thereby it
   // just has to add one new block of color to the right side
   // of the stack and remove the leftmost color. The remaining
   // colors on the topmost layer of the stack are either added on
   // or reduced by one, depending on if they are on the right or
   // on the left side of the stack.  

   // this implementation is based on the agg-version, but rewritten for my needs
    static uint16_t const mMulTable[255];
    static uint8_t const mShiftTable[255];

public:
/*  static void blur(GuiPaintInfo &drawInfo, int x0, int y0, int x1, int y1, int radiusX, int radiusY) {

      if(radiusX > 0) {
         if(radiusX > 254)
            radiusX = 254;
         int div = radiusX+radiusX+1;
         uint_t multiplicator = mMulTable[radiusX];
         uint_t shift = mShiftTable[radiusX];

         int width = x1-x0;
         int height = y1-y0;

         uint8_t *stack = new uint8_t[div*3];
         for(int y=0; y<height; y++) {
            int redSum = 0, greenSum = 0, blueSum = 0;
            int redInSum = 0, greenInSum = 0, blueInSum = 0;
            int redOutSum = 0, greenOutSum = 0, blueOutSum = 0;
            
            uint32_t *srcPointer = &drawInfo.pixelbuffer[x0+(y0+y)*drawInfo.pitch];
            uint32_t pixel = *srcPointer;
            uint8_t red = (pixel & 0x00ff0000) >> 16;
            uint8_t green = (pixel & 0x0000ff00) >> 8;
            uint8_t blue = pixel & 0x000000ff;
            for(int i=0; i<=radiusX; i++) {
               stack[i*3+0] = red;
               stack[i*3+1] = green;
               stack[i*3+2] = blue;

               redSum += red * (i+1);
               greenSum += green * (i+1);
               blueSum += blue * (i+1);

               redOutSum += red;
               greenOutSum += green;
               blueOutSum += blue;
            }

            for(int i=1; i<=radiusX; i++) {
               if(i <= width-1)
                  srcPointer += drawInfo.pitch;
               
               uint32_t pixel = *srcPointer;
               uint8_t red   = (pixel & 0x00ff0000) >> 16;
               uint8_t green = (pixel & 0x0000ff00) >> 8;
               uint8_t blue  = pixel & 0x000000ff;

               stack[(i+radiusX)*3+0] = red;
               stack[(i+radiusX)*3+1] = green;
               stack[(i+radiusX)*3+2] = blue;

               redSum   += red*(radiusX+1-i);
               greenSum += green*(radiusX+1-i);
               blueSum  += blue*(radiusX+1-i);

               redInSum   += red;
               greenInSum += green;
               blueInSum  += blue;
            }

            int stackpointer = radiusX;
            int xp = radiusX;
            if(xp > width-1)
               xp = width-1;

            srcPointer = &drawInfo.pixelbuffer[xp+x0+(y0+y)*drawInfo.pitch];
            uint32_t *dstPointer = &drawInfo.pixelbuffer[x0+(y0+y)*drawInfo.pitch];
            for(int x=0; x<width; x++) {
               uint8_t red, green, blue;
               red   = (redSum   * multiplicator) >> shift;
               green = (greenSum * multiplicator) >> shift;
               blue  = (blueSum  * multiplicator) >> shift;
               *(dstPointer++) = (red<<16) + (green<<8) + blue;

               redSum -= redOutSum;
               greenSum -= greenOutSum;
               blueSum -= blueOutSum;

               int stackStart = stackpointer + div - radiusX;
               if(stackStart >= div)
                  stackStart -= div;

               stackStart *= 3;
              
               redOutSum -= stack[stackStart+0];
               greenOutSum -= stack[stackStart+1];
               blueOutSum -= stack[stackStart+2];

               if(xp < width-1) {
                  srcPointer++;
                  xp++;
               }

               pixel = *srcPointer;
               red   = (pixel & 0x00ff0000) >> 16;
               green = (pixel & 0x0000ff00) >> 8;
               blue  = pixel & 0x000000ff;

               stack[stackStart+0] = red;
               stack[stackStart+1] = green;
               stack[stackStart+2] = blue;

               redInSum += red;
               greenInSum += green;
               blueInSum += blue;

               redSum += redInSum;
               greenSum += greenInSum;
               blueSum += blueInSum;

               stackpointer++;
               if(stackpointer >= div)
                  stackpointer = 0;

               red = stack[stackpointer*3+0];
               green = stack[stackpointer*3+1];
               blue = stack[stackpointer*3+2];

               redOutSum += red;
               greenOutSum += green;
               blueOutSum += blue;
               
               redInSum -= red;
               greenInSum -= green;
               blueInSum -= blue;
            }
         }
         delete[] stack;
      }
      
      if(radiusY > 0) {
         if(radiusY > 254)
            radiusY = 254;
         int div = radiusY + radiusY +1;
         uint_t multiplicator = mMulTable[radiusY];
         uint_t shift = mShiftTable[radiusY];

         int width = x1-x0;
         int height = y1-y0;

         uint8_t *stack = new uint8_t[div*3];

         for(int x=0; x<width; x++) {
            int redSum = 0, greenSum = 0, blueSum = 0;
            int redInSum = 0, greenInSum = 0, blueInSum = 0;
            int redOutSum = 0, greenOutSum = 0, blueOutSum = 0;

            uint32_t *srcPointer = &drawInfo.pixelbuffer[x0+x+y0*drawInfo.pitch];
            uint32_t pixel = *srcPointer;
            uint8_t red = (pixel & 0x00ff0000) >> 16;
            uint8_t green = (pixel & 0x0000ff00) >> 8;
            uint8_t blue = pixel & 0x000000ff;
            for(int i=0; i<=radiusY; i++) {
               stack[i*3+0] = red;
               stack[i*3+1] = green;
               stack[i*3+2] = blue;

               redSum += red * (i+1);
               greenSum += green * (i+1);
               blueSum += blue * (i+1);

               redOutSum += red;
               greenOutSum += green;
               blueOutSum += blue;
            }
            for(int i=1; i<=radiusY; i++) {
               if(i<=height-1)
                  srcPointer += drawInfo.pitch;
                  pixel = *srcPointer;
                  red = (pixel & 0x00ff0000) >> 16;
                  green = (pixel & 0x0000ff00) >> 8;
                  blue = pixel & 0x000000ff;
                  
                  stack[(i+radiusY)*3+0] = red;
                  stack[(i+radiusY)*3+1] = green;
                  stack[(i+radiusY)*3+2] = blue;

                  redSum += red * (radiusY+1-i);
                  greenSum += green * (radiusY+1-i);
                  blueSum += blue * (radiusY+1-i);

                  redInSum += red;
                  greenInSum += green;
                  blueInSum += blue;
            }
            int stackpointer = radiusY;
            int yp = radiusY;
            if(yp > height-1)
               yp = height-1;
            srcPointer = &drawInfo.pixelbuffer[x0+x+(y0+yp)*drawInfo.pitch];
            uint32_t *dstPointer = &drawInfo.pixelbuffer[x0+x+y0*drawInfo.pitch];
            for(int y=0; y<height; y++) {
               red = (redSum*multiplicator) >> shift;
               green = (greenSum*multiplicator) >> shift;
               blue = (blueSum*multiplicator) >> shift;
               *dstPointer = (red<<16) + (green<<8) + blue;
               dstPointer += drawInfo.pitch;

               redSum -= redOutSum;
               greenSum -= greenOutSum;
               blueSum -= blueOutSum;

               int stackStart = stackpointer + div - radiusY;
               if(stackStart >= div)
                  stackStart -= div;

               stackStart *= 3;

               red = stack[stackStart+0];
               green = stack[stackStart+1];
               blue = stack[stackStart+2];

               redOutSum -= red;
               greenOutSum -= green;
               blueOutSum -= blue;

               if(yp < height-1) {
                  srcPointer += drawInfo.pitch;
                  yp++;
               }

               pixel = *srcPointer;
               red   = (pixel & 0x00ff0000) >> 16;
               green = (pixel & 0x0000ff00) >> 8;
               blue  = pixel & 0x000000ff;

               stack[stackStart+0] = red;
               stack[stackStart+1] = green;
               stack[stackStart+2] = blue;

               redInSum += red;
               greenInSum += green;
               blueInSum += blue;

               redSum += redInSum;
               greenSum += greenInSum;
               blueSum += blueInSum;

               stackpointer++;
               if(stackpointer >= div)
                  stackpointer = 0;

               red = stack[stackpointer*3+0];
               green = stack[stackpointer*3+1];
               blue = stack[stackpointer*3+2];

               redOutSum += red;
               greenOutSum += green;
               blueOutSum += blue;

               redInSum -= red;
               greenInSum -= green;
               blueInSum -= blue;
            }
         }
         delete[] stack;
      }
   }*/
};

#endif   //#ifndef __IMAGEUTILS_STACKBLUR_H__
