
/*
 * Symbol SE4500 imaging module driver
 *
 * Copyright (C) 2012 Motorola solutions
 * Copyright (C) 2012 MM Solutions
 *
 * Author Stanimir Varbanov <svarbanov@mm-sol.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation version 2.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 * kind, whether express or implied; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * Raghav - Restructed se4500 to use se4500_command to remove redudancy.
 * Read back AutoLowPower to actually enter into Low power mode.
 */

#include <linux/videodev2.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <linux/log2.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <media/v4l2-subdev.h>
#include <media/v4l2-chip-ident.h>
#include <media/soc_camera.h>
#include <linux/clk.h>
#include <linux/miscdevice.h>
#include <linux/fsl_devices.h>
#include <media/v4l2-chip-ident.h>
#include <media/v4l2-int-device.h>
#include "mxc_v4l2_capture.h"
#include "fsl_csi.h"

#define SE4500_GPIO_PWN           IMX_GPIO_NR(3, 26)
#define SE4500_SIZE_WVGA			0
#define SE4500_I2C_DEVICE_ADDR		0x5c
#define SE4500_ACQ					0x58
#define SE4500_ACQ_ON				0x01
#define SE4500_ACQ_OFF				0x00
#define SE4500_AIM					0x55
#define SE4500_AIM_ON				0x01
#define SE4500_AIM_OFF				0x00
#define SE4500_AIM_DURING_EXPOSURE  0x56
#define SE4500_ILLUM				0x59
#define SE4500_ILLUM_ON				0x01
#define SE4500_ILLUM_OFF			0x00
#define SE4500_AUTO_POWER			0x74
#define SE4500_AUTO_POWER_EN		0x01
#define SE4500_TIME_TO_LOW_POWER	0x75
#define SE4500_FRAME_RATE			0x5E // FPS  


#define SE4500_AIM_POWER_FAILURE	0x88


#define SE4500_TIME_TO_LOWPOWER		1000 /* 1 second */
static struct sensor_data se4710_data;
#define USE_SE4710
static struct fsl_mxc_camera_platform_data *camera_plat;
struct se4500_datafmt {
	enum v4l2_mbus_pixelcode	code;
	enum v4l2_colorspace		colorspace;
};

static const struct se4500_datafmt se4500_fmts[] = {
	{ V4L2_MBUS_FMT_Y8_1X8, V4L2_COLORSPACE_JPEG },
};

#if defined(USE_SE4500)
static const struct v4l2_frmsize_discrete se4500_frmsizes[] = {
	{ 752, 480 },
};
#elif defined(USE_SE4750)
static const struct v4l2_frmsize_discrete se4500_frmsizes[] = {
	{ 1360, 960 },
};
#elif defined(USE_SE4710)
static const struct v4l2_frmsize_discrete se4500_frmsizes[] = {
	{ 1280, 800 },
};
#endif

static struct v4l2_subdev_sensor_serial_parms mipi_cfgs[] = {
	[SE4500_SIZE_WVGA] = {
		.lanes = 1,
		.channel = 0,
		.phy_rate = (192 * 2 * 1000000),
		.pix_clk = 21,
	},
};

struct se4500 {
	struct v4l2_subdev subdev;
	struct v4l2_subdev_sensor_interface_parms *plat_parms;
	int i_size;
	int i_fmt;
	int streaming;
	atomic_t open_excl;
};

// WA for se4500_misc dev moto_sdl
static struct se4500* se4500_misc_wa = NULL;

static unsigned int GetTickCount()
{
	struct timeval tv;
	do_gettimeofday(&tv);
	return((tv.tv_sec * 1000) + (tv.tv_usec /1000));
}

static struct se4500 *to_se4500(const struct i2c_client *client)
{
	return container_of(i2c_get_clientdata(client), struct se4500, subdev);
}

static int se4500_find_datafmt(enum v4l2_mbus_pixelcode code)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(se4500_fmts); i++)
		if (se4500_fmts[i].code == code)
			break;

	/* If not found, select latest */
	if (i >= ARRAY_SIZE(se4500_fmts))
		i = ARRAY_SIZE(se4500_fmts) - 1;

	return i;
}

static int se4500_find_framesize(u32 width, u32 height)
{
	unsigned int num = ARRAY_SIZE(se4500_frmsizes);
	int i;

	for (i = 0; i < num; i++) {
		if ((se4500_frmsizes[i].width >= width) &&
		    (se4500_frmsizes[i].height >= height))
			break;
	}

	/* If not found, select biggest */
	if (i >= num)
		i = num - 1;

	return i;
}

static u8 se4500_calc_checksum(u8 *data, u16 count)
{
	u16 sum = 0;
	u16 i;
	for (i = 0; i < count; i++)
		sum += data[i];

	sum = ~sum + 1;

	return (sum & 0xff);
}

