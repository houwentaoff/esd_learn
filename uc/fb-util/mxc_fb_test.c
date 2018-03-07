/**
 *imx6 framebuffer test
*/
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <math.h>
#include <string.h>
#include <malloc.h>
#include <linux/fb.h>


int fdfb = 0;
unsigned short *fb;
int g_fb_size = 0;


int fb_color_test(struct fb_var_screeninfo *screen_info){
	int i;
	unsigned int screensize;
	screensize = screen_info->xres*screen_info->yres*screen_info->bits_per_pixel/8;
	printf("Fill in RGB size = 0x%08x\n",screensize);
	for(i=0; i<screensize/4; i++)
		((unsigned int*)fb)[i] = 0x00FF0000;//R
	sleep(3);
	for(i=0; i<screensize/4; i++)
		((unsigned int*)fb)[i] = 0x0000FF00;//G
	sleep(3);
	for(i=0; i<screensize/4; i++)
		((unsigned int*)fb)[i] = 0x000000FF;//B
	sleep(3);

	return 0;
}


int main(int argc , char **argv){
	int retval = -1;
	//struct mxcfb_gbl_alpha gbl_alpha;
	struct fb_var_screeninfo screen_info;//用户可修改的显示器参数
	unsigned int screensize = 0;
	
	if(argc < 3){
		printf("Usage:\n%s /dev/graphics/fbx [0|1|2|3]\n",argv[0]);
		printf("[0]:fb color test\n");
		return 0;
	}
	fdfb=open(argv[1], O_RDWR, 0);
	if(fdfb < 0){
		printf("unable to open %s\n",argv[1]);
		return -1;
	}
	printf("-----------open [%s]----------\n",argv[1]);

	if(retval = ioctl(fdfb, FBIOGET_VSCREENINFO, &screen_info) < 0){
		printf("Error:reading variable information\n");
		return -1;
	}
	printf("xres*yres=[%dx%d] bpp=%d\n", screen_info.xres, screen_info.yres,
			screen_info.bits_per_pixel);
	
	//计算缓冲区大小
	g_fb_size = screen_info.xres*screen_info.yres_virtual*screen_info.bits_per_pixel/8;
	printf("\n screen size = [%u]\n", g_fb_size);
	//map the device to memory
	fb = (unsigned short *)mmap(0, g_fb_size, PROT_READ|PROT_WRITE,
				MAP_SHARED, fdfb, 0);

	if((int)fb <=0){
		printf("Error: failed to map framebuffer device  to memory\n");
		return -1;
	}
	printf("-----The framebuffer device was mapped to memory successfully\n");

	switch(atoi(argv[2])){
		case 0:
			fb_color_test(&screen_info);
	}

	munmap(fb, g_fb_size);
	close(fdfb);
	return 0;
}
