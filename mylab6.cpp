//
//Author: Shane Wilkerson
//date: 10-20-2024
//
//purpose:         OpenGL sample program
// Display a waterfall that also represents the common waterfall method that is used in software engineering
//
//
#include <iostream>
using namespace std;
#include <stdio.h>
#include <unistd.h>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cmath>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <GL/glx.h>
#include "fonts.h"

#define rnd() (float)rand() / (float)RAND_MAX

//some structures
const int MAX_PARTICLES = 500;

const int numberboxes = 5; //array to make number of boxes
//const int textboxes = 5; //array for textboxes

class Global {
public:
   // int red = 100; //set back to 200
	int xres, yres;
	float w;
	float vel;
	float pos[2];
    int red = 255;
    int blue = 0;
    int red2 = 0;
    int green = 0;
	Global() {
	    xres = 640;
	    yres = 480;
        
	   // w = 20.0f;
	   // vel = 30.0f;
	   // pos[0] = 0.0f + w;
	   // pos[1] = yres / 2.0f;
	}
} g;


class Circle {
    public:
        float pos[2];
        float vel[2];
        float radius;
        const int num_segments = 100;
        Circle() {
            pos[0] = 600;
            pos[1] = -50;
            radius = 200;

} 
} circle;



class Box {
public:
    float pos[2];
    float last_pos[2];
    float vel[2];
    float offx = 60;
    float offy = 50;
    int w;
    int h;
    Box() {
        pos[0] = 100; //g.xres / 2
        pos[1] = 420; //g.xres / 2
        vel[0] = vel[1] = 0.0f;
        w = 70;
        h = 13;
    }

} box;
Box number[numberboxes];
Box particle[MAX_PARTICLES];
//Box text[textboxes];
int n = 0;


class X11_wrapper {
private:
	Display *dpy;
	Window win;
	GLXContext glc;
public:
	~X11_wrapper();
	X11_wrapper();
	void set_title();
	bool getXPending();
	XEvent getXNextEvent();
	void swapBuffers();
	void reshape_window(int width, int height);
	void check_resize(XEvent *e);
	void check_mouse(XEvent *e);
	int check_keys(XEvent *e);
} x11;

//Function prototypes
void init_opengl(void);
void physics(void);
void render(void);


int main()
{
	init_opengl();
	int done = 0;
	//main game loop
	while (!done) {
		//look for external events such as keyboard, mouse.
		while (x11.getXPending()) {
			XEvent e = x11.getXNextEvent();
			x11.check_resize(&e);
			x11.check_mouse(&e);
			done = x11.check_keys(&e);
		}
		physics();
		render();
		x11.swapBuffers();
		usleep(200);
	}
	return 0;
}

/*Global::Global()
{
    xres = 640;
    yres = 480;
}*/


X11_wrapper::~X11_wrapper()
{
	XDestroyWindow(dpy, win);
	XCloseDisplay(dpy);
}

X11_wrapper::X11_wrapper()
{
	GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
	int w = g.xres, h = g.yres;
	dpy = XOpenDisplay(NULL);
	if (dpy == NULL) {
		cout << "\n\tcannot connect to X server\n" << endl;
		exit(EXIT_FAILURE);
	}
	Window root = DefaultRootWindow(dpy);
	XVisualInfo *vi = glXChooseVisual(dpy, 0, att);
	if (vi == NULL) {
		cout << "\n\tno appropriate visual found\n" << endl;
		exit(EXIT_FAILURE);
	} 
	Colormap cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
	XSetWindowAttributes swa;
	swa.colormap = cmap;
	swa.event_mask =
		ExposureMask | KeyPressMask | KeyReleaseMask |
		ButtonPress | ButtonReleaseMask |
		PointerMotionMask |
		StructureNotifyMask | SubstructureNotifyMask;
	win = XCreateWindow(dpy, root, 0, 0, w, h, 0, vi->depth,
		InputOutput, vi->visual, CWColormap | CWEventMask, &swa);
	set_title();
	glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
	glXMakeCurrent(dpy, win, glc);
}

void X11_wrapper::set_title()
{
	//Set the window title bar.
	XMapWindow(dpy, win);
	XStoreName(dpy, win, "3350 Lab-6");
}

bool X11_wrapper::getXPending()
{
	//See if there are pending events.
	return XPending(dpy);
}

XEvent X11_wrapper::getXNextEvent()
{
	//Get a pending event.
	XEvent e;
	XNextEvent(dpy, &e);
	return e;
}

void X11_wrapper::swapBuffers()
{
	glXSwapBuffers(dpy, win);
}

void X11_wrapper::reshape_window(int width, int height)
{
	//Window has been resized.
	g.xres = width;
	g.yres = height;
	//
	glViewport(0, 0, (GLint)width, (GLint)height);
	glMatrixMode(GL_PROJECTION); glLoadIdentity();
	glMatrixMode(GL_MODELVIEW); glLoadIdentity();
	glOrtho(0, g.xres, 0, g.yres, -1, 1);
}

