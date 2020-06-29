#include<netdb.h>
#include<unistd.h>
#include<signal.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<fcntl.h>
#include<string.h>
#include<arpa/inet.h>
#include<stdio.h>
#include<stdlib.h>
struct package
{
    uint64_t data_size;
    char data[4096];
};

int main(int argc,char *argv[])
{
	uint8_t loop_sync=~0;
	int8_t temp[sizeof(struct package)];
	struct package package1;
	int i;
	uint64_t size;
	uint64_t size2,j;
	struct addrinfo hints,*res;
	unsigned int sockfd,filefd,new_fd;
	struct sockaddr_storage their_addr;
	if(argc<4)
	{
			fprintf(stderr,"usege: %s listen-address listen-port filename\n",argv[0]);
			exit(-1);
	}
	memset(&hints,0,sizeof(hints));
	hints.ai_family=AF_INET;
	hints.ai_socktype=SOCK_STREAM;
	i=getaddrinfo(argv[1],argv[2],&hints,&res);
	if(i!=0)
	{
		fprintf(stderr,"getaddrinfo: %s\n",gai_strerror(i));
		exit(-1);
	}
	if(!(sockfd=socket(res->ai_family,res->ai_socktype,res->ai_protocol)))
	{
		perror("get sockfd error\n");
		exit(-1);
	}
	if(bind(sockfd,res->ai_addr,res->ai_addrlen))
	{
		perror("bind error\n");
		exit(-1);
	}
	if((listen(sockfd,5)))
	{
		perror("listen error\n");
		exit(-1);
	}
	freeaddrinfo(res);
	socklen_t addr_size;
	char a[4096];
	addr_size=sizeof(their_addr);
	pid_t pid;
	filefd=open(argv[3],O_WRONLY|O_CREAT|O_TRUNC,S_IRUSR|S_IWUSR);
	if(filefd<0)
	{
		perror("open file error\n");
		exit(-1);
	}
	char check[]="OK";
	new_fd=accept(sockfd,(struct sockaddr *)&their_addr,&addr_size);
	j=read(new_fd,&size,sizeof(size));
	while(j<sizeof(size))
		j=j+read(new_fd,(uint8_t *)(&size)[j],sizeof(size)-j);
	for(;size>0;)
	{
		i=read(new_fd,temp,sizeof(temp));
		while(i<sizeof(temp))
		{
			i+=read(new_fd,&(temp[i]),sizeof(temp)-i);
		}
		memcpy(&package1,temp,sizeof(struct package));
		write(filefd,&package1.data,package1.data_size);
		size=size-package1.data_size;
		loop_sync--;
		if(loop_sync==0)
		{
			j=write(new_fd,check,3);
			while(j<3)
				j=j+write(new_fd,&check[j],3-j);
			loop_sync=~loop_sync;
		}
	}
	close(new_fd);
	close(filefd);
	close(sockfd);
	return 0;
}
