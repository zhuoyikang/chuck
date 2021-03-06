#include "chuck.h"


int      client_count = 0;
double   totalbytes   = 0;
engine  *e;

int32_t timer_callback(uint32_t event,uint64_t _,void *ud){
	if(event == TEVENT_TIMEOUT){
		printf("client_count:%d,totalbytes:%f MB/s\n",client_count,totalbytes/1024/1024);
		totalbytes = 0.0;
	}
	return 0;
}

static void snd_fnish_cb(connection *c)
{
	connection_close(c);
	--client_count;	
}

static void on_packet(connection *c,packet *p,int32_t error){
	if(p){
		printf("on_packet\n");
		//engine_del(e);
		connection_send(c,make_writepacket(p),snd_fnish_cb);
	}else{
		printf("here,%d\n",error);
		connection_close(c);
		--client_count;
	}
}

static void on_connection(acceptor *a,int32_t fd,sockaddr_ *_,void *ud,int32_t err){
	if(err == EENGCLOSE){
		acceptor_del(a);
		return;
	}	
	printf("on_connection\n");
	engine *e = (engine*)ud;
	connection *c = connection_new(fd,64,NULL);
	//SLEEPMS(2000);
	//connection_set_recvtimeout(c,2000);
	engine_associate(e,c,on_packet);
	++client_count;
}


int main(int argc,char **argv){
	signal(SIGPIPE,SIG_IGN);
	e = engine_new();
	sockaddr_ server;
	if(0 != easy_sockaddr_ip4(&server,argv[1],atoi(argv[2]))){
		printf("invaild address:%s\n",argv[1]);
	}
	int32_t fd = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	easy_addr_reuse(fd,1);
	if(0 == easy_listen(fd,&server)){
		acceptor *a = acceptor_new(fd,e);
		engine_associate(e,a,on_connection);
		engine_regtimer(e,1000,timer_callback,NULL);
		if(EENGCLOSE != engine_run(e))
			engine_del(e);
	}else{
		close(fd);
		printf("server start error\n");
	}
	return 0;
}



