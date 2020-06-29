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
struct package
{
	uint64_t data_size;
	char data[4096];
};
int main(int argc,char *argv[])
{
	uint8_t temp[sizeof(struct package)],loop_sync=~0;
	uint64_t size;
	uint64_t i;
	struct package package1;
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
	i=write(sockfd,&size,sizeof(size));
	while(i<sizeof(size))
	{
		i=i+write(sockfd,(uint8_t *)(&size)[i],sizeof(size)-i);
	}
	for(;size>0;)
	{
		package1.data_size=read(filefd,package1.data,4096);
		memcpy(temp,&package1,sizeof(struct package));
		i=write(sockfd,temp,sizeof(temp));
		while(i<sizeof(temp))
		{
			i=i+write(sockfd,&(temp[i]),sizeof(temp)-i);
		}
		size-=package1.data_size;
		loop_sync--;
		if(loop_sync==0)
		{
			i=read(sockfd,check,3);
			while(i<3)
			{
				i+=read(sockfd,&check[i],3-i);
			}
			loop_sync=~0;
		}
	}
	close(filefd);
	close(sockfd);
	return 0;
}
