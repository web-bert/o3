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
#ifndef O3_C_CANVAS_H
#define O3_C_CANVAS_H

#include <lib_zlib.h>
#include <tools_math.h>
#include <math.h>


#include <lib_agg.h>
#include <lib_freetype.h>
#include <lib_png.h>

#include "cCanvas_colors.h"
#include "cCanvas_utils.h"
#include "cCanvas_others.h"
#include "cCanvas_curves.h"

namespace o3 
{
	struct cCanvas : cScr, iImage
	{
#pragma region O3_SCR
		
		o3_begin_class(cScr)
		    o3_add_iface(iImage)
		o3_end_class()

		o3_glue_gen()

		static o3_ext("cO3") o3_fun siScr canvas()
		{
			o3_trace3 trace;
			return o3_new(cCanvas)();
		}

		static o3_ext("cO3") o3_fun siScr canvas(size_t w, size_t h, const char* mode = "argb" )
		{
			return o3_new(cCanvas)(w,h,mode);
		}

#pragma endregion O3_SCR


#pragma region LocalClassDefitions
		struct Path 
		{
			tVec<V2<double> > m_path;
		};


		struct RenderState
		{
			unsigned int FillColor;
			unsigned int ClearColor;
			unsigned int StrokeColor;
			double StrokeWidth;

	/*
		// css 1.0 based font properties:
			Str FontFamily; // Serif, [Sans-serif], Monospace, fontfilenames
			Str FontSize; // xx-small x-small small medium large x-large xx-large smaller larger length %
			Str FontStyle; // [normal] italic oblique
			Str FontVariant; // [normal] small-caps
			Str FontWeight; // [normal] bold bolder lighter 100 200 300 400 500 600 700 800 900			
			Str TextDirectionality; // [LTR], RTL -> needs to be dealt with because of align = start/end
			Str TextBaseline; // top hanging middle [alphabetic] ideographic bottom
		*/		
			
			int FillStyle;
			M33<double> Transformation;
			bool ClippingEnabled;

			V2<double> ClipTopLeft;
			V2<double> ClipBottomRight;

			// todo -> add clipping path!
			double miterLimit;
		};
#pragma endregion LocalClassDefitions
		
		Str m_mode;
		int m_mode_int;
		bool m_graphics_attached;

		agg::Agg2D m_graphics;

		tVec<Path> m_paths;
		V2<double> m_lastpoint;
		tVec<RenderState> m_renderstates;
		RenderState *m_currentrenderstate;

		size_t m_w, m_h, m_stride;
		int    m_bytesperpixel;
		int    m_bitdepth;
		Buf	   m_mem;
		Buf	   m_alphamem;
#pragma region ObjectLifeCycleManagement
		cCanvas()
		{
			m_w = m_h = m_stride = 0;
			m_bitdepth = 32;
			m_bytesperpixel = 4;
			m_graphics_attached = false;
			m_mode = Str("argb");
			SetupRenderState();
			Ensure32BitSurface();
		};

		cCanvas(size_t w, size_t h, const Str &mode)
		{
			SetupMode(w,h,mode);
		};

		void SetupMode(size_t w, size_t h, const Str &mode)
		{
			m_w = w;
			m_h = h;
			m_stride = (m_w+7)&~(7);
			m_graphics_attached = false;
			m_mode = mode;
			m_bytesperpixel = 4;
			m_bitdepth = 32;
			if (m_mode == "argb")
			{
				m_bytesperpixel = 4;
				m_bitdepth = 32;
				m_mode_int = Image::MODE_ARGB;
			}
			else if (m_mode == "gray" || m_mode == "grey")
			{
				m_bitdepth = 8;
				m_bytesperpixel= 1;
				m_mode_int = Image::MODE_GRAY;
			}
			else if (m_mode == "bw")
			{
				m_bitdepth = 1;
				m_bytesperpixel = 1;
				m_mode_int = Image::MODE_BW;

			}
			else if (m_mode == "rgb")
			{
				m_bitdepth = 24;
				m_bytesperpixel = 3;
				m_mode_int = Image::MODE_RGB;
			}

			SetupBuffer();
			SetupRenderState();

			if (m_mode_int == Image::MODE_ARGB)
			{
				Ensure32BitSurface();
			};
		};

		void SetupBuffer()
		{	
			size_t newsize = 0;
			switch(m_mode_int)
			{
			case Image::MODE_BW:
				newsize = (m_stride * m_h)/8;
				break;
			default:
				newsize = m_stride * m_h * m_bytesperpixel;
				break;
			}
			m_mem.resize(newsize);
			m_graphics_attached = false;
		};

		void Ensure32BitSurface()
		{
			if (m_mode_int != Image::MODE_ARGB)
			{
				// TODO -- convert existing bitmap to 32 bit and remember old mode. 
			};

			if (!m_graphics_attached && m_mode_int == Image::MODE_ARGB) 
			{
				m_graphics.attach((unsigned char *)m_mem.ptr(), m_w, m_h, m_stride*4);
				// TODO -- check different pixel alignments
				//				m_graphics.viewport(0,0,m_w, m_h, 0,0,m_w, m_h, agg::Agg2D::ViewportOption::XMidYMid);
				RestoreStateToGraphicsObject();
				m_graphics_attached = true;
			};
		};

		void AttachAlpha()
		{
			if (m_alphamem.size() < m_stride*m_h)
			{
				m_alphamem.resize(m_stride*m_h);
				m_alphamem.set<unsigned char>(0, 0, m_alphamem.size());
			};
			m_graphics.attachalpha((unsigned char *)m_alphamem.ptr(), m_w, m_h, m_stride);
		};

#pragma endregion ObjectLifeCycleManagement

