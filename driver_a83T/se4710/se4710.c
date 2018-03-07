/*************************************************************************
	> File Name: se4710.c
	> Author: shuixianbing
	> Mail: shui6666@126.com 
	> Created Time: 2016年09月21日 星期三 15时48分02秒
 ************************************************************************/
#include <linux/init.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <linux/miscdevice.h>
#include <media/v4l2-subdev.h>
#include <linux/videodev2.h>
#include <linux/v4l2-mediabus.h>
#include <media/v4l2-device.h>
#include <media/v4l2-chip-ident.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/miscdevice.h>
#include "cci_helper.h"
#include "vfe_subdev.h"
#include "camera_cfg.h"

#define SE4710_NAME "se4710"
#define V4L2_IDENT_SENSOR 0x4710
#define regval_list reg_list_a16_d8

#define SE4710_I2C_DEVICE_ADDR 0x5c
#define SE4710_AUTO_POWER    0x74
#define SE4710_AUTO_POWER_EN 0X01
#define SE4710_ACQ     0X58
#define SE4710_ACQ_ON  0X01
#define SE4710_ACQ_OFF 0x00
#define SE4710_AIM     0x55
#define SE4710_AIM_ON  0X01
#define SE4710_AIM_OFF 0x00
#define SE4710_ILLUM   0X59
#define SE4710_ILLUM_ON  0X01
#define SE4710_ILLUM_OFF 0x00
#define SE4710_TIME_TO_LOW_POWER 0x75
#define SE4710_TIME_TO_LOWPOWER 1000 //1 second
#define SE4710_AIM_POWER_FAILURE 0x88


#define VREF_POL V4L2_MBUS_VSYNC_ACTIVE_HIGH
#define HREF_POL V4L2_MBUS_HSYNC_ACTIVE_HIGH
#define CLK_POL  V4L2_MBUS_PCLK_SAMPLE_RISING
#define SE4710_WIDTH 1280
#define SE4710_HEIGHT 800

static struct cci_driver cci_drv = {
	.name = SE4710_NAME,
	.addr_width = 8,
	.data_width = 8,
};

static struct v4l2_subdev *glb_sd;
static struct regval_list sensor_fmt_y8[] = {

};
static struct sensor_format_struct {
	__u8 *desc;
	enum v4l2_mbus_pixelcode mbus_code;
	struct regval_list *regs;
	int regs_size;
	int bpp;
} sensor_formats[] = {
	{
		.desc      = "",
		.mbus_code = V4L2_MBUS_FMT_Y8_1X8,
		.regs      = sensor_fmt_y8,
		.regs_size = ARRAY_SIZE(sensor_fmt_y8),
		.bpp       = 1,
	}
}; 

struct sensor_info {
	struct v4l2_subdev sd;
	struct sensor_format_struct *fmt;
	enum standby_mode stby_mode;
	unsigned int width;
	unsigned int height;
	unsigned int capture_mode;

	unsigned int hflip;
	unsigned int vflip;

};

static struct sensor_win_size sensor_win_sizes[] = {
	{
		.width  = SE4710_WIDTH,
		.height = SE4710_HEIGHT,
		.hoffset = 0,
		.voffset = 0,
		.regs = NULL,
		.regs_size = 0,
		.set_size = NULL,
 	},
};

#define N_FMTS ARRAY_SIZE(sensor_formats)
#define N_WIN_SIZES (ARRAY_SIZE(sensor_win_sizes))

static inline struct sensor_info *to_state(struct v4l2_subdev *sd){
	return container_of(sd, struct sensor_info, sd);
}

static u8 se4710_calc_checksum(u8 *data, u16 count){
	u16 sum = 0;
	u16 i;
	for(i=0; i<count; i++)
	  sum += data[i];

	sum = ~sum + 1;
	return (sum&0xff);
}

