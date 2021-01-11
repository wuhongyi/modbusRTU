/*	This file is part of Advantech VCOM Linux.
 *
 *	Copyright (c) 2009 - 2018 ADVANTECH CORPORATION.  All rights reserved. 
 *
 *	Advantech VCOM Linux is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License 2 as published by
 *	the Free Software Foundation.
 *
 *	Advantech VCOM Linux  is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with Advantech VCOM Linux.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <linux/module.h>

#include <linux/mm.h>           /* everything */
#include <linux/errno.h>        /* error codes */
#include <linux/fs.h>
#include <asm/pgtable.h>
#include <linux/version.h>
#include "advvcom.h"

void adv_vma_open(struct vm_area_struct *vma)
{
//	printk("%s(%d)\n", __func__, __LINE__);
}

void adv_vma_close(struct vm_area_struct *vma)
{
//	printk("%s(%d)\n", __func__, __LINE__);
}

#if  LINUX_VERSION_CODE >= KERNEL_VERSION(4,10,0)
int adv_vma_nopage(struct vm_fault *vmf)
{
	unsigned long offset;
	struct adv_vcom * data;
	struct vm_area_struct *vma = vmf->vma;
	struct page *page = NULL;
//	void * pageptr = NULL;
	int ret = 0;

//	printk("%s(%d)\n", __func__, __LINE__);
	data = vma->vm_private_data;

//	printk("data = %x\n", data);
	offset = (unsigned long)(vmf->address - vma->vm_start) + (vma->vm_pgoff << PAGE_SHIFT);
//	printk("offset = %u \n", offset);
//	printk("totalsize = %d\n", (data->rx.size + data->tx.size));


	if(offset > (data->rx.size + data->tx.size) ){
		printk("%s(%d)\n", __func__, __LINE__);
		goto out;
	}
	
//	page = virt_to_page(data->data);
	if(offset < (data->tx.size + data->tx.begin)){
//		printk("mapping tx page\n");
		page = virt_to_page(data->tx.data);
	}else if(offset < (data->rx.size + data->rx.begin)){
//		printk("mapping rx page\n");
		page = virt_to_page(data->rx.data);
	}else if(offset < (data->attr.size + data->attr.begin)){
//		printk("mapping attr page\n");
		page = virt_to_page(data->attr.data);
	}else{
//		printk("%s(%d)\n", __func__, __LINE__);
		goto out;
	}

	get_page(page);
	vmf->page = page;
		
	out:
	return ret;
}
#else
int adv_vma_nopage(struct vm_area_struct *vma, struct vm_fault *vmf)
{
	unsigned long offset;
	struct adv_vcom * data;
	struct page *page = NULL;
	int ret = 0;

	data = vma->vm_private_data;

	offset = (unsigned long)(vmf->virtual_address - vma->vm_start) + (vma->vm_pgoff << PAGE_SHIFT);


	if(offset > (data->rx.size + data->tx.size) ){
		printk("%s(%d)\n", __func__, __LINE__);
		goto out;
	}
	
	if(offset < (data->tx.size + data->tx.begin)){
		page = virt_to_page(data->tx.data);
	}else if(offset < (data->rx.size + data->rx.begin)){
		page = virt_to_page(data->rx.data);
	}else if(offset < (data->attr.size + data->attr.begin)){
		page = virt_to_page(data->attr.data);
	}else{
		goto out;
	}

	get_page(page);
	vmf->page = page;
		
	out:
	return ret;
}
#endif

struct vm_operations_struct adv_vm_ops = {
        .open =     adv_vma_open,
        .close =    adv_vma_close,
        .fault =    adv_vma_nopage,
};

int adv_proc_mmap(struct file *filp, struct vm_area_struct *vma)
{
//	printk("%s(%d)\n", __func__, __LINE__);

//	struct inode *inode = filp->f_dentry->d_inode;

	/* refuse to map if order is not 0 */
//	if (scullp_devices[iminor(inode)].order)
//		return -ENODEV;

	/* don't do anything here: "nopage" will set up page table entries */
	vma->vm_ops = &adv_vm_ops;
	vma->vm_private_data = filp->private_data;
//	adv_vma_open(vma);
	return 0;

}