        // -------------------------------------------------------
        // 
        // Image API
        //
        // -------------------------------------------------------
#pragma region ImageAPI_and_iImage
        o3_get Str mode(){return m_mode;}

		o3_get size_t x(){return m_w;}
		o3_get size_t y(){return m_h;}

		virtual o3_get size_t width(){return m_w;}
		virtual o3_get size_t height(){return m_h;}

		virtual size_t stride(){return m_stride;};
		virtual size_t bpp(){return m_bitdepth;};
		virtual size_t mode_int(){return m_mode_int;}

		virtual unsigned char *getbufptr(){return (unsigned char*)m_mem.ptr();};
		virtual unsigned char *getrowptr(size_t y){return _getRowPtr(y);};

		__inline unsigned char *_getRowPtr(size_t y)
		{
			if (m_mode_int == Image::MODE_BW) 
			{
				if (y < (int)m_h) 
				{
					return ((unsigned char *)m_mem.ptr() + (m_stride * y) / 8);
				};
			}
			else
			{
				if (y < m_h) 
				{
					return ((unsigned char *)m_mem.ptr() + (m_stride * y) * m_bytesperpixel);
				}
			};
			return 0;
		};

o3_fun void clear(int signed_color)
		{
			unsigned int color = (unsigned int) signed_color;
			switch(m_mode_int)
			{
			case Image::MODE_ARGB:
				m_mem.set<int>(0,color, m_mem.size());
				break;
			case Image::MODE_BW:
				if (color &0xffffff)
				{
					m_mem.set<unsigned char>(0,0xff, m_mem.size());
				}
				else
				{
					m_mem.set<unsigned char>(0,0, m_mem.size());
				}

				break;

			default:
				for (size_t y = 0;y<m_h;y++)
				{
					for (size_t x=0;x<m_w;x++) 
					{
						setPixel(x,y,color);
					};
				};
				break;
			};
		};        
		o3_fun void setPixel(size_t x, size_t y, int signed_color)
		{
			unsigned int color = (unsigned int) signed_color;
			unsigned char *D = _getRowPtr(y);
			if(D)
			{
				if (x >= 0 && x < m_w)
				{
					switch(m_mode_int)
					{

					case Image::MODE_BW:
						{
							int shift = x&7;
							x>>=3;
							int mask = 1<<(7-shift);
							unsigned char *pixeldest = D + x;
							if (color&0xffffff)
							{
								*pixeldest |= mask;
							}
							else
							{
								*pixeldest &= ~mask;
							}
						};break;
					case Image::MODE_GRAY:
						{
							unsigned char *pixeldest = D+x;
							unsigned char *srcchannels = (unsigned char *) &color;
							unsigned char a = srcchannels[3];
							if (a == 255)
							{
								*pixeldest = srcchannels[0];
							}
							else
							{
								unsigned char *dstchannels = (unsigned char *) pixeldest;
								unsigned char inva = ~a;

								srcchannels[0]= (dstchannels[0]*inva + srcchannels[0]*a)>>8;
								*pixeldest = srcchannels[0];
							}

						}break;
					case Image::MODE_ARGB:
						{
							unsigned int *pixeldest = ((unsigned int *)(D)) + x;

							unsigned char *srcchannels = (unsigned char *) &color;
							unsigned char a = srcchannels[3];
							if (a == 255)
							{
								*pixeldest = color;
							}
							else
							{
								unsigned char *dstchannels = (unsigned char *) pixeldest;
								unsigned char inva = ~a;
								srcchannels[3]= 0xff; //TODO dst alpha needs to get some meaning!
								for (int j= 0;j<3;j++)
								{
									srcchannels[j]= (dstchannels[j]*inva + srcchannels[j]*a)>>8;
								}
								//TODO: add blendpixel stuff that properly uses the alpha information.

								*pixeldest = color;
							};
						};break;
					};
				};
			};
		};

		o3_fun int getPixel(size_t x, size_t y)
		{
			unsigned char *D = _getRowPtr(y);
			if(D)
			{
				if (x >= 0 && x < m_w)
				{
					switch (m_mode_int)
					{
					case Image::MODE_BW:
						{
							int shift = x&7;
							x>>=3;
							int mask = 1<<(7-shift);
							unsigned char *pixeldest = D + x;
							if (*pixeldest & mask) return 0xffffffff;
							return 0xff000000;
						};break;

					case Image::MODE_ARGB:
						{
							unsigned int *pixeldest = ((unsigned int *)(D)) + x;
							return *pixeldest;
						};break;
					};
				};
			};
			return 0;
		};

		o3_fun void rect(int x, int y, int w, int h, int signed_color)    // !ALLMODES!
		{
			unsigned int color = (unsigned int) signed_color;
			switch (m_mode_int)
			{
			case Image::MODE_ARGB:
				{
					int x1 = __max(0,x);
					int x2 = __min(x+w, (int)m_w);
					int actualw = x2-x1;
					if (actualw <= 0 ) return;
					int y1 = __max(0, y);
					int y2 = __min(y+h, (int)m_h);
					for (int sy = y1;sy<y2;sy++)
					{
						unsigned char *S = _getRowPtr(sy);
						m_mem.set<unsigned int>((int)(S-(unsigned char*)m_mem.ptr())+x1*sizeof(unsigned int), color, actualw*sizeof(unsigned int));
					};
				};
				break;
			default:
				for (int sy = y;sy<y+h;sy++)
				{
					for (int sx = x;sx<x+w;sx++)
					{
						setPixel(sx,sy,color);
					};
				};
			}
		};

