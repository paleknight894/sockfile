#include<string.h>
#include<stdio.h>
#include<fcntl.h>
#include<unistd.h>
#include<netdb.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/ioctl.h>
#include<stdlib.h>
#include<linux/fs.h>
int main(int argc,char *argv[])
{
	uint64_t size;
	uint64_t i,size2,b;
	unsigned int sockfd,filefd;
	struct addrinfo hints,*res;
	struct stat statbuf;
	if(argc<4)
	{
		fprintf(stderr,"usege: %s target-addres target-port filename \n",argv[0]);
		exit(-1);
	}
	char a[4096];
	memset(&hints,0,sizeof(hints));
	hints.ai_family=AF_INET;
	hints.ai_socktype=SOCK_STREAM;
	i=getaddrinfo(argv[1],argv[2],&hints,&res);
	if(i!=0)
	{
		fprintf(stderr,"getaddrinfo: %s\n",gai_strerror(i));
	}
	if(!(sockfd=socket(res->ai_family,res->ai_socktype,res->ai_protocol)))
	{
		perror("get sockfd error\n");
		exit(-1);
	}
	if(connect(sockfd,res->ai_addr,res->ai_addrlen))
	{
		perror("connect error\n");
		exit(-1);
	}
	freeaddrinfo(res);
	filefd=open(argv[3],O_RDONLY);
	if(filefd<0)
	{
		perror("open file error\n");
	}
	stat(argv[3],&statbuf);
	if((statbuf.st_mode&S_IFMT)==S_IFBLK)
		{
			ioctl(filefd,BLKGETSIZE64,&size);
		}
	else
		size=statbuf.st_size;
	char check[3];
	b=write(sockfd,&size,sizeof(size));
	while(b<sizeof(size))
	{
		b=b+write(sockfd,(uint8_t *)(&size)[b],sizeof(size)-b);
	}
	for(;size>0;)
	{
		i=read(filefd,a,4096);
		size2=i;
		b=write(sockfd,&size2,sizeof(size2));
		while(b<sizeof(size2))
		{
			b=b+write(sockfd,(uint8_t *)(&size2)[b],sizeof(size2)-b);
		}
		size=size-i;
		b=write(sockfd,a,i);
		while(b<i)
		{
			b=b+write(sockfd,&a[b],i-b);
		}
		b=read(sockfd,check,3);
		while(b<3)
		{
			b+=read(sockfd,&check[b],3-b);
		}
	}
	close(filefd);
	close(sockfd);
	return 0;
}
