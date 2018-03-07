#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

int main(int argc, char **argv)
{
	int ret;
	ret = open("/dev/ed_rec", O_RDWR);
	if(ret < 0)
		printf("open fail\n");
	else
	printf("open ok\n");

	return 0;
}
