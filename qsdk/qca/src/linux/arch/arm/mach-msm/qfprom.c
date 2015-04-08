/*
 * Copyright (c) 2014 The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/kobject.h>
#include <linux/string.h>
#include <linux/sysfs.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/sysdev.h>
#include <mach/msm_iomap.h>
#include <mach/scm.h>
#include <asm/mach-types.h>

#define QFPROM_RAW_SPARE_REG27_ROW0_LSB IPQ806X_QFPROM_PHYS + 0x440

#define QFPROM_ROW_READ_CMD 		0x5
#define QFPROM_ROW_ROLLBACK_WRITE_CMD 	0x6

static ssize_t
qfprom_show_version(struct sys_device *dev,
			struct sysdev_attribute *attr,
			char *buf)
{
	uint64_t version;
	uint32_t qfprom_api_status;
	int32_t ret;

	struct qfprom_read_ip {
		uint32_t row_reg_addr;
		uint32_t addr_type;
		uint32_t row_data_addr;
		uint32_t qfprom_ret_ptr;
	} rdip;

	rdip.row_reg_addr = QFPROM_RAW_SPARE_REG27_ROW0_LSB;
	rdip.addr_type = 0;
	rdip.row_data_addr = virt_to_phys(&version);
	rdip.qfprom_ret_ptr = virt_to_phys(&qfprom_api_status);

	ret = scm_call(SCM_SVC_FUSE, QFPROM_ROW_READ_CMD,
				&rdip, sizeof(rdip), NULL, 0);

	if (ret && qfprom_api_status) {
		pr_err("%s: Error in QFPROM read (%d, 0x%x)\n",
				__func__, ret, qfprom_api_status);
		return ret;
	}

	return sprintf(buf, "0x%llX\n", version);
}

static ssize_t
qfprom_store_version(struct sys_device *dev,
			struct sysdev_attribute *attr,
			const char *buf, size_t count)
{
	uint64_t version;
	uint32_t qfprom_api_status;
	int32_t ret;

	struct qfprom_write_ip {
		uint32_t row_reg_addr;
		uint32_t row_data_addr;
		uint32_t bus_clk;
		uint32_t qfprom_ret_ptr;
	} wrip;

	/* Input validation handled here */
	ret = kstrtoull(buf, 0, &version);
	if (ret)
		return ret;

	wrip.row_reg_addr = QFPROM_RAW_SPARE_REG27_ROW0_LSB;
	wrip.row_data_addr = virt_to_phys(&version);
	wrip.bus_clk = (64 * 1000);
	wrip.qfprom_ret_ptr = virt_to_phys(&qfprom_api_status);

	ret = scm_call(SCM_SVC_FUSE, QFPROM_ROW_ROLLBACK_WRITE_CMD,
				&wrip, sizeof(wrip), NULL, 0);

	if (ret && qfprom_api_status) {
		pr_err("%s: Error in QFPROM write (%d, 0x%x)\n",
				__func__, ret, qfprom_api_status);
		return ret;
	}

	/* Return the count argument since entire buffer is used */
	return count;
}

static struct sysdev_attribute qfprom_files[] = {
	_SYSDEV_ATTR(version, 0666, qfprom_show_version,
					qfprom_store_version),
};

static struct sysdev_class qfprom_sysdev_class = {
	.name = "qfprom",
};

static struct sys_device qfprom_sys_device = {
	.id = 0,
	.cls = &qfprom_sysdev_class,
};

static int __init qfprom_create_files(struct sys_device *dev,
					struct sysdev_attribute files[],
					int size)
{
	int i;
	for (i = 0; i < size; i++) {
		int err = sysdev_create_file(dev, &files[i]);
		if (err) {
			pr_err("%s: sysdev_create_file(%s)=%d\n",
				__func__, files[i].attr.name, err);
			return err;
		}
	}
	return 0;
}

static int __init qfprom_init_sysdev(void)
{
	int err;

	err = sysdev_class_register(&qfprom_sysdev_class);
	if (err) {
		pr_err("%s: sysdev_class_register fail (%d)\n",
			__func__, err);
		return err;
	}

	err = sysdev_register(&qfprom_sys_device);
	if (err) {
		pr_err("%s: sysdev_register fail (%d)\n",
			__func__, err);
		return err;
	}

	return qfprom_create_files(&qfprom_sys_device, qfprom_files,
				ARRAY_SIZE(qfprom_files));
}

arch_initcall(qfprom_init_sysdev);