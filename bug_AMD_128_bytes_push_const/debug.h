void print_surface_capabilities(struct vk_swapchain *swapchain)
{
  const char *transforms[] = {
    "IDENTITY",
    "ROTATE_90",
    "ROTATE_180",
    "ROTATE_270",
    "HORIZONTAL_MIRROR",
    "HORIZONTAL_MIRROR_ROTATE_90",
    "HORIZONTAL_MIRROR_ROTATE_180",
    "HORIZONTAL_MIRROR_ROTATE_270",
    "INHERIT",
  };
  const char *alphas[] = {
    "OPAQUE",
    "PRE_MULTIPLIED",
    "POST_MULTIPLIED",
    "INHERIT",
  };
  const char *image_usages[] = {
    "TRANSFER_SRC",
    "TRANSFER_DST",
    "SAMPLED",
    "STORAGE",
    "COLOR_ATTACHMENT",
    "DEPTH_STENCIL_ATTACHMENT",
    "TRANSIENT_ATTACHMENT",
    "INPUT_ATTACHMENT",
  };
  const char *present_modes[] = {
    "IMMEDIATE",
    "MAILBOX",
    "FIFO",
    "FIFO_RELAXED",
  };
  VkSurfaceCapabilitiesKHR *caps = &swapchain->surface_caps;

  printf( "Surface capabilities:\n"
    " - image count in range [%u, %u]\n"
    " - image extent between (%u, %u) and (%u, %u) (current: (%u, %u))\n"
    " - stereoscopic possible? %s\n"
    " - supported transforms:\n",
    caps->minImageCount,
    caps->maxImageCount,
    caps->minImageExtent.width,
    caps->minImageExtent.height,
    caps->maxImageExtent.width,
    caps->maxImageExtent.height,
    caps->currentExtent.width,
    caps->currentExtent.height,
    caps->maxImageArrayLayers > 1?"Yes":"No");

  for (size_t i = 0; i < sizeof transforms / sizeof *transforms; ++i)
    if ((caps->supportedTransforms & 1 << i))
      printf("    * %s%s\n", transforms[i], caps->currentTransform == 1 << i?" (current)":"");
  if (caps->supportedTransforms >= 1 << sizeof transforms / sizeof *transforms)
    printf("    * ...%s\n", caps->currentTransform >= 1 << sizeof transforms / sizeof *transforms?" (current)":"");

  printf(" - supported alpha composition:\n");
  for (size_t i = 0; i < sizeof alphas / sizeof *alphas; ++i)
    if ((caps->supportedCompositeAlpha & 1 << i))
      printf("    * %s\n", alphas[i]);
  if (caps->supportedCompositeAlpha >= 1 << sizeof alphas / sizeof *alphas)
    printf("    * ...\n");

  printf(" - supported image usages:\n");
  for (size_t i = 0; i < sizeof image_usages / sizeof *image_usages; ++i)
    if ((caps->supportedUsageFlags & 1 << i))
      printf("    * %s\n", image_usages[i]);
  if (caps->supportedUsageFlags >= 1 << sizeof image_usages / sizeof *image_usages)
    printf("    * ...\n");
  printf(" - supported present modes:\n");
  for (int i = 0; i < swapchain->present_modes_count; ++i)
    if (swapchain->present_modes[i] >= sizeof present_modes / sizeof *present_modes)
      printf("    * <UNKNOWN MODE(%u)>\n", swapchain->present_modes[i]);
    else
      printf("    * %s\n", present_modes[swapchain->present_modes[i]]);
}

void print_present_mode(VkPresentModeKHR present_mode)
{
    printf("Using present mode: %s\n", present_mode == VK_PRESENT_MODE_IMMEDIATE_KHR ? "VK_PRESENT_MODE_IMMEDIATE_KHR" :
    present_mode == VK_PRESENT_MODE_MAILBOX_KHR ? "VK_PRESENT_MODE_MAILBOX_KHR" :
    present_mode == VK_PRESENT_MODE_FIFO_KHR ? "VK_PRESENT_MODE_FIFO_KHR" :
    present_mode == VK_PRESENT_MODE_FIFO_RELAXED_KHR ? "VK_PRESENT_MODE_FIFO_RELAXED_KHR" : "nan");
}

void print_vkinfo(struct vk_physical_device **phy_dev, struct vk_swapchain **swapchains, uint32_t dev_count, VkPresentModeKHR present_mode){
    for (uint32_t i = 0; i < dev_count; ++i)
  {
    struct vk_physical_device *dev = phy_dev[i];
    VkPhysicalDeviceProperties *pr = &dev->properties;

    printf("  - %s: %s (id: 0x%04X) from vendor 0x%04X [driver version: 0x%04X, API version: 0x%04X]\n",
        vk_VkPhysicalDeviceType_string(pr->deviceType), pr->deviceName,
        pr->deviceID, pr->vendorID, pr->driverVersion, pr->apiVersion);
    if (dev->queue_families_incomplete)
    {
      printf("    (INFO)vkGetPhysicalDeviceQueueFamilyProperties return more then %u members.\n", dev->queue_family_count);
    }
    else
      printf("    The device supports the following %u queue famil%s:\n", dev->queue_family_count, dev->queue_family_count == 1?"y":"ies");

    for (uint32_t j = 0; j < dev->queue_family_count; ++j)
    {
      VkQueueFamilyProperties *qf = &dev->queue_families[j];

      printf("    * %u queue%s with the following capabilit%s:\n", qf->queueCount, qf->queueCount == 1?"":"s",
          qf->queueFlags && (qf->queueFlags & (qf->queueFlags - 1)) == 0?"y":"ies");
      if (qf->queueFlags == 0)
        printf("          None\n");
      if ((qf->queueFlags & VK_QUEUE_GRAPHICS_BIT))
        printf("          Graphics\n");
      if ((qf->queueFlags & VK_QUEUE_COMPUTE_BIT))
        printf("          Compute\n");
      if ((qf->queueFlags & VK_QUEUE_TRANSFER_BIT))
        printf("          Transfer\n");
      if ((qf->queueFlags & VK_QUEUE_SPARSE_BINDING_BIT))
        printf("          Sparse binding\n");
    }

    printf("    The device supports memories of the following types:\n");
    for (uint32_t j = 0; j < dev->memories.memoryTypeCount; ++j)
    {
      printf("    *");
      if (dev->memories.memoryTypes[j].propertyFlags == 0)
        printf(" <no properties>");
      if ((dev->memories.memoryTypes[j].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT))
        printf(" device-local");
      if ((dev->memories.memoryTypes[j].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT))
        printf(" host-visible");
      if ((dev->memories.memoryTypes[j].propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
        printf(" host-coherent");
      if ((dev->memories.memoryTypes[j].propertyFlags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT))
        printf(" host-cached");
      if ((dev->memories.memoryTypes[j].propertyFlags & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT))
        printf(" lazy");
      printf(": Available in Heap of size %luMB\n", dev->memories.memoryHeaps[dev->memories.memoryTypes[j].heapIndex].size / (1024 * 1024));
    }
    print_surface_capabilities(swapchains[i]);
    print_present_mode(present_mode);
  }
}
