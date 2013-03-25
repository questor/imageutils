
#ifndef __IMAGEUTILS_SOFTBLITTER_H__
#define __IMAGEUTILS_SOFTBLITTER_H__

#include "eastl/types.h"
#include "eastl/extra/fixedpoint.h"
#include <memory.h>

class Blitter {
public:
   template<typename PixelType> struct CopyDescr {
      int posX, posY;
      int width, height;
      PixelType *data;
      int picWidth, picHeight;
      void set(int x, int y, int w, int h) {
         posX = x; posY = y; width = w; height = h;
      }
   };

   //=== the pixel processors

   struct CopyPixel {
      template<typename PixelTypeSrc, typename PixelTypeDst> static
            void processPixel(PixelTypeSrc *src, PixelTypeDst *dest) {
         static_assert(sizeof(PixelTypeSrc) == sizeof(PixelTypeDst), "pixel types are not of the same size");
         *dest = *src;
      }
      template<typename PixelTypeSrc, typename PixelTypeDst> static
            void processLine(PixelTypeSrc *src, PixelTypeDst *dest, size_t numberPixels) {
         //is this specialization needed?
         static_assert(sizeof(PixelTypeSrc) == sizeof(PixelTypeDst), "pixel types are not of the same size");
         memcpy(dest, src, sizeof(PixelTypeSrc)*numberPixels);
      }
   };
   template<int shift> struct ConvertGrayscaleToPixel {
      template<typename PixelTypeSrc, typename PixelTypeDst> static
            void processPixel(PixelTypeSrc *src, PixelTypeDst *dest) {
         static_assert(sizeof(PixelTypeSrc) == 4, "pixel types in wrong format");
         static_assert(sizeof(PixelTypeSrc) == 4, "pixel types in wrong format");
         uint32_t val = (*src) >> shift;
         *dest = 0xff000000 | (val<<16) | (val<<8) | val;
      }
      template<typename PixelTypeSrc, typename PixelTypeDst> static
            void processLine(PixelTypeSrc *src, PixelTypeDst *dest, size_t numberPixels) {
         //is this specialization needed?
         static_assert(sizeof(PixelTypeSrc) == 4, "pixel types in wrong format");
         static_assert(sizeof(PixelTypeSrc) == 4, "pixel types in wrong format");
         for(unsigned int i=0; i<numberPixels; ++i) {
            uint32_t val = (*src) >> shift;
            ++src;
            *(dest++) = 0xff000000 | (val<<16) | (val<<8) | val;
         }
      }
   };

   struct BlendPixelFullTransparence {
      static inline void pixelBlend(uint32_t &d, const uint32_t s) {
         const uint32_t a     = (s >> 24) + 1;

         const uint32_t dstrb = d & 0xFF00FF;
         const uint32_t dstg  = d & 0xFF00;

         const uint32_t srcrb = s & 0xFF00FF;
         const uint32_t srcg  = s & 0xFF00;

         uint32_t drb = srcrb - dstrb;
         uint32_t dg  =  srcg - dstg;

         drb *= a;
         dg  *= a;
         drb >>= 8;
         dg  >>= 8;

         uint32_t rb = (drb + dstrb) & 0xFF00FF;
         uint32_t g  = (dg  + dstg) & 0xFF00;

         d = rb | g;
      }

      template<typename PixelTypeSrc, typename PixelTypeDst> static
            void processPixel(PixelTypeSrc *src, PixelTypeDst *dest) {
         static_assert(sizeof(PixelTypeSrc) == 4, "pixel types in wrong format");
         static_assert(sizeof(PixelTypeSrc) == 4, "pixel types in wrong format");
         pixelBlend(*dest, *src);
      }
      template<typename PixelTypeSrc, typename PixelTypeDst> static
            void processLine(PixelTypeSrc *src, PixelTypeDst *dest, size_t numberPixels) {
         //is this specialization needed?
         static_assert(sizeof(PixelTypeSrc) == 4, "pixel types in wrong format");
         static_assert(sizeof(PixelTypeSrc) == 4, "pixel types in wrong format");
         for(unsigned int i=0; i<numberPixels; ++i) {
            pixelBlend(*src, *dest);
            ++src;
            ++dest;
         }
      }
   };