static int se4500_write(struct i2c_client *client, u8 reg, u8 val, u8 len)
{
	struct i2c_msg msg;
	u8 data[4];
	int i;
	if (len > 3)
	{
		return -EINVAL;
	}
	data[0] = reg;
	data[1] = val;
	data[2] = se4500_calc_checksum(data, 2);
	for(i=0; i<3;i++)
	msg.addr = client->addr;
	msg.flags = 0;
	msg.len = len;
	msg.buf = data;

	return i2c_transfer(client->adapter, &msg, 1);
}

static int se4500_read(struct i2c_client *client, u8 reg, u8 *val, u8 len)
{
	struct i2c_msg msg;
	u8 data[4];
	int ret;

	if (len > 3)
		return -EINVAL;

	data[0] = reg;
	data[1] = 0;
	data[2] = se4500_calc_checksum(data, 2);
	msg.addr = client->addr;
	msg.flags = I2C_M_RD;
	msg.len = len;
	msg.buf = data;

	ret = i2c_transfer(client->adapter, &msg, 1);
	if (ret < 0) {
		dev_err(&client->dev, "%s: i2c (line: %d)\n", __func__,
			__LINE__);
		goto out;
	}

	*val = data[1];
out:
	return ret;
}

/*
 * As per the block buster support, only mutltiples of 1, 10 and 100 is
 * supported with a max of 23500 sec. For example if want to set 1 sec,
 * we need to send value 0x15
 * 0x01 - 0x0A = 10-100 ms, 10 ms increments
 * 0x0B - 0x14 = 100-900 ms, 100 ms  increments
 * 0x15 - 0xFF = 1s - 235 s, 1 s increments
 * 0x00 = 5 ms
 */
static u8 se4500_convert_to_low_power(int time)
{
	if ((time >= 1000) && (time <= 235000))
		return (u8) ((time / 1000) + 20);
	else if( (time >= 100) &&  (time < 1000) )
		return (u8) ((time / 100) + 10);
	else if(time >= 10)
		return (u8) ((time / 10) + 1);
	else
		return (u8) time;

	return 0;
}
static int se4500_AcqCtrl(struct i2c_client *client, u8 cmd, u8 value)
{
	int ret;
	u8 val;
	int retry = 10;
	
	printk(KERN_INFO "%s: Enter cmd 0x%x value %d\n", __func__,cmd,value);
	
retry:
	ret = se4500_write(client, cmd, value, 3);
	if (ret < 0 && retry) {
		dev_err(&client->dev, "%s: i2c (line: %d, client addr %x)\n", __func__,__LINE__, client->addr);
		msleep(100);
		retry--;
		goto retry;		
	}
	if(ret < 0){
		printk(KERN_INFO "%s: cmd 0x%x value %d Failed retry %d\n", __func__,cmd,value,retry);
		goto out;
	}
	msleep(1);
	val = 0;
	ret = se4500_read(client, cmd, &val,3);
	if(ret < 0){
		dev_err(&client->dev, "%s: i2c (line: %d, client addr %x)\n", __func__,__LINE__, client->addr);		
		goto out;
	}else{
		printk(KERN_INFO "Read status val %d\n",val);
	}
out:	
	printk(KERN_INFO "%s: Exit cmd 0x%x value %d\n", __func__,cmd,value);
	return ret;
}

static int se4500_command(struct i2c_client *client, u8 cmd, u8 value)
{
	
	int ret;
	u8 val;
	int retry = 1;
	printk(KERN_INFO "%s: Enter cmd 0x%x value %d %u\n", __func__,cmd,value,GetTickCount());
	
retry:
	printk("\n######i2c write retry %s##########\n", client->name);
	ret = se4500_write(client, cmd, value, 3);
	if (ret < 0 && retry) {
		dev_err(&client->dev, "%s: i2c (line: %d, client addr %x)\n", __func__,__LINE__, client->addr);
		msleep(100);
		retry--;
		goto retry;		
	}
	if(ret < 0){
		printk(KERN_INFO "%s: cmd 0x%x value %d Failed retry %d\n", __func__,cmd,value,retry);
		goto out;
	}
	if(SE4500_ACQ != cmd){
		msleep(1);
		val = 0;
		ret = se4500_read(client, cmd, &val,3);
		if(ret < 0){
			dev_err(&client->dev, "%s: i2c (line: %d, client addr %x)\n", __func__,__LINE__, client->addr);		
			goto out;
		}else{
			printk(KERN_INFO "Read status val %d\n",val);
			if((SE4500_AIM == cmd) && (val == SE4500_AIM_POWER_FAILURE)){
				//Restart Acq
				se4500_AcqCtrl(client,SE4500_ACQ,SE4500_ACQ_OFF);
				mdelay(1);
				se4500_AcqCtrl(client,SE4500_ACQ,SE4500_ACQ_ON);
			}
		}
	}
out:	
	printk(KERN_INFO "%s: Exit cmd 0x%x value %d %u\n", __func__,cmd,value,GetTickCount());
	return ret;
}

