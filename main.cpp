// V3DC-Z99W3X3N

//#include "back.cpp"

#include <windows.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include <math.h>
#include <stdio.h>

#define APPNAME "Shooter"
#define WIDTH 1024
#define HEIGHT 768
#define FOV 90

#define ASPECTRATIO (float)WIDTH/(float)HEIGHT
#define DRR 0.0174532925199432957692362

struct v3 { float x,y,z; };
struct obj { float m; v3 r,dr,p,dp; };

HDC		hDC=NULL;
HGLRC	hRC=NULL;
HWND	hWnd=NULL;

GLuint textures[8];
GLuint DLbase;
GLuint DLfontbase;

char str[100] = "";

BOOL kbd[5];

unsigned int mouseX=1, mouseY=1, mouseOldX, mouseOldY;
float sensitivity = 0.1;

int jump = 0;

__int64 pcf,t1,t2; float dt; // TIME


v3 f = { .0, .0, .0 };
v3 g = { .0, -9.8, .0 };

GLfloat lpos[4] = { 15, 15, 15, 1 };

obj camera = { 65.0, { .0, .0, .0 }, { .0, .0, .0 }, { .0, 4.0, 4.0 }, { .0, .0, .0 } };

BOOL createWindow();
void killWindow();
void initScene();
void killScene();
void render();
unsigned int loadTexture( const char *, BOOL );

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int iCmdShow);
LRESULT CALLBACK WndProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

void force( obj o, v3 fo ){ // v += (f/m)*t
	o.dp.x += (fo.x/o.m)*dt;
	o.dp.y += (fo.y/o.m)*dt;
	o.dp.z += (fo.z/o.m)*dt;
}

void accelerate( obj o, v3 a ){ // v += a*t
	o.dp.x += a.x*dt;
	o.dp.y += a.y*dt;
	o.dp.z += a.z*dt;
}

void move( obj o ){ // p += v*t
	o.p.x += o.dp.x;
	o.p.y += o.dp.y;
	o.p.z += o.dp.z;
}