void X11_wrapper::check_resize(XEvent *e)
{
	//The ConfigureNotify is sent by the
	//server if the window is resized.
	if (e->type != ConfigureNotify)
		return;
	XConfigureEvent xce = e->xconfigure;
	if (xce.width != g.xres || xce.height != g.yres) {
		//Window size did change.
		reshape_window(xce.width, xce.height);
	}
}
//-----------------------------------------------------------------------------

void X11_wrapper::check_mouse(XEvent *e)
{
	static int savex = 0;
	static int savey = 0;

	//Weed out non-mouse events
	if (e->type != ButtonRelease &&
		e->type != ButtonPress &&
		e->type != MotionNotify) {
		//This is not a mouse event that we care about.
		return;
	}
	//
	if (e->type == ButtonRelease) {
		return;
	}
	if (e->type == ButtonPress) {
  //  glPopMatrix();uttonPress) { //line 228 error why?
		if (e->xbutton.button==1) {
			//Left button was pressed.
			int y = g.yres - e->xbutton.y;
            //make particle
            if (n < MAX_PARTICLES) {
                particle[n].pos[0] = e->xbutton.x;
                particle[n].pos[1] = y;
                particle[n].w = 4;
                particle[n].h = 4;
                particle[n].vel[0] = 0.0f;
                particle[n].vel[1] = 0.0f;
                ++n;
            }
			return;
		}
		if (e->xbutton.button==3) {
			//Right button was pressed.
			return;

		}
	}
	if (e->type == MotionNotify) {
		//The mouse moved!
		if (savex != e->xbutton.x || savey != e->xbutton.y) {
			savex = e->xbutton.x;
			savey = e->xbutton.y;
            int y = g.yres - e->xbutton.y;
			//Code placed here will execute whenever the mouse moves.
            for (int i=0; i<2; i++) {
                if (n < MAX_PARTICLES) {
                    particle[n].pos[0] = e->xbutton.x;
                    particle[n].pos[1] = y;
                    particle[n].w = 4;
                    particle[n].h = 4;
                    particle[n].vel[0] = rnd() * 1.0 - 0.5;
                    particle[n].vel[1] = rnd() * 1.0 - 0.5;
                    ++n;
                    return;
            }


            }
                


		}
	}
}

int X11_wrapper::check_keys(XEvent *e)
{
	if (e->type != KeyPress && e->type != KeyRelease)
		return 0;
	int key = XLookupKeysym(&e->xkey, 0);
	if (e->type == KeyPress) {
		switch (key) {
			case XK_a:
				//the 'a' key was pressed
				break;
			case XK_Escape:
				//Escape key was pressed
				return 1;
		}
	}
	return 0;
}

void init_opengl(void)
{
	//OpenGL initialization
	glViewport(0, 0, g.xres, g.yres);
	//Initialize projection matrix
	glMatrixMode(GL_PROJECTION); 
	glLoadIdentity();
	//Set 2D mode (no perspective)
        glOrtho(0, g.xres, 0, g.yres, -1, 1);
	glMatrixMode(GL_MODELVIEW); 
	glLoadIdentity();
    glEnable(GL_TEXTURE_2D);
    initialize_fonts();
	/*Set 2D mode (no perspective)
	glOrtho(0, g.xres, 0, g.yres, -1, 1);*/
	//Set the screen background color
	//Set when screen is cleared
	glClearColor(0.1, 0.1, 0.1, 1.0);
}

const float GRAVITY = -0.1;
//#define rnd() (float)rand() / (float)RAND_MAX
void physics()
{
    for (int i=0; i<n; i++) {
        particle[i].vel[1] += GRAVITY;
        particle[i].last_pos[0] = particle[i].pos[0];
        particle[i].last_pos[1] = particle[i].pos[1];
        particle[i].pos[0] += particle[i].vel[0];
        particle[i].pos[1] += particle[i].vel[1];
    }

    for (int i=0; i<n; i++) {
        for (int j=0; j<5; j++) {
        if (particle[i].pos[1] < (box.pos[1]-(box.offy*j))+box.h &&
           particle[i].pos[1] > (box.pos[1]-(box.offy*j))-box.h &&
           particle[i].pos[0] > (box.pos[0]+(box.offx*j))-box.w &&
           particle[i].pos[0] < (box.pos[0]+(box.offx*j))+box.w) {
            //in a collision state
           // particle[i].pos[0] = particle[i].last_pos[0];
            particle[i].pos[1] = particle[i].last_pos[1];
            particle[i].vel[1] = -particle[i].vel[1] * 0.5;
            particle[i].vel[0] += rnd() * 0.04;
        }
        
         

        if (particle[i].pos[1] < 0) {
            //delete this particle 
            particle[i] = particle[--n];
            
        }
        
        float dx = particle[i].pos[0] - circle.pos[0];
        float dy = particle[i].pos[1] - circle.pos[1];
        float distance = sqrt(dx * dx + dy * dy); 

    //Check for collisions with circle
    if (distance < circle.radius) {
        
        particle[i].pos[0] = particle[i].last_pos[0]; 
        particle[i].pos[1] = particle[i].last_pos[1];

        //slide left or right
        if (dx > 0) {
            particle[i].pos[0] += 1.0f; //slide right
        } else {
            particle[i].pos[0] -= 1.0f; //slide left
        }
    } else {
        
        particle[i].last_pos[0] = particle[i].pos[0];
        particle[i].last_pos[1] = particle[i].pos[1];
    }
    }
    }


}