static int se4500_auto_power(struct i2c_client *client)
{
	//struct i2c_client *client = v4l2_get_subdevdata(sd);
	int ret;
	u8 val;
	printk(KERN_INFO "%s: Enter\n", __func__);
	
	//Set Auto Power
	ret  = se4500_command(client,SE4500_AUTO_POWER,SE4500_AUTO_POWER_EN);
	if(ret >= 0){
		val = se4500_convert_to_low_power(SE4500_TIME_TO_LOWPOWER);
		//Set Time to Low Power
		ret = se4500_command(client,SE4500_TIME_TO_LOW_POWER,val);	
		if(ret < 0){
			printk(KERN_INFO "%s: SE4500_TIME_TO_LOW_POWER Failed\n", __func__);
			goto out;
		}		
	}else{
		printk(KERN_INFO "%s: Turning On Auto Power Failed\n", __func__);	
		goto out;
	}
out:
	printk(KERN_INFO "%s: sucess\n", __func__);
	return ret;
}


static int se4500_start_sensor(struct i2c_client *client, int enable)
{
	
	int ret;

	if (enable) {
		ret = se4500_command(client,SE4500_ILLUM,SE4500_ILLUM_ON);
		if(ret >= 0){
			ret = se4500_command(client,SE4500_AIM,SE4500_AIM_ON);
			if(ret >= 0){
				ret = se4500_command(client,SE4500_ACQ,SE4500_ACQ_ON);
				if(ret < 0){
					printk(KERN_INFO "Error %s Turn ON ACQ Aim Failed \n",__func__);
					goto out;
				}
			}else{
				printk(KERN_INFO "Error %s Turn ON Aim Failed \n",__func__);
				goto out;
			}
		}else{
			printk(KERN_INFO "Error %s Turn On Illumination Failed \n",__func__);
			goto out;
		}
	}else{
		ret = se4500_command(client,SE4500_ACQ,SE4500_ACQ_OFF);
		if(ret < 0){
			dev_err(&client->dev, "%s: Turn Off Acq Failed; ret = %d", __func__, ret);
			goto out;
		}
	}
out:
	if (ret < 0)
		dev_err(&client->dev, "%s: %s (ret = %d)\n", __func__,
			enable ? "start" : "stop",
			ret);

	return ret;
}

static int se4500_s_stream(struct v4l2_subdev *sd, int enable)
{
	int ret;
	// This function turns ACQ ON from the external application.
	// We dont want it since IAL turns it on for SE4500
	//printk(KERN_INFO "%s: Enter, enable : %d\n", __func__, enable);

	ret = 1; //se4500_start_sensor(sd, enable); // Palak

	//printk(KERN_INFO "%s: Exit, enable : %d\n", __func__, enable);
	return ret;
}

static int se4500_set_bus_param(struct soc_camera_device *icd,
				unsigned long flags)
{
	return 0;
}

static unsigned long se4500_query_bus_param(struct soc_camera_device *icd)
{
	unsigned long flags = SOCAM_PCLK_SAMPLE_FALLING |
		SOCAM_HSYNC_ACTIVE_HIGH | SOCAM_VSYNC_ACTIVE_HIGH |
		SOCAM_DATA_ACTIVE_HIGH | SOCAM_MASTER;

	/* TODO: Do the right thing here, and validate bus params */

	flags |= SOCAM_DATAWIDTH_8;

	return flags;
}

static int se4500_g_fmt(struct v4l2_subdev *sd,
			struct v4l2_mbus_framefmt *mf)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct se4500 *se4500 = to_se4500(client);

	mf->width	= se4500_frmsizes[se4500->i_size].width;
	mf->height	= se4500_frmsizes[se4500->i_size].height;
	mf->code	= se4500_fmts[se4500->i_fmt].code;
	mf->colorspace	= se4500_fmts[se4500->i_fmt].colorspace;
	mf->field	= V4L2_FIELD_NONE;

	dev_info(&client->dev, "%s: %dx%d, code: %d\n", __func__,
		mf->width, mf->height, mf->code);

	return 0;
}

static int se4500_try_fmt(struct v4l2_subdev *sd,
			  struct v4l2_mbus_framefmt *mf)
{
	int i_fmt;
	int i_size;

	i_fmt = se4500_find_datafmt(mf->code);

	mf->code = se4500_fmts[i_fmt].code;
	mf->colorspace = se4500_fmts[i_fmt].colorspace;
	mf->field = V4L2_FIELD_NONE;

	i_size = se4500_find_framesize(mf->width, mf->height);