static int se4710_write(struct i2c_client *client, u8 reg, u8 val, u8 len){
	struct i2c_msg msg;
	u8 data[4];
	int i;
	if(len > 3)
	  return -EINVAL;

	data[0] = reg;
	data[1] = val;
	data[2] = se4710_calc_checksum(data, 2);
	for(i=0; i<3; i++)
	msg.addr = client->addr;
	msg.flags = 0;
	msg.len = len;
	msg.buf = data;

	return i2c_transfer(client->adapter, &msg, 1);
}

static int se4710_read(struct i2c_client *client, u8 reg, u8 *val, u8 len){
	struct i2c_msg msg;
	u8 data[4];
	int ret;
	if(ret > 3)
	  return -EINVAL;
	data[0] = reg;
	data[1] = 0;
	data[2] = se4710_calc_checksum(data, 2);
	msg.addr = client->addr;
	msg.flags = I2C_M_RD;
	msg.len = len;
	msg.buf = data;

	ret = i2c_transfer(client->adapter, &msg, 1);
	if(ret < 0){
		printk("se4710 read fail\n");

		return ret;
	}

	*val = data[1];

	return ret;
}

static int se4710_AcqCtrl(struct i2c_client *client, u8 cmd, u8 value){
	int ret;
	u8 val;
	int retry = 10;
	printk("se4710 enter cmd 0x%x acqctrl\n", cmd);

retry:
	ret = se4710_write(client,cmd, value, 3);
	if(ret < 0 && retry){
		msleep(100);
		retry--;
		goto retry;
	}
	if(ret <0){
		printk("se4710 %s cmd 0x%x failed retry %d\n", __func__,
					cmd, retry);
		goto out;
	}
	msleep(1);
	val = 0;
	ret = se4710_read(client, cmd, &val, 3);
	if(ret < 0){
		goto out;
	}

out:
	return ret;
}

static int se4710_command(struct v4l2_subdev *sd, u8 cmd, u8 value){
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	int ret;
	int retry = 1;
	printk("se4710 command cmd 0x%x value 0x%x client addr 0x%x\n",
				cmd, value, client->addr);
retry:
	ret = se4710_write(client, cmd, value, 3);
	if(ret <0 && retry){
		printk("se4710 command retry\n");;
		msleep(100);
		retry --;
		goto retry;
	}
	if(ret < 0){
		printk("se4710 cmd 0x%x failed retry\n", cmd);
		goto out;;
	}
#if 0
	if(SE4710_ACQ != cmd){
		msleep(1);
		ret = se4710_read(client, cmd, &val, 3);
		if(ret < 0){
			printk("comman acq fail\n");
			goto out;
		}else{
			if((SE4710_AIM == cmd)&& (val == SE4710_AIM_POWER_FAILURE)){
				//restart ACQ
			se4710_AcqCtrl(client, SE4710_ACQ, SE4710_ACQ_OFF);
			msleep(1);
			se4710_AcqCtrl(client, SE4710_ACQ, SE4710_ACQ_ON);
			}
		}
	}
#endif
out:
	return ret;
}

static int se4710_start_sensor(struct v4l2_subdev *sd, int enable){
	int ret;
	if(enable){
		ret = se4710_command(sd, SE4710_ILLUM, SE4710_ILLUM_ON);
		if(ret>=0){
			ret = se4710_command(sd, SE4710_AIM, SE4710_AIM_ON);
		if(ret >= 0){
			ret = se4710_command(sd, SE4710_ACQ, SE4710_ACQ_ON);
		if(ret < 0){
			printk("se4710 turn on ACQ Aim failed\n");
			goto out;
			}
		}else{
			printk("se4710 Turn on Aim failed\n");
			goto out;
		}
		}else{
			printk("se4710 turn on illumination failed\n");
			goto out;
		}
	}else{
		ret = se4710_command(sd, SE4710_ACQ, SE4710_ACQ_OFF);
		if(ret < 0){
			printk("se4710 turn off acq failed\n");
			goto out;
		}
		se4710_command(sd, SE4710_ILLUM, SE4710_ILLUM_OFF);
	}

out:
	return ret;
}

