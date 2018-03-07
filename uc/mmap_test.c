/*************************************************************************
	> File Name: mmap_test.c
	> Author: shuixianbing
	> Mail: shui6666@126.com 
	> Created Time: 2016年06月15日 星期三 13时25分01秒
 ************************************************************************/

#include<stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>

int main(void)
{
	int fd = 0;
	char *start;
	char buf[100];

	fd = open("testfile", O_RDWR);

	start = mmap(NULL, 100, PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0);
	strcpy(buf, start);
	printf("buf = %s\n", buf);

	strcpy(start, "Hello I am a linux developer");



	munmap(start, 100);
	close(fd);
	return 0;
}