void render(){

	glColor4f( 1,1,1,1 );

	glLoadIdentity();                                                   // clear modelview

	glRotatef( camera.r.x , 1, 0, 0 );                                   // rotate
	glRotatef( camera.r.y , 0, 1, 0 );
	glRotatef( camera.r.z , 0, 0, 1 );

		glCallList( DLbase );                                               // draw sky

	glTranslatef( -camera.p.x, -camera.p.y, -camera.p.z );                 // translate

	glEnable( GL_LIGHTING );
	glEnable( GL_LIGHT0 );
	glLightfv( GL_LIGHT0, GL_POSITION, lpos );

	glEnable( GL_DEPTH_TEST );
	glClear( GL_DEPTH_BUFFER_BIT );

		glCallList( DLbase+1 );

//	glDisable( GL_TEXTURE_2D );
//	glEnable( GL_BLEND );

		glTranslatef( 0, 1, 0 );
		glBindTexture( GL_TEXTURE_2D, textures[4] );
		glCallList( DLbase+2 );

	glEnable( GL_TEXTURE_2D );
//	glDisable( GL_BLEND );
	glDisable( GL_DEPTH_TEST );
	glDisable( GL_LIGHTING );
	glDisable( GL_LIGHT0 );

	glLoadIdentity();
	glTranslatef( -WIDTH/2, HEIGHT/2, -HEIGHT/2 );
	glBindTexture( GL_TEXTURE_2D, textures[7] );
	glListBase( DLfontbase );
	glPushMatrix();
	glCallLists( strlen(str), GL_UNSIGNED_BYTE, str );
	glPopMatrix();

}

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int iCmdShow){

    BOOL Quit = FALSE;
    MSG msg;

	createWindow();
	initScene();

	QueryPerformanceFrequency( (_LARGE_INTEGER*) &pcf );
    QueryPerformanceCounter( (_LARGE_INTEGER*) &t2 );

    while( !Quit ){

		if( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) ){

            if( msg.message == WM_QUIT ){

                Quit = TRUE;

            } else {

                TranslateMessage( &msg );
                DispatchMessage( &msg );

            }

        } else {

			t1 = t2;
            QueryPerformanceCounter( (_LARGE_INTEGER*) &t2 );
			dt = ((float)t2-(float)t1)/pcf;

			f.x = f.y = f.z = .0;

			if( kbd[0] ){
				f.x += sin( camera.r.y * DRR )*55.0f;
				f.z += -cos( camera.r.y * DRR )*55.0f;
			}

			if( kbd[1] ){
				f.x += sin( (camera.r.y-180) * DRR )*55.0f;
				f.z += -cos( (camera.r.y-180) * DRR )*55.0f;
			}

			if( kbd[2] ){
				f.x += sin( (camera.r.y-90) * DRR )*55.0f;
				f.z += -cos( (camera.r.y-90) * DRR )*55.0f;
			}

			if( kbd[3] ){
				f.x += sin( (camera.r.y+90) * DRR )*55.0f;
				f.z += -cos( (camera.r.y+90) * DRR )*55.0f;
			}

			if( kbd[4] && !jump ){
				jump = 1;
				f.y = 10040.0f;
			}

	camera.dp.x += (f.x/camera.m)*dt;
	camera.dp.y += (f.y/camera.m)*dt;
	camera.dp.z += (f.z/camera.m)*dt;

	camera.dp.x += g.x*dt;
	camera.dp.y += g.y*dt;
	camera.dp.z += g.z*dt;

	camera.dp.x -= camera.dp.x*dt*10;
	camera.dp.y -= camera.dp.y*dt*10;
	camera.dp.z -= camera.dp.z*dt*10;

	camera.p.x += camera.dp.x;
	camera.p.y += camera.dp.y;
	camera.p.z += camera.dp.z;

			if( camera.p.y <= 1.6 ){
				camera.p.y = 1.6;
				camera.dp.y = 0;
				jump = 0;
			}

			sprintf( str, "dt %5.1f ms\nfps %5.1f\ny %5.1f", dt*1000, 1/dt, camera.p.y);
			render();
			SwapBuffers (hDC);

        }

    }

	killScene();
	killWindow();

    return msg.wParam;
}

LRESULT CALLBACK WndProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam){
	switch (message){

    	case WM_CREATE:
			return 0;

		case WM_CLOSE:
			PostQuitMessage (0);
			return 0;

		case WM_DESTROY:
			return 0;

		case WM_KEYDOWN:
			switch( wParam ){

				case 27: PostQuitMessage(0); return 0;

				case 87: kbd[0] = true; return 0;
				case 83: kbd[1] = true;	return 0;
				case 65: kbd[2] = true; return 0;
				case 68: kbd[3] = true;	return 0;
				case VK_SPACE: kbd[4] = true;	return 0;

			}
			return 0;

		case WM_KEYUP:
			switch( wParam ){

				case 87: kbd[0] = false; return 0;
				case 83: kbd[1] = false; return 0;
				case 65: kbd[2] = false; return 0;
				case 68: kbd[3] = false; return 0;
				case VK_SPACE: kbd[4] = false; return 0;

			}
			return 0;

		case WM_MOUSEMOVE:

				mouseOldX = mouseX;
				mouseOldY = mouseY;

				mouseX = LOWORD( lParam );
				mouseY = HIWORD( lParam );

				camera.r.x += ((float)mouseY - (float)mouseOldY)*sensitivity;
				camera.r.y += ((float)mouseX - (float)mouseOldX)*sensitivity;

				if( camera.r.x > 90 ) camera.r.x = 90;
				if( camera.r.x < -90 ) camera.r.x = -90;

				if( mouseX == 0 ){ mouseX = WIDTH-2; SetCursorPos( mouseX, mouseY ); }
				if( mouseX == WIDTH-1 ){ mouseX = 1; SetCursorPos( mouseX, mouseY ); }
				if( mouseY == 0 ){ mouseY = HEIGHT-2; SetCursorPos( mouseX, mouseY ); }
				if( mouseY == HEIGHT-1 ){ mouseY = 1; SetCursorPos( mouseX, mouseY ); }

			return 0;

		default:
			return DefWindowProc (hWnd, message, wParam, lParam);

    }
}