   struct BlendPixel1BitTransparence {
      template<typename PixelTypeSrc, typename PixelTypeDst> static
            void processPixel(PixelTypeSrc *src, PixelTypeDst *dest) {
         static_assert(sizeof(PixelTypeSrc) == 4, "pixel types in wrong format");
         static_assert(sizeof(PixelTypeSrc) == 4, "pixel types in wrong format");
         PixelTypeSrc srcPixel = *src;
         if((srcPixel & 0xff000000)!=0x0)
            *dest = srcPixel;
      }
      template<typename PixelTypeSrc, typename PixelTypeDst> static
            void processLine(PixelTypeSrc *src, PixelTypeDst *dest, size_t numberPixels) {
         //is this specialization needed?
         static_assert(sizeof(PixelTypeSrc) == 4, "pixel types in wrong format");
         static_assert(sizeof(PixelTypeSrc) == 4, "pixel types in wrong format");
         for(unsigned int i=0; i<numberPixels; ++i) {
            PixelTypeSrc srcPixel = *src;
            if((srcPixel & 0xff000000)!=0x0)
               *dest = srcPixel;
            ++src;
            ++dest;
         }
      }
   };

   //=== the blitting routines

   template<typename PixelTypeSrc, typename PixelTypeDst, class Processor> static
         void blit(CopyDescr<PixelTypeSrc> &source, CopyDescr<PixelTypeDst> &dest) {
      if(source.width == dest.width) {
         if(source.height == dest.height) {
            //hier reicht kopieren
            PixelTypeDst *dst = dest.data+dest.posX+dest.posY*dest.picWidth;
            PixelTypeSrc *src = source.data+source.posX+source.posY*source.picWidth;
            for(int y=0; y<dest.height; ++y) {
               Processor::processLine<PixelTypeSrc, PixelTypeDst>(src, dst, dest.width);
               dst += dest.picWidth;
               src += source.picWidth;
            }
         } else {
            //bild nur horizontal skalieren
            eastl::FixedPoint32 posY = 0;
            eastl::FixedPoint32 addY;
            addY.set((float)source.height / (float)dest.height);
            PixelTypeDst *dst = dest.data+dest.posX+dest.posY*dest.picWidth;
            PixelTypeSrc *src = source.data+source.posX+source.posY*source.picWidth;
            for(int y=0; y<dest.height; ++y) {
               PixelTypeSrc *srcLine = src+((int)posY)*source.picWidth;
               Processor::processLine<PixelTypeSrc, PixelTypeDst>(srcLine, dst, dest.width);
               posY += addY;
               dst += dest.picWidth;
            }
         }
      } else {
         //bild komplett skalieren
         eastl::FixedPoint32 posX = 0;
         eastl::FixedPoint32 posY = 0;
         eastl::FixedPoint32 addX;
         eastl::FixedPoint32 addY;
         addX.set((float)source.width / (float)dest.width);
         addY.set((float)source.height / (float)dest.height);
         PixelTypeDst *dst = dest.data+dest.posX+dest.posY*dest.picWidth;
         PixelTypeSrc *src = source.data+source.posX+source.posY*source.picWidth;
         for(int y=0; y<dest.height; ++y) {
            PixelTypeSrc *srcLine = src+((int)posY)*source.picWidth;
            eastl::FixedPoint32 workX = posX;
            PixelTypeDst *workDst = dst;
            for(int x=0; x<dest.width; ++x) {
               Processor::processPixel<PixelTypeSrc, PixelTypeDst>(srcLine+((int)workX), workDst);
               workX += addX;
               ++workDst;
            }
            posY += addY;
            dst += dest.picWidth;
         }
      }
   }

