#include "socket/socket_helper.h"


int32_t easy_listen(int32_t fd,sockaddr_ *server)
{
	errno = 0;
	if(easy_bind(fd,server) != 0)
		 return -errno;
	if(listen(fd,SOMAXCONN) != 0)
		return -errno;
	return 0;
}

int32_t easy_connect(int32_t fd,sockaddr_ *server,sockaddr_ *local)
{
	int32_t ret;
	errno = 0;	
	if(local && 0 != easy_bind(fd,local))
		return -errno;
	ret = connect(fd,cast(struct sockaddr*,server),sizeof(*server));
	return ret == 0 ? ret : -errno;
}

#ifdef _CHUCKLUA

static int32_t lua_easy_connect(lua_State *L)
{
	int32_t fd;
	const char *ip;
	uint16_t port;

	if(lua_type(L,1) != LUA_TNUMBER)
		return luaL_error(L,"invaild arg1");

	if(lua_type(L,2) != LUA_TSTRING)
		return luaL_error(L,"invaild arg2");

	if(lua_type(L,3) != LUA_TNUMBER)
		return luaL_error(L,"invaild arg3");

	fd   = lua_tonumber(L,1);
	ip   = lua_tostring(L,2);
	port = lua_tonumber(L,3);

	sockaddr_ addr;
	if(0 != easy_sockaddr_ip4(&addr,ip,port))
		return 	luaL_error(L,"invaild address or port");

	lua_pushinteger(L,easy_connect(fd,&addr,NULL));
	return 1;				
}

static int32_t lua_socket(lua_State *L)
{
	int32_t family;
	int32_t type;
	int32_t protocol;

	if(lua_type(L,1) != LUA_TNUMBER)
		return luaL_error(L,"invaild arg1");

	if(lua_type(L,2) != LUA_TNUMBER)
		return luaL_error(L,"invaild arg2");

	if(lua_type(L,3) != LUA_TNUMBER)
		return luaL_error(L,"invaild arg3");

	family   = lua_tonumber(L,1);
	type   	 = lua_tonumber(L,2);
	protocol = lua_tonumber(L,3);	

	lua_pushinteger(L,socket(family,type,protocol));
	return 1;
}

static int32_t lua_easy_listen(lua_State *L)
{
	int32_t fd;
	const char *ip;
	uint16_t port;
	sockaddr_ addr;
	if(lua_type(L,1) != LUA_TNUMBER)
		return luaL_error(L,"invaild arg1");

	if(lua_type(L,2) != LUA_TSTRING)
		return luaL_error(L,"invaild arg2");

	if(lua_type(L,3) != LUA_TNUMBER)
		return luaL_error(L,"invaild arg3");

	fd   = lua_tonumber(L,1);
	ip   = lua_tostring(L,2);
	port = lua_tonumber(L,3);

	if(0 != easy_sockaddr_ip4(&addr,ip,port))
		return 	luaL_error(L,"invaild address or port");

	lua_pushinteger(L,easy_listen(fd,&addr));
	return 1;
}

static int32_t lua_easy_noblock(lua_State *L)
{
	int32_t fd;
	int32_t yes;
	if(lua_type(L,1) != LUA_TNUMBER)
		return luaL_error(L,"invaild arg1");

	if(lua_type(L,2) != LUA_TNUMBER)
		return luaL_error(L,"invaild arg2");

	fd   = lua_tonumber(L,1);
	yes = lua_tonumber(L,2);	

	lua_pushinteger(L,easy_noblock(fd,yes));
	return 1;
}

static int32_t lua_easy_addr_reuse(lua_State *L)
{
	int32_t fd;
	int32_t yes;
	if(lua_type(L,1) != LUA_TNUMBER)
		return luaL_error(L,"invaild arg1");

	if(lua_type(L,2) != LUA_TNUMBER)
		return luaL_error(L,"invaild arg2");

	fd   = lua_tonumber(L,1);
	yes = lua_tonumber(L,2);	

	lua_pushinteger(L,easy_addr_reuse(fd,yes));
	return 1;
}

static int32_t lua_close_socket(lua_State *L)
{
	int32_t fd;
	if(lua_type(L,1) != LUA_TNUMBER)
		return luaL_error(L,"invaild arg1");
	fd = lua_tonumber(L,1);
	close(fd);		
	return 0;
}

static int32_t lua_gethostbyname_ipv4(lua_State *L)
{
	int     i,c;
	char    ip[32];
	int     h_err;
    char    buf[8192];
    struct hostent ret, *result;	
	if(lua_type(L,1) != LUA_TSTRING)
		return luaL_error(L,"invaild arg1");
    if(gethostbyname_r(lua_tostring(L,1), &ret, buf, 8192, &result, &h_err) != 0)
        return 0;
    lua_newtable(L);
    for (i = 0,c = 1; result->h_addr_list[i] != NULL; i++)
    {	
    	if(inet_ntop(AF_INET, result->h_addr_list[i],ip, 32) != NULL)
    	{
    		lua_pushstring(L,ip);
    		lua_rawseti(L,-2,c++);
    	}
    }
	return 1;
}

#define SET_CONST(L,N) do{\
		lua_pushstring(L, #N);\
		lua_pushinteger(L, N);\
		lua_settable(L, -3);\
}while(0)

#define SET_FUNCTION(L,NAME,FUNC) do{\
	lua_pushstring(L,NAME);\
	lua_pushcfunction(L,FUNC);\
	lua_settable(L, -3);\
}while(0)

void reg_socket_helper(lua_State *L)
{
	lua_newtable(L);
	SET_CONST(L,AF_INET);
	SET_CONST(L,SOCK_STREAM);
	SET_CONST(L,IPPROTO_TCP);

	SET_FUNCTION(L,"socket",lua_socket);
	SET_FUNCTION(L,"connect",lua_easy_connect);
	SET_FUNCTION(L,"listen",lua_easy_listen);
	SET_FUNCTION(L,"noblock",lua_easy_noblock);
	SET_FUNCTION(L,"addr_reuse",lua_easy_addr_reuse);
	SET_FUNCTION(L,"close",lua_close_socket);
	SET_FUNCTION(L,"gethostbyname_ipv4",lua_gethostbyname_ipv4);
}

#endif