void initScene(){

	ShowCursor( FALSE );
	SetCursorPos( 1, 1 );

//	glColor4f( 0, 0, 0, 1 ); // used where alpha = 0 ?! wtf ?!

//	glEnable( GL_TEXTURE_2D );

	glShadeModel( GL_SMOOTH );

//	glBlendFunc( GL_SRC_COLOR, GL_SRC_ALPHA );

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	glViewport( 0, 0, WIDTH, HEIGHT );
	glFrustum( -ASPECTRATIO, ASPECTRATIO, -1, 1, 1/tan(FOV*DRR/2), 1000 );
	glMatrixMode( GL_MODELVIEW );

	textures[0] = loadTexture( "d:\\cubemaps\\a\\front.bmp", false );
	textures[1] = loadTexture( "d:\\cubemaps\\a\\right.bmp", false );
	textures[2] = loadTexture( "d:\\cubemaps\\a\\up.bmp", false );
	textures[3] = loadTexture( "d:\\cubemaps\\a\\back.bmp", false );
	textures[4] = loadTexture( "d:\\green.bmp", false );
	textures[5] = loadTexture( "d:\\cubemaps\\a\\left.bmp", false );
	textures[6] = loadTexture( "d:\\grass.bmp", true );
	textures[7] = loadTexture( "d:\\myfont.bmp", false );

	DLbase = glGenLists( 3 );

	glNewList( DLbase, GL_COMPILE );

	    glBindTexture( GL_TEXTURE_2D, textures[0] ); // front
	    glBegin( GL_QUADS );
			glNormal3f( 0, 0, 1 );
			glTexCoord2f( 0, 1 );
			glVertex3f( -10, 10, -10 );
			glTexCoord2f( 0, 0 );
			glVertex3f( -10, -10, -10 );
			glTexCoord2f( 1, 0 );
			glVertex3f( 10, -10, -10 );
			glTexCoord2f( 1, 1 );
			glVertex3f( 10, 10, -10 );
		glEnd();

	    glBindTexture( GL_TEXTURE_2D, textures[1] ); // x+
		glBegin( GL_QUADS );
			glNormal3f( -1, 0, 0 );
			glTexCoord2f( 0, 1 );
			glVertex3f( 10, 10, -10 );
			glTexCoord2f( 0, 0 );
			glVertex3f( 10, -10, -10 );
			glTexCoord2f( 1, 0 );
			glVertex3f( 10, -10, 10 );
			glTexCoord2f( 1, 1 );
			glVertex3f( 10, 10, 10 );
	    glEnd();

	    glBindTexture( GL_TEXTURE_2D, textures[2] ); // y+
		glBegin( GL_QUADS );
			glNormal3f( 0, -1, 0 );
			glTexCoord2f( 0, 0 );
			glVertex3f( -10, 10, -10 );
			glTexCoord2f( 0, 1 );
			glVertex3f( -10, 10, 10 );
			glTexCoord2f( 1, 1 );
			glVertex3f( 10, 10, 10 );
			glTexCoord2f( 1, 0 );
			glVertex3f( 10, 10, -10 );
	    glEnd();

	    glBindTexture( GL_TEXTURE_2D, textures[3] ); // x++
	    glBegin( GL_QUADS );
			glNormal3f( 0, 0, -1 );
			glTexCoord2f( 1, 1 );
			glVertex3f( -10, 10, 10 );
			glTexCoord2f( 1, 0 );
			glVertex3f( -10, -10, 10 );
			glTexCoord2f( 0, 0 );
			glVertex3f( 10, -10, 10 );
			glTexCoord2f( 0, 1 );
			glVertex3f( 10, 10, 10 );
		glEnd();

/*	    glBindTexture( GL_TEXTURE_2D, textures[4] ); // y-
		glBegin( GL_QUADS );
			glNormal3f( 0, 1, 0 );
			glTexCoord2f( 0, 0 );
			glVertex3f( -10, -10, -10 );
			glTexCoord2f( 0, 1 );
			glVertex3f( -10, -10, 10 );
			glTexCoord2f( 1, 1 );
			glVertex3f( 10, -10, 10 );
			glTexCoord2f( 1, 0 );
			glVertex3f( 10, -10, -10 );
	    glEnd();
*/
	    glBindTexture( GL_TEXTURE_2D, textures[5] ); // x-
		glBegin( GL_QUADS );
			glNormal3f( 1, 0, 0 );
			glTexCoord2f( 1, 1 );
			glVertex3f( -10, 10, -10 );
			glTexCoord2f( 1, 0 );
			glVertex3f( -10, -10, -10 );
			glTexCoord2f( 0, 0 );
			glVertex3f( -10, -10, 10 );
			glTexCoord2f( 0, 1 );
			glVertex3f( -10, 10, 10 );
	    glEnd();

	glEndList();

	glNewList( DLbase+1, GL_COMPILE );
		glBindTexture( GL_TEXTURE_2D, textures[6] );
		glBegin( GL_QUADS );
			glNormal3f( 0, 1, 0 );
			glTexCoord2f( 0, 100);
			glVertex3f( -50, 0, 50 );
			glTexCoord2f( 0, 0);
			glVertex3f( -50, 0, -50 );
			glTexCoord2f( 100, 0);
			glVertex3f( 50, 0, -50 );
			glTexCoord2f( 100, 100);
			glVertex3f( 50, 0, 50 );
		glEnd();
	glEndList();

	glNewList( DLbase+2, GL_COMPILE );

	    glBegin( GL_QUADS );
			glNormal3f( 0, 0, -1 );
			glTexCoord2f( 0, 1 );
			glVertex3f( -1, 1, -1 );
			glTexCoord2f( 0, 0 );
			glVertex3f( -1, -1, -1 );
			glTexCoord2f( 1, 0 );
			glVertex3f( 1, -1, -1 );
			glTexCoord2f( 1, 1 );
			glVertex3f( 1, 1, -1 );

			glNormal3f( 1, 0, 0 );
			glTexCoord2f( 0, 1 );
			glVertex3f( 1, 1, -1 );
			glTexCoord2f( 0, 0 );
			glVertex3f( 1, -1, -1 );
			glTexCoord2f( 1, 0 );
			glVertex3f( 1, -1, 1 );
			glTexCoord2f( 1, 1 );
			glVertex3f( 1, 1, 1 );

			glNormal3f( 0, 1, 0 );
			glTexCoord2f( 0, 0 );
			glVertex3f( -1, 1, -1 );
			glTexCoord2f( 0, 1 );
			glVertex3f( -1, 1, 1 );
			glTexCoord2f( 1, 1 );
			glVertex3f( 1, 1, 1 );
			glTexCoord2f( 1, 0 );
			glVertex3f( 1, 1, -1 );

			glNormal3f( 0, 0, 1 );
			glTexCoord2f( 1, 1 );
			glVertex3f( -1, 1, 1 );
			glTexCoord2f( 1, 0 );
			glVertex3f( -1, -1, 1 );
			glTexCoord2f( 0, 0 );
			glVertex3f( 1, -1, 1 );
			glTexCoord2f( 0, 1 );
			glVertex3f( 1, 1, 1 );

			glNormal3f( 0, -1, 0 );
			glTexCoord2f( 0, 0 );
			glVertex3f( -1, -1, -1 );
			glTexCoord2f( 0, 1 );
			glVertex3f( -1, -1, 1 );
			glTexCoord2f( 1, 1 );
			glVertex3f( 1, -1, 1 );
			glTexCoord2f( 1, 0 );
			glVertex3f( 1, -1, -1 );

			glNormal3f( -1, 0, 0 );
			glTexCoord2f( 1, 1 );
			glVertex3f( -1, 1, -1 );
			glTexCoord2f( 1, 0 );
			glVertex3f( -1, -1, -1 );
			glTexCoord2f( 0, 0 );
			glVertex3f( -1, -1, 1 );
			glTexCoord2f( 0, 1 );
			glVertex3f( -1, 1, 1 );
	    glEnd();

	glEndList();

	DLfontbase = glGenLists( 128 );

	glNewList( DLfontbase+10, GL_COMPILE );
		glPopMatrix();
		glTranslatef( 0, -14, 0 );
		glPushMatrix();
	glEndList();

	GLuint i,j;

	for( j=0; j<6; j++ ){
		for( i=0; i<16; i++ ){
			glNewList( DLfontbase+32+j*16+i, GL_COMPILE );
				glBegin( GL_QUADS );
					glTexCoord2f( i*0.0546875f, 1-j*0.109375f );
					glVertex2f( 0, 0 );
					glTexCoord2f( i*0.0546875f, 1-j*0.109375f -0.109375f );
					glVertex2f( 0, -14 );
					glTexCoord2f( i*0.0546875f+0.0546875f, 1-j*0.109375f -0.109375f );
					glVertex2f( 7, -14 );
					glTexCoord2f( i*0.0546875f+0.0546875f, 1-j*0.109375f );
					glVertex2f( 7, 0 );
				glEnd();
				glTranslatef( 7, 0, 0 );
			glEndList();

		}
	}
	
}

