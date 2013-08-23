/*
  Copyright 1999-2013 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.
  obtain a copy of the License at
  
    http://www.imagemagick.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore private methods for accelerated functions.
*/

#ifndef _MAGICKCORE_ACCELERATE_PRIVATE_H
#define _MAGICKCORE_ACCELERATE_PRIVATE_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif


#if defined(MAGICKCORE_OPENCL_SUPPORT)


#define STRINGIFY(...) #__VA_ARGS__ "\n"

char* accelerate_kernels[] =
{
  STRINGIFY(
     typedef enum
     {
       UndefinedChannel,
       RedChannel = 0x0001,
       GrayChannel = 0x0001,
       CyanChannel = 0x0001,
       GreenChannel = 0x0002,
       MagentaChannel = 0x0002,
       BlueChannel = 0x0004,
       YellowChannel = 0x0004,
       AlphaChannel = 0x0008,
       OpacityChannel = 0x0008,
       MatteChannel = 0x0008,     /* deprecated */
       BlackChannel = 0x0020,
       IndexChannel = 0x0020,
       CompositeChannels = 0x002F,
       AllChannels = 0x7ffffff,
       /*
       Special purpose channel types.
       */
       TrueAlphaChannel = 0x0040, /* extract actual alpha channel from opacity */
       RGBChannels = 0x0080,      /* set alpha from  grayscale mask in RGB */
       GrayChannels = 0x0080,
       SyncChannels = 0x0100,     /* channels should be modified equally */
       DefaultChannels = ((AllChannels | SyncChannels) &~ OpacityChannel)
     } ChannelType;
     )
  ,
  STRINGIFY(
    inline int ClampToCanvas(const int offset,const int range)
      {
      	return clamp(offset, (int)0, range-1);
      }
  ),
  STRINGIFY(
    inline CLQuantum ClampToQuantum(const float value)
      {
      	return (CLQuantum) (clamp(value, 0.0f, (float) QuantumRange) + 0.5f);
      }
  ),
  STRINGIFY(
    inline float PerceptibleReciprocal(const float x)
    {
      float sign = x < (float) 0.0 ? (float) -1.0 : (float) 1.0;
      return((sign*x) >= MagickEpsilon ? (float) 1.0/x : sign*((float) 1.0/MagickEpsilon));
    }
  ),
  STRINGIFY(
      __kernel 
      void ConvolveOptimized(const __global CLPixelType *input, __global CLPixelType *output,
                                      const unsigned int imageWidth, const unsigned int imageHeight,
                                      __constant float *filter, 
                                      const unsigned int filterWidth, const unsigned int filterHeight,
                                      const uint matte, const ChannelType channel,
                                      __local CLPixelType *pixelLocalCache, __local float* filterCache) {

          int2 blockID;
          blockID.x = get_group_id(0);
          blockID.y = get_group_id(1);

          // image area processed by this workgroup
          int2 imageAreaOrg;
          imageAreaOrg.x = blockID.x * get_local_size(0);
          imageAreaOrg.y = blockID.y * get_local_size(1);

          int2 midFilterDimen;
          midFilterDimen.x = (filterWidth-1)/2;
          midFilterDimen.y = (filterHeight-1)/2;


          int2 cachedAreaOrg = imageAreaOrg - midFilterDimen;

          // dimension of the local cache
          int2 cachedAreaDimen;
          cachedAreaDimen.x = get_local_size(0) + filterWidth - 1;
          cachedAreaDimen.y = get_local_size(1) + filterHeight - 1;

          // cache the pixels accessed by this workgroup in local memory
          int localID = get_local_id(1)*get_local_size(0)+get_local_id(0);
          int cachedAreaNumPixels = cachedAreaDimen.x * cachedAreaDimen.y;
          int groupSize = get_local_size(0) * get_local_size(1);
          for (int i = localID; i < cachedAreaNumPixels; i+=groupSize) {

            int2 cachedAreaIndex;
            cachedAreaIndex.x = i % cachedAreaDimen.x;
            cachedAreaIndex.y = i / cachedAreaDimen.x;

            int2 imagePixelIndex;
            imagePixelIndex = cachedAreaOrg + cachedAreaIndex;

	    // only support EdgeVirtualPixelMethod through ClampToCanvas
	    // TODO: implement other virtual pixel method
            imagePixelIndex.x = ClampToCanvas(imagePixelIndex.x, imageWidth);
            imagePixelIndex.y = ClampToCanvas(imagePixelIndex.y, imageHeight);

            pixelLocalCache[i] = input[imagePixelIndex.y * imageWidth + imagePixelIndex.x];
          }

          // cache the filter
          for (int i = localID; i < filterHeight*filterWidth; i+=groupSize) {
            filterCache[i] = filter[i];
          }
          barrier(CLK_LOCAL_MEM_FENCE);


          int2 imageIndex;
          imageIndex.x = imageAreaOrg.x + get_local_id(0);
          imageIndex.y = imageAreaOrg.y + get_local_id(1);

          // if out-of-range, stops here and quit
          if (imageIndex.x >= imageWidth
            || imageIndex.y >= imageHeight) {
            return;
          }

          int filterIndex = 0;
          float4 sum = (float4)0.0f;
          float gamma = 0.0f;
          if (((channel & OpacityChannel) == 0) || (matte == 0)) {
            int cacheIndexY = get_local_id(1);
            for (int j = 0; j < filterHeight; j++) {
              int cacheIndexX = get_local_id(0);
              for (int i = 0; i < filterWidth; i++) {
                CLPixelType p = pixelLocalCache[cacheIndexY*cachedAreaDimen.x + cacheIndexX];
                float f = filterCache[filterIndex];

                sum.x += f * p.x;
                sum.y += f * p.y;
                sum.z += f * p.z; 
		sum.w += f * p.w;

                gamma += f;
                filterIndex++;
                cacheIndexX++;
              }
              cacheIndexY++;
            }
          }
          else {
            int cacheIndexY = get_local_id(1);
            for (int j = 0; j < filterHeight; j++) {
              int cacheIndexX = get_local_id(0);
              for (int i = 0; i < filterWidth; i++) {

                CLPixelType p = pixelLocalCache[cacheIndexY*cachedAreaDimen.x + cacheIndexX];
                float alpha = QuantumScale*(QuantumRange-p.w);
                float f = filterCache[filterIndex];
                float g = alpha * f;

                sum.x += g*p.x;
                sum.y += g*p.y;
                sum.z += g*p.z;
                sum.w += f*p.w;

                gamma += g;
                filterIndex++;
                cacheIndexX++;
              }
              cacheIndexY++;
            }
	    gamma = PerceptibleReciprocal(gamma);
	    sum.xyz = gamma*sum.xyz;
          }
	  CLPixelType outputPixel;
          outputPixel.x = ClampToQuantum(sum.x);
          outputPixel.y = ClampToQuantum(sum.y);
          outputPixel.z = ClampToQuantum(sum.z);
	  outputPixel.w = ((channel & OpacityChannel)!=0)?ClampToQuantum(sum.w):input[imageIndex.y * imageWidth + imageIndex.x].w;

          output[imageIndex.y * imageWidth + imageIndex.x] = outputPixel;
      }
  )
  ,
  NULL   // Last entry has to be NULL
};

#endif // MAGICKCORE_OPENCL_SUPPORT

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif // _MAGICKCORE_ACCELERATE_PRIVATE_H
