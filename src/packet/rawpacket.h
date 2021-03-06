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

#ifndef _RAWPACKET_H
#define _RAWPACKET_H

#include "comm.h"
#include "packet/packet.h"
#include "util/endian.h"


typedef struct
{
    packet          base;
    buffer_writer   writer;
}rawpacket;

rawpacket *rawpacket_new(uint32_t size);

//will add reference count of b
rawpacket *rawpacket_new_by_buffer(bytebuffer *b,uint32_t spos);

static inline void rawpacket_expand(rawpacket *raw,uint32_t newsize)
{
	bytebuffer *newbuff,*oldbuff;
    newsize = size_of_pow2(newsize);
    if(newsize < MIN_BUFFER_SIZE) newsize = MIN_BUFFER_SIZE;
    newbuff = bytebuffer_new(newsize);
   	oldbuff = cast(packet*,raw)->head;
   	memcpy(newbuff->data,oldbuff->data,oldbuff->size);
   	newbuff->size = oldbuff->size;
   	refobj_dec(cast(refobj*,oldbuff));
    cast(packet*,raw)->head = newbuff;
    //set writer to the end
    buffer_writer_init(&raw->writer,newbuff,cast(packet*,raw)->len_packet);
}

static inline void rawpacket_copy_on_write(rawpacket *raw)
{
	rawpacket_expand(raw,cast(packet*,raw)->len_packet);
}

static inline void rawpacket_append(rawpacket *raw,void *_,uint32_t size)
{
    uint32_t packet_len,new_size;
	char *in = cast(char*,_);
    if(!raw->writer.cur)
        rawpacket_copy_on_write(raw);
    else{
		packet_len = cast(packet*,raw)->len_packet;
    	new_size = packet_len + size;
    	assert(new_size >= packet_len);
    	if(new_size <= packet_len) return;
    	if(new_size > cast(packet*,raw)->head->cap)
    		rawpacket_expand(raw,new_size);
    	buffer_write(&raw->writer,in,size);
    	cast(packet*,raw)->len_packet = new_size;
    }	
}

static inline void *rawpacket_data(rawpacket *raw,uint32_t *size)
{
	if(size) *size = cast(packet*,raw)->len_packet;
	return cast(void*,cast(packet*,raw)->head->data + cast(packet*,raw)->spos);
}

#endif