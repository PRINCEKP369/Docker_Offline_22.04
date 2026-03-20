#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>

typedef struct {
  int x,y,x1,y1;
} coordinate;

/* Second box: x2_origin = x1 + 1024 + 20, width = 150, height = 400 */

void DrawData(Display *d,Window w,GC gc,int *Data,coordinate *g,int y) {
  for(int i=0;i<1023;i++) {
    if(Data[i]>30 && Data[i]<60) XSetForeground(d,gc,0x00FF00);
    else                          XSetForeground(d,gc,0xFF0000);
    XFillRectangle(d,w,gc,g->x1+1+i,g->y1+y,1,1);
  }
}

void DrawData2(Display *d,Window w,GC gc,int *Data,coordinate *g,int y) {
  int x2=g->x1+1024+20;
  for(int i=0;i<149;i++) {
    if(Data[i]>30 && Data[i]<60) XSetForeground(d,gc,0x00FF00);
    else                          XSetForeground(d,gc,0xFF0000);
    XFillRectangle(d,w,gc,x2+1+i,g->y1+y,1,1);
  }
}

void DrawBG(Display *d,Window w,GC gc,coordinate *g,int s) {
  char *msg="Active Display", *msg2="Box 2";
  int x2=g->x1+1024+20;

  XSetForeground(d,gc,BlackPixel(d,s));
  XFillRectangle(d,w,gc,0,0,g->x,g->y);

  XSetForeground(d,gc,WhitePixel(d,s));
  XFillRectangle(d,w,gc,g->x1,     g->y1,    1024,1);
  XFillRectangle(d,w,gc,g->x1,     g->y1,    1,400);
  XFillRectangle(d,w,gc,g->x1,     g->y1+400,1024,1);
  XFillRectangle(d,w,gc,g->x1+1024,g->y1,    1,401);

  XFillRectangle(d,w,gc,x2,    g->y1,    150,1);
  XFillRectangle(d,w,gc,x2,    g->y1,    1,400);
  XFillRectangle(d,w,gc,x2,    g->y1+400,150,1);
  XFillRectangle(d,w,gc,x2+150,g->y1,    1,401);

  XSetForeground(d,gc,0xFF0000);
  XDrawString(d,w,gc,g->x1+512,g->y1-25,msg, strlen(msg));
  XDrawString(d,w,gc,x2+40,    g->y1-25,msg2,strlen(msg2));

  XSetForeground(d,gc,WhitePixel(d,s));
  XFillRectangle(d,w,gc,g->x1,g->y1-20,60,15);
  XFillRectangle(d,w,gc,x2,   g->y1-20,60,15);
}

/* Box 1 crosshair: horizontal spans 1024, vertical spans 400 */
void CrossLine(Display *d,Window w,GC gc,coordinate *g,int x,int y) {
  XSetFunction(d,gc,GXxor);
  XSetForeground(d,gc,0x00FFFFFF);
  for(int i=1;i<1024;i+=3) XFillRectangle(d,w,gc,g->x1+i,y,1,1);
  for(int i=1;i<400; i+=3) XFillRectangle(d,w,gc,x,g->y1+1+i,1,1);
  XSetFunction(d,gc,GXcopy);
}

/* Box 2 crosshair: horizontal spans 150, vertical spans 400 */
void CrossLine2(Display *d,Window w,GC gc,coordinate *g,int x,int y) {
  int x2=g->x1+1024+20;
  XSetFunction(d,gc,GXxor);
  XSetForeground(d,gc,0x00FFFFFF);
  for(int i=1;i<150;i+=3) XFillRectangle(d,w,gc,x2+i,y,1,1);
  for(int i=1;i<400;i+=3) XFillRectangle(d,w,gc,x,g->y1+1+i,1,1);
  XSetFunction(d,gc,GXcopy);
}

/* Box 1 coord label */
void ShowMsg(Display *d,Window w,GC gc,coordinate *g,int x,int y,int s) {
  char xi[10],yi[10];
  sprintf(xi,"%d",x); sprintf(yi,"%d",y);
  XSetForeground(d,gc,WhitePixel(d,s));
  XFillRectangle(d,w,gc,g->x1,g->y1-20,60,15);
  XSetForeground(d,gc,0x0000FF);
  XDrawString(d,w,gc,g->x1+3, g->y1-10,xi,strlen(xi));
  XDrawString(d,w,gc,g->x1+30,g->y1-10,yi,strlen(yi));
}

/* Box 2 coord label — anchored above box 2's top-left */
void ShowMsg2(Display *d,Window w,GC gc,coordinate *g,int x,int y,int s) {
  char xi[10],yi[10];
  sprintf(xi,"%d",x); sprintf(yi,"%d",y);
  int x2=g->x1+1024+20;
  XSetForeground(d,gc,WhitePixel(d,s));
  XFillRectangle(d,w,gc,x2,g->y1-20,60,15);
  XSetForeground(d,gc,0x0000FF);
  XDrawString(d,w,gc,x2+3, g->y1-10,xi,strlen(xi));
  XDrawString(d,w,gc,x2+30,g->y1-10,yi,strlen(yi));
}

