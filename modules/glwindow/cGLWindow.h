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
#ifndef O3_C_GLWINDOW_H
#define O3_C_GLWINDOW_H

#include "Glee-5.4/GLee.h"
#include "Glee-5.4/GLee.c"
#ifdef WIN32
#pragma comment(lib,"opengl32.lib");
#pragma comment(lib,"glu32.lib");
#endif
#include <gl/glu.h>

#include "canvas/canvas.h"

namespace o3 
{
	
	
	o3_iid(iTexture, 0x1d657e65, 0xf75a, 0x48f0, 0x8e, 0x67, 0xab, 0xee, 0x5, 0xc2, 0x8b, 0x43);

	struct iTexture: iUnk 
	{
		virtual void upload(iScr* target, size_t width, size_t height) = 0;
		virtual void bind() = 0;
		virtual void unbind() = 0;
	};


	struct cGLTexture: cScr, iTexture
	{
		cGLTexture(int w, int h)
		{
			if (w<1 || h<1) return;
			GLeeInit();
			glGenTextures(1, &mTextureID);
			mW = w;
			mH = h;
			mActualW = 1;
			mActualH = 1;
			while (mActualW < mW)  mActualW <<= 1;
			while (mActualH < mH)  mActualH <<= 1;
			bind();
			glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR);
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
			mLocalData = new unsigned char[mActualW*mActualH*4];

			unsigned char *blankimage = mLocalData;
			for (int i= 0;i<mActualW*mActualH;i++)
			{
				int x = (i/32);
				int y = (x/32)/32;
				x%= (1024/32);
				unsigned char check = ((x+y)%2 ==1)?0xff:0;
				blankimage[i*4+0] = check;
				blankimage[i*4+1] = check;
				blankimage[i*4+2] = check;
				blankimage[i*4+3] = 0xff;
			};

			glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA, mActualW,mActualH, 0,GL_RGBA, GL_UNSIGNED_BYTE, blankimage);
		//	delete [] blankimage;
		};
		unsigned char *mLocalData ;

		~cGLTexture()
		{
			if (mLocalData) delete [] mLocalData;
			glDeleteTextures(1, &mTextureID);
		};
		static o3_ext("cGLWindow") o3_fun siTexture texture(size_t width, size_t height)
		{
			cGLTexture* ret = o3_new(cGLTexture)(width, height);
			return ret;
		};

		int mW, mH, mActualW, mActualH;

		o3_begin_class(cScr);
		o3_add_iface(iTexture);
		o3_end_class();

		o3_glue_gen();

		unsigned int mColorMode;
		unsigned int mTextureID;
		
		virtual o3_fun void bind()
		{
			glBindTexture(GL_TEXTURE_2D, mTextureID);
		};

		virtual o3_fun void unbind()
		{
			glBindTexture(GL_TEXTURE_2D, 0);
		};

		virtual o3_fun void upload(iScr* source, size_t width, size_t height)
		{
			tSi<iImage> Image(source);
			
			if (Image)
			{
				for (int y =0;y<__min(__min(mActualH, height), Image->height());y++)
				{
					unsigned char *P = Image->getrowptr(y);
					unsigned char *localp = mLocalData + (y*mActualW)*4;
					for (int x = 0;x<__min(__min(mActualW, width),Image->width());x++)
					{
						localp[0] = P[2];
						localp[1] =  P[1];
						localp[2] =  P[0];
						localp[3] =  P[3];

						P+=4;
						localp+=4;
					};
				};
			};
			bind();
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0,0,mActualW,__min(height, mActualH), GL_RGBA, GL_UNSIGNED_BYTE, mLocalData);
		};
	};

	struct cGLWindow : cWindow
	{
		enum ButtonType {
			TYPE_PUSH,
			TYPE_RADIO 
		};

		cGLWindow()
		{
			m_def_proc = NULL;
			mGLContext = NULL;
			mCurrentDC = NULL;
		}

		virtual ~cGLWindow(){}

		o3_begin_class(cWindow)
			o3_add_iface(iWindow)
			o3_add_iface(iWindowProc)
		o3_end_class()

		o3_glue_gen()

		WNDPROC         m_def_proc;
		HGLRC mGLContext ;
		HDC mCurrentDC;

		o3_enum("GLCONSTANTS", 

			GL_COLOR_BUFFER_BIT  = GL_COLOR_BUFFER_BIT,
			GL_DEPTH_BUFFER_BIT  = GL_DEPTH_BUFFER_BIT,
			GL_TEXTURE_2D = GL_TEXTURE_2D,
			GL_BLEND  = GL_BLEND,
			GL_ALPHA_TEST  = GL_ALPHA_TEST,
			GL_RGB  = GL_RGB,
			GL_RGBA =GL_RGBA,
			GL_MODELVIEW =GL_MODELVIEW,
			GL_PROJECTION = GL_PROJECTION,
			GL_TEXTURE = GL_TEXTURE,
			GL_POINTS = GL_POINTS,
			GL_LINES = GL_LINES,
			GL_QUADS = GL_QUADS,
			GL_QUAD_STRIP = GL_QUAD_STRIP,
			GL_TRIANGLES = GL_TRIANGLES,
			GL_TRIANGLE_STRIP = GL_TRIANGLE_STRIP
			);

		o3_fun void BeginFrame()
		{
			if (mCurrentDC == NULL)
			{
				mCurrentDC  = GetDC(m_hwnd);
				wglMakeCurrent ( mCurrentDC , mGLContext );
			}


		};

		o3_fun void EndFrame()
		{
			if (mCurrentDC != NULL)
			{
				ReleaseDC(m_hwnd, mCurrentDC );
				mCurrentDC  = NULL;
			}
			invalidate();
		};

		o3_fun void invalidate()
		{
			InvalidateRect(m_hwnd,NULL, false);
		};

		o3_fun void ClearColor(double r,double g, double b, double a)
		{
			glClearColor(r,g,b,a);
		};

		o3_fun void Clear(size_t bits)
		{
			glClear(bits);
		};

		o3_fun void Enable(size_t bits)
		{
			glEnable(bits);
		};

		o3_fun void Disable(size_t bits)
		{
			glDisable(bits);
		};

		o3_fun void PushMatrix()
		{
			glPushMatrix();
		};

		o3_fun void LoadIdentity()
		{
			glLoadIdentity();
		};

		o3_fun void PopMatrix()
		{
			glPopMatrix();
		};

		o3_fun void Rotate(double angle, double x, double y, double z)
		{
			glRotated(angle, x,y,z);
		};

		o3_fun void PointSize(double ps)
		{
			glPointSize(ps);
		};

		o3_fun void LineWidth(double lw)
		{
			glLineWidth(lw);
		};

		o3_fun void Vertex3(double x, double y, double z)
		{
			glVertex3d(x,y,z);
		};

		o3_fun void TexCoord2(double x, double y)
		{
			glTexCoord2d(x,y);
		};

		o3_fun void Begin(size_t mode)
		{
			glBegin(mode);
		};

		o3_fun void End()
		{
			glEnd();
		};

		o3_fun void Vertex2(double x, double y, double z)
		{
			glVertex2d(x,y);
		};

		o3_fun void Color3(double r, double g, double b)
		{
			glColor3d(r,g,b);
		};

		o3_fun void Color4(double r, double g, double b, double a)
		{
			glColor4d(r,g,b,a);
		}

		o3_fun void Translate(double x, double y, double z)
		{
			glTranslated(x,y,z);
		};

		o3_fun void MatrixMode(size_t Mode)
		{
			glMatrixMode(Mode);
		};

		o3_fun void Perspective(double fov, double aspect, double nearplane, double farplane)
		{
			gluPerspective(fov, aspect,nearplane, farplane);
		};

		o3_fun void LookAt(double xeye, double yeye, double zeye, double xtarget, double ytarget, double ztarget, double xup, double yup, double zup)
		{
			gluLookAt(xeye,yeye, zeye, xtarget, ytarget, ztarget, xup, yup, zup);
		};

		o3_fun void ColorMask(bool r, bool g, bool b, bool a)
		{
			glColorMask(r,g,b,a);
		};

		static o3_ext("cO3") o3_fun siWindow createGLWindow(o3_tgt iScr* target, const char* text, 
			int x, int y, int w, int h)
		{              
			o3_trace_scrfun("createGLWindow");              
			target = 0;
			return create(0,text,x,y,w,h);
		}

		static siWindow create(HWND parent, const char* caption, int x, int y, 
			int width, int height, int style = 0)
		{
			o3_trace_scrfun("create");
			// create the component
			cGLWindow* ret = o3_new(cGLWindow)();

			WNDCLASSW wnd_class;        
			regWndClass(wnd_class);

			// convert o3 style flags to native flags
			DWORD flags = getFlags(style);
			if (parent)
				flags |= WS_CHILD;

			// create the object and the window
			ret->m_hwnd = CreateWindowExW(0,o3_wnd_class_name, 
				WStr(caption).ptr(), flags, x, y, width, height, 
				parent, 0, GetModuleHandle(0), (LPVOID)(iWindowProc*)ret);  

			SetWindowLongPtr( ret->m_hwnd, GWL_USERDATA, (LONG_PTR)(iWindowProc*)ret );

			PIXELFORMATDESCRIPTOR pfd;  
			pfd.cColorBits = pfd.cDepthBits = 32; 
			pfd.dwFlags    = PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;	


			HDC hDC = GetDC (  ret->m_hwnd);     
			int PixelFormat = ChoosePixelFormat ( hDC, &pfd) ;
			SetPixelFormat ( hDC, PixelFormat , &pfd );
			ret->mGLContext = wglCreateContext(hDC);
			wglMakeCurrent ( hDC, ret->mGLContext );
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			GLeeInit();
			ReleaseDC(ret->m_hwnd, hDC);

			return ret;
		}

		virtual LRESULT CALLBACK wndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
		{
			o3_trace_scrfun("wndProc");
			siCtx ctx(m_ctx);
			switch(message)
			{
			case WM_ERASEBKGND: return FALSE;
			case WM_PAINT:
				{
					PAINTSTRUCT ps;
					HDC dc = BeginPaint(hwnd, &ps);
					wglMakeCurrent ( dc, mGLContext );
					SwapBuffers(dc);
					EndPaint(hwnd, &ps);
				};
				break;
			default:
				if (m_def_proc)	return m_def_proc(hwnd, message, wparam, lparam);
				return cWindow::wndProc(hwnd, message, wparam, lparam);
				//return(DefWindowProc(hwnd, message, wparam, lparam));
			}
			return 0;
		}

	};

}

#endif 