		o3_fun void line(int x0,int y0,int x1,int y1,int signed_color)    // !ALLMODES!
		{
			unsigned int color = (unsigned int) signed_color;
			bool steep = (abs(y1 - y0) > abs(x1 - x0));
			if (steep)
			{			 
				swap(x0, y0);
				swap(x1, y1);
			}
			if (x0 > x1)
			{
				swap(x0, x1);
				swap(y0, y1);
			}
			int deltax = x1 - x0;
			int deltay = abs(y1 - y0);
			int error = deltax / 2;
			int ystep;
			int y = y0;
			if (y0 < y1) 
			{
				ystep = 1;
			}
			else 
			{
				ystep = -1;
			};

			for (int x=x0;x<x1;x++)
			{
				if (steep)
				{
					setPixel(y,x, color);
				}
				else 
				{
					setPixel(x,y, color);
				}
				error = error - deltay;
				if( error < 0) 
				{
					y = y + ystep;
					error = error + deltax;
				}

			}
		};
#pragma endregion ImageAPI_and_iImage
#pragma region PNG_load_and_save

		o3_set Buf src(const Buf &data, siEx *ex = 0)
		{
			using namespace png;			
			// create read struct
			png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);

			// check pointer
			if (png_ptr == 0)
			{
				cEx::fmt(ex,"Creating PNG read struct failed."); 
				return data;
			}

			// create info struct
			png_infop info_ptr = png_create_info_struct(png_ptr);

			// check pointer
			if (info_ptr == 0)
			{
				png_destroy_read_struct(&png_ptr, 0, 0);
				cEx::fmt(ex,"Creating PNG info struct failed."); 
				return data;
			}

			// set error handling
			if (setjmp(png_jmpbuf(png_ptr)))
			{
				png_destroy_read_struct(&png_ptr, &info_ptr, 0);
				cEx::fmt(ex,"Setting up PNG error handling failed."); 				
				return data;
			}

			// I/O initialization using custom o3 methods
			
			cBufStream stream(*(Buf*)(&data));

			png_set_read_fn(png_ptr,(void*) &stream, (png_rw_ptr) &o3_read_data_bufstream);

			// read entire image (high level)
			png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_EXPAND, 0);

			// convert the png bytes to BGRA			
			int W = png_get_image_width(png_ptr, info_ptr);
			int H = png_get_image_height(png_ptr, info_ptr);

			// get color information
			int color_type = png_get_color_type(png_ptr, info_ptr);

			png_bytep* row_pointers = png_get_rows(png_ptr, info_ptr);

			switch(color_type)
			{ 
			case PNG_COLOR_TYPE_RGB:
				{
					SetupMode(W,H, "argb");

					//						int pos = 0;

					// get color values
					for(int i = 0; i < (int) m_h; i++)
					{
						unsigned char *D = getrowptr(i);
						for(int j = 0; j < (int)(3 * m_w); j += 3)
						{
							*D++ = row_pointers[i][j + 2];	// blue
							*D++ = row_pointers[i][j + 1];	// green
							*D++ = row_pointers[i][j];		// red
							*D++ = 0xff;						// alpha
						}
					}

				};break;
			case PNG_COLOR_TYPE_RGB_ALPHA:
				{
					SetupMode(W,H, "argb");
				};break;

			case PNG_COLOR_TYPE_GRAY:
				{
					SetupMode(W,H, "gray");
				};break;
			case PNG_COLOR_TYPE_GRAY_ALPHA:
				{
					SetupMode(W,H, "argb");
				};break;
				break;

			default:
				png_destroy_read_struct(&png_ptr, &info_ptr, 0);
				cEx::fmt(ex,"PNG unsupported color type.");
				return data;
			}

