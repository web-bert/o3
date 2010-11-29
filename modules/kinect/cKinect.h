/*
* Copyright (C) 2010 Ajax.org BV
*
* This library is free software; you can redistribute it and/or modify it under
* the terms of the GNU General Public License as published by the Free Software
* Foundation; either version 2 of the License, or (at your option) any later
* version.
*
* This library is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
* FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
* details.
*
* You should have received a copy of the GNU General Public License along with
* this library; if not, write to the Free Software Foundation, Inc., 51
* Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
#ifndef O3_C_KINECT_H
#define O3_C_KINECT_H

#include <libfreenect.h>

#include "canvas/canvas.h"

namespace o3 
{
	
	freenect_context *gFreenectContext= NULL;
	int gFreenectContextCount = 0;
	void FreenectLog(freenect_context *dev, freenect_loglevel level, const char *msg, ...)
	{
		// nothing logged yet...
	};
	
	void gDepthCallback(freenect_device *dev, freenect_depth *depth, uint32_t timestamp);
	void gRGBCallback(freenect_device *dev, freenect_pixel *rgb, uint32_t timestamp);
	


	struct cKinect : cScr
	{
		freenect_device *mDevice;

		static o3_ext("cO3") o3_fun siScr kinect(size_t index= 0)
		{              
			cKinect* ret = o3_new(cKinect)(index);
			return ret;		
		}
		
		int mRGBFrameCount;
		int mDepthFrameCount;
		int mRGBFrame;
		int mDepthFrame;
		unsigned char mRGBFrameStore[640*480*3];
		unsigned short mDepthFrameStore[640*480];

		cKinect(size_t kinectnumber = 0)
		{
			mRGBFrameCount = 0;
			mRGBFrame = 0;
			mDepthFrameCount = 0;
			mDepthFrame = 0;
			mDevice = NULL;
			if (gFreenectContextCount == 0)
			{
				freenect_init(&gFreenectContext, NULL);
				freenect_set_log_level(gFreenectContext, FREENECT_LOG_FLOOD);
				freenect_set_log_callback(gFreenectContext, (freenect_log_cb) FreenectLog);
			}

			gFreenectContextCount++;


			unsigned int DeviceCount = freenect_num_devices(gFreenectContext);
			if (DeviceCount > 0)
			{
				if (kinectnumber>=DeviceCount) kinectnumber = 0;
				freenect_open_device(gFreenectContext, &mDevice, kinectnumber);
				if (mDevice)
				{
					freenect_set_user(mDevice, (void*)this);
					freenect_set_depth_callback(mDevice, gDepthCallback);
					freenect_set_rgb_callback(mDevice, gRGBCallback);
					freenect_start_depth(mDevice);
					freenect_start_rgb(mDevice);	
				};
			};
		};

		o3_fun void processEvents()
		{
			if (gFreenectContext) freenect_process_events(gFreenectContext);
		};

		void DepthCallback(unsigned short *depth)
		{
			mDepthFrameCount++;
			memcpy(mDepthFrameStore, depth, sizeof(unsigned short)*640*480);
		};

		void RGBCallback(unsigned char *rgb)
		{
			mRGBFrameCount++;
			memcpy(mRGBFrameStore, rgb, 3*640*480);
		};
		
		o3_fun bool newDepthAvailable()
		{
			if (mDepthFrameCount > mDepthFrame)
			{
				mDepthFrame = mDepthFrameCount;
				return true;
			};
			return false;
		};

		o3_fun bool newRGBAvailable()
		{
			if (mRGBFrameCount > mRGBFrame)
			{
				mRGBFrame = mRGBFrameCount;
				return true;
			};
			return false;
		};

		o3_fun void RGBToCanvas(iScr *canvas)
		{
			siImage img(canvas);
			if (img)
			{
				switch (img->mode_int())					
				{
				case Image::MODE_ARGB:
					{
						int w = __min(img->width(), 640);
						int h = __min(img->height(), 480);
						for (int y =0;y<h;y++)
						{
							unsigned char *src = mRGBFrameStore + (640*y*3);
							unsigned char *dst = img->getrowptr(y);
							for (int x = 0;x<w;x++)
							{
								dst[2]  = *src++;
								dst[1]  = *src++;
								dst[0]  = *src++;
								dst[3] = 255;
								dst+=4;
							};
						}
					};
					break;
				case Image::MODE_RGB:
					{
						int w = __min(img->width(), 640);
						int h = __min(img->height(), 480);
						for (int y =0;y<h;y++)
						{
							unsigned char *src = mRGBFrameStore + 640*y;
							unsigned char *dst = img->getrowptr(y);
							for (int x = 0;x<w;x++)
							{
								dst[2]  = *src++;
								dst[1]  = *src++;
								dst[0]  = *src++;
								dst+=3;
							};
						}
					};
					break;
				};					
			};
		};

		o3_fun void DepthToCanvas(iScr *canvas)
		{

		};

		o3_fun void DepthToVBO(iScr *vbo)
		{
		};

		virtual ~cKinect()
		{
			if (mDevice)
			{
				freenect_close_device(mDevice);
				mDevice = NULL;
			}
			gFreenectContextCount--;
			if (gFreenectContextCount == 0)
			{
				freenect_shutdown(gFreenectContext);
				gFreenectContext = NULL;
			};
		}

		o3_begin_class(cScr)
		o3_end_class()

		o3_glue_gen()
	};

	void gDepthCallback(freenect_device *dev, freenect_depth *depth, uint32_t timestamp)
	{
		cKinect *K = (cKinect *)freenect_get_user(dev);
		if (K)
		{
			K->DepthCallback(depth);
		};
	};
	
	void gRGBCallback(freenect_device *dev, freenect_pixel *rgb, uint32_t timestamp)
	{
		cKinect *K = (cKinect *)freenect_get_user(dev);
		if (K)
		{
			K->RGBCallback(rgb);
		};
	};
}

#endif 