void killScene(){
    glDeleteTextures( 8, textures );
    glDeleteLists( DLbase, 2 );
    glDeleteLists( DLfontbase, 128 );
}

unsigned int loadTexture( const char * fileName, BOOL genMipmaps ){

	FILE * fp;
	BITMAPFILEHEADER header;
	BITMAPINFOHEADER info;

	unsigned int size=0, i=0, j=0, bpp=0;

	GLsizei width=0, height=0;
	GLint n=0;
	GLuint tex;

	if(	!( fp = fopen( fileName, "rb" ) ) ) return 0;

	fread( &header, 1, sizeof( header ), fp );
	fread( &info, 1, sizeof( info ), fp );

	bpp = info.biBitCount/8;
	width = info.biWidth;
	height = info.biHeight;

	if( bpp != 3 && bpp != 4 ) return 0;
	if( width != height ) return 0;

	for( size=0; size<=13; size++ ) if( width == pow( 2.0f, size ) ) break;
	if( size == 13 ) return 0;

	unsigned char * data = (unsigned char *) malloc( width*height*bpp );

	fread( data, 1, width*height*bpp, fp );

	fclose( fp );

	for( i=0; i<width*height*bpp; i+=bpp ){
		j = *(data+i+2);
		*(data+i+2) = *(data+i+0);
        *(data+i+0) = (unsigned char) j;
	}

	glGenTextures( 1, &tex );
	glBindTexture( GL_TEXTURE_2D, tex );

    glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

	if( genMipmaps ){
        glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
		if( bpp == 4 )
		gluBuild2DMipmaps( GL_TEXTURE_2D, GL_RGBA, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data );
		else
		gluBuild2DMipmaps( GL_TEXTURE_2D, GL_RGB, width, height, GL_RGB, GL_UNSIGNED_BYTE, data );
	} else {
        glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		if( bpp == 4 )
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data );
		else
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data );
	}

	free( data );

    return tex;

}

