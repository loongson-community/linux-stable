int loongson_drv_open(struct inode* inode, struct file* filp);

int loongson_drv_release(struct inode* inode, struct file* filp);

long loongson_drv_ioctl(struct file* filp, unsigned int ioctlCode, unsigned long arg);

int loongson_drv_mmap(struct file* filp, struct vm_area_struct* vma);

int loongson_gpu_plat_probe(struct platform_device *pdev);

int loongson_gpu_plat_remove(struct platform_device *pdev);

int loongson_gpu_plat_suspend(struct platform_device *dev, pm_message_t state);

int loongson_gpu_plat_resume(struct platform_device *dev);

int loongson_gpu_pci_probe(struct pci_dev *pdev, const struct pci_device_id *ent);

void loongson_gpu_pci_remove(struct pci_dev *pdev);

int loongson_gpu_pci_suspend(struct pci_dev *pdev, pm_message_t mesg);

int loongson_gpu_pci_resume(struct pci_dev *pdev);
