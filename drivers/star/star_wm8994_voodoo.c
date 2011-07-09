
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/kthread.h>
#include <linux/device.h>
#include <linux/kernel.h>
//#include <linux/tegra_devices.h>      //20100716  blocking for compile error [LGE]

#include <nvodm_services.h>

#include <linux/interrupt.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/irq.h>
#include <linux/miscdevice.h>
#include <asm/uaccess.h>
#include <linux/delay.h>
#include <linux/input.h>
#include <linux/workqueue.h>

#include <linux/kobject.h>
#include "nvcommon.h"
#include "nvodm_services.h"
#include "nvodm_query.h"
#include "nvodm_query_discovery.h"
#include <linux/wakelock.h>

#define WM8994_I2C_RETRY_COUNT 5
#define WM8994_I2C_TIMEOUT 20

#define VOODOO_SOUND_VERSION 0

typedef struct star_wm8994_device_data {
	NvOdmServicesI2cHandle h_gen2_i2c;
	NvU32 i2c_address;
	struct wake_lock wm8994_wake_lock;
} star_wm8994_device;

static star_wm8994_device *g_wm8994;

/* ASoC code compatibilty */

int codec = 0;

static NvBool
WriteWolfsonRegister(star_wm8994_device * wm8994, NvU32 RegIndex, NvU32 Data)
{
	int i;
	NvOdmI2cStatus I2cTransStatus = NvOdmI2cStatus_Timeout;
	NvU8 pTxBuffer[4];
	NvOdmI2cTransactionInfo TransactionInfo;

	for (i = 0;
	     i < WM8994_I2C_RETRY_COUNT
	     && I2cTransStatus != NvOdmI2cStatus_Success; i++) {
		pTxBuffer[0] = (NvU8) ((RegIndex >> 8) & 0xFF);
		pTxBuffer[1] = (NvU8) (RegIndex & 0xFF);
		pTxBuffer[2] = (NvU8) ((Data >> 8) & 0xFF);
		pTxBuffer[3] = (NvU8) ((Data) & 0xFF);

		TransactionInfo.Address = wm8994->i2c_address;
		TransactionInfo.Buf = pTxBuffer;
		TransactionInfo.Flags = NVODM_I2C_IS_WRITE;
		TransactionInfo.NumBytes = 4;

		printk("Voodoo sound: RegIndex: 0x%X, Data: 0x%X\n", RegIndex,
		       Data);

		I2cTransStatus =
		    NvOdmI2cTransaction(wm8994->h_gen2_i2c, &TransactionInfo, 1,
					400, WM8994_I2C_TIMEOUT);
	}

	if (I2cTransStatus == NvOdmI2cStatus_Success) {
		return NV_TRUE;
	}
	printk("[star wm8994 driver] i2c transaction error\n");
	return NV_FALSE;
}

static NvBool ReadWolfsonRegister(star_wm8994_device * wm8994, NvU32 RegIndex,
				  NvU32 * Data)
{
	int i;
	NvU8 *pReadBuffer;
	NvOdmI2cStatus status = NvOdmI2cStatus_Timeout;
	NvOdmI2cTransactionInfo *pTransactionInfo;

	pReadBuffer = NvOdmOsAlloc(2);
	if (!pReadBuffer) {
		return NV_FALSE;
	}

	pTransactionInfo = NvOdmOsAlloc(sizeof(NvOdmI2cTransactionInfo) * 2);
	if (!pTransactionInfo) {
		NvOdmOsFree(pReadBuffer);
		return NV_FALSE;
	}

	for (i = 0;
	     i < WM8994_I2C_RETRY_COUNT && status != NvOdmI2cStatus_Success;
	     i++) {
		pReadBuffer[0] = (NvU8) ((RegIndex >> 8) & 0xFF);
		pReadBuffer[1] = (NvU8) (RegIndex & 0xFF);

		pTransactionInfo[0].Address = wm8994->i2c_address;
		pTransactionInfo[0].Buf = pReadBuffer;
		pTransactionInfo[0].Flags = NVODM_I2C_IS_WRITE;
		pTransactionInfo[0].NumBytes = 2;

		pTransactionInfo[1].Address = (wm8994->i2c_address | 0x1);
		pTransactionInfo[1].Buf = pReadBuffer;
		pTransactionInfo[1].Flags = 0;
		pTransactionInfo[1].NumBytes = 2;

		status =
		    NvOdmI2cTransaction(wm8994->h_gen2_i2c, pTransactionInfo, 2,
					400, WM8994_I2C_TIMEOUT);
	}

	if (status != NvOdmI2cStatus_Success) {
		printk("NvOdmWM8994I2cRead Failed: %d\n", status);
		NvOdmOsFree(pReadBuffer);
		NvOdmOsFree(pTransactionInfo);
		return NV_FALSE;
	}

	*Data = (NvU32) ((pReadBuffer[0] << 8) | pReadBuffer[1]);

	NvOdmOsFree(pReadBuffer);
	NvOdmOsFree(pTransactionInfo);
	return NV_TRUE;
}

