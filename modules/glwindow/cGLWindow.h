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

namespace o3 {

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