void DrawHrLine(Display *d,Window w,GC gc,coordinate *g,int y,int thickness) {
  int x2=g->x1+1024+20;
  XSetForeground(d,gc,0xFFFF00);
  XFillRectangle(d,w,gc,g->x1+1,g->y1+1+y,1023,thickness);
  XFillRectangle(d,w,gc,x2+1,   g->y1+1+y,149, thickness);
}

int main() {
  Display *d;
  Window w;
  XEvent e;
  int s;
  coordinate arg;
  arg.x=1126+20+152; arg.y=552; arg.x1=50; arg.y1=100;

  int xprev=-1,yprev=-1;       /* box 1 click */
  int xprev2=-1,yprev2=-1;     /* box 2 click */
  int yval=398;

  int mouse_x=-1,mouse_y=-1,mouse_in=0;   /* box 1 mouse */
  int mouse_x2=-1,mouse_y2=-1,mouse_in2=0;/* box 2 mouse */

  int *data=(int*)malloc(1024*sizeof(int));

  d=XOpenDisplay(NULL);
  if(!d){printf("Can't open display\n");return 0;}
  s=DefaultScreen(d);
  int run=1;

  w=XCreateSimpleWindow(d,RootWindow(d,s),100,100,arg.x,arg.y,1,
                        BlackPixel(d,s),WhitePixel(d,s));
  XSelectInput(d,w,ExposureMask|KeyPressMask|PointerMotionMask|ButtonPressMask);
  XMapWindow(d,w);

  GC gc=XCreateGC(d,w,0,NULL);
  Pixmap back=XCreatePixmap(d,w,arg.x,arg.y,DefaultDepth(d,s));
  GC bg_gc=XCreateGC(d,back,0,NULL);

  DrawBG(d,back,bg_gc,&arg,s);

  int x2_left=arg.x1+1024+20, x2_right=x2_left+150;

  while(run) {
    while(XPending(d)) {
      XNextEvent(d,&e);

      if(e.type==Expose)
        XCopyArea(d,back,w,gc,0,0,arg.x,arg.y,0,0);

      if(e.type==MotionNotify) {
        int x=e.xmotion.x, y=e.xmotion.y;
        if(x>arg.x1 && x<arg.x1+1024 && y>arg.y1 && y<arg.y1+400)
          { mouse_x=x; mouse_y=y; mouse_in=1; } else mouse_in=0;
        if(x>x2_left && x<x2_right && y>arg.y1 && y<arg.y1+400)
          { mouse_x2=x; mouse_y2=y; mouse_in2=1; } else mouse_in2=0;
      }

      if(e.type==ButtonPress) {
        int x=e.xbutton.x, y=e.xbutton.y;
        /* box 1 */
        if(x>arg.x1 && x<arg.x1+1024 && y>arg.y1 && y<arg.y1+400) {
          if(xprev!=-1) CrossLine(d,w,gc,&arg,xprev,yprev);
          xprev=x; yprev=y;
          CrossLine(d,w,gc,&arg,x,y);
        }
        /* box 2 */
        if(x>x2_left && x<x2_right && y>arg.y1 && y<arg.y1+400) {
          if(xprev2!=-1) CrossLine2(d,w,gc,&arg,xprev2,yprev2);
          xprev2=x; yprev2=y;
          CrossLine2(d,w,gc,&arg,x,y);
        }
      }

      if(e.type==KeyPress)
        if(XLookupKeysym(&e.xkey,0)==XK_Escape) run=0;
    }

    for(int i=0;i<1024;i++) data[i]=rand()%65;
    DrawData(d,back,bg_gc,data,&arg,yval+1);
    DrawData2(d,back,bg_gc,data,&arg,yval+1);
    XCopyArea(d,back,w,gc,0,0,arg.x,arg.y,0,0);

    DrawHrLine(d,w,gc,&arg,yval,1);
    srand(time(NULL));

    if(xprev!=-1)  CrossLine(d,w,gc,&arg,xprev,yprev);
    if(xprev2!=-1) CrossLine2(d,w,gc,&arg,xprev2,yprev2);

    if(mouse_in)  ShowMsg(d,w,gc,&arg,mouse_x,mouse_y,s);
    if(mouse_in2) ShowMsg2(d,w,gc,&arg,mouse_x2,mouse_y2,s);

    XFlush(d);
    yval--; if(yval==0) yval=398;
    usleep(10000);
  }

  XCloseDisplay(d);
  return 0;
}
