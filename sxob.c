#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <string.h> 
#include <fcntl.h> 
#include <sys/stat.h> 
#include <sys/types.h> 
#include <unistd.h>
#include <pthread.h>
#include<X11/X.h>
#include<GL/gl.h>
#include<GL/glx.h>
#include<GL/glu.h>

Window root;
GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
XVisualInfo *vi;
Colormap cmap;
GLXContext glc;

Display *dis;
Window win;

pthread_t tmp_thread;
//pthread_mutex_t lock;

typedef struct DrwStruct{
	Display *dpy;
	Window win;
	GC gc;
	GLXContext glc;
	XVisualInfo *vi;
	int timer;
	int multi;
	int alt;
}Drw;

void *timer_fun(void *drw){
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	tmp_thread = pthread_self();
	sleep(1);
	Drw *d = (Drw *)drw;
	XLockDisplay(d->dpy);
	XUnmapWindow(d->dpy, d->win);
	XUnlockDisplay(d->dpy);
}


void DrawAQuad(int multi, int alt) {
	double sirina = 0.02*multi;

	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/*
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-1., 1., -1., 1., 1., 20.);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0., 0., 10., 0., 0., 0., 0., 1., 0.);
	*/

	if(alt){
		glColor3f(1., 0., 0.);
	}else{
		glColor3f(0., 0., 1.);
	}
	glRectf(-1.0, 1.0, -1+sirina, -1.0);
} 

void *draw_fun(void *drw){
	Drw *d = (Drw *)drw;
	XWindowAttributes wa;
	//XLockDisplay(d->dpy);
	pthread_t timer;

	if(!d->timer){
		pthread_cancel(tmp_thread);
	}
	pthread_create(&timer, NULL, timer_fun, (void *)d);
	glXMakeCurrent(dis, win, glc);
	glEnable(GL_DEPTH_TEST); 

	XGetWindowAttributes(d->dpy, d->win, &wa);
	glViewport(0, 0, wa.width, wa.height);
	DrawAQuad(d->multi, d->alt);
	glXSwapBuffers(dis, win);

	XEvent ev;
	pthread_join(timer, NULL);
	while(XPending(d->dpy)){
		XNextEvent(dis, &ev);
	}
	//XUnlockDisplay(d->dpy);
}
void *input_fun(void *drw){
	char volume[4];
	Drw *d = (Drw *)drw;
	pthread_t draw;
	XWindowAttributes wa;
	int multiplier;
	while(1){
		//get volume from pipe
		scanf("%s", volume);
		d->alt = volume[strlen(volume)-1] == '!' ? 1 : 0;
		multiplier = atoi(volume);
		d->multi = multiplier;

		XLockDisplay(d->dpy);
		XGetWindowAttributes(d->dpy, d->win, &wa);
		//check if it needs to be mapped
		if(wa.map_state == IsUnmapped){
			XMapWindow(d->dpy, d->win);
			d->timer = 1;
		}else{
			d->timer = 0;
		}
		XUnlockDisplay(d->dpy);
		pthread_create(&draw, NULL, draw_fun, (void *)d);
	}
}

int main() {
	XInitThreads();
	pthread_t input;
	dis = XOpenDisplay(NULL);
	if(dis == NULL){
		printf("cannot connect to X\n");
	}
	root = DefaultRootWindow(dis);
    	vi = glXChooseVisual(dis, 0, att);
    	if(vi == NULL){
	    	printf("no appropriate VIUSAL FOUND\n");
    	}else {
		printf("visual %p selected\n", (void *)vi->visualid); /* %p creates hexadecimal output like in glxinfo */
    	}
	cmap = XCreateColormap(dis, root, vi->visual, AllocNone);
	XSetWindowAttributes swa;
	swa.colormap = cmap;
	swa.override_redirect = True;
	swa.background_pixmap = None;
	win = XCreateWindow(dis, root, 1920/2-200, 5, 400, 20, 0, vi->depth, InputOutput, vi->visual, CWOverrideRedirect | CWColormap, &swa);

	glc = glXCreateContext(dis, vi, NULL, GL_TRUE);

	XGCValues values;
	values.graphics_exposures = True;
	GC gc = XCreateGC(dis, win, GCGraphicsExposures, &values);

	Drw *drw = (Drw *)malloc(sizeof(Drw));
	drw->dpy = dis;
	drw->win = win;
	drw->gc = gc;
	drw->glc = glc;
	drw->vi = vi;

	pthread_create(&input, NULL, input_fun, (void *)drw);
	pthread_join(input, NULL);

	return(0);
}