unsigned int wm8994_read(int codec, unsigned int reg)
{
	NvU32 r_data;
	ReadWolfsonRegister(g_wm8994, reg, &r_data);
	return r_data;
}

ssize_t wm8994_reg_store(struct device * dev, struct device_attribute * attr,
			 const char *buf, size_t count)
{
	int reg, data;
	char *r, *d;

	r = &buf[0];
	d = &buf[7];

	reg = simple_strtoul(r, NULL, 16);
	data = simple_strtoul(d, NULL, 16);

	if (reg == 0) {
		return count;	//bolck reset cmd.
	} else {
		WriteWolfsonRegister(g_wm8994, reg, data);
	}

	return count;
}

/* Custom Code */

static ssize_t show_wm8994_register_dump(struct device *dev,
					 struct device_attribute *attr,
					 char *buf)
{
	// modified version of register_dump from wm8994_aries.c
	// r = wm8994 register
	int r;

	for (r = 0; r <= 0x6; r++)
		sprintf(buf, "%s0x%X 0x%X\n", buf, r, wm8994_read(codec, r));

	sprintf(buf, "%s0x%X 0x%X\n", buf, 0x15, wm8994_read(codec, 0x15));

	for (r = 0x18; r <= 0x3C; r++)
		sprintf(buf, "%s0x%X 0x%X\n", buf, r, wm8994_read(codec, r));

	sprintf(buf, "%s0x%X 0x%X\n", buf, 0x4C, wm8994_read(codec, 0x4C));

	for (r = 0x51; r <= 0x5C; r++)
		sprintf(buf, "%s0x%X 0x%X\n", buf, r, wm8994_read(codec, r));

	sprintf(buf, "%s0x%X 0x%X\n", buf, 0x60, wm8994_read(codec, 0x60));
	sprintf(buf, "%s0x%X 0x%X\n", buf, 0x101, wm8994_read(codec, 0x101));
	sprintf(buf, "%s0x%X 0x%X\n", buf, 0x110, wm8994_read(codec, 0x110));
	sprintf(buf, "%s0x%X 0x%X\n", buf, 0x111, wm8994_read(codec, 0x111));

	for (r = 0x200; r <= 0x212; r++)
		sprintf(buf, "%s0x%X 0x%X\n", buf, r, wm8994_read(codec, r));

	for (r = 0x220; r <= 0x224; r++)
		sprintf(buf, "%s0x%X 0x%X\n", buf, r, wm8994_read(codec, r));

	for (r = 0x240; r <= 0x244; r++)
		sprintf(buf, "%s0x%X 0x%X\n", buf, r, wm8994_read(codec, r));

	for (r = 0x300; r <= 0x317; r++)
		sprintf(buf, "%s0x%X 0x%X\n", buf, r, wm8994_read(codec, r));

	for (r = 0x400; r <= 0x411; r++)
		sprintf(buf, "%s0x%X 0x%X\n", buf, r, wm8994_read(codec, r));

	for (r = 0x420; r <= 0x423; r++)
		sprintf(buf, "%s0x%X 0x%X\n", buf, r, wm8994_read(codec, r));

	for (r = 0x440; r <= 0x444; r++)
		sprintf(buf, "%s0x%X 0x%X\n", buf, r, wm8994_read(codec, r));

	for (r = 0x450; r <= 0x454; r++)
		sprintf(buf, "%s0x%X 0x%X\n", buf, r, wm8994_read(codec, r));

	for (r = 0x480; r <= 0x493; r++)
		sprintf(buf, "%s0x%X 0x%X\n", buf, r, wm8994_read(codec, r));

	for (r = 0x4A0; r <= 0x4B3; r++)
		sprintf(buf, "%s0x%X 0x%X\n", buf, r, wm8994_read(codec, r));

	for (r = 0x500; r <= 0x503; r++)
		sprintf(buf, "%s0x%X 0x%X\n", buf, r, wm8994_read(codec, r));

	sprintf(buf, "%s0x%X 0x%X\n", buf, 0x510, wm8994_read(codec, 0x510));
	sprintf(buf, "%s0x%X 0x%X\n", buf, 0x520, wm8994_read(codec, 0x520));
	sprintf(buf, "%s0x%X 0x%X\n", buf, 0x521, wm8994_read(codec, 0x521));

	for (r = 0x540; r <= 0x544; r++)
		sprintf(buf, "%s0x%X 0x%X\n", buf, r, wm8994_read(codec, r));

	for (r = 0x580; r <= 0x593; r++)
		sprintf(buf, "%s0x%X 0x%X\n", buf, r, wm8994_read(codec, r));

	for (r = 0x600; r <= 0x614; r++)
		sprintf(buf, "%s0x%X 0x%X\n", buf, r, wm8994_read(codec, r));

	sprintf(buf, "%s0x%X 0x%X\n", buf, 0x620, wm8994_read(codec, 0x620));
	sprintf(buf, "%s0x%X 0x%X\n", buf, 0x621, wm8994_read(codec, 0x621));

	for (r = 0x700; r <= 0x70A; r++)
		sprintf(buf, "%s0x%X 0x%X\n", buf, r, wm8994_read(codec, r));

	return sprintf(buf, "%s", buf);
}

