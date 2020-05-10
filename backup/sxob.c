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

Display *dis;
Window win;
int x11_fd, input_fd;
fd_set in_fds;
int multiplier;

struct timeval tv;
XEvent ev;
pthread_t tmp_thread;
int create_timer;

typedef struct DrwStruct{
	Display *dpy;
	Window win;
	GC gc;
	int timer;
}Drw;

void *timer_fun(void *drw){
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	tmp_thread = pthread_self();
	printf("zacel merit cas\n");
	sleep(1);
	Drw *d = (Drw *)drw;
	XLockDisplay(d->dpy);
	printf("unmappiram!\n");
	XUnmapWindow(d->dpy, d->win);
	XUnlockDisplay(d->dpy);
}

void *draw_fun(void *drw){
	printf("risem create timer: %d\n", create_timer);
	printf("haha");
	Drw *d = (Drw *)drw;
	XWindowAttributes wa;
	//XLockDisplay(d->dpy);
	XGetWindowAttributes(d->dpy, d->win, &wa);
	XEvent event;
	pthread_t timer;
	/*
	if(create_timer){
		pthread_create(&timer, NULL, timer_fun, (void *)d);
	}else{
		pthread_cancel(tmp_thread);
		pthread_create(&timer, NULL, timer_fun, (void *)d);
	}
	*/
	if(!create_timer){
		pthread_cancel(tmp_thread);
	}
	pthread_create(&timer, NULL, timer_fun, (void *)d);
	pthread_join(timer, NULL);
	printf("grem merit cas\n");
	while(XPending(d->dpy)){
		XNextEvent(d->dpy, &event);
		switch(event.type){
			default:
				printf("event.type: %d\n", event.type);
				break;
		}
		/*
		if(XPending(d->dpy) == 0){
			pthread_join(tmp_thread, NULL);
		}
		*/
	}
	//XUnlockDisplay(d->dpy);
	printf("koncal risanje...\n");
}
void *input_fun(void *drw){
    printf("listening for input...\n");
    char volume[4];
    Drw *d = (Drw *)drw;
    pthread_t draw, timer;
    XWindowAttributes wa;
    int alt;
    while(1){
	    //get volume from pipe
	    scanf("%s", volume);
	    alt = volume[strlen(volume)-1] == '!' ? 1 : 0;
	    printf("ALT: %d\n", alt);
	    multiplier = atoi(volume);

	    XGetWindowAttributes(d->dpy, d->win, &wa);
	    //check if it needs to be mapped
	    if(wa.map_state == IsUnmapped){
		    XMapWindow(d->dpy, d->win);
		    create_timer = 1;
		    //pthread_create(&timer, NULL, timer_fun, (void *)d);
	    }else{
		    //posl signal naj resitira stopanje timerja
		    //za unmappat window
		    printf("KENSLAM TAJMER\n");
		    create_timer = 0;
		    //pthread_cancel(timer);
	    }

	    //draw background
	    if(!alt){
	    XSetForeground(d->dpy, d->gc, BlackPixel(d->dpy, 0));
	    XFillRectangle(d->dpy, d->win, d->gc, 0, 0, 400, 50);
	    
	    //draw volume level
	    XSetForeground(d->dpy, d->gc, WhitePixel(d->dpy, 0));
	    XFillRectangle(d->dpy, d->win, d->gc, 0, 0, multiplier*4, 50);
	    //pthread_create(&draw, NULL, draw_fun, (void *)d);
	    }else{
	    XSetForeground(d->dpy, d->gc, WhitePixel(d->dpy, 0));
	    XFillRectangle(d->dpy, d->win, d->gc, 0, 0, 400, 50);
	    
	    //draw volume level
	    XSetForeground(d->dpy, d->gc, BlackPixel(d->dpy, 0));
	    XFillRectangle(d->dpy, d->win, d->gc, 0, 0, multiplier*4, 50);
	    }
	    pthread_create(&draw, NULL, draw_fun, (void *)d);
    }
}

int main() {
    XInitThreads();
    dis = XOpenDisplay(NULL);
    XSetWindowAttributes swa;
    swa.event_mask = EnterWindowMask | LeaveWindowMask |VisibilityChangeMask | KeyPressMask | ExposureMask | StructureNotifyMask;
    swa.override_redirect = True;
    swa.background_pixel = BlackPixel(dis, 0);
    win = XCreateWindow(dis, RootWindow(dis, 0), 1920/2-200, 50, 400, 30, 2, CopyFromParent, CopyFromParent, \
		    CopyFromParent, CWEventMask | CWOverrideRedirect | CWBackPixel, &swa);

    // You don't need all of these. Make the mask as you normally would.
    //XMapWindow(dis, win);

    XGCValues values;
    values.graphics_exposures = True;
    GC gc = XCreateGC(dis, win, GCGraphicsExposures, &values);

    Drw *drw = (Drw *)malloc(sizeof(Drw));
    drw->dpy = dis;
    drw->win = win;
    drw->gc = gc;

    pthread_t input;
    pthread_create(&input, NULL, input_fun, (void *)drw);
    pthread_join(input, NULL);


    return(0);
}