void render()
{
    static int iteration = 0;
    Rect r;	
	glClear(GL_COLOR_BUFFER_BIT);
    iteration++;
    //


    //draw the other boxes
    for (int i=0; i<5; i++)
    {
        
        glPushMatrix();
        glColor3ub(g.red, 120, 200);
        if (iteration % 3 == 0) {
        if (g.red - 10 > 20) {
           g.red = g.red - 1;
        } else {
            g.red = 255; 
          }
        }  
        //g.red = g.red + 10;
        glTranslatef(number[i].pos[0] + (box.offx*i), number[i].pos[1] - (box.offy*i), 0.0f);
        //number[i].pos[0] = number[i].pos[0] + 1;
        //number[i].pos[1] = number[i].pos[1] - 1;
        glBegin(GL_QUADS);
            glVertex2f(-box.w, -box.h); 
            glVertex2f(-box.w,  box.h);
            glVertex2f( box.w,  box.h);
            glVertex2f( box.w, -box.h);
        glEnd();
        glPopMatrix();

    }

    //draw a particle
    for (int i=0; i<n; i++) {
        if (i % 2 ==1) {
            g.blue = 225;
            g.red2 = 92;
            g.green = 181;
        } else {
            g.blue = 231;
            g.red2 = 93;
            g.green = 151;
        }

    glPushMatrix();
    glColor3ub(g.red2, g.green, g.blue); //red initally 200 //100, 120, 200// 173, 216, 230
   // if (g.blue > 180) {
    //    g.blue = g.blue - 1;
   // } else {
   //     g.blue = 250;
   // }
    glTranslatef(particle[i].pos[0], particle[i].pos[1], 0.0f);
    glBegin(GL_QUADS);
        glVertex2f(-particle[i].w, -particle[i].h); //changed these
        glVertex2f(-particle[i].w,  particle[i].h);
       // glColor3ub(250, 200, 200);
        glVertex2f( particle[i].w,  particle[i].h);
        glVertex2f( particle[i].w, -particle[i].h);
    glEnd();
    glPopMatrix();
    }
        r.bot = 415; //r.bot = g.yres - 100;
            r.left = 57; //r.left = 100;
            r.center = 0; //r.center = 50;

    for (int i=0; i<5; i++) {
 //           r.bot = 420; //r.bot = g.yres - 100;
  //          r.left = 100; //r.left = 100;
  //          r.center = 0; //r.center = 50;
            if (i == 0) {
                ggprint8b(&r, 16, 0xffffffff, "Requirements");
            }
            else if (i == 1) {
                ggprint8b(&r, 16, 0xfffffff, "     Design");
            }
            else if (i == 2) {
                ggprint8b(&r, 16, 0xffffffff, "     Coding");
            }
            else if (i == 3) {
                ggprint8b(&r, 16, 0xffffffff, "     Testing");
            }
            else if (i == 4) {
                ggprint8b(&r, 16, 0xffffffff, " Maintenance");
            }
            r.bot = r.bot - 34;
            r.left = r.left + 60;
            //r.center = r.center + ;
    }

    r.bot = 60;
    r.left = 57;
    r.center = 0;
    ggprint8b(&r, 16, 0xfffffff, "Waterfall Model");
    
    //draw a circle
   glPushMatrix();
  // const int num_sements = 100;
    glColor3ub(128, 0, 128);
  //  glTranslatef(circle.pos[0], circle.pos[1], 0.0f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(circle.pos[0], circle.pos[1]);
    for (int i = 0; i <= circle.num_segments; ++i) {
            float theta = 2.0f * M_PI * float(i) / float(circle.num_segments); //gets angle
            float x = circle.radius * cosf(theta); //calculates x
            float y = circle.radius * sinf(theta); //calculates y
            glVertex2f(circle.pos[0] + x, circle.pos[1] + y); //gives vertex
    

        }


    glEnd();
    glPopMatrix(); 
    






}






