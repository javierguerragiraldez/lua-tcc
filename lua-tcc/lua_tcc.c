#include "lua.h"
#include "lauxlib.h"
#include "libtcc.h"

#define RELEASE_MEM 0			/* there's a bug somewhere */

static const char TCCStateType[] = "__TCCStateType__";

typedef struct {
	TCCState *s;
} tcc_userdata;

static tcc_userdata *check_tcc (lua_State *L, int index) {
	tcc_userdata *tcc = (tcc_userdata *)luaL_checkudata(L, index, TCCStateType);
	luaL_argcheck(L, tcc, index, "TCC state expected");
	return tcc;
}


/*
 * tcc.new ()
 */
static tcc_userdata * tccnew (lua_State *L)
{
	tcc_userdata *tcc = (tcc_userdata *)lua_newuserdata (L, sizeof (tcc_userdata));
	/*printf ("allocated tcc=%X\n", tcc);*/

	tcc->s = tcc_new ();
	if (!tcc->s)
		luaL_error (L, "Can't create tcc state");
	
	/*printf ("allocated tcc->s=%X\n", tcc->s);*/
	
	tcc_set_output_type (tcc->s, TCC_OUTPUT_MEMORY);
	
	luaL_getmetatable (L, TCCStateType);
	lua_setmetatable (L, -2);
	return tcc;
}

static void add_library (lua_State *L, TCCState *s, int index)
{
	const char *libname = luaL_checkstring (L, index);
	if (tcc_add_library (s, libname))
		luaL_error (L, "can't load library %s", libname);
}

static void push_symbol (lua_State *L, int tccindex, int nameindex)
{
	unsigned long sym;
	lua_CFunction f;
	tcc_userdata *tcc = check_tcc (L, tccindex);
	const char *funcname = luaL_checkstring (L, nameindex);
	
	if (tcc_get_symbol (tcc->s, &sym, funcname) < 0)
		luaL_error (L, "can't get symbol %s", funcname);
	f = (lua_CFunction)sym;
	
	lua_pushvalue (L, tccindex);
	lua_pushcclosure (L, f, 1);
}

/*
 * tcc.compile (code, funcname [, libs])
 */
static int compile (lua_State *L)
{
	const char *code = luaL_checkstring (L, 1);
	tcc_userdata *tcc;

	/* check params */
	if (!lua_isstring (L, 2) && !lua_istable (L, 2))
		luaL_error (L, "arg 2: string or table expected, got %s", lua_typename (L, lua_type (L,2)));
	
	if (lua_gettop (L) > 2 && !lua_isstring (L, 3) && !lua_istable (L, 3))
		luaL_error (L, "arg 3: optional string or table expected, got %s", lua_typename (L, lua_type (L,3)));
	
	/* TCC state is at top of stack */
	tcc = tccnew (L);
	
	/* add libs */
	if (lua_istable (L, 3)) {
		lua_pushnil (L);
		while (lua_next (L, 3)) {
			add_library (L, tcc->s, -1);
			lua_pop (L, 1);
		}
	} else if (lua_isstring (L, 3)) {
		add_library (L, tcc->s, 3);
	}
	
	/* compile & link */
	if (tcc_compile_string (tcc->s, code))
		luaL_error (L, "can't compile %s", code);
	
	if (tcc_relocate (tcc->s))
		luaL_error (L, "bad tcc_relocate");

	/* get named symbol(s) as cfunction (s) */
	if (lua_istable (L, 2)) {
		lua_newtable (L);
		lua_pushnil (L);
		while (lua_next (L, 2)) {
			push_symbol (L, -4, -1);
			lua_settable (L, -4);
		}
		
	} else if (lua_isstring (L, 2)) {
		push_symbol (L, -1, 2);
		
	} else {
		luaL_error (L, "arg 2: string or table expected, got %s", lua_typename (L, lua_type (L,2)));
	}
	
	return 1;
}



/*
 * __gc
 */
static int gc (lua_State *L)
{
#if RELEASE_MEM
	tcc_userdata *tcc = check_tcc (L, 1);
	printf ("gcc (tcc=%X)\n", tcc);
	if (tcc && tcc->s) {
		printf ("deleting tcc->s=%X\n", tcc->s);
		tcc_delete (tcc->s);
		tcc->s = NULL;
	}
	printf ("fin\n");
#endif

	return 0;
}

static const struct luaL_reg tcc_meths [] = {
	{"__gc", gc},
	{NULL, NULL}
};

static const struct luaL_reg tcc_funcs[] = {
	{"compile", compile},
	{NULL, NULL}
};

/*
** Assumes the table is on top of the stack.
*/
static void set_info (lua_State *L) {
	lua_pushliteral (L, "_COPYRIGHT");
	lua_pushliteral (L, "Copyright (C) 2006 Javier Guerra");
	lua_settable (L, -3);
	lua_pushliteral (L, "_DESCRIPTION");
	lua_pushliteral (L, "TCC runtime C compiler");
	lua_settable (L, -3);
	lua_pushliteral (L, "_NAME");
	lua_pushliteral (L, "lua_tcc");
	lua_settable (L, -3);
	lua_pushliteral (L, "_VERSION");
	lua_pushliteral (L, "0.1");
	lua_settable (L, -3);
}

int luaopen_lua_tcc (lua_State *L);
int luaopen_lua_tcc (lua_State *L)
{
	luaL_newmetatable(L, TCCStateType);
	lua_pushliteral(L, "__index");
	lua_pushvalue(L, -2);
	lua_rawset(L, -3);
	luaL_openlib (L, NULL, tcc_meths, 0);
	luaL_openlib (L, "tcc", tcc_funcs, 0);
	set_info (L);

	return 1;
}