ssize_t wm8994_wakelock_store(struct device *dev, struct device_attribute *attr,
			      const char *buf, size_t count)
{
	unsigned int n, lock;

	n = sscanf(buf, "%u", &lock);
	if (n != 1)
		return -1;

	if ((bool) lock) {
		wake_lock(&g_wm8994->wm8994_wake_lock);
	} else {
		wake_unlock(&g_wm8994->wm8994_wake_lock);
	}

	return count;
}

static DEVICE_ATTR(wm8994_wakelock_voodoo, 0666, NULL, wm8994_wakelock_store);

void star_headsetdet_bias(int bias)
{
	NvU32 r_data = 0;
	ReadWolfsonRegister(g_wm8994, 0x0001, &r_data);
	if (bias == 0) {
		r_data = r_data & (~0x0020);
		printk("star_headsetdet_bias headset disabled %4x\n", r_data);
	} else {
		r_data = r_data | (0x0020);
		printk("star_headsetdet_bias headset enabled %4x\n", r_data);
	}
	WriteWolfsonRegister(g_wm8994, 0x0001, r_data);
	return;
}

static DEVICE_ATTR(wm8994_register_dump, S_IRUGO, show_wm8994_register_dump, NULL);

static struct attribute *voodoo_sound_attributes[] = {
	&dev_attr_wm8994_register_dump.attr,
	NULL
};

static struct attribute_group voodoo_sound_group = {
	.attrs = voodoo_sound_attributes,
};

static struct miscdevice voodoo_sound_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "voodoo_sound",
};

/**
 * All the device spefic initializations happen here. 
 */

static int __init voodoo_sound_init(void)
{

	NvS32 err = 0;

	const NvOdmPeripheralConnectivity *pcon = NULL;

	printk("Voodoo sound: initializing driver v%d\n", VOODOO_SOUND_VERSION);

	pcon =
	    NvOdmPeripheralGetGuid(NV_ODM_GUID
				   ('w', 'o', 'l', 'f', '8', '9', '9', '4'));
	if (pcon == NULL) {
		return err;
	}
	g_wm8994 = kzalloc(sizeof(*g_wm8994), GFP_KERNEL);
	g_wm8994->i2c_address = pcon->AddressList[2].Address;
	g_wm8994->h_gen2_i2c =
	    NvOdmI2cPinMuxOpen(NvOdmIoModule_I2c, 1, NvOdmI2cPinMap_Config2);

	if (g_wm8994->h_gen2_i2c == NULL) {
		return err;
	}

	misc_register(&voodoo_sound_device);
	if (sysfs_create_group(&voodoo_sound_device.this_device->kobj,
			       &voodoo_sound_group) < 0) {
		printk("%s sysfs_create_group fail\n", __FUNCTION__);
		pr_err("Failed to create sysfs group for (%s)!\n",
		       voodoo_sound_device.name);
	}

	return 0;
}

void __exit voodoo_sound_exit(void)
{
	printk("Voodoo sound: removing driver v%d\n", VOODOO_SOUND_VERSION);

	sysfs_remove_group(&voodoo_sound_device.this_device->kobj,
			   &voodoo_sound_group);
	misc_deregister(&voodoo_sound_device);
}

MODULE_DESCRIPTION("Voodoo sound for LG Optimus 2x");
MODULE_AUTHOR("supercurio");
MODULE_LICENSE("GPL");

module_init(voodoo_sound_init);
module_exit(voodoo_sound_exit);
