/*
    Copyright (C) <2015>  <sniperHW@163.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __SIGNALER__H
#define __SIGNALER__H

#include "comm.h"
#include "lua/lua_util.h"
//void (*)(struct signaler *,int32_t,void *ud)    

typedef struct signaler{
    handle  base;
    int32_t signum;
    int32_t notifyfd;  
    void   *ud;
    void    (*callback)(struct signaler *,int32_t,void *ud);
#ifdef _CHUCKLUA    
    luaRef  luacallback;
#endif    
}signaler;

#ifdef _CHUCKLUA

void reg_luasignaler(lua_State *L); 

#else

signaler *signaler_new(int32_t signum,void *ud);

void signaler_del(signaler*);

#endif


#endif