	mf->width = se4500_frmsizes[i_size].width;
	mf->height = se4500_frmsizes[i_size].height;

	return 0;
}

static int se4500_s_fmt(struct v4l2_subdev *sd,
			 struct v4l2_mbus_framefmt *mf)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct se4500 *se4500 = to_se4500(client);
	int ret;

	ret = se4500_try_fmt(sd, mf);
	if (ret < 0)
		return ret;

	se4500->i_size = se4500_find_framesize(mf->width, mf->height);
	se4500->i_fmt = se4500_find_datafmt(mf->code);

	switch ((u32)se4500_fmts[se4500->i_fmt].code) {
	case V4L2_MBUS_FMT_Y8_1X8:
		break;
	default:
		ret = -EINVAL;
	}

	return ret;
}

static struct soc_camera_ops se4500_ops = {
	.set_bus_param		= se4500_set_bus_param,
	.query_bus_param	= se4500_query_bus_param,
};

static int se4500_init(struct i2c_client *client)
{
	dev_dbg(&client->dev, "Sensor initialized\n");

	return 0;
}

/*
 * Interface active, can use i2c. If it fails, it can indeed mean, that
 * this wasn't our capture interface, so, we wait for the right one
 */
static int se4500_video_probe(struct soc_camera_device *icd,
			      struct i2c_client *client)
{
	u8 revision = 0;
	int ret = 0;

	/*
	 * We must have a parent by now. And it cannot be a wrong one.
	 * So this entire test is completely redundant.
	 */
	if (!icd->dev.parent ||
	    to_soc_camera_host(icd->dev.parent)->nr != icd->iface)
		return -ENODEV;

	dev_info(&client->dev, "Detected a se4500 chip, revision %x\n",
		 revision);

	return ret;
}

static void se4500_video_remove(struct soc_camera_device *icd)
{
	dev_dbg(&icd->dev, "Video removed: %p, %p\n",
		icd->dev.parent, icd->vdev);
}


static struct v4l2_subdev_core_ops se4500_subdev_core_ops = {
	.s_ext_ctrls = NULL,
};

static int se4500_enum_fmt(struct v4l2_subdev *sd, unsigned int index,
			   enum v4l2_mbus_pixelcode *code)
{
	if (index >= ARRAY_SIZE(se4500_fmts))
		return -EINVAL;

	*code = se4500_fmts[index].code;
	return 0;
}

static int se4500_enum_framesizes(struct v4l2_subdev *sd,
				  struct v4l2_frmsizeenum *fsize)
{
	if (fsize->index >= ARRAY_SIZE(se4500_frmsizes))
		return -EINVAL;

	fsize->type = V4L2_FRMSIZE_TYPE_DISCRETE;
	fsize->pixel_format = V4L2_PIX_FMT_GREY;

	fsize->discrete = se4500_frmsizes[fsize->index];

	return 0;
}

static int se4500_g_parm(struct v4l2_subdev *sd, struct v4l2_streamparm *param)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct se4500 *se4500 = to_se4500(client);
	struct v4l2_captureparm *cparm;

	if (param->type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
		return -EINVAL;

	cparm = &param->parm.capture;

	memset(param, 0, sizeof(*param));
	param->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	cparm->capability = V4L2_CAP_TIMEPERFRAME;

	switch (se4500->i_size) {
	case SE4500_SIZE_WVGA:
	default:
		cparm->timeperframe.numerator = 1;
		cparm->timeperframe.denominator = 30;
		break;
	}

	return 0;
}

static int se4500_s_parm(struct v4l2_subdev *sd, struct v4l2_streamparm *param)
{
	return se4500_g_parm(sd, param);
}

static struct v4l2_subdev_video_ops se4500_subdev_video_ops = {
	.s_stream = se4500_s_stream,
	.s_mbus_fmt = se4500_s_fmt,
	.g_mbus_fmt = se4500_g_fmt,
	.try_mbus_fmt = se4500_try_fmt,
	.enum_mbus_fmt = se4500_enum_fmt,
	.enum_framesizes = se4500_enum_framesizes,
	.enum_mbus_fsizes = se4500_enum_framesizes,
	.g_parm = se4500_g_parm,
	.s_parm = se4500_s_parm,
};

static int se4500_g_skip_frames(struct v4l2_subdev *sd, u32 *frames)
{
	*frames = 0;
	return 0;
}

static int se4500_g_interface_parms(struct v4l2_subdev *sd,
			struct v4l2_subdev_sensor_interface_parms *parms)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct se4500 *se4500 = to_se4500(client);

	if (!parms)
		return -EINVAL;

	parms->if_type = se4500->plat_parms->if_type;
	parms->if_mode = se4500->plat_parms->if_mode;
	parms->parms.serial = mipi_cfgs[se4500->i_size];

	return 0;
}