BOOL createWindow(){

	GLuint		PixelFormat;			// Holds The Results After Searching For A Match
	HINSTANCE	hInstance;				// Holds The Instance Of The Application
	WNDCLASS	wc;						// Windows Class Structure
	DWORD		dwExStyle;				// Window Extended Style
	DWORD		dwStyle;				// Window Style

	hInstance			= GetModuleHandle(NULL);				// Grab An Instance For Our Window
	wc.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;	// Redraw On Size, And Own DC For Window.
	wc.lpfnWndProc		= (WNDPROC) WndProc;					// WndProc Handles Messages
	wc.cbClsExtra		= 0;									// No Extra Window Data
	wc.cbWndExtra		= 0;									// No Extra Window Data
	wc.hInstance		= hInstance;							// Set The Instance
	wc.hIcon			= LoadIcon(NULL, IDI_WINLOGO);			// Load The Default Icon
	wc.hCursor			= LoadCursor(NULL, IDC_ARROW);			// Load The Arrow Pointer
	wc.hbrBackground	= NULL;									// No Background Required For GL
	wc.lpszMenuName		= NULL;									// We Don't Want A Menu
	wc.lpszClassName	= "OpenGL";								// Set The Class Name

    RegisterClass(&wc);

	DEVMODE dmScreenSettings;								// Device Mode
	memset(&dmScreenSettings,0,sizeof(dmScreenSettings));	// Makes Sure Memory's Cleared
	dmScreenSettings.dmSize=sizeof(dmScreenSettings);		// Size Of The Devmode Structure
	dmScreenSettings.dmPelsWidth	= WIDTH;				// Selected Screen Width
	dmScreenSettings.dmPelsHeight	= HEIGHT;				// Selected Screen Height
	dmScreenSettings.dmBitsPerPel	= 32;					// Selected Bits Per Pixel
	dmScreenSettings.dmFields=DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT;

    ChangeDisplaySettings( &dmScreenSettings, CDS_FULLSCREEN );

	dwExStyle = WS_EX_APPWINDOW;								// Window Extended Style
	dwStyle = WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP;		// Windows Style

	if( !( hWnd = CreateWindowEx(	dwExStyle,			// Extended Style For The Window
								"OpenGL",			// Class Name
								APPNAME,				// Window Title
								dwStyle,			// Window Style
								0, 0,				// Window Position
								WIDTH, HEIGHT,		// Selected Width And Height
								NULL,				// No Parent Window
								NULL,				// No Menu
								hInstance,			// Instance
								NULL)))				// Dont Pass Anything To WM_CREATE
	{
		killWindow();								// Reset The Display
		MessageBox(NULL,"Window Creation Error.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	static	PIXELFORMATDESCRIPTOR pfd=				// pfd Tells Windows How We Want Things To Be
	{
		sizeof(PIXELFORMATDESCRIPTOR),				// Size Of This Pixel Format Descriptor
		1,											// Version Number
		PFD_DRAW_TO_WINDOW |						// Format Must Support Window
		PFD_SUPPORT_OPENGL |						// Format Must Support OpenGL
		PFD_DOUBLEBUFFER,							// Must Support Double Buffering
		PFD_TYPE_RGBA,								// Request An RGBA Format
		32,										// Select Our Color Depth
		0, 0, 0, 0, 0, 0,							// Color Bits Ignored
		0,											// No Alpha Buffer
		0,											// Shift Bit Ignored
		0,											// No Accumulation Buffer
		0, 0, 0, 0,									// Accumulation Bits Ignored
		16,											// 16Bit Z-Buffer (Depth Buffer)
		0,											// No Stencil Buffer
		0,											// No Auxiliary Buffer
		PFD_MAIN_PLANE,								// Main Drawing Layer
		0,											// Reserved
		0, 0, 0										// Layer Masks Ignored
	};

    hDC=GetDC(hWnd);
	PixelFormat=ChoosePixelFormat(hDC,&pfd);
	SetPixelFormat(hDC,PixelFormat,&pfd);
    hRC=wglCreateContext(hDC);
	wglMakeCurrent(hDC,hRC);

	ShowWindow(hWnd,SW_SHOW);						// Show The Window
	SetForegroundWindow(hWnd);						// Slightly Higher Priority
	SetFocus(hWnd);									// Sets Keyboard Focus To The Window

	return TRUE;									// Success
}

void killWindow(){
    wglMakeCurrent(NULL,NULL);
	wglDeleteContext(hRC);
	hRC=NULL;

    ReleaseDC(hWnd,hDC);
	hDC=NULL;

	DestroyWindow(hWnd);
	hWnd=NULL;

	ChangeDisplaySettings(NULL,0);
	ShowCursor(TRUE);
}
