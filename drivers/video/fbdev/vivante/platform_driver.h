int loongson_drv_open(struct inode* inode, struct file* filp);

int loongson_drv_release(struct inode* inode, struct file* filp);

long loongson_drv_ioctl(struct file* filp, unsigned int ioctlCode, unsigned long arg);

int loongson_drv_mmap(struct file* filp, struct vm_area_struct* vma);

int loongson_gpu_probe(struct platform_device *pdev);

int loongson_gpu_remove(struct platform_device *pdev);

int loongson_gpu_suspend(struct platform_device *dev, pm_message_t state);

int loongson_gpu_resume(struct platform_device *dev);
