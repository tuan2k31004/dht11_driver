#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main(){
	int fd = open("/dev/dht11_driver",O_RDONLY);
	if(fd<0){
		printf("cannot open\n");
		return -1;
	}

	unsigned char data[5];

	ssize_t r = read(fd,data,sizeof(data));

	printf("hum: %d,%d, tem: %d,%d\n",data[0],data[1],data[2],data[3]);
	printf("sum:%d\n",data[4]);
       	close(fd);
	return 0;
}


