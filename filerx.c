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
int main(int argc,char *argv[])
{
	int i;
	uint64_t size;
	uint64_t size2,j;
	uint8_t mask=0;
	mask=~mask;
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
	filefd=open(argv[3],O_WRONLY|O_CREAT|O_TRUNC,mask);
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
		j=read(new_fd,&size2,sizeof(size2));
		while(j<sizeof(size2))
			j=j+read(new_fd,(uint8_t *)(&size2)[j],sizeof(size2)-j);
		i=read(new_fd,a,size2);
		write(filefd,a,i);
		if(i<size2)
		{
			while(i<size2)
			{
				j=read(new_fd,a,size2-i);
				write(filefd,a,j);
				i=i+j;
			}
		}
//		printf("i=%d size2=%ld\n",i,size2);
		if(i<=0)
		{
			perror("got early EOF\n");
			exit(-2);
		}
		size=size-i;
		j=write(new_fd,check,3);
		while(j<3)
			j=j+write(new_fd,&check[j],3-j);
	}
	close(new_fd);
	close(filefd);
	close(sockfd);
	return 0;
}
