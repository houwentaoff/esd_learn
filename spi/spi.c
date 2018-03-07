/*
 * spi.c
 *
 *  Created on: 2014年6月3日
 *      Author: root
 */

/*
 * SPI testing utility (using spidev driver)
 *
 * Copyright (c) 2007  MontaVista Software, Inc.
 * Copyright (c) 2007  Anton Vorontsov <avorontsov@ru.mvista.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License.
 *
 * Cross-compile with cross-gcc -I/path/to/cross-kernel/include
 */

//#include <stdint.h>
//#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <string.h>
//#include <types.h>

//#include <linux/spi/spidev.h>
#include "spi.h"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#define uint8_t     unsigned char
#define uint32_t    unsigned int
#define uint16_t    unsigned short

static void pabort(const char *s)
{
	perror(s);
	abort();
}

static const char *device = "/dev/spi_u2";

static uint8_t mode = (SPI_MODE_0);
static uint8_t bits = 8;
static uint32_t speed = 8000000;
static uint16_t delay;

 void transfer(int fd)
{
	int ret;
#if 0
	uint8_t tx[] = {
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0x40, 0x00, 0x00, 0x00, 0x00, 0x95,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xDE, 0xAD, 0xBE, 0xEF, 0xBA, 0xAD,
		0xF0, 0x0D,
	};
#else
	uint8_t tx[] = {
		0x10, 0x40, 0xFF,0xFF,
	};
#endif
	uint8_t rx[ARRAY_SIZE(tx)] = {0, };
	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)tx,
		.rx_buf = (unsigned long)rx,
		.len = ARRAY_SIZE(tx),
		.delay_usecs = delay,
		.speed_hz = speed,
		.bits_per_word = bits,
	};

	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	if (ret < 1)
		pabort("can't send spi message");

	for (ret = 0; ret < ARRAY_SIZE(tx); ret++) {
		if (!(ret % 6))
			puts("");
		printf("%.2X ", rx[ret]);
	}
	puts("");
}

static void print_usage(const char *prog)
{
	printf("Usage: %s [-DsbdlHOLC3]\n", prog);
	puts("  -D --device   device to use (default /dev/spidev0.0)\n"
	     "  -s --speed    max speed (Hz)\n"
	     "  -d --delay    delay (usec)\n"
	     "  -b --bpw      bits per word \n"
	     "  -l --loop     loopback\n"
	     "  -H --cpha     clock phase\n"
	     "  -O --cpol     clock polarity\n"
	     "  -L --lsb      least significant bit first\n"
	     "  -C --cs-high  chip select active high\n"
	     "  -3 --3wire    SI/SO signals shared\n");
	exit(1);
}

static void parse_opts(int argc, char *argv[])
{
	while (1) {
		static const struct option lopts[] = {
			{ "device",  1, 0, 'D' },
			{ "speed",   1, 0, 's' },
			{ "delay",   1, 0, 'd' },
			{ "bpw",     1, 0, 'b' },
			{ "loop",    0, 0, 'l' },
			{ "cpha",    0, 0, 'H' },
			{ "cpol",    0, 0, 'O' },
			{ "lsb",     0, 0, 'L' },
			{ "cs-high", 0, 0, 'C' },
			{ "3wire",   0, 0, '3' },
			{ "no-cs",   0, 0, 'N' },
			{ "ready",   0, 0, 'R' },
			{ NULL, 0, 0, 0 },
		};
		int c;

		c = getopt_long(argc, argv, "D:s:d:b:lHOLC3NR", lopts, NULL);

		if (c == -1)
			break;

		switch (c) {
		case 'D':
			device = optarg;
			break;
		case 's':
			speed = atoi(optarg);
			break;
		case 'd':
			delay = atoi(optarg);
			break;
		case 'b':
			bits = atoi(optarg);
			break;
		case 'l':
			mode |= SPI_LOOP;
			break;
		case 'H':
			mode |= SPI_CPHA;
			break;
		case 'O':
			mode |= SPI_CPOL;
			break;
		case 'L':
			mode |= SPI_LSB_FIRST;
			break;
		case 'C':
			mode |= SPI_CS_HIGH;
			break;
		case '3':
			mode |= SPI_3WIRE;
			break;
		case 'N':
			mode |= SPI_NO_CS;
			break;
		case 'R':
			mode |= SPI_READY;
			break;
		default:
			print_usage(argv[0]);
			break;
		}
	}
}
int spi_init(void)
{
	int fd = 0;
	int ret = 0;

	fd = open(device, O_RDWR);
	if (fd < 0)
	{
		printf("can't open device");
		return fd;
	}
	/* spi mode */
	ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
	if (ret == -1)
	{
		printf("can't set spi mode");
		return ret;
	}


	/* bits per word */
	ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
	if (ret == -1)
	{
		printf("can't set bits per word");
		return ret;
	}

	/* max speed hz */
	ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (ret == -1)
	{
		printf("can't set max speed hz");
		return ret;
	}

	//printf("spi mode: %d\n", mode);
	//printf("bits per word: %d\n", bits);
	//printf("max speed: %d Hz (%d KHz)\n", speed, speed/1000);

	return fd;
}

