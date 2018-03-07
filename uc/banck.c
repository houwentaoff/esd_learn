/*************************************************************************
	> File Name: banck.c
	> Author: shuixianbing
	> Mail: shui6666@126.com 
	> Created Time: 2014年10月21日 星期二 11时47分04秒
 ************************************************************************/

#include<stdio.h>
#include<unistd.h>
#include<fcntl.h>
#include<stdlib.h>
#include<string.h>
int main(void)
{
	char name[20] ={};
	printf("name\n");
	scanf("%s", name);
	char gender[10] = "male";
	int age;
	double salary;
	printf("输入年龄\n");
	scanf("%d", &age);
	printf("输入存款金额\n");
	scanf("%lf", &salary);

	int fd = open("emp.txt", O_WRONLY | O_CREAT |O_TRUNC);
	write(fd, name, sizeof(name));
	write(fd, gender, sizeof(gender));
	write(fd, &age, sizeof(age));
	write(fd, &salary, sizeof(salary));
	close(fd);

	fd = open("emp.txt", O_RDONLY);
	char buf[100] = {};
	read(fd, buf, 20);
	printf(" %s", buf);
	memset(buf, 0, sizeof(buf));
	read(fd, gender, 10);
	printf(" %s", gender);
	read(fd, &age, sizeof(int));
	printf(" %d", age);
	read(fd, &salary, sizeof(double));

	printf(" %lf", salary);
	close(fd);


}