static struct v4l2_subdev_sensor_ops se4500_subdev_sensor_ops = {
	.g_skip_frames = se4500_g_skip_frames,
	.g_interface_parms = se4500_g_interface_parms,
};

static struct v4l2_subdev_ops se4500_subdev_ops = {
	.core	= &se4500_subdev_core_ops,
	.video	= &se4500_subdev_video_ops,
	.sensor	= &se4500_subdev_sensor_ops,
};
void ToggleCamRestGPIO()
{
	int error = 0;
	printk(KERN_INFO "ToggleCamRestGPIO ++\n");
	error = gpio_request(SE4500_GPIO_PWN, "CAM_GBL_RESET");
	if (error < 0) {
		pr_err("%s: GPIO configuration failed: GPIO 90, error %d\n"
			, __func__, error);
	}
	
	error = gpio_direction_output(SE4500_GPIO_PWN, 1);
	if (error < 0) {
		pr_err("%s: GPIO gpio_direction_output failed: GPIO90, error %d\n"
			, __func__, error);
	}
	
	gpio_set_value(SE4500_GPIO_PWN, 1);
	
	mdelay(5);
	
	gpio_set_value(SE4500_GPIO_PWN, 0);
	
	gpio_free(SE4500_GPIO_PWN);
	printk(KERN_INFO "ToggleCamRestGPIO --\n");
}

// Support for moto_sdl to be exposed to the IAL
static int se4500_misc_open(struct inode* node, struct file* file)
{
	/*
	if ( atomic_inc_return(&se4500_misc_wa->open_excl) != 1 )
	{
		atomic_dec(&se4500_misc_wa->open_excl);
		return -EBUSY;
	}
	*/
	file->private_data = se4500_misc_wa;
	return(0);
}

static long se4500_misc_ioctl(struct file* file, unsigned int cmd, unsigned long arg)
{
	struct se4500* se4500;
	struct i2c_client* client;
	struct i2c_rdwr_ioctl_data rdwr_data;
	struct i2c_msg msg;
	u8 __user* usr_data;
	int ret = 0;

	se4500 = file->private_data;

	if ( (se4500 == NULL) || (cmd != I2C_RDWR) || !arg ) {
		return -EINVAL;
	}

	client = v4l2_get_subdevdata(&se4500->subdev);

	if ( copy_from_user(&rdwr_data, (struct i2c_rdwr_ioctl_data __user*) arg, sizeof(rdwr_data)) ) {
		return -EFAULT;
	}

	if ( rdwr_data.nmsgs != 1 ) {
		return -EINVAL;
	}

	if ( copy_from_user(&msg, rdwr_data.msgs, sizeof(struct i2c_msg)) ) {
		return -EFAULT;
	}

	// Only allow transfers to the SE4500, limit the size of the message and don't allow received length changes
	if ( (msg.addr != SE4500_I2C_DEVICE_ADDR) || (msg.len > 256) || (msg.flags & I2C_M_RECV_LEN) ) {
		return -EINVAL;
	}

	// Map the data buffer from user-space to kernel space
	// WA reuse same structure for message
	usr_data = (u8 __user*) msg.buf;
	msg.buf = memdup_user(usr_data, msg.len);
	if ( IS_ERR(msg.buf) )
	{
		return(PTR_ERR(msg.buf));
	}

	ret = i2c_transfer(client->adapter, &msg, 1);
	if ( (ret >= 0) && (msg.flags & I2C_M_RD) ) {
		// Successful read, copy data to user-space
		if ( copy_to_user(usr_data, msg.buf, msg.len) ) {
			ret = -EFAULT;
		}
	}
	kfree(msg.buf);
	return ret;
}

static int se4500_misc_release(struct inode* node, struct file* file)
{
	//atomic_dec(&file->open_excl);
	return(0);
}

static const struct file_operations se4500_misc_fops =
{
	.owner = THIS_MODULE,
	.unlocked_ioctl = se4500_misc_ioctl,
	.open = se4500_misc_open,
	.release = se4500_misc_release,
};

static struct miscdevice se4500_misc_device =
{
	.minor = MISC_DYNAMIC_MINOR,
	.name = "moto_sdl",
	.fops = &se4500_misc_fops,
};


static int se4500_suspend (struct i2c_client *client, pm_message_t mesg){
	printk(KERN_INFO "%s ++\n",__func__);
	printk(KERN_INFO "%s --\n",__func__);
	return 0;
}

static int se4500_resume (struct i2c_client *client){
	
	struct v4l2_subdev *sd = i2c_get_clientdata(client);
	printk(KERN_INFO "%s ++\n",__func__);	
	ToggleCamRestGPIO();
	//se4500_auto_power(sd);
	printk(KERN_INFO "%s --\n",__func__);
	return 0;
}


