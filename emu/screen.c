#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>


#define x2 1

void updateImage(XImage *);

int WINDOW_WIDTH = 320;
int WINDOW_HEIGHT = 200;
int width,height;
void* framebuffer;

Display *display;
Window win;
GC gc;
XImage *img;

void *updateScreen (void *p) {
  for (;;) {
    updateImage(img);
	  XPutImage(display, win, gc, img, 0, 0, 0, 0, width, height);
		usleep(20000);
  }
}

void updateImage(XImage *img)
{
  int rgb,i,r,g,b;
  unsigned short *pixel=(unsigned short *)framebuffer;

  if (x2) i=2;
  else i=1;
  for (int y = 0; y < height; y+=i) {
    for (int x = 0; x < width; x+=i) {
      rgb=*pixel++;
      if (rgb&0x8000) {
        r=(rgb&0x7c00)<<9;
        g=(rgb&0x03e0)<<6;
        b=(rgb&0x001f)<<3;
      } else {
        r=(rgb&0x7c00)<<7;
        g=(rgb&0x03e0)<<4;
        b=(rgb&0x001f)<<1;
      }
      rgb=0xff000000|r|g|b;
      XPutPixel(img,x,y,rgb);
      if (x2) {
        XPutPixel(img,x+1,y,rgb);
        XPutPixel(img,x,y+1,rgb);
        XPutPixel(img,x+1,y+1,rgb);
      }
    }
  }
}

void initScreen (void *fb) {
	int i,x,y;
	
	if (x2) {width=WINDOW_WIDTH<<1;height=WINDOW_HEIGHT<<1;}
	else {width=WINDOW_WIDTH;height=WINDOW_HEIGHT;}
	framebuffer = fb;
	unsigned char* pixel = (unsigned char*)framebuffer;

	//init display
	display = XOpenDisplay(NULL);
	int screen_num = DefaultScreen(display);
	Window root = RootWindow(display, screen_num);
	Visual *visual = DefaultVisual(display, screen_num);
	int colorBits = DefaultDepth(display, screen_num);
	char *data = (char*)malloc(width * height * colorBits);
	
	//Create/Show window
	win = XCreateSimpleWindow(display, root, 50, 50, width, height, 1, 0, 0);
	XSelectInput(display, win, ExposureMask | ButtonPressMask| ButtonReleaseMask | PointerMotionMask | KeyPressMask);
	XMapWindow(display, win);

    //create img for framebuffer
    img = XCreateImage(display, visual, colorBits, ZPixmap, 0, data, width, height, 32, 0);

	//Main loop
    gc = DefaultGC(display,screen_num);

    pthread_t pid;
    pthread_create(&pid, NULL, updateScreen, NULL);

	return;
}