static u8 se4710_convert_to_low_power(int time)
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
static int se4710_auto_power(struct v4l2_subdev *sd){
	int ret;
	printk("se4710 auto power\n");

	ret = se4710_command(sd, SE4710_AUTO_POWER, SE4710_AUTO_POWER_EN);

	return ret;
}

static int se4710_misc_open(struct inode *node, struct file *file){
	
	return 0;
}

static int se4710_misc_release(struct inode *node, struct file *file){
	return 0;
}

static long se4710_misc_ioctl(struct file *file, unsigned int cmd, unsigned long arg){
	struct i2c_client *client;
	struct i2c_rdwr_ioctl_data rdwr_data;
	struct i2c_msg msg;
	u8 __user *usr_data;
	int ret = 0;

	if((cmd != I2C_RDWR) || arg)
	  return -EINVAL;
	
	client = v4l2_get_subdevdata(glb_sd);

	if(copy_from_user(&rdwr_data, (struct i2c_rdwr_ioctl_data __user*)arg,
					sizeof(rdwr_data)))
	  return -EFAULT;
	if(rdwr_data.nmsgs != 1)
	  return -EINVAL;
	if(copy_from_user(&msg, rdwr_data.msgs, sizeof(struct i2c_msg)))
	  return -EFAULT;

	if((msg.addr != SE4710_I2C_DEVICE_ADDR) || (msg.len > 256) || (msg.flags & I2C_M_RECV_LEN))
	  return -EINVAL;
	//Map the data buffer from user-space to kernel space
	usr_data = (u8 __user*)msg.buf;
	msg.buf = memdup_user(usr_data, msg.len);
	if(IS_ERR(msg.buf))
	  return(PTR_ERR(msg.buf));

	ret = i2c_transfer(client->adapter, &msg, 1);
	if((ret >= 0)&&(msg.flags & I2C_M_RD)){
		if(copy_to_user(usr_data, msg.buf, msg.len)){
			ret = -EFAULT;
		}
	}
	kfree(msg.buf);
	return ret;
}

static const struct file_operations se4710_misc_fops = {
	.owner = THIS_MODULE,
	.unlocked_ioctl = se4710_misc_ioctl,
	.open    = se4710_misc_open,
	.release = se4710_misc_release,
};

static struct miscdevice se4710_misc_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name  = "sdl_control",
	.fops  = &se4710_misc_fops,
};

static int se4710_hw_init(struct v4l2_subdev *sd, u32 val){
	printk("se4710 %s\n", __func__);
	se4710_start_sensor(sd, 1);

	return 0;
}

static int sensor_g_vflip(struct v4l2_subdev *sd, signed int *value){
	struct sensor_info *info = to_state(sd);
	unsigned char val = 0;

	*value = val;
	info->vflip = *value;
	return 0;
}

static int sensor_g_hflip(struct v4l2_subdev *sd, signed int *value){
	struct sensor_info *info = to_state(sd);
	unsigned char val = 0;

	*value = val;
	info->hflip = *value;

	return 0;
}

static int sensor_s_hflip(struct v4l2_subdev *sd, int value){
	struct sensor_info *info = to_state(sd);

	info->hflip = value;
	return 0;
}

static int sensor_s_vflip(struct v4l2_subdev *sd, int value){
	struct sensor_info *info = to_state(sd);

	info->vflip = value;
	return 0;
}



static int se4710_power(struct v4l2_subdev *sd, int on){
		
	printk("se4710 ---%s\n", __func__);
	if(on == 0x08)
	  se4710_auto_power(sd);
	return 0;
}

static long se4710_ioctl(struct v4l2_subdev *sd, unsigned int cmd, void *arg){
	
	return 0;
}


static int se4710_enum_fmt(struct v4l2_subdev *sd, unsigned index, enum v4l2_mbus_pixelcode *code){
	printk("se4710 %s\n", __func__);
	if(index >= N_FMTS)
	  return -EINVAL;
	*code = sensor_formats[index].mbus_code;
	return 0;
}