   template<typename PixelTypeSrc, typename PixelTypeDst, class Processor> static
         void blitClipped(CopyDescr<PixelTypeSrc> &source, CopyDescr<PixelTypeDst> &dest) {
      if(source.width == dest.width) {
         if(source.height == dest.height) {
            //hier reicht kopieren
            if(dest.posX < 0) {           //links clippen
               source.posX += -dest.posX;
               dest.width -= -dest.posX;
               dest.posX = 0;
            }
            if(dest.posX+dest.width >= dest.picWidth) {      //rechts clippen
               dest.width -= (dest.posX+dest.width - dest.picWidth);
            }
            if(dest.posY < 0) {           //oben clippen
               source.posY += -dest.posY;
               dest.height -= -dest.posY;
               dest.posY = 0;
            }
            if(dest.posY+dest.height >= dest.picHeight) {    //unten clippen
               dest.height -= (dest.posY+dest.height - dest.picHeight);
            }

            PixelTypeDst *dst = dest.data+dest.posX+dest.posY*dest.picWidth;
            PixelTypeSrc *src = source.data+source.posX+source.posY*source.picWidth;
            for(int y=0; y<dest.height; ++y) {
               Processor::processLine<PixelTypeSrc, PixelTypeDst>(src, dst, dest.width);
               dst += dest.picWidth;
               src += source.picWidth;
            }
         } else {
            //bild nur horizontal skalieren
            eastl::FixedPoint32 posY = 0;
            eastl::FixedPoint32 addY;
            addY.set((float)source.height / (float)dest.height);

            if(dest.posX < 0) {           //links clippen
               source.posX += -dest.posX;
               dest.width -= -dest.posX;
               dest.posX = 0;
            }
            if(dest.posX+dest.width >= dest.picWidth) {      //rechts clippen
               dest.width -= (dest.posX+dest.width - dest.picWidth);
            }
            if(dest.posY < 0) {           //oben clippen
               posY = eastl::FixedPoint32((float)(-dest.posY))*addY;
               dest.height -= -dest.posY;
               dest.posY = 0;
            }
            if(dest.posY+dest.height >= dest.picHeight) {    //unten clippen
               dest.height -= (dest.posY+dest.height - dest.picHeight);
            }

            PixelTypeDst *dst = dest.data+dest.posX+dest.posY*dest.picWidth;
            PixelTypeSrc *src = source.data+source.posX+source.posY*source.picWidth;
            for(int y=0; y<dest.height; ++y) {
               PixelTypeSrc *srcLine = src+((int)posY)*source.picWidth;
               Processor::processLine<PixelTypeSrc, PixelTypeDst>(srcLine, dst, dest.width);
               posY += addY;
               dst += dest.picWidth;
            }
         }
      } else {
         //bild komplett skalieren
         eastl::FixedPoint32 posX = 0;
         eastl::FixedPoint32 posY = 0;
         eastl::FixedPoint32 addX;
         eastl::FixedPoint32 addY;
         addX.set((float)source.width / (float)dest.width);
         addY.set((float)source.height / (float)dest.height);

         if(dest.posX < 0) {           //links clippen
            posX = eastl::FixedPoint32(-dest.posX)*addX;
            dest.width -= -dest.posX;
            dest.posX = 0;
         }
         if(dest.posX+dest.width >= dest.picWidth) {      //rechts clippen
            dest.width -= (dest.posX+dest.width - dest.picWidth);
         }
         if(dest.posY < 0) {           //oben clippen
            posY = eastl::FixedPoint32((float)(-dest.posY))*addY;
            dest.height -= -dest.posY;
            dest.posY = 0;
         }
         if(dest.posY+dest.height >= dest.picHeight) {    //unten clippen
            dest.height -= (dest.posY+dest.height - dest.picHeight);
         }

         PixelTypeDst *dst = dest.data+dest.posX+dest.posY*dest.picWidth;
         PixelTypeSrc *src = source.data+source.posX+source.posY*source.picWidth;
         for(int y=0; y<dest.height; ++y) {
            PixelTypeSrc *srcLine = src+((int)posY)*source.picWidth;
            eastl::FixedPoint32 workX = posX;
            PixelTypeDst *workDst = dst;
            for(int x=0; x<dest.width; ++x) {
               Processor::processPixel<PixelTypeSrc, PixelTypeDst>(srcLine+((int)workX), workDst);
               workX += addX;
               ++workDst;
            }
            posY += addY;
            dst += dest.picWidth;
         }
      }
   }

   //beware of the code bloat!
   template<typename PixelTypeSrc, typename PixelTypeDst, class Processor> static
         void drawImage(CopyDescr<PixelTypeSrc> &source, CopyDescr<PixelTypeDst> &dest) {
      //check for complete offscreen drawing
      if(dest.posX+dest.width <= 0)
         return;
      if(dest.posX > dest.picWidth)
         return;
      if(dest.posY+dest.height <= 0)
         return;
      if(dest.posY > dest.picHeight)
         return;

      //check for clipping
      bool clipping = false;
      if(dest.posX < 0)
         clipping = true;
      if(dest.posY < 0)
         clipping = true;
      if(dest.posX+dest.width > dest.picWidth)
         clipping = true;
      if(dest.posY+dest.height > dest.picHeight)
         clipping = true;

      if(clipping)
         blitClipped<PixelTypeSrc, PixelTypeDst, Processor>(source, dest);
      else
         blit<PixelTypeSrc, PixelTypeDst, Processor>(source, dest);
   }
};

#endif   //#ifndef __IMAGEUTILS_SOFTBLITTER_H__