static int ioctl_dev_init(struct v4l2_int_device *s)
{
	struct sensor_data *sensor = s->priv;
	u32 tgt_xclk;	/* target xclk */
	u32 tgt_fps;	/* target frames per secound */
	
	int ret;
printk("---------%s------------\n", __func__);
	se4710_data.on = true;



	return 0;
}
static int ioctl_g_chip_ident(struct v4l2_int_device *s, int *id)
{
	printk("---------%s------------\n", __func__);
	((struct v4l2_dbg_chip_ident *)id)->match.type =
					V4L2_CHIP_MATCH_I2C_DRIVER;
	strcpy(((struct v4l2_dbg_chip_ident *)id)->match.name, "se4710_camera");

	return 0;
}

static int ioctl_g_ifparm(struct v4l2_int_device *s, struct v4l2_ifparm *p)
{
	if (s == NULL) {
		pr_err("   ERROR!! no slave device set!\n");
		return -1;
	}
	printk("---------%s------------\n", __func__);
	memset(p, 0, sizeof(*p));
	p->u.bt656.clock_curr = se4710_data.mclk;
	//pr_debug("   clock_curr=mclk=%d\n", se4710.mclk);
	p->if_type = V4L2_IF_TYPE_BT656;
	p->u.bt656.mode = V4L2_IF_TYPE_BT656_MODE_NOBT_8BIT;
	p->u.bt656.clock_min = 6000000;
	p->u.bt656.clock_max = 24000000;
	p->u.bt656.bt_sync_correct = 1;  /* Indicate external vsync */
	p->u.bt656.nobt_hs_inv = 0;
	p->u.bt656.nobt_vs_inv = 1;
	return 0;
}

static int ioctl_g_fmt_cap(struct v4l2_int_device *s, struct v4l2_format *f)
{
	struct sensor_data *sensor = s->priv;
	printk("---------%s------------\n", __func__);
	f->fmt.pix = sensor->pix;

	return 0;
}

static int ioctl_enum_fmt_cap(struct v4l2_int_device *s,
			      struct v4l2_fmtdesc *fmt)
{
	if (fmt->index != 0)
		return -EINVAL;
//printk("---------%s------------\n", __func__);
	fmt->index = 0;
	fmt->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt->flags = 0;
	strncpy(fmt->description, "8-bit Grey-Scale Format",
		sizeof(fmt->description)-1);
	
	fmt->pixelformat = V4L2_PIX_FMT_GREY;
	
	

	return 0;
}
static int ioctl_dev_exit(struct v4l2_int_device *s)
{
	return 0;
}

static int ioctl_s_power(struct v4l2_int_device *s, int on)
{
	struct sensor_data *sensor = s->priv;
	u8 val,ret;
	printk("------se4710 set power--------\n");
	ToggleCamRestGPIO();
	
		if (on && !sensor->on) {
		se4500_command(se4710_data.i2c_client, 0x5b, 0x01);
		se4500_command(se4710_data.i2c_client, 0x59, 0x01);
		//se4500_command(se4710_data.i2c_client, 0x55, 0x01);
		//se4500_command(se4710_data.i2c_client, 0x56, 0x01);
		se4500_command(se4710_data.i2c_client, 0x82, 0x01);
			se4500_command(se4710_data.i2c_client, 0x58, 0x01);

		ret = se4500_read(se4710_data.i2c_client, 0x70, &val,3);
	 printk("-----read 0x70 ret=%x val=%x\n", ret, val);
	 ret = se4500_read(se4710_data.i2c_client, 0x5d, &val,3);
	 printk("-----read 0x5d ret=%x val=%x\n", ret, val);
	 ret = se4500_read(se4710_data.i2c_client, 0x5c, &val,3);
	 printk("-----read 0x5c ret=%x val=%x\n", ret, val);
		}else if(!on && sensor->on){
		
			se4500_command(se4710_data.i2c_client, 0x58, 0x00);
		}

	
	
	sensor->on = on;

	return 0;
}

static int ioctl_init(struct v4l2_int_device *s)
{

	return 0;
}