static int se4710_enum_size(struct v4l2_subdev *sd, struct v4l2_frmsizeenum *fsize){

	if(fsize->index > N_WIN_SIZES-1)
	  return -EINVAL;

	printk("se4710 %s\n", __func__);
	fsize->type = V4L2_FRMSIZE_TYPE_DISCRETE;
	fsize->pixel_format = V4L2_PIX_FMT_GREY;
	fsize->discrete.width = sensor_win_sizes[fsize->index].width;
	fsize->discrete.height = sensor_win_sizes[fsize->index].height;

	return 0;
}

static int se4710_try_fmt_internal(struct v4l2_subdev *sd, 
			struct v4l2_mbus_framefmt *fmt,
			struct sensor_format_struct **ret_fmt,
			struct sensor_win_size **ret_wsize){
	int index;
	struct sensor_win_size *wsize;
	printk("se4710 %s 0\n", __func__);
	for(index = 0; index < N_FMTS; index++)
	  if(sensor_formats[index].mbus_code == fmt->code)
		break;

	printk("se4710 %s 1\n", __func__);
	if(index >= N_FMTS)
	  return -EINVAL;

	if(ret_fmt != NULL)
	  *ret_fmt = sensor_formats + index;

	printk("se4710 %s 2\n", __func__);
	fmt->field = V4L2_FIELD_NONE;

	for(wsize = sensor_win_sizes; wsize < sensor_win_sizes + N_WIN_SIZES;
				wsize++)
	  if(fmt->width >= wsize->width && fmt->height >= wsize->height)
		break;

	if(wsize >= sensor_win_sizes + N_WIN_SIZES)
	  wsize--;
	if(ret_wsize != NULL)
	  *ret_wsize = wsize;

	printk("se4710 %s 3\n", __func__);
	fmt->width = wsize->width;
	fmt->height = wsize->height;
//	fmt->code = V4L2_MBUS_FMT_Y8_1X8;
//	fmt->colorspace = V4L2_COLORSPACE_JPEG;

	printk("se4710 %s 4\n", __func__);
	return 0;
}
static int se4710_try_fmt(struct v4l2_subdev *sd, struct v4l2_mbus_framefmt *fmt){
	printk("se4710 %s\n", __func__);
	return se4710_try_fmt_internal(sd, fmt, NULL, NULL);
}


static int se4710_g_mbus_config(struct v4l2_subdev *sd, struct 
			v4l2_mbus_config *cfg){
	
	printk("se4710 %s\n", __func__);
	cfg->type = V4L2_MBUS_PARALLEL;
	cfg->flags = V4L2_MBUS_MASTER | VREF_POL | HREF_POL | CLK_POL;

	return 0;
}

static int se4710_s_fmt(struct v4l2_subdev *sd, struct v4l2_mbus_framefmt *fmt){
	return 0;
}

static int se4710_g_parm(struct v4l2_subdev *sd, struct v4l2_streamparm *parms){
	struct v4l2_captureparm *cp = &parms->parm.capture;
	
	printk("se4710 %s\n", __func__);
	if(parms->type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
	  return -EINVAL;
	memset(cp, 0, sizeof(struct v4l2_captureparm));
	cp->capability = V4L2_CAP_TIMEPERFRAME;
	cp->timeperframe.numerator = 1;
	cp->timeperframe.denominator = 30;

	return 0;
}

static int se4710_s_parm(struct v4l2_subdev *sd, struct v4l2_streamparm *parms){

	return 0;
}


static int se4710_queryctrl(struct v4l2_subdev *sd, struct v4l2_queryctrl *qc){
	
	printk("se4710 %s\n", __func__);
	switch(qc->id){
		case V4L2_CID_VFLIP:
		case V4L2_CID_HFLIP:
			return v4l2_ctrl_query_fill(qc, 0, 1, 1, 0);

	}
	return -EINVAL;
}

static int se4710_g_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl){
	printk("se4710 ---%s\n", __func__);
	switch(ctrl->id){
		case V4L2_CID_VFLIP:
			return sensor_g_vflip(sd, &ctrl->value);
		case V4L2_CID_HFLIP:
			return sensor_g_hflip(sd, &ctrl->value);
	}
	return -EINVAL;
}