			png_destroy_read_struct(&png_ptr, &info_ptr, 0);			
			return data;
		};

		o3_set siFs src(iFs* file, siEx* ex=0)
		{
			using namespace png;			

			// unable to open
			if (!file || !file->exists()) 
			{
				cEx::fmt(ex,"Invalid file."); 
				return file;
			}

			siStream stream = file->open("r", ex);
			if (!stream)				
				return file;

			// create read struct
			png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);

			// check pointer
			if (png_ptr == 0)
			{
				cEx::fmt(ex,"Creating PNG read struct failed."); 
				return file;
			}

			// create info struct
			png_infop info_ptr = png_create_info_struct(png_ptr);

			// check pointer
			if (info_ptr == 0)
			{
				png_destroy_read_struct(&png_ptr, 0, 0);
				cEx::fmt(ex,"Creating PNG info struct failed."); 
				return file;
			}

			// set error handling
			if (setjmp(png_jmpbuf(png_ptr)))
			{
				png_destroy_read_struct(&png_ptr, &info_ptr, 0);
				cEx::fmt(ex,"Setting up PNG error handling failed."); 				
				return file;
			}

			// I/O initialization using custom o3 methods
			png_set_read_fn(png_ptr,(void*) stream.ptr(), (png_rw_ptr) &o3_read_data);

			// read entire image (high level)
			png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_EXPAND, 0);

			// convert the png bytes to BGRA			
			int W = png_get_image_width(png_ptr, info_ptr);
			int H = png_get_image_height(png_ptr, info_ptr);

			// get color information
			int color_type = png_get_color_type(png_ptr, info_ptr);

			png_bytep* row_pointers = png_get_rows(png_ptr, info_ptr);

			switch(color_type)
			{ 
			case PNG_COLOR_TYPE_RGB:
				{
					SetupMode(W,H, "argb");

					//						int pos = 0;

					// get color values
					for(int i = 0; i < (int) m_h; i++)
					{
						unsigned char *D = getrowptr(i);
						for(int j = 0; j < (int)(3 * m_w); j += 3)
						{
							*D++ = row_pointers[i][j + 2];	// blue
							*D++ = row_pointers[i][j + 1];	// green
							*D++ = row_pointers[i][j];		// red
							*D++ = 0xff;						// alpha
						}
					}

				};break;
			case PNG_COLOR_TYPE_RGB_ALPHA:
				{
					SetupMode(W,H, "argb");
				};break;

			case PNG_COLOR_TYPE_GRAY:
				{
					SetupMode(W,H, "gray");
				};break;
			case PNG_COLOR_TYPE_GRAY_ALPHA:
				{
					SetupMode(W,H, "argb");
				};break;
				break;

			default:
				png_destroy_read_struct(&png_ptr, &info_ptr, 0);
				cEx::fmt(ex,"PNG unsupported color type.");
				return file;
			}

			png_destroy_read_struct(&png_ptr, &info_ptr, 0);			
			return file;
		};


		o3_fun int savePng(iFs* file, siEx* ex = 0)
		{
			using namespace png;
			png_structp png_ptr;
			png_infop info_ptr;

			if (m_w==0 ||m_h == 0)
			{
				cEx::fmt(ex,"[write_png_file] image must have both width and height >0 before something can be written!");			
				return 0;
			}

			/* create file */
			if (!file) 
			{
				cEx::fmt(ex,"Invalid file."); 
				return 0;
			}

			siStream stream = file->open("w", ex);
			if (!stream)				
				return 0;

			/* initialize stuff */
			png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

			if (!png_ptr)
			{
				cEx::fmt(ex,"[write_png_file] png_create_write_struct failed");
				return 0;
			}

			info_ptr = png_create_info_struct(png_ptr);

			if (!info_ptr)
			{
				cEx::fmt(ex,"[write_png_file] png_create_info_struct failed");
				return 0; 
			}

			if (setjmp(png_jmpbuf(png_ptr)))
			{
				cEx::fmt(ex,"[write_png_file] Error during init_io");
				return 0;
			}

			png_set_write_fn(png_ptr,(void*) stream.ptr(), 
				(png_rw_ptr) &o3_write_data, (png_flush_ptr) &o3_flush_data);


			/* write header */
			if (setjmp(png_jmpbuf(png_ptr)))
			{
				cEx::fmt(ex,"[write_png_file] Error during writing header");
				return 0;
			};

			int color_type = 0;
			int bitdepth = 8;
			switch (m_mode_int )
			{

			case Image::MODE_BW: 
				color_type = PNG_COLOR_TYPE_GRAY; 
				bitdepth = 1;
				break;
			case Image::MODE_GRAY: 
				color_type = PNG_COLOR_TYPE_GRAY; 
				break;
			default: 
				color_type = PNG_COLOR_TYPE_RGB_ALPHA;
				break;
			}
			// TODO! add 1bit save

			png_set_IHDR(png_ptr, info_ptr, m_w, m_h,
				bitdepth, color_type, PNG_INTERLACE_NONE,
				PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

			png_write_info(png_ptr, info_ptr);

			if (setjmp(png_jmpbuf(png_ptr)))
			{
				png_destroy_write_struct(&png_ptr,0);
				cEx::fmt(ex,"[write_png_file] Error during writing bytes");
				return 0;
			};

			tVec<png_bytep> row_pointers(m_h);
			switch (m_mode_int)
			{
			case Image::MODE_ARGB:
				{
					tVec <unsigned int> row(m_w);
					for (size_t y = 0;y<m_h;y++)
					{
						unsigned int *D = (unsigned int *)_getRowPtr(y);
						for (size_t i =0 ;i<m_w;i++)
						{
							unsigned int const pixel = *D++;
							unsigned int shuffled = ((pixel >> 24)&0x255) + ((pixel << 8)&0xffffff00);
							row[i] = shuffled;

							unsigned char *c = (unsigned char*)&row[i];
							c[0] = (unsigned char)(pixel>>16);
							c[1] = (unsigned char)(pixel>>8);
							c[2] = (unsigned char)(pixel);
							c[3] = (unsigned char)(pixel>>24);
						}
						png_write_row(png_ptr, (png_bytep)row.ptr());
					};
				}
				break;
			case Image::MODE_RGB:
				{
					tVec <unsigned int> row(m_w);
					for (size_t y = 0;y<m_h;y++)
					{
						unsigned char *D = (unsigned char *)_getRowPtr(y);
						for (size_t i =0 ;i<m_w;i++)
						{
							unsigned char R = *D++;
							unsigned char G = *D++;
							unsigned char B = *D++;
							unsigned int const pixel = (R << 24) + (G << 16) + (B << 8) + 0xff ;
							row[i] = pixel;
						}
						png_write_row(png_ptr, (png_bytep)row.ptr());
					};
				}
				break;
			case Image::MODE_GRAY:
				{
					tVec <unsigned char> row(m_w);
					for (size_t y = 0;y<m_h;y++)
					{
						unsigned char *D = (unsigned char *)_getRowPtr(y);
						png_write_row(png_ptr, D);
					};
				}
				break;
			case Image::MODE_BW:
				{
					tVec <unsigned int> row(m_w);
					for (size_t y = 0;y<m_h;y++)
					{
						unsigned char *D = (unsigned char *)_getRowPtr(y);
						png_write_row(png_ptr, D);
					};
				}
				break;

			};

			//png_write_image(png_ptr, row_pointers.ptr());


			if (setjmp(png_jmpbuf(png_ptr)))
			{
				png_destroy_write_struct(&png_ptr,&info_ptr);
				cEx::fmt(ex,"[write_png_file] Error during end of write");
				return 0;
			};

			png_write_end(png_ptr, NULL);
			png_destroy_write_struct(&png_ptr,&info_ptr);

			/* cleanup heap allocation */

			return 1;
		};

#pragma endregion PNG_load_and_save



        // -------------------------------------------------------
        // 
        // W3C Canvas API
        //
        // -------------------------------------------------------
#pragma region CSS_HELPERS

		o3_fun int decodeColor(const Str &style)
		{
			return o3::decodeColor(style);
        };

		enum
		{
			CSSUnit_pixel,
			CSSUnit_percentage,
			CSSUnit_inch,
			CSSUnit_cm,
			CSSUnit_mm,
			CSSUnit_em, // 1em = current fontsize
			CSSUnit_ex, // 1ex = the x-height of a font (x-height is usually about half the font-size)
			CSSUnit_point, // 1pt = 1/72 inch
			CSSUnit_pica // 1pc = 12pt = 12/72 inch
		};
		
		double GetCurrentFontHeight()
		{
			// TODO! read renderstate
			return 10.0;
		};

		double CSSUnitToPixel(double inamount, int unittype, double DPI = 72.0)
		{

			switch (unittype)
			{
				case CSSUnit_pixel: return inamount;
				case CSSUnit_percentage: return (inamount*GetCurrentFontHeight()*0.01); 
				case CSSUnit_inch: return inamount * DPI;
				case CSSUnit_cm: return inamount * (DPI/2.54);
				case CSSUnit_mm: return inamount * (DPI/25.4);

				case CSSUnit_em: return GetCurrentFontHeight() * inamount;
				case CSSUnit_ex: return GetCurrentFontHeight()/2 * inamount;
				case CSSUnit_point: return inamount * (DPI/72.0);
				case CSSUnit_pica: return inamount * ((12*DPI)/72.0);
			};
			return inamount;
		};
#pragma endregion CSS_HELPERS


#pragma region CanvasProperties

#pragma region CanvasEnums
		// enums are ordered so that the top enum is the default value.
		enum
		{
			// [start], end, left, right, center
			TextAlign_start = 0,
			TextAlign_end,
			TextAlign_left,
			TextAlign_right,
			TextAlign_center
		};

		enum
		{
			 // [normal], italic, oblique
			FontStyle_normal = 0,
			FontStyle_italic,
			FontStyle_oblique
		};

		enum
		{
			// top, hanging, middle, [alphabetic], ideographic, bottom 
			TextBaseline_alphabetic = 0,
			TextBaseline_top,
			TextBaseline_hanging,
			TextBaseline_middle,
			TextBaseline_ideographic,
			TextBaseline_bottom
		};
		
		enum
		{
			// [LTR], RTL 
			TextDirectionality_ltr = 0,
			TextDirectionality_rtl 
		};
		
		enum
		{
			FontSize_normal = 0, // default 10pt!
			FontSize_xx_small,
			FontSize_x_small,
			FontSize_small,
			FontSize_medium,
			FontSize_large,
			FontSize_x_large,
			FontSize_xx_large,
			FontSize_smaller,
			FontSize_larger,
			FontSize_absolute_size, // number is an absolute unit to be converted
			FontSize_relative_size // relative to 10pt
		};


		enum
		{
			FontVariant_normal = 0,
			FontVariant_small_caps
		};

		enum
		{
			FontWeight_normal = 0,
			FontWeight_bold,
			FontWeight_bolder,
			FontWeight_lighter,
			FontWeight_100,
			FontWeight_200,
			FontWeight_300,
			FontWeight_400,
			FontWeight_500,
			FontWeight_600,
			FontWeight_700,
			FontWeight_800,
			FontWeight_900
		};



#pragma endregion CanvasEnums
		o3_set void fillStyle(const Str &style)
		{
			m_currentrenderstate->FillColor = decodeColor(style);

			unsigned int color =  m_currentrenderstate->FillColor;
			unsigned char *c = (unsigned char *)&color;
			m_graphics.fillColor(c[2], c[1], c[0], c[3]);
		};

		o3_set void strokeStyle(const Str &style)
		{
			m_currentrenderstate->StrokeColor = decodeColor(style);

			unsigned int color =  m_currentrenderstate->StrokeColor;
			unsigned char *c = (unsigned char *)&color;
			m_graphics.lineColor(c[2], c[1], c[0], c[3]);
		};

		o3_set void strokeWidth (double Width)
		{
			m_currentrenderstate->StrokeWidth = Width;
		};

		o3_set void lineCap(const Str &cap)
		{
			cap;
		};

		o3_set void lineJoin(const Str &join)
		{
			join;
		};

		o3_set void miterLimit(double limit)
		{
			limit;
		};

#pragma region TextProperties

		/*
		// css 1.0 based font properties:
			Str FontFamily; // Serif, [Sans-serif], Monospace, fontfilenames
			Str FontSize; // xx-small x-small small medium large x-large xx-large smaller larger length %
			Str FontStyle; // [normal] italic oblique
			Str FontVariant; // [normal] small-caps
			Str FontWeight; // [normal] bold bolder lighter 100 200 300 400 500 600 700 800 900			
			Str TextDirectionality; // [LTR], RTL -> needs to be dealt with because of align = start/end
			Str TextBaseline; // top hanging middle [alphabetic] ideographic bottom
		*/

		o3_set void fontFamily(const Str &fontstring)
		{
			fontstring;
			
		};

		o3_set void fontSize(const Str &fontstring)
		{
			fontstring;	
		};

		o3_set void fontStyle(const Str &fontstring)
		{
			fontstring;	
		};

		o3_set void fontVariant(const Str &fontstring)
		{
			fontstring;	
		};

		o3_set void fontWeight(const Str &fontstring)
		{
			fontstring;	
		};

		o3_set void textDirectionality(const Str &fontstring)// [LTR] RTL
		{
			fontstring;	
		};

		o3_set void textAlign(const Str &newAlign) // ["start"], "end", "left", "right", "center"
		{
			newAlign;	
		};
		
		o3_set void textBaseline(const Str &newBaseline) // "top", "hanging", "middle", ["alphabetic"], "ideographic", "bottom"
		{
			newBaseline;	
		};

#pragma endregion TextProperties

#pragma endregion CanvasProperties
	

#pragma region CanvasFunctions

#pragma region TextFunctions
		
		o3_fun void fillText(const Str & text, double x, double y)
		{
		};
		
		o3_fun void fillText(const Str & text, double x, double y, double maxWidth)
		{
		};

		o3_fun void strokeText(const Str & text, double x, double y)
		{
		};

		o3_fun void strokeText(const Str & text, double x, double y, double maxWidth)
		{
		};

		o3_fun siScr measureText(const Str & text) //cImage_TextMetrics 
		{
		};

#pragma endregion TextFunctions

		o3_fun void clearRect(double xx, double yy, double ww, double hh)
		{
			Ensure32BitSurface();
			unsigned int color =  m_currentrenderstate->ClearColor;
			unsigned char *c = (unsigned char *)&color;

			m_graphics.resetPath();

			V2<double> p1(xx,yy);
			V2<double> p2(xx+ww,yy);
			V2<double> p3(xx+ww,yy+hh);
			V2<double> p4(xx,yy+hh);

			p1  = m_currentrenderstate->Transformation.Multiply(p1);
			p2  = m_currentrenderstate->Transformation.Multiply(p2);
			p3  = m_currentrenderstate->Transformation.Multiply(p3);
			p4  = m_currentrenderstate->Transformation.Multiply(p4);

			m_graphics.moveTo(p1.x,p1.y);
			m_graphics.lineTo(p2.x,p2.y);
			m_graphics.lineTo(p3.x,p3.y);
			m_graphics.lineTo(p4.x,p4.y);

			m_graphics.closePolygon();
			m_graphics.fillColor(c[2], c[1], c[0], c[3]);
			m_graphics.drawPath(agg::Agg2D::FillOnly);
			{

				unsigned int color =  m_currentrenderstate->FillColor;
				unsigned char *c = (unsigned char *)&color;

				m_graphics.fillColor(c[2], c[1], c[0], c[3]);
			};
		};

		o3_fun void fillRect(double xx, double yy, double ww, double hh)
		{
			Ensure32BitSurface();

			m_graphics.resetPath();

			V2<double> p1(xx,yy);
			V2<double> p2(xx+ww,yy);
			V2<double> p3(xx+ww,yy+hh);
			V2<double> p4(xx,yy+hh);

			p1  = m_currentrenderstate->Transformation.Multiply(p1);
			p2  = m_currentrenderstate->Transformation.Multiply(p2);
			p3  = m_currentrenderstate->Transformation.Multiply(p3);
			p4  = m_currentrenderstate->Transformation.Multiply(p4);

			m_graphics.moveTo(p1.x,p1.y);
			m_graphics.lineTo(p2.x,p2.y);
			m_graphics.lineTo(p3.x,p3.y);
			m_graphics.lineTo(p4.x,p4.y);
			m_graphics.closePolygon();

			m_graphics.drawPath(agg::Agg2D::FillOnly);
		};

		o3_fun void strokeRect(double xx, double yy, double ww, double hh)
		{
			Ensure32BitSurface();

			m_graphics.resetPath();

			V2<double> p1(xx,yy);
			V2<double> p2(xx+ww,yy);
			V2<double> p3(xx+ww,yy+hh);
			V2<double> p4(xx,yy+hh);

			p1  = m_currentrenderstate->Transformation.Multiply(p1);
			p2  = m_currentrenderstate->Transformation.Multiply(p2);
			p3  = m_currentrenderstate->Transformation.Multiply(p3);
			p4  = m_currentrenderstate->Transformation.Multiply(p4);

			m_graphics.moveTo(p1.x,p1.y);
			m_graphics.lineTo(p2.x,p2.y);
			m_graphics.lineTo(p3.x,p3.y);
			m_graphics.lineTo(p4.x,p4.y);

			m_graphics.closePolygon();

			m_graphics.drawPath(agg::Agg2D::StrokeOnly);
		};


		
		
		o3_fun void closePath()
		{
			if (m_paths.size() == 0) return;
			if (m_paths[0].m_path.size()<2) return;

			V2<double> first;
			first.x = m_paths[m_paths.size()-1].m_path[0].x;
			first.y = m_paths[m_paths.size()-1].m_path[0].y;

			m_paths[m_paths.size()-1].m_path.push(first);
			m_lastpoint = first;
		}

		o3_fun void beginPath()
		{
			m_paths.clear();
		}


		o3_fun void fill()
		{
			Ensure32BitSurface();
			m_graphics.resetPath();

			//			TransformCurrentPath();

			for (size_t i =0 ;i<m_paths.size();i++)
			{
				if (m_paths[i].m_path.size()>1)
				{
					V2<double> Prev = m_paths[i].m_path[0];
					m_graphics.moveTo(Prev.x, Prev.y);
					for (size_t j = 1;j<m_paths[i].m_path.size();j++)
					{
						V2<double> Cur;
						Cur.x = m_paths[i].m_path[j].x;
						Cur.y = m_paths[i].m_path[j].y;
						m_graphics.lineTo(Cur.x, Cur.y);
						//						line(Prev.x, Prev.y, Cur.x, Cur.y, color);
						Prev.x = Cur.x;
						Prev.y = Cur.y;
					};
					m_graphics.closePolygon();
				};
			};
			m_graphics.drawPath(agg::Agg2D::FillOnly);
			//m_paths.clear();


		};

		o3_fun void stroke()
		{
			Ensure32BitSurface();
			m_graphics.resetPath();
			m_graphics.lineWidth(m_currentrenderstate->StrokeWidth);
			//			m_graphics.line(0,0,m_w, m_h);

			//			TransformCurrentPath();

			for (size_t i =0 ;i<m_paths.size();i++)
			{
				if (m_paths[i].m_path.size()>1)
				{
					V2<double> Prev = m_paths[i].m_path[0];
					m_graphics.moveTo(Prev.x, Prev.y);
					for (size_t j = 1;j<m_paths[i].m_path.size();j++)
					{
						V2<double> Cur;
						Cur.x = m_paths[i].m_path[j].x;
						Cur.y = m_paths[i].m_path[j].y;
						m_graphics.lineTo(Cur.x, Cur.y);
						//						line(Prev.x, Prev.y, Cur.x, Cur.y, color);
						Prev.x = Cur.x;
						Prev.y = Cur.y;
					};
				};
			};
			m_graphics.drawPath(agg::Agg2D::StrokeOnly);
			//			m_paths.clear();
		};

#pragma region Path_Generating_Functions
		o3_fun void moveTo(double x, double y)
		{
			m_paths.push(Path());
			V2<double> point(x,y);
			point = TransformPoint(point);
			m_paths[m_paths.size()-1].m_path.push(point);
			m_lastpoint = point;
		}

		o3_fun void lineTo(double x, double y)
		{
			if (m_paths.size() == 0)
			{
				m_paths.push(Path());
				m_paths[m_paths.size()-1].m_path.push(m_lastpoint);
			};
			V2<double> point(x,y);
			m_paths[m_paths.size()-1].m_path.push(TransformPoint(point));
			m_lastpoint.x = x;
			m_lastpoint.y = y;
		};


		o3_fun void arc(double x0, double y0, double radius, double startAngle, double endAngle, bool anticlockwise)
		{

			ArcGen Gen(x0,y0,radius,radius, startAngle, endAngle, (anticlockwise)?true:false);
			double x, y;

			if (m_paths.size() == 0)
			{
				m_paths.push(Path());
			};

			if (Gen.vertex(&x,&y) != agg::agg::path_cmd_stop)
			{
				moveTo(x,y);
			};

			while (Gen.vertex(&x,&y) != agg::agg::path_cmd_stop)
			{
				V2<double> point(x,y);
				m_paths[m_paths.size()-1].m_path.push(TransformPoint(point));
			}

			int lastpathsize = m_paths[m_paths.size()-1].m_path.size();
			if (lastpathsize >0)
			{
				m_lastpoint = m_paths[m_paths.size()-1].m_path[lastpathsize-1];
			}
			else
			{
				m_paths[m_paths.size()-1].m_path.push(m_lastpoint);
			}

		}

		
		o3_fun void quadraticCurveTo(double cp1x, double cp1y, double x0, double y0)
		{

			V2<double> target(x0,y0);
			V2<double> cp(cp1x,cp1y);

			target = TransformPoint(target);
			cp = TransformPoint(cp);
			QuadraticCurveGen Gen(m_lastpoint.x,m_lastpoint.y, cp.x,cp.y, target.x, target.y);
			double x, y;

			if (m_paths.size() == 0)
			{
				m_paths.push(Path());
				m_paths[m_paths.size()-1].m_path.push(m_lastpoint);
			};


			while (Gen.vertex(&x,&y) != agg::agg::path_cmd_stop)
			{
				V2<double> point(x,y);
				m_paths[m_paths.size()-1].m_path.push(point);
			}

			m_lastpoint = target;

		};

		o3_fun void bezierCurveTo(double cp1x, double cp1y, double cp2x, double cp2y, double x0, double y0)
		{
			V2<double> target(x0,y0);
			V2<double> cp1(cp1x,cp1y);
			V2<double> cp2(cp2x,cp2y);

			target = TransformPoint(target);
			cp1 = TransformPoint(cp1);
			cp2 = TransformPoint(cp2);

			BezierCurveGen Gen(m_lastpoint.x,m_lastpoint.y, cp1.x, cp1.y, cp2.x, cp2.y, target.x, target.y);
			double x, y;

			if (m_paths.size() == 0)
			{
				m_paths.push(Path());
				m_paths[m_paths.size()-1].m_path.push(m_lastpoint);
			};

			while (Gen.vertex(&x,&y) != agg::agg::path_cmd_stop)
			{
				V2<double> point(x,y);
				m_paths[m_paths.size()-1].m_path.push(TransformPoint(point));
			}

			m_lastpoint = target;
		}


#pragma endregion Path_Generating_Functions
#pragma region RenderState_Management
		void SetupRenderState()
		{
			RenderState RS;
			RS.ClipTopLeft.x = 0;
			RS.ClipTopLeft.y = 0;
			RS.ClipBottomRight.x = m_w;
			RS.ClipBottomRight.y = m_h;

			RS.ClearColor = 0xffffffff;
			RS.StrokeWidth = 1;
			RS.ClippingEnabled = false;

			m_renderstates.push(RS);
			m_currentrenderstate = &m_renderstates[m_renderstates.size()-1];

			strokeStyle("black");
			fillStyle("black");

		};

		void RestoreStateToGraphicsObject()
		{
			m_graphics.clipBox(m_currentrenderstate->ClipTopLeft.x,
				m_currentrenderstate->ClipTopLeft.y,
				m_currentrenderstate->ClipBottomRight.x,
				m_currentrenderstate->ClipBottomRight.y);

			unsigned char *sc = (unsigned char *)&m_currentrenderstate->StrokeColor;
			m_graphics.lineColor(sc[2], sc[1], sc[0], sc[3]);
			unsigned char *fc = (unsigned char *)&m_currentrenderstate->FillColor;
			m_graphics.fillColor(fc[2], fc[1], fc[0], fc[3]);
		};

		o3_fun void save()
		{
			//			RenderState *PreviousState = m_currentrenderstate;
			RenderState RS = *m_currentrenderstate;
			m_renderstates.push(RS);
			m_currentrenderstate = &m_renderstates[m_renderstates.size()-1];
			//			for (size_t i = 0;i<PreviousState->ClippingPaths.size();i++)
			//			{
			//				m_currentrenderstate->ClippingPaths.push(PreviousState->ClippingPaths[i]);
			//			}
		};

		o3_fun void restore()
		{
			if (m_renderstates.size()>1) 
			{
				m_renderstates.pop();
				m_currentrenderstate = &m_renderstates[m_renderstates.size()-1];
				RestoreStateToGraphicsObject();				

			};
		};

#pragma endregion RenderState_Management
#pragma region Transformation_Matrix_Related
		
		
		o3_fun void setTransform(double m11, double m12, double m21, double m22, double dx, double dy)
		{
			m_currentrenderstate->Transformation.M[0][0] = m11;
			m_currentrenderstate->Transformation.M[0][1] = m12;
			m_currentrenderstate->Transformation.M[1][0] = m21;
			m_currentrenderstate->Transformation.M[1][1] = m22;
			m_currentrenderstate->Transformation.M[2][0] = dx;
			m_currentrenderstate->Transformation.M[2][1] = dy;

			m_currentrenderstate->Transformation.M[0][2] = 0;
			m_currentrenderstate->Transformation.M[1][2] = 0;
			m_currentrenderstate->Transformation.M[2][2] = 1.0;
		};

		o3_fun void transform(double m11, double m12, double m21, double m22, double dx, double dy)
		{
			M33<double> trans;

			trans.M[0][0] = m11;
			trans.M[0][1] = m12;
			trans.M[1][0] = m21;
			trans.M[1][1] = m22;
			trans.M[2][0] = dx;
			trans.M[2][1] = dy;

			trans.M[0][2] = 0;
			trans.M[1][2] = 0;
			trans.M[2][2] = 1.0;

			m_currentrenderstate->Transformation = m_currentrenderstate->Transformation.Multiply(trans);
		};
		o3_fun void translate(double _x, double _y)
		{
			M33<double> TransMat;
			TransMat.setTranslation(_x, _y);
			m_currentrenderstate->Transformation = m_currentrenderstate->Transformation.Multiply(TransMat);
		};

		o3_fun void rotate(double _angle)
		{
			M33<double> RotMat;
			RotMat.setRotation(_angle);//(_angle*pi*2.0f)/360.0f);
			m_currentrenderstate->Transformation = m_currentrenderstate->Transformation.Multiply(RotMat);
		};

		o3_fun void scale(double xscale, double yscale)
		{
			M33<double> ScaleMat;
			ScaleMat.setScale(xscale, yscale);
			m_currentrenderstate->Transformation = m_currentrenderstate->Transformation.Multiply(ScaleMat);
		};


		V2<double> TransformPoint(V2<double> &p)
		{
			return m_currentrenderstate->Transformation.Multiply(p);
		}

#pragma endregion Transformation_Matrix_Related



		
		o3_fun void clip()
		{
			double x2=0,y2=0,x1=m_w,y1=m_h;
			// calculate extends, set 2d clipping rect for now


			if (m_paths.size() == 0)
			{
				m_currentrenderstate->ClippingEnabled = false;
			}
			else
			{
				m_currentrenderstate->ClippingEnabled = true;
#ifdef IMAGE_ALPHAMAP_ENABLED
				AttachAlpha();
				m_graphics.EnableAlphaMask( true ) ;


				agg::agg::rasterizer_scanline_aa<> m_ras;

				typedef agg::agg::renderer_base<agg::agg::pixfmt_gray8> ren_base;
				typedef agg::agg::renderer_scanline_aa_solid<ren_base> renderer;

				agg::agg::pixfmt_gray8 pixf(*m_graphics.GetAlphaBuffer());
				ren_base rb(pixf);
				renderer ren(rb);
				agg::agg::scanline_p8 m_sl;

				agg::agg::path_storage     path;

#endif

				for (size_t i = 0 ;i<m_paths.size();i++)
				{
					size_t pathlen = m_paths[i].m_path.size();
					if (pathlen >1)
					{
						{
							V2<double> &p = m_paths[i].m_path[0];
							x1 = __min(p.x, x1);
							x2 = __max(p.x, x2);
							y1 = __min(p.y, y1);
							y2 = __max(p.y, y2);
#ifdef IMAGE_ALPHAMAP_ENABLED
							path.move_to(p.x,p.y);
#endif
						}

						for (size_t j = 0 ;j <pathlen ;j++)
						{
							V2<double> &p = m_paths[i].m_path[j];
							x1 = __min(p.x, x1);
							x2 = __max(p.x, x2);
							y1 = __min(p.y, y1);
							y2 = __max(p.y, y2);
#ifdef IMAGE_ALPHAMAP_ENABLED
							path.line_to(p.x,p.y);
#endif
						}
#ifdef IMAGE_ALPHAMAP_ENABLED
						path.close_polygon();
#endif

					};
				};
#ifdef IMAGE_ALPHAMAP_ENABLED
				m_ras.add_path(path);
				ren.color(agg::agg::gray8(255));
				agg::agg::render_scanlines(m_ras, m_sl, ren);
#endif
			};
			m_currentrenderstate->ClipBottomRight.x = x2;
			m_currentrenderstate->ClipTopLeft.x = x1;
			m_currentrenderstate->ClipBottomRight.y = y2;
			m_currentrenderstate->ClipTopLeft.y = y1;
			m_graphics.clipBox(x1,y1, x2,y2);
		}

#pragma endregion CanvasFunctions
	};

	void CopyAlphaMaskToVisible()
	{

	};
};

#endif 