int spi_exit(int fd)
{
	int state = 0;

	state = close(fd);
	if (state < 0)
	{
		printf("can't close device");
	}

	return state;
}

char spi_write(int fd,char *data, int len)
{
	int ret = 0;
	int i = 0;

	uint8_t rx[128];

	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)data,
		.rx_buf = (unsigned long)rx,
		.len = len,
		.delay_usecs = delay,
		.speed_hz = speed,
		.bits_per_word = bits,
	};

	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	if (ret < 1)
	{
		printf("can't send spi message");
		return -1;
	}
#if 0
	for (i = 0; i < len; i++) {
		if (!(i % 8))
			puts("");
		printf("%.2X ", rx[i]);
	}
	puts("");
#endif
	return rx[len-1];
}
char spi_write_byte(int fd,char data)
{
	char temp = 0;
	temp = spi_write(fd,(char*)&data,1);

	return temp;
}

int spi_read(int fd,char *data,int len)
{
	int ret;
	int i = 0;
	uint8_t tx[128];
	memset(tx,0xFF,sizeof(tx));
	if(len > sizeof(tx))
	{
		printf("spi_read len too long!\n");
		return -1;
	}

	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)tx,
		.rx_buf = (unsigned long)data,
		.len = len,
		.delay_usecs = delay,
		.speed_hz = speed,
		.bits_per_word = bits,
	};

	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	if (ret < 1)
	{
		pabort("can't send spi message");
		return -1;
	}
#if 0
	for (i = 0; i < len; i++) {

		if (!(i % 8))
			puts("");
		printf("%.2X ", data[i]);
	}
	puts("");
#endif
	return len;
}
char spi_read_byte(int fd)
{
	char data = 0xff;
	spi_read(fd,(char*)&data,1);
	return data;
}
char spi_read_bytes_by_reg(int fd,char reg,char *data,int len)
{
	int ret;
	int i = 0;
	uint8_t tx[128];
	uint8_t rx[128];
	memset(tx,0xFF,sizeof(tx));
	tx[0] = reg;
	if((len+1) > sizeof(tx))
	{
		printf("spi_read len too long!\n");
		return -1;
	}

	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)tx,
		.rx_buf = (unsigned long)rx,
		.len = len+1,
		.delay_usecs = delay,
		.speed_hz = speed,
		.bits_per_word = bits,
	};

	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	if (ret < 1)
	{
		pabort("can't send spi message");
		return -1;
	}
	memcpy(data,&rx[1],len);
#if 0
	for (i = 0; i < len; i++) {

		if (!(i % 8))
			puts("");
		printf("%.2X ", data[i]);
	}
	puts("");
#endif
	return len;
}
//int main_test(int argc, char *argv[])
int spi_test(int fd)
{
	int ret = 0;
	//int fd;

	//parse_opts(argc, argv);

	//fd = open(device, O_RDWR);
	//if (fd < 0)
	//	pabort("can't open device");

	/*
	 * spi mode
	 */
	ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
	if (ret == -1)
		pabort("can't set spi mode");

	ret = ioctl(fd, SPI_IOC_RD_MODE, &mode);
	if (ret == -1)
		pabort("can't get spi mode");

	/*
	 * bits per word
	 */
	ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
	if (ret == -1)
		pabort("can't set bits per word");

	ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
	if (ret == -1)
		pabort("can't get bits per word");

	/*
	 * max speed hz
	 */
	ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		pabort("can't set max speed hz");

	ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		pabort("can't get max speed hz");

	printf("spi mode: %d\n", mode);
	printf("bits per word: %d\n", bits);
	printf("max speed: %d Hz (%d KHz)\n", speed, speed/1000);
while(1)
{
	transfer(fd);
	sleep(3);
}
	//close(fd);

	return ret;
}
