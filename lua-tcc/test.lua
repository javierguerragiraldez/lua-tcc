require "lua_tcc"

m = tcc.compile ([[

	#include "lua.h"

	typedef struct {
		void *s;
	} tcc_userdata;
	
	void hi (void) {
		printf ("hi!\n");
	}
	
	void bye (void) {
		printf ("bye..!\n");
	}
	
	int count (lua_State *L) {
		printf ("hay %d parametros\n", lua_gettop (L));
		lua_pushnumber (L, lua_gettop (L));
		return 1;
	}
	
	int tell (lua_State *L) {
		tcc_userdata *p = lua_touserdata (L, lua_upvalueindex(1));
		lua_pushnumber (L, lua_upvalueindex(1));
		lua_pushlightuserdata (L, p);
		lua_pushlightuserdata (L, p->s);
		return 3;
	}

]], {"hi", "bye", "count", "tell"})

x = tcc.compile ([[
	#include <X11/Xlib.h>
	#include <raptor.h>
	
	#include "lua.h"
	
	int main(int argc, char **argv)
	{
		Display *display;
		Screen *screen;
	
		raptor_init ();
	
		display = XOpenDisplay("");
		if (!display) {
			fprintf(stderr, "Could not open X11 display\n");
			exit(1);
		}
		printf("X11 display opened.\n");
		screen = XScreenOfDisplay(display, 0);
		printf("width = %d\nheight = %d\ndepth = %d\n", 
			screen->width, 
			screen->height,
			screen->root_depth);
		XCloseDisplay(display);
		return 0;
	}
	
	typedef struct {
		void *s;
	} tcc_userdata;
	
	int tell (lua_State *L) {
		tcc_userdata *p = lua_touserdata (L, lua_upvalueindex(1));
		lua_pushnumber (L, lua_upvalueindex(1));
		lua_pushlightuserdata (L, p);
		lua_pushlightuserdata (L, p->s);
		return 3;
	}
	
]], {"main", "tell"}, {"X11", "raptor"})

y = tcc.compile ([[
	#include "lua.h"

	typedef struct {
		void *s;
	} tcc_userdata;
	
	int tell (lua_State *L) {
		tcc_userdata *p = lua_touserdata (L, lua_upvalueindex(1));
		lua_pushnumber (L, lua_upvalueindex(1));
		lua_pushlightuserdata (L, p);
		lua_pushlightuserdata (L, p->s);
		return 3;
	}
	]], "tell")