static int se4710_s_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl){
	struct v4l2_queryctrl qc;
	int ret;
	qc.id = ctrl->id;
	
	printk("se4710 ---%s\n", __func__);
	ret = se4710_queryctrl(sd, &qc);
	if(ret < 0)
	  return ret;

	switch(ctrl->id){
		case V4L2_CID_VFLIP:
			return sensor_s_vflip(sd, ctrl->value);
		case V4L2_CID_HFLIP:
			return sensor_s_hflip(sd, ctrl->value);
	}
	
	return -EINVAL;
}
static int se4710_g_chip_ident(struct v4l2_subdev *sd,
			struct v4l2_dbg_chip_ident *chip){
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	return v4l2_chip_ident_i2c_client(client, chip, V4L2_IDENT_SENSOR, 0);
}

static const struct v4l2_subdev_core_ops se4710_subdev_core_ops = {
	.g_chip_ident = se4710_g_chip_ident,
	.g_ctrl = se4710_g_ctrl,
	.s_ctrl = se4710_s_ctrl,
	.queryctrl = se4710_queryctrl,

	.init = se4710_hw_init,
	.s_power = se4710_power,
	.ioctl = se4710_ioctl,
};

static const struct v4l2_subdev_video_ops se4710_subdev_video_ops = {
	.enum_mbus_fmt = se4710_enum_fmt,
	.enum_framesizes = se4710_enum_size,
	.try_mbus_fmt = se4710_try_fmt,
	.s_mbus_fmt = se4710_s_fmt,
	.s_parm = se4710_s_parm,
	.g_parm = se4710_g_parm,
	.g_mbus_config = se4710_g_mbus_config,

};
static const struct v4l2_subdev_ops sensor_ops = {
	.core  = &se4710_subdev_core_ops,
	.video = &se4710_subdev_video_ops,
};

static int se4710_probe(struct i2c_client *client,
			const struct i2c_device_id *id){
	struct v4l2_subdev *sd;
	struct sensor_info *info;
	
	info = kzalloc(sizeof(struct sensor_info), GFP_KERNEL);
	if(info == NULL)
	  return -ENOMEM;

	sd = &info->sd;
	glb_sd = sd;
	cci_dev_probe_helper(sd, client, &sensor_ops, &cci_drv);

	info->fmt = &sensor_formats[0];
	info->hflip = 0;
	info->vflip = 0;
	
	//se4710_auto_power(sd);
	se4710_command(sd, 0x86, 0x04);
	//se4710_command(sd, 0x99, 0x06);

	misc_register(&se4710_misc_device);

	printk("se4710 probe\n");

	
	return 0;
}

static int se4710_remove(struct i2c_client *client){
	struct v4l2_subdev *sd;
	sd = i2c_get_clientdata(client);

	se4710_start_sensor(sd, 0);
	cci_dev_remove_helper(client, &cci_drv);
	printk("se4710 sensor remove sd=%p\n", sd);
	kfree(to_state(sd));
	misc_deregister(&se4710_misc_device);
	return 0;
}
static const struct i2c_device_id se4710_id[] = {
	{SE4710_NAME, 0},
	{}
};

static struct i2c_driver se4710_driver = {
	.driver = {
		.owner = THIS_MODULE,
		.name  = SE4710_NAME,
	},
	.probe  = se4710_probe,
	.remove = se4710_remove,
	.id_table = se4710_id,
};


static int __init se4710_init(void){
		
	return cci_dev_init_helper(&se4710_driver);
}

static void __exit se4710_exit(void){
	cci_dev_exit_helper(&se4710_driver);
}

module_init(se4710_init);
module_exit(se4710_exit);

MODULE_LICENSE("GPL");