static int ioctl_g_parm(struct v4l2_int_device *s, struct v4l2_streamparm *param)
{
	struct v4l2_captureparm *cparm;

	if (param->type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
		return -EINVAL;

	cparm = &param->parm.capture;

	memset(param, 0, sizeof(*param));
	param->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	cparm->capability = V4L2_CAP_TIMEPERFRAME;


	cparm->timeperframe.numerator = 1;
	cparm->timeperframe.denominator = 30;
	
}

static int ioctl_s_parm(struct v4l2_int_device *s, struct v4l2_streamparm *a)
{
	struct sensor_data *sensor = s->priv;
	struct v4l2_fract *timeperframe = &a->parm.capture.timeperframe;
	u32 tgt_fps;	/* target frames per secound */
	
	int ret = 0;



	switch (a->type) {
	/* This is the only case currently handled. */
	case V4L2_BUF_TYPE_VIDEO_CAPTURE:
		/* Check that the new frame rate is allowed. */
		if ((timeperframe->numerator == 0) ||
		    (timeperframe->denominator == 0)) {
			timeperframe->denominator = 30;
			timeperframe->numerator = 1;
		}

		tgt_fps = timeperframe->denominator /
			  timeperframe->numerator;

		if (tgt_fps > 30) {
			timeperframe->denominator = 30;
			timeperframe->numerator = 1;
		} else if (tgt_fps < 15) {
			timeperframe->denominator = 15;
			timeperframe->numerator = 1;
		}

		/* Actual frame rate we use */
		tgt_fps = timeperframe->denominator /
			  timeperframe->numerator;

	

	

		sensor->streamcap.timeperframe = *timeperframe;
		sensor->streamcap.capturemode = a->parm.capture.capturemode;

		break;

	/* These are all the possible cases. */
	case V4L2_BUF_TYPE_VIDEO_OUTPUT:
	case V4L2_BUF_TYPE_VIDEO_OVERLAY:
	case V4L2_BUF_TYPE_VBI_CAPTURE:
	case V4L2_BUF_TYPE_VBI_OUTPUT:
	case V4L2_BUF_TYPE_SLICED_VBI_CAPTURE:
	case V4L2_BUF_TYPE_SLICED_VBI_OUTPUT:
		pr_debug("   type is not " \
			"V4L2_BUF_TYPE_VIDEO_CAPTURE but %d\n",
			a->type);
		ret = -EINVAL;
		break;

	default:
		pr_debug("   type is unknown - %d\n", a->type);
		ret = -EINVAL;
		break;
	}

	return ret;
}

static int ioctl_enum_framesizes(struct v4l2_int_device *s,
				 struct v4l2_frmsizeenum *fsize)
{
	if (fsize->index >= ARRAY_SIZE(se4500_frmsizes))
		return -EINVAL;

	fsize->type = V4L2_FRMSIZE_TYPE_DISCRETE;
	fsize->pixel_format = V4L2_PIX_FMT_GREY;

	fsize->discrete = se4500_frmsizes[fsize->index];

	return 0;
}

static int ioctl_enum_frameintervals(struct v4l2_int_device *s,
					 struct v4l2_frmivalenum *fival)
{
	int i, j, count;

	if (fival->index < 0 || fival->index > 3)
		return -EINVAL;

	if (fival->pixel_format == 0 || fival->width == 0 || fival->height == 0) {
		pr_warning("Please assign pixelformat, width and height.\n");
		return -EINVAL;
	}

	fival->type = V4L2_FRMIVAL_TYPE_DISCRETE;
	fival->discrete.numerator = 1;
	fival->pixel_format == se4710_data.pix.pixelformat;
	fival->width = 640;
	fival->height = 480;

	return -EINVAL;
}

static struct v4l2_int_ioctl_desc se4710_ioctl_desc[] = {
	{vidioc_int_dev_init_num, (v4l2_int_ioctl_func *)ioctl_dev_init},
	{vidioc_int_dev_exit_num, ioctl_dev_exit},
	{vidioc_int_s_power_num, (v4l2_int_ioctl_func *)ioctl_s_power},
	{vidioc_int_g_ifparm_num, (v4l2_int_ioctl_func *)ioctl_g_ifparm},
	{vidioc_int_init_num, (v4l2_int_ioctl_func *)ioctl_init},
	{vidioc_int_g_fmt_cap_num, (v4l2_int_ioctl_func *)ioctl_g_fmt_cap},
	{vidioc_int_g_parm_num, (v4l2_int_ioctl_func *)ioctl_g_parm},
	{vidioc_int_s_parm_num, (v4l2_int_ioctl_func *)ioctl_s_parm},
	{vidioc_int_g_chip_ident_num,(v4l2_int_ioctl_func *)ioctl_g_chip_ident},
	{vidioc_int_enum_framesizes_num,
				(v4l2_int_ioctl_func *)ioctl_enum_framesizes},
	{vidioc_int_enum_frameintervals_num,
				(v4l2_int_ioctl_func *)ioctl_enum_frameintervals},

	{vidioc_int_enum_fmt_cap_num,
				(v4l2_int_ioctl_func *)ioctl_enum_fmt_cap},

};
static struct v4l2_int_slave se4710_slave = {
	.ioctls = se4710_ioctl_desc,
	.num_ioctls = ARRAY_SIZE(se4710_ioctl_desc),
};

static struct v4l2_int_device se4710_int_device = {
	.module = THIS_MODULE,
	.name = "se4710",
	.type = v4l2_int_type_slave,
	.u = {
		.slave = &se4710_slave,
	},
};
static int se4500_probe(struct i2c_client *client,
			const struct i2c_device_id *did)
{
	struct se4500 *se4500;
	struct fsl_mxc_camera_platform_data *plat_data = client->dev.platform_data;
	
	int ret;
	u8 val;



	memset(&se4710_data, 0, sizeof(se4710_data));
	se4710_data.i2c_client = client;
	se4710_data.pix.pixelformat = V4L2_PIX_FMT_GREY;//V4L2_PIX_FMT_YUYV;
	se4710_data.pix.width = 1280;
	se4710_data.pix.height = 800;
	se4710_data.csi = 1;
	se4710_data.mclk = 24000000;
	se4710_data.mclk_source = 0;
	se4710_data.streamcap.capability = V4L2_MODE_HIGHQUALITY |
					   V4L2_CAP_TIMEPERFRAME;
	se4710_data.streamcap.capturemode = 0;
	se4710_data.streamcap.timeperframe.denominator = 30;
	se4710_data.streamcap.timeperframe.numerator = 1;
	/*********************************************/
	

#if 0
	ret = se4500_write(client, 0x59, 0x01, 3);
	 printk("-----write 0x59 ret=%x val=1\n", ret);
	 ret = se4500_read(client, 0x59, &val,3);
	 printk("-----read 0x59 ret=%x val=%x\n", ret, val);


	ret = se4500_write(client, 0x55, 0x01, 3);
	 printk("-----write 0x55 ret=%x val=1\n", ret);
	 ret = se4500_read(client, 0x55, &val,3);
	 printk("-----read 0x55 ret=%x val=%x\n", ret, val);
	 
	 printk("-----se4500 probe i2c addr=[0x%x]\n", client->addr);
	ret = se4500_write(client, 0x58, 0x01, 3);
	 printk("-----write 0x58 ret=%x val=1\n", ret);
	 ret = se4500_read(client, 0x58, &val,3);
	 printk("-----read 0x58 ret=%x val=%x\n", ret, val);
#endif
       /*******************************************/

	se4500 = kzalloc(sizeof(*se4500), GFP_KERNEL);
	if (!se4500)
	{
		printk("\r\n se4500 allocation fails\n");
		return -ENOMEM;
	}
	 se4710_int_device.priv = &se4710_data;
	 v4l2_int_device_register(&se4710_int_device);
	//v4l2_i2c_subdev_init(&se4500->subdev, client, &se4500_subdev_ops);

	/* Second stage probe - when a capture adapter is there */
	//icd->ops = &se4500_ops;

	se4500->i_size = SE4500_SIZE_WVGA;
	se4500->i_fmt = 0; /* First format in the list */
	//se4500->plat_parms = icl->priv;
#if 0
	ret = se4500_video_probe(icd, client);
	if (ret) {
		icd->ops = NULL;
		kfree(se4500);
		return ret;
	}
#endif
	ret = se4500_init(client);
	if (ret) {
		dev_err(&client->dev, "Failed to initialize sensor\n");
		ret = -EINVAL;
	}
printk("----6------%s----------\n",__func__);
	//se4500_auto_power(&se4500->subdev);
	
	ToggleCamRestGPIO();
	se4500_command(client, 0x86, 0x04);

	se4500_misc_wa = se4500;
	//atomic_set(&se4500_misc_wa->open_excl, 0);
	misc_register(&se4500_misc_device);

	printk(KERN_ERR "%s: sucess\n", __func__);

	return ret;
}

static int se4500_remove(struct i2c_client *client)
{
	struct se4500 *se4500 = to_se4500(client);
	//struct soc_camera_device *icd = client->dev.platform_data;

	misc_deregister(&se4500_misc_device);
	se4500_misc_wa = NULL;
	//icd->ops = NULL;
	//se4500_video_remove(icd);
		se4500_command(client, 0x58, 0x0);
	client->driver = NULL;
	kfree(se4500);

	return 0;
}


static const struct i2c_device_id se4500_id[] = {
	{ "se4710", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, se4500_id);

static struct i2c_driver se4500_i2c_driver = {
	.driver = {
		.name = "se4710",
	},
	.probe = se4500_probe,
	.remove = se4500_remove,
	.suspend = se4500_suspend,
	.resume = se4500_resume,
	.id_table = se4500_id,
};

static int __init se4500_mod_init(void)
{
	return i2c_add_driver(&se4500_i2c_driver);
}

static void __exit se4500_mod_exit(void)
{
	i2c_del_driver(&se4500_i2c_driver);
}

module_init(se4500_mod_init);
module_exit(se4500_mod_exit);

MODULE_DESCRIPTION("Symbol SE4500 Imaging module driver");
MODULE_AUTHOR("Stanimir Varbanov <svarbanov@mm-sol.com>");
MODULE_LICENSE("GPL v2");
