// Danil, 2020 Vulkan shader launcher, self https://github.com/danilw/vulkan-shader-launcher

#include <stdio.h>
#include <stdlib.h>

// define to not resize offscreen-buf on resize
//#define NO_RESIZE_BUF

#if defined(VK_USE_PLATFORM_XCB_KHR)||defined(VK_USE_PLATFORM_WAYLAND_KHR)
#include <unistd.h>
#endif

#ifdef _WIN32
#pragma comment(linker, "/subsystem:windows")
#endif

#include <time.h>
#include "../vk_utils/vk_utils.h"
#include "../vk_utils/vk_render_helper.h"
#include "../os_utils/utils.h"

#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
static int resize_size[2] = {1280, 720}; // in Wayland surface should set own size
#endif

static bool main_image_srgb = false; // srgb surface fix

#ifdef YARIV_SHADER
const unsigned char buf_fs_code[] = {
#include "shaders/src/bin/buf.frag.hex"
};
const unsigned char buf_vs_code[] = {
#include "shaders/src/bin/buf.vert.hex"
};
const unsigned char fs_code[] = {
#include "shaders/src/bin/main.frag.hex"
};
const unsigned char vs_code[] = {
#include "shaders/src/bin/main.vert.hex"
};
#endif

struct shaders_uniforms
{
    float iMouse[4];
    float iDate[4];
    int iMouse_lr[2];
    float iResolution[2];
    int debugdraw;
    int pCustom; //custom data
    float iTime;
    float iTimeDelta;
    int iFrame;
};

enum
{
    BUFFER_VERTICES = 0,
    BUFFER_INDICES = 1,
};
enum
{
    SHADER_BUF_VERTEX = 0,
    SHADER_BUF_FRAGMENT = 1,
    SHADER_MAIN_VERTEX = 2,
    SHADER_MAIN_FRAGMENT = 3,
};

struct render_data
{
    struct objects
    {
        struct vertex
        {
            float pos[3];
        } vertices[3];

        uint16_t indices[3];
    } objects;

    struct shaders_uniforms push_constants;

    struct vk_buffer buffers[2];
    struct vk_shader shaders[4];
    struct vk_graphics_buffers *main_gbuffers;
    struct vk_offscreen_buffers *buf_obuffers;

    VkRenderPass buf_render_pass;
    struct vk_layout buf_layout;
    struct vk_pipeline buf_pipeline;
    VkDescriptorSet buf_desc_set;

    VkRenderPass main_render_pass;
    struct vk_layout main_layout;
    struct vk_pipeline main_pipeline;
    VkDescriptorSet main_desc_set;
};

struct render_data render_data = {
    .main_gbuffers = NULL,
    .buf_obuffers = NULL,
};
VkInstance vk;
struct vk_physical_device phy_dev;
struct vk_device dev;
struct vk_swapchain swapchain = {0};
struct app_os_window os_window;
struct vk_render_essentials essentials;

VkFence offscreen_fence = VK_NULL_HANDLE;
VkQueue offscreen_queue = VK_NULL_HANDLE;
bool use_one_VkQueue = false; // I check if more then 1 VK_QUEUE_GRAPHICS_BIT suported, then it set to true
VkCommandBuffer offscreen_cmd_buffer = VK_NULL_HANDLE;
VkSemaphore wait_buf_sem = VK_NULL_HANDLE, wait_main_sem = VK_NULL_HANDLE;
bool first_submission = true;

#if defined(VK_USE_PLATFORM_WIN32_KHR)
#include <shellapi.h>

static bool render_loop_draw(struct vk_physical_device *phy_dev, struct vk_device *dev, struct vk_swapchain *swapchain,
                             struct app_os_window *os_window);
static bool on_window_resize(struct vk_physical_device *phy_dev, struct vk_device *dev,
                             struct vk_render_essentials *essentials, struct vk_swapchain *swapchain,
                             struct render_data *render_data, struct app_os_window *os_window);

#include "../os_utils/os_win_utils.h"
#elif defined(VK_USE_PLATFORM_XCB_KHR)
#include "../os_utils/xcb_x11_utils.h"
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
#include "../os_utils/wayland_utils.h"
#endif

static vk_error allocate_render_data(struct vk_physical_device *phy_dev, struct vk_device *dev,
                                     struct vk_swapchain *swapchain, struct vk_render_essentials *essentials,
                                     struct render_data *render_data, bool reload_shaders)
{

    static bool load_once = false;
    vk_error retval = VK_ERROR_NONE;
    VkResult res;
    if (!load_once)
    {
        render_data->buffers[BUFFER_VERTICES] = (struct vk_buffer){
            .size = sizeof render_data->objects.vertices,
            .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            .host_visible = false,
        };

        render_data->buffers[BUFFER_INDICES] = (struct vk_buffer){
            .size = sizeof render_data->objects.indices,
            .usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            .host_visible = false,
        };

        retval = vk_create_buffers(phy_dev, dev, render_data->buffers, 2);
        if (!vk_error_is_success(&retval))
        {
            vk_error_printf(&retval, "Failed to create vertex, index and transformation buffers\n");
            return retval;
        }
    }
    if (!load_once)
    {
        render_data->objects = (struct objects){
            .vertices =
                {
                    [0] =
                        (struct vertex){
                            .pos = {3.001, 1.001, 0.0},
                        },
                    [1] =
                        (struct vertex){
                            .pos = {-1.001, -3.001, 0.0},
                        },
                    [2] =
                        (struct vertex){
                            .pos = {-1.001, 1.001, 0.0},
                        },
                },
            .indices =
                {
                    0,
                    1,
                    2,
                },
        };
    }
    if (!load_once)
    {
        retval = vk_render_init_buffer(phy_dev, dev, essentials, &render_data->buffers[BUFFER_VERTICES],
                                       render_data->objects.vertices, "vertex");
        if (!vk_error_is_success(&retval))
            return retval;
        retval = vk_render_init_buffer(phy_dev, dev, essentials, &render_data->buffers[BUFFER_INDICES],
                                       render_data->objects.indices, "index");
        if (!vk_error_is_success(&retval))
            return retval;
    }
    if ((!load_once) || (reload_shaders))
    {
        render_data->shaders[SHADER_MAIN_VERTEX] = (struct vk_shader){
            .spirv_file = "shaders/spv/main.vert.spv",
            .stage = VK_SHADER_STAGE_VERTEX_BIT,
        };
        render_data->shaders[SHADER_MAIN_FRAGMENT] = (struct vk_shader){
            .spirv_file = "shaders/spv/main.frag.spv",
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        };

        render_data->shaders[SHADER_BUF_VERTEX] = (struct vk_shader){
            .spirv_file = "shaders/spv/buf.vert.spv",
            .stage = VK_SHADER_STAGE_VERTEX_BIT,
        };
        render_data->shaders[SHADER_BUF_FRAGMENT] = (struct vk_shader){
            .spirv_file = "shaders/spv/buf.frag.spv",
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        };
#ifdef YARIV_SHADER
        retval = vk_load_shader_yariv(dev, (const uint32_t *)buf_vs_code,
                                      &render_data->shaders[SHADER_BUF_VERTEX].shader, sizeof(buf_vs_code));
        if (!vk_error_is_success(&retval))
        {
            vk_error_printf(&retval, "Could not load the shaders\n");
            return retval;
        }
        retval = vk_load_shader_yariv(dev, (const uint32_t *)buf_fs_code,
                                      &render_data->shaders[SHADER_BUF_FRAGMENT].shader, sizeof(buf_fs_code));
        if (!vk_error_is_success(&retval))
        {
            vk_error_printf(&retval, "Could not load the shaders\n");
            return retval;
        }
        retval = vk_load_shader_yariv(dev, (const uint32_t *)vs_code, &render_data->shaders[SHADER_MAIN_VERTEX].shader,
                                      sizeof(vs_code));
        if (!vk_error_is_success(&retval))
        {
            vk_error_printf(&retval, "Could not load the shaders\n");
            return retval;
        }
        retval = vk_load_shader_yariv(dev, (const uint32_t *)fs_code,
                                      &render_data->shaders[SHADER_MAIN_FRAGMENT].shader, sizeof(fs_code));
        if (!vk_error_is_success(&retval))
        {
            vk_error_printf(&retval, "Could not load the shaders\n");
            return retval;
        }
#else
        retval = vk_load_shaders(dev, render_data->shaders, 4);
        if (!vk_error_is_success(&retval))
        {
            vk_error_printf(&retval, "Could not load the shaders\n");
            return retval;
        }
#endif
    }
  struct VkExtent2D init_size;
#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
  init_size.width = resize_size[0];
  init_size.height = resize_size[1];
#else
  init_size.width = swapchain->surface_caps.currentExtent.width;
  init_size.height = swapchain->surface_caps.currentExtent.height;
#endif
    render_data->main_gbuffers = malloc(essentials->image_count * sizeof *render_data->main_gbuffers);
    for (uint32_t i = 0; i < essentials->image_count; ++i)
        render_data->main_gbuffers[i] = (struct vk_graphics_buffers){
            .surface_size = init_size,
            .swapchain_image = essentials->images[i],
        };

#ifdef NO_RESIZE_BUF
    if (!load_once)
    {
#endif
        render_data->buf_obuffers = malloc(2 * sizeof *render_data->buf_obuffers);
        for (uint32_t i = 0; i < 2; i++)
            render_data->buf_obuffers[i] = (struct vk_offscreen_buffers){
                .surface_size = init_size,
            };
#ifdef NO_RESIZE_BUF
    }
#endif

    // 8bit BGRA for main_image VK_FORMAT_B8G8R8A8_UNORM
    if(swapchain->surface_format.format!=VK_FORMAT_B8G8R8A8_UNORM)main_image_srgb=true;
    retval = vk_create_graphics_buffers(phy_dev, dev, swapchain->surface_format.format, render_data->main_gbuffers,
                                        essentials->image_count, &render_data->main_render_pass, VK_C_CLEAR,
                                        VK_WITHOUT_DEPTH);
    if (!vk_error_is_success(&retval))
    {
        vk_error_printf(&retval, "Could not create graphics buffers\n");
        return retval;
    }

#ifdef NO_RESIZE_BUF
    if (!load_once)
    {
#endif
        // 32 bit format RGBA for buffers VK_FORMAT_R32G32B32A32_SFLOAT
        retval = vk_create_offscreen_buffers(phy_dev, dev, VK_FORMAT_R32G32B32A32_SFLOAT, render_data->buf_obuffers, 2,
                                             &render_data->buf_render_pass, VK_C_CLEAR, VK_WITHOUT_DEPTH);
        if (!vk_error_is_success(&retval))
        {
            vk_error_printf(&retval, "Could not create off-screen buffers\n");
            return retval;
        }
#ifdef NO_RESIZE_BUF
    }
#endif

    struct vk_image **image_pointer;
    image_pointer = malloc(1 * sizeof(struct vk_image *));
    image_pointer[0] = &render_data->buf_obuffers[0].color;
    // image_pointer[1]=&render_data->buf_obuffers[1].color;

    VkPushConstantRange push_constant_range = {
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
        .offset = 0,
        .size = sizeof render_data->push_constants,
    };

#ifdef NO_RESIZE_BUF
    if (!load_once)
    {
#endif

        /*******************
         * BUF PART *
         *******************/
        {
            /* Layouts */

            struct vk_resources resources = {
                .images = *image_pointer,
                .image_count = 1,
                .buffers = render_data->buffers,
                .buffer_count = 2,
                .shaders = &render_data->shaders[SHADER_BUF_VERTEX],
                .shader_count = 2,
                .push_constants = &push_constant_range,
                .push_constant_count = 1,
                .render_pass = render_data->buf_render_pass,
            };
            render_data->buf_layout = (struct vk_layout){
                .resources = &resources,
            };
            retval = vk_make_graphics_layouts(dev, &render_data->buf_layout, 1);
            if (!vk_error_is_success(&retval))
            {
                vk_error_printf(&retval, "BUF: Could not create descriptor set or pipeline layouts\n");
                return retval;
            }

            /* Pipeline */
            VkVertexInputBindingDescription vertex_binding = {
                .binding = 0,
                .stride = sizeof *render_data->objects.vertices,
                .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
            };

            VkVertexInputAttributeDescription vertex_attributes[1] = {
                [0] =
                    {
                        .location = 0,
                        .binding = 0,
                        .format = VK_FORMAT_R32G32B32_SFLOAT,
                        .offset = 0,
                    },
            };
            render_data->buf_pipeline = (struct vk_pipeline){
                .layout = &render_data->buf_layout,
                .vertex_input_state =
                    {
                        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
                        .vertexBindingDescriptionCount = 1,
                        .pVertexBindingDescriptions = &vertex_binding,
                        .vertexAttributeDescriptionCount = 1,
                        .pVertexAttributeDescriptions = vertex_attributes,
                    },
                .input_assembly_state =
                    {
                        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
                        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
                    },
                .tessellation_state =
                    {
                        .sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO,
                    },
                .thread_count = 1,
            };

            retval = vk_make_graphics_pipelines(dev, &render_data->buf_pipeline, 1, false);
            if (!vk_error_is_success(&retval))
            {
                vk_error_printf(&retval, "BUF: Could not create graphics pipeline\n");
                return retval;
            }

            /* Descriptor Set */
            VkDescriptorSetAllocateInfo set_info = {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                .descriptorPool = render_data->buf_pipeline.set_pool,
                .descriptorSetCount = 1,
                .pSetLayouts = &render_data->buf_layout.set_layout,
            };
            res = vkAllocateDescriptorSets(dev->device, &set_info, &render_data->buf_desc_set);
            retval = VK_ERROR_NONE;
            vk_error_set_vkresult(&retval, res);
            if (res)
            {
                vk_error_printf(&retval, "BUF: Could not allocate descriptor set from pool\n");
                return retval;
            }
        }

#ifdef NO_RESIZE_BUF
    }
#endif

    /*******************
     * MAIN_IMAGE PART *
     *******************/
    {
        /* Layouts */

        struct vk_resources resources = {
            .images = *image_pointer,
            .image_count = 1,
            .buffers = render_data->buffers,
            .buffer_count = 2,
            .shaders = &render_data->shaders[SHADER_MAIN_VERTEX],
            .shader_count = 2,
            .push_constants = &push_constant_range,
            .push_constant_count = 1,
            .render_pass = render_data->main_render_pass,
        };
        render_data->main_layout = (struct vk_layout){
            .resources = &resources,
        };
        retval = vk_make_graphics_layouts(dev, &render_data->main_layout, 1);
        if (!vk_error_is_success(&retval))
        {
            vk_error_printf(&retval, "Could not create descriptor set or pipeline layouts\n");
            return retval;
        }

        /* Pipeline */
        VkVertexInputBindingDescription vertex_binding = {
            .binding = 0,
            .stride = sizeof *render_data->objects.vertices,
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
        };

        VkVertexInputAttributeDescription vertex_attributes[1] = {
            [0] =
                {
                    .location = 0,
                    .binding = 0,
                    .format = VK_FORMAT_R32G32B32_SFLOAT,
                    .offset = 0,
                },
        };
        render_data->main_pipeline = (struct vk_pipeline){
            .layout = &render_data->main_layout,
            .vertex_input_state =
                {
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
                    .vertexBindingDescriptionCount = 1,
                    .pVertexBindingDescriptions = &vertex_binding,
                    .vertexAttributeDescriptionCount = 1,
                    .pVertexAttributeDescriptions = vertex_attributes,
                },
            .input_assembly_state =
                {
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
                    .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
                },
            .tessellation_state =
                {
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO,
                },
            .thread_count = 1,
        };

        retval = vk_make_graphics_pipelines(dev, &render_data->main_pipeline, 1, false);
        if (!vk_error_is_success(&retval))
        {
            vk_error_printf(&retval, "Could not create graphics pipeline\n");
            return retval;
        }

        /* Descriptor Set */
        VkDescriptorSetAllocateInfo set_info = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .descriptorPool = render_data->main_pipeline.set_pool,
            .descriptorSetCount = 1,
            .pSetLayouts = &render_data->main_layout.set_layout,
        };
        res = vkAllocateDescriptorSets(dev->device, &set_info, &render_data->main_desc_set);
        retval = VK_ERROR_NONE;
        vk_error_set_vkresult(&retval, res);
        if (res)
        {
            vk_error_printf(&retval, "Could not allocate descriptor set from pool\n");
            return retval;
        }
    }

    load_once = true;
    free(image_pointer);

    return retval;
}

static void free_render_data(struct vk_device *dev, struct vk_render_essentials *essentials,
                             struct render_data *render_data)
{
    vkDeviceWaitIdle(dev->device);

    vk_free_pipelines(dev, &render_data->main_pipeline, 1);
    vk_free_layouts(dev, &render_data->main_layout, 1);
    vk_free_pipelines(dev, &render_data->buf_pipeline, 1);
    vk_free_layouts(dev, &render_data->buf_layout, 1);

    vk_free_buffers(dev, render_data->buffers, 2);
    vk_free_shaders(dev, render_data->shaders, 4);

    vk_free_offscreen_buffers(dev, render_data->buf_obuffers, 2, render_data->buf_render_pass);
    vk_free_graphics_buffers(dev, render_data->main_gbuffers, essentials->image_count, render_data->main_render_pass);

    free(render_data->main_gbuffers);
    free(render_data->buf_obuffers);
}

static void exit_cleanup_render_loop(struct vk_device *dev, struct vk_render_essentials *essentials,
                                     struct render_data *render_data, VkSemaphore wait_buf_sem,
                                     VkSemaphore wait_main_sem, VkFence offscreen_fence)
{
    vkDeviceWaitIdle(dev->device);
    if (offscreen_fence != VK_NULL_HANDLE)
        vkDestroyFence(dev->device, offscreen_fence, NULL);
    if (wait_main_sem != VK_NULL_HANDLE)
        vkDestroySemaphore(dev->device, wait_main_sem, NULL);
    if (wait_buf_sem != VK_NULL_HANDLE)
        vkDestroySemaphore(dev->device, wait_buf_sem, NULL);
    free_render_data(dev, essentials, render_data);
    vk_render_cleanup_essentials(essentials, dev);
}

static void render_loop_init(struct vk_physical_device *phy_dev, struct vk_device *dev, struct vk_swapchain *swapchain,
                             struct app_os_window *os_window)
{
    int res;
    vk_error retval = VK_ERROR_NONE;
    static bool once = false;

    res = vk_render_get_essentials(&essentials, phy_dev, dev, swapchain);
    if (res)
    {
        vk_render_cleanup_essentials(&essentials, dev);
        return;
    }

    if (!once)
    {
        uint32_t *presentable_queues = NULL;
        uint32_t presentable_queue_count = 0;

        retval =
            vk_get_presentable_queues(phy_dev, dev, swapchain->surface, &presentable_queues, &presentable_queue_count);
        if (!vk_error_is_success(&retval) || presentable_queue_count == 0)
        {
            printf(
                "No presentable queue families.  You should have got this error in vk_render_get_essentials before.\n");
            free(presentable_queues);
            vk_render_cleanup_essentials(&essentials, dev);
            return;
        }

        // check to use 1 cmd_buffer per Queues or 1 Queue for many cmd_buffers
        if (dev->command_pools[presentable_queues[0]].queue_count < 2)
        {
            retval.error.type = VK_ERROR_ERRNO;
            printf("Not enough queues in the presentable queue family %u, used only one queue.\n",
                   presentable_queues[0]);
            use_one_VkQueue = false;
        }
        else
            use_one_VkQueue = true;

        if (use_one_VkQueue)
        {
            offscreen_queue = dev->command_pools[presentable_queues[0]].queues[1];
            offscreen_cmd_buffer = dev->command_pools[presentable_queues[0]].buffers[1];
        }
        else
        {
            offscreen_queue = dev->command_pools[presentable_queues[0]].queues[0];
            offscreen_cmd_buffer = dev->command_pools[presentable_queues[0]].buffers[1];
        }

        free(presentable_queues);
    }

    retval =
        allocate_render_data(phy_dev, dev, swapchain, &essentials, &render_data, os_window->reload_shaders_on_resize);
    if (!vk_error_is_success(&retval))
    {
        free_render_data(dev, &essentials, &render_data);
        vk_render_cleanup_essentials(&essentials, dev);
        return;
    }
#ifdef NO_RESIZE_BUF
    if (!once)
    {
#endif
        retval = vk_render_transition_images(dev, &essentials, &render_data.buf_obuffers[0].color, 1,
                                             VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                             VK_IMAGE_ASPECT_COLOR_BIT, "off-screen color");
        if (!vk_error_is_success(&retval))
        {
            free_render_data(dev, &essentials, &render_data);
            vk_render_cleanup_essentials(&essentials, dev);
            return;
        }

        retval = vk_render_transition_images(dev, &essentials, &render_data.buf_obuffers[1].color, 1,
                                             VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                             VK_IMAGE_ASPECT_COLOR_BIT, "off-screen color");
        if (!vk_error_is_success(&retval))
        {
            free_render_data(dev, &essentials, &render_data);
            vk_render_cleanup_essentials(&essentials, dev);
            return;
        }
#ifdef NO_RESIZE_BUF
    }
#endif
    if (!once)
    {
        VkSemaphoreCreateInfo sem_info = {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        };
        res = vkCreateSemaphore(dev->device, &sem_info, NULL, &wait_buf_sem);
        vk_error_set_vkresult(&retval, res);
        if (res)
        {
            vk_error_printf(&retval, "Failed to create wait-render semaphore\n");
            exit_cleanup_render_loop(dev, &essentials, &render_data, wait_buf_sem, wait_main_sem, offscreen_fence);
            return;
        }
        res = vkCreateSemaphore(dev->device, &sem_info, NULL, &wait_main_sem);
        vk_error_set_vkresult(&retval, res);
        if (res)
        {
            vk_error_printf(&retval, "Failed to create wait-post-process semaphore\n");
            exit_cleanup_render_loop(dev, &essentials, &render_data, wait_buf_sem, wait_main_sem, offscreen_fence);
            return;
        }

        VkFenceCreateInfo fence_info = {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        };
        res = vkCreateFence(dev->device, &fence_info, NULL, &offscreen_fence);
        vk_error_set_vkresult(&retval, res);
        if (res)
        {
            vk_error_printf(&retval, "Failed to create fence\n");
            exit_cleanup_render_loop(dev, &essentials, &render_data, wait_buf_sem, wait_main_sem, offscreen_fence);
            return;
        }
    }
    once = true;
    os_window->prepared = true;
    os_window->resize_event = false;

    return;
}

static void exit_cleanup(VkInstance vk, struct vk_device *dev, struct vk_swapchain *swapchain,
                         struct app_os_window *os_window)
{
    if(swapchain)vk_free_swapchain(vk, dev, swapchain);
    if(dev)vk_cleanup(dev);
#if defined(VK_USE_PLATFORM_XCB_KHR)
    xcb_destroy_window(os_window->connection, os_window->xcb_window);
    xcb_disconnect(os_window->connection);
    free(os_window->atom_wm_delete_window);
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
    xdg_toplevel_destroy(os_window->xdg_toplevel);
    xdg_surface_destroy(os_window->xdg_surface);
    wl_keyboard_destroy(os_window->keyboard);
    wl_pointer_destroy(os_window->pointer);
    wl_seat_destroy(os_window->seat);
    wl_surface_destroy(os_window->surface);
    xdg_wm_base_destroy(os_window->shell);
    wl_compositor_destroy(os_window->compositor);
    wl_registry_destroy(os_window->registry);
    wl_display_disconnect(os_window->display);
#endif
    vk_exit(vk);
}

static bool on_window_resize(struct vk_physical_device *phy_dev, struct vk_device *dev,
                             struct vk_render_essentials *essentials, struct vk_swapchain *swapchain,
                             struct render_data *render_data, struct app_os_window *os_window)
{
    vk_error res = VK_ERROR_NONE;

    if (!os_window->prepared)
        return true;

    vkDeviceWaitIdle(dev->device);
    os_window->prepared = false;

#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
  resize_size[0] = os_window->app_data.iResolution[0];
  resize_size[1] = os_window->app_data.iResolution[1];
#endif

    vk_free_pipelines(dev, &render_data->main_pipeline, 1);
    vk_free_graphics_buffers(dev, render_data->main_gbuffers, essentials->image_count, render_data->main_render_pass);
    vk_free_layouts(dev, &render_data->main_layout, 1);

#ifndef NO_RESIZE_BUF
    vk_free_pipelines(dev, &render_data->buf_pipeline, 1);
    vk_free_offscreen_buffers(dev, render_data->buf_obuffers, 2, render_data->buf_render_pass);
    vk_free_layouts(dev, &render_data->buf_layout, 1);
#endif

    // vk_free_buffers(dev, render_data->buffers, 2);

    if (os_window->reload_shaders_on_resize)
        vk_free_shaders(dev, render_data->shaders, 4);

    vk_render_cleanup_essentials(essentials, dev);

    free(render_data->main_gbuffers);

#ifndef NO_RESIZE_BUF
    free(render_data->buf_obuffers);
#endif

    res = vk_get_swapchain(vk, phy_dev, dev, swapchain, os_window, 1, &os_window->present_mode);
    if (vk_error_is_error(&res))
    {
        vk_error_printf(&res, "Could not create surface and swapchain\n");
        exit_cleanup(vk, dev, swapchain, os_window);
        return false;
    }

    render_loop_init(phy_dev, dev, swapchain, os_window);

    return true;
}

void update_params(struct app_data_struct *app_data, bool fps_lock)
{
    float rdelta = 0;
    if (fps_lock)
        FPS_LOCK(60);
    float delta = update_fps_delta();
    if (!app_data->pause)
    {
        app_data->iFrame++;
        app_data->iTime += delta;
    }
    app_data->iTimeDelta = delta;
    float pause_time = pres_pause(app_data->pause);
}

void set_push_constants(struct app_os_window *os_window)
{
    struct my_time_struct my_time;
    get_local_time(&my_time);
    float day_sec = ((float)my_time.msec) / 1000.0 + my_time.sec + my_time.min * 60 + my_time.hour * 3600;

    render_data.push_constants = (struct shaders_uniforms){
        .iResolution[0] = os_window->app_data.iResolution[0],
        .iResolution[1] = os_window->app_data.iResolution[1],
        .iTime = os_window->app_data.iTime,
        .iTimeDelta = os_window->app_data.iTimeDelta,
        .iFrame = os_window->app_data.iFrame,
        .iMouse[0] = os_window->app_data.iMouse[0],
        .iMouse[1] = os_window->app_data.iMouse[1],
        .iMouse[2] = os_window->app_data.iMouse_lclick[0],
        .iMouse[3] = os_window->app_data.iMouse_lclick[1],
        .iMouse_lr[0] = (int)os_window->app_data.iMouse_click[0],
        .iMouse_lr[1] = (int)os_window->app_data.iMouse_click[1],
        .iDate[0] = my_time.year,
        .iDate[1] = my_time.month,
        .iDate[2] = my_time.day,
        .iDate[3] = day_sec,
        .debugdraw = (int)os_window->app_data.drawdebug,
        .pCustom=(os_window->app_data.pause?1:0)+(main_image_srgb?10:0),
    };
}

void update_push_constants_window_size(struct app_os_window *os_window)
{
    render_data.push_constants.iMouse[0] = os_window->app_data.iMouse[0];
    render_data.push_constants.iMouse[1] = os_window->app_data.iMouse[1];
    render_data.push_constants.iMouse[2] = os_window->app_data.iMouse_lclick[0];
    render_data.push_constants.iMouse[3] = os_window->app_data.iMouse_lclick[1];
    render_data.push_constants.iResolution[0] = os_window->app_data.iResolution[0];
    render_data.push_constants.iResolution[1] = os_window->app_data.iResolution[1];
}

#define sign(x) ((x > 0) ? 1 : ((x < 0) ? -1 : 0))
void update_push_constants_local_size(float width, float height)
{
    render_data.push_constants.iMouse[0] =
        ((render_data.push_constants.iMouse[0] / render_data.push_constants.iResolution[1]) -
         0.5 * (render_data.push_constants.iResolution[0] / render_data.push_constants.iResolution[1])) *
            height +
        0.5 * width;
    render_data.push_constants.iMouse[1] =
        ((render_data.push_constants.iMouse[1] / render_data.push_constants.iResolution[1]) - 0.5) * height +
        0.5 * height;
    render_data.push_constants.iMouse[2] =
        sign(render_data.push_constants.iMouse[2]) *
        (((abs((int)render_data.push_constants.iMouse[2]) / render_data.push_constants.iResolution[1]) -
          0.5 * (render_data.push_constants.iResolution[0] / render_data.push_constants.iResolution[1])) *
             height +
         0.5 * width);
    render_data.push_constants.iMouse[3] =
        sign(render_data.push_constants.iMouse[3]) *
        (((abs((int)render_data.push_constants.iMouse[3]) / render_data.push_constants.iResolution[1]) - 0.5) * height +
         0.5 * height);
    render_data.push_constants.iResolution[0] = width;
    render_data.push_constants.iResolution[1] = height;
}

static bool render_loop_buf(struct vk_physical_device *phy_dev, struct vk_device *dev,
                            struct vk_render_essentials *essentials, struct render_data *render_data,
                            VkCommandBuffer cmd_buffer, int render_index, struct app_data_struct *app_data)
{
    vk_error retval = VK_ERROR_NONE;
    VkResult res;

    if (!essentials->first_render)
    {
        res = vkWaitForFences(dev->device, 1, &essentials->exec_fence, true, 1000000000);
        vk_error_set_vkresult(&retval, res);
        if (res)
        {
            vk_error_printf(&retval, "Wait for fence failed\n");
            return false;
        }
    }
#ifdef NO_RESIZE_BUF
    update_push_constants_local_size(render_data->buf_obuffers[render_index].surface_size.width,
                                     render_data->buf_obuffers[render_index].surface_size.height);
#endif
    vkResetCommandBuffer(cmd_buffer, 0);
    VkCommandBufferBeginInfo begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    };
    res = vkBeginCommandBuffer(cmd_buffer, &begin_info);
    vk_error_set_vkresult(&retval, res);
    if (res)
    {
        vk_error_printf(&retval, "BUF: Couldn't even begin recording a command buffer\n");
        return false;
    }

    VkImageMemoryBarrier image_barrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .srcAccessMask = VK_ACCESS_MEMORY_READ_BIT,
        .dstAccessMask = VK_ACCESS_MEMORY_WRITE_BIT,
        .oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = render_data->buf_obuffers[render_index].color.image,
        .subresourceRange =
            {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
    };

    vkCmdPipelineBarrier(cmd_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, 0, 0, NULL,
                         0, NULL, 1, &image_barrier);

    VkClearValue clear_values = {
        .color =
            {
                .float32 = {0.0, 0.0, 0.0, 1.0},
            },
    };
    VkRenderPassBeginInfo pass_info = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = render_data->buf_render_pass,
        .framebuffer = render_data->buf_obuffers[render_index].framebuffer,
        .renderArea =
            {
                .offset =
                    {
                        .x = 0,
                        .y = 0,
                    },
                .extent = render_data->buf_obuffers[render_index].surface_size,
            },
        .clearValueCount = 1,
        .pClearValues = &clear_values,
    };

    vkCmdBeginRenderPass(cmd_buffer, &pass_info, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, render_data->buf_pipeline.pipeline);

    int render_index_t = render_index - 1;
    if (render_index_t < 0)
        render_index_t = 1;
    VkDescriptorImageInfo set_write_image_info = {
        .sampler = render_data->buf_obuffers[render_index_t].color.sampler,
        .imageView = render_data->buf_obuffers[render_index_t].color.view,
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    };
    VkWriteDescriptorSet set_write[1] = {
        [0] =
            {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = render_data->buf_desc_set,
                .dstBinding = 0,
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                .pImageInfo = &set_write_image_info,
            },
    };
    vkUpdateDescriptorSets(dev->device, 1, set_write, 0, NULL);

    vkCmdBindDescriptorSets(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, render_data->buf_layout.pipeline_layout, 0, 1,
                            &render_data->buf_desc_set, 0, NULL);
    VkDeviceSize vertices_offset = 0;
    vkCmdBindVertexBuffers(cmd_buffer, 0, 1, &render_data->buffers[BUFFER_VERTICES].buffer, &vertices_offset);
    vkCmdBindIndexBuffer(cmd_buffer, render_data->buffers[BUFFER_INDICES].buffer, 0, VK_INDEX_TYPE_UINT16);

    VkViewport viewport = {
        .x = 0,
        .y = 0,
        .width = render_data->buf_obuffers[render_index].surface_size.width,
        .height = render_data->buf_obuffers[render_index].surface_size.height,
        .minDepth = 0,
        .maxDepth = 1,
    };
    vkCmdSetViewport(cmd_buffer, 0, 1, &viewport);

    VkRect2D scissor = {
        .offset =
            {
                .x = 0,
                .y = 0,
            },
        .extent = render_data->buf_obuffers[render_index].surface_size,
    };
    vkCmdSetScissor(cmd_buffer, 0, 1, &scissor);

    vkCmdPushConstants(cmd_buffer, render_data->buf_layout.pipeline_layout, VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                       sizeof render_data->push_constants, &render_data->push_constants);

    vkCmdDrawIndexed(cmd_buffer, 4, 1, 0, 0, 0);
    vkCmdEndRenderPass(cmd_buffer);

    image_barrier = (VkImageMemoryBarrier){
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT,
        .dstAccessMask = VK_ACCESS_MEMORY_READ_BIT,
        .oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = render_data->buf_obuffers[render_index].color.image,
        .subresourceRange =
            {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
    };

    vkCmdPipelineBarrier(cmd_buffer, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0,
                         NULL, 0, NULL, 1, &image_barrier);

    vkEndCommandBuffer(cmd_buffer);

    return true;
}

int render_index = 0;

static bool render_loop_draw(struct vk_physical_device *phy_dev, struct vk_device *dev, struct vk_swapchain *swapchain,
                             struct app_os_window *os_window)
{
    int res;
    vk_error retval = VK_ERROR_NONE;

    set_push_constants(os_window);

    if (!render_loop_buf(phy_dev, dev, &essentials, &render_data, offscreen_cmd_buffer, render_index,
                         &os_window->app_data))
    {
        printf("Error on rendering buffers \n");
        return false;
    }
    update_push_constants_window_size(os_window);

    if (!first_submission)
    {
        res = vkWaitForFences(dev->device, 1, &offscreen_fence, true, 1000000000);
        vk_error_set_vkresult(&retval, res);
        if (res)
        {
            vk_error_printf(&retval, "Wait for fence failed\n");
            return false;
        }
    }

    VkPipelineStageFlags wait_sem_stages[1] = {VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT};
    VkSubmitInfo submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = first_submission ? 0 : 1,
        .pWaitSemaphores = &wait_main_sem,
        .pWaitDstStageMask = wait_sem_stages,
        .commandBufferCount = 1,
        .pCommandBuffers = &offscreen_cmd_buffer,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &wait_buf_sem,
    };
    res = vkResetFences(dev->device, 1, &offscreen_fence);
    vk_error_set_vkresult(&retval, res);
    if (res)
    {
        vk_error_printf(&retval, "Failed to reset fence\n");
        return false;
    }
    vkQueueSubmit(offscreen_queue, 1, &submit_info, offscreen_fence);
    first_submission = false;

    uint32_t image_index;

    res = vk_render_start(&essentials, dev, swapchain, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, &image_index);
    if (res == VK_ERROR_OUT_OF_DATE_KHR)
    {
        os_window->resize_event = true;
        res = 0;
        first_submission = true;
        return true;
    }
    else if (res == VK_ERROR_SURFACE_LOST_KHR)
    {
        vkDestroySurfaceKHR(vk, swapchain->surface, NULL);
        retval = vk_create_surface(vk, &swapchain->surface, os_window);
        if (!vk_error_is_success(&retval))
            return false;
        os_window->resize_event = true;
        res = 0;
        first_submission = true;
        return true;
    }
    if (res)
        return false;

    VkClearValue clear_values = {
        .color =
            {
                .float32 = {0.0, 0.0, 0.0, 1.0},
            },
    };
    VkRenderPassBeginInfo pass_info = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = render_data.main_render_pass,
        .framebuffer = render_data.main_gbuffers[image_index].framebuffer,
        .renderArea =
            {
                .offset =
                    {
                        .x = 0,
                        .y = 0,
                    },
                .extent = render_data.main_gbuffers[image_index].surface_size,
            },
        .clearValueCount = 1,
        .pClearValues = &clear_values,
    };

    vkCmdBeginRenderPass(essentials.cmd_buffer, &pass_info, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(essentials.cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, render_data.main_pipeline.pipeline);

    VkDescriptorImageInfo set_write_image_info = {
        .sampler = render_data.buf_obuffers[render_index].color.sampler,
        .imageView = render_data.buf_obuffers[render_index].color.view,
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    };
    VkWriteDescriptorSet set_write[1] = {
        [0] =
            {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = render_data.main_desc_set,
                .dstBinding = 0,
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                .pImageInfo = &set_write_image_info,
            },
    };
    vkUpdateDescriptorSets(dev->device, 1, set_write, 0, NULL);

    vkCmdBindDescriptorSets(essentials.cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            render_data.main_layout.pipeline_layout, 0, 1, &render_data.main_desc_set, 0, NULL);

    VkDeviceSize vertices_offset = 0;
    vkCmdBindVertexBuffers(essentials.cmd_buffer, 0, 1, &render_data.buffers[BUFFER_VERTICES].buffer, &vertices_offset);
    vkCmdBindIndexBuffer(essentials.cmd_buffer, render_data.buffers[BUFFER_INDICES].buffer, 0, VK_INDEX_TYPE_UINT16);

    VkViewport viewport = {
        .x = 0,
        .y = 0,
        .width = render_data.main_gbuffers[image_index].surface_size.width,
        .height = render_data.main_gbuffers[image_index].surface_size.height,
        .minDepth = 0,
        .maxDepth = 1,
    };
    vkCmdSetViewport(essentials.cmd_buffer, 0, 1, &viewport);

    VkRect2D scissor = {
        .offset =
            {
                .x = 0,
                .y = 0,
            },
        .extent = render_data.main_gbuffers[image_index].surface_size,
    };
    vkCmdSetScissor(essentials.cmd_buffer, 0, 1, &scissor);

    vkCmdPushConstants(essentials.cmd_buffer, render_data.main_layout.pipeline_layout, VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                       sizeof render_data.push_constants, &render_data.push_constants);

    // vkCmdDraw(essentials.cmd_buffer, 4, 1, 0, 0);
    vkCmdDrawIndexed(essentials.cmd_buffer, 4, 1, 0, 0, 0);

    vkCmdEndRenderPass(essentials.cmd_buffer);

    res = vk_render_finish(&essentials, dev, swapchain, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, image_index,
                           wait_buf_sem, wait_main_sem);

    if (res == VK_ERROR_OUT_OF_DATE_KHR)
    {
        os_window->resize_event = true;
        res = 0;
    }
    else if (res == VK_ERROR_SURFACE_LOST_KHR)
    {
        vkDestroySurfaceKHR(vk, swapchain->surface, NULL);
        retval = vk_create_surface(vk, &swapchain->surface, os_window);
        if (!vk_error_is_success(&retval))
            return false;
        os_window->resize_event = true;
        res = 0;
    }

    if (res)
        return false;

    update_params(&os_window->app_data, os_window->fps_lock);
    if (render_index >= 1)
        render_index = 0;
    else
        render_index += 1;
    return true;
}

void init_win_params(struct app_os_window *os_window)
{
    os_window->app_data.iResolution[0] = 1280;
    os_window->app_data.iResolution[1] = 720;
#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
    resize_size[0] = os_window->app_data.iResolution[0];
    resize_size[1] = os_window->app_data.iResolution[1];
#endif
    os_window->app_data.iFrame = 0;
    os_window->app_data.iMouse[0] = 0;
    os_window->app_data.iMouse[1] = 0;
    os_window->app_data.iMouse_click[0] = false;
    os_window->app_data.iMouse_click[1] = false;
    os_window->app_data.iMouse_lclick[0] = 0;
    os_window->app_data.iMouse_lclick[1] = 0;
    os_window->app_data.iTime = 0;
    os_window->app_data.pause = false;
    os_window->app_data.quit = false;
    os_window->app_data.drawdebug = false;
    os_window->fps_lock = false;
    os_window->is_minimized = false;
    os_window->prepared = false;
    os_window->resize_event = false;
    os_window->reload_shaders_on_resize = false;
    os_window->print_debug = false;
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    os_window->connection = NULL;
    os_window->window = NULL;
    os_window->minsize.x = 1;
    os_window->minsize.y = 1;
#elif defined(VK_USE_PLATFORM_XCB_KHR)
    os_window->atom_wm_delete_window = NULL;
    os_window->xcb_window = 0;
    os_window->screen = NULL;
    os_window->connection = NULL;
    os_window->display = NULL;
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
    os_window->display = NULL;
    os_window->registry = NULL;
    os_window->compositor = NULL;
    os_window->surface = NULL;
    os_window->shell = NULL;
    os_window->seat = NULL;
    os_window->pointer = NULL;
    os_window->keyboard = NULL;
    os_window->xdg_surface = NULL;
    os_window->xdg_toplevel = NULL;
    os_window->configured = false;
#endif
    strncpy(os_window->name, "Vulkan Shader launcher | twitter.com/AruGL", APP_NAME_STR_LEN);
}

#if defined(VK_USE_PLATFORM_XCB_KHR)
static void render_loop_xcb(struct vk_physical_device *phy_dev, struct vk_device *dev, struct vk_swapchain *swapchain,
                            struct app_os_window *os_window)
{
    while (!os_window->app_data.quit)
    {
        xcb_generic_event_t *event;

        if (os_window->app_data.pause)
        {
            event = xcb_wait_for_event(os_window->connection);
        }
        else
        {
            event = xcb_poll_for_event(os_window->connection);
        }
        while (event)
        {
            app_handle_xcb_event(os_window, event);
            free(event);
            event = xcb_poll_for_event(os_window->connection);
        }
        if ((!os_window->is_minimized) && (!os_window->resize_event))
        {
            if (!os_window->app_data.quit)
            {
                os_window->app_data.quit = !render_loop_draw(phy_dev, dev, swapchain, os_window);
            }
            else
                break;
        }
        else
        {
            if ((!os_window->is_minimized) && os_window->resize_event)
            {
                on_window_resize(phy_dev, dev, &essentials, swapchain, &render_data,
                                 os_window); // execute draw or resize per frame, not together
            }
        }
        if (os_window->is_minimized)
        { // I do not delete everything on minimize, only stop rendering
            sleep_ms(10);
        }
    }
    exit_cleanup_render_loop(dev, &essentials, &render_data, wait_buf_sem, wait_main_sem, offscreen_fence);
}
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
static void render_loop_wayland(struct vk_physical_device *phy_dev, struct vk_device *dev, struct vk_swapchain *swapchain,
                            struct app_os_window *os_window)
{
    static bool init_surface_size_once=true; // in Wayland surface should set window size, I do it on start, on start window size 0,0
    while (!os_window->app_data.quit)
    {
        while (!os_window->configured)
          wl_display_dispatch(os_window->display);
        while (wl_display_prepare_read(os_window->display) != 0)
          wl_display_dispatch_pending(os_window->display);
        wl_display_flush(os_window->display);
        wl_display_read_events(os_window->display);
        wl_display_dispatch_pending(os_window->display);

        if (os_window->app_data.pause)
        {
          sleep_ms(10);
        }
        
        if (((!os_window->is_minimized) && (!os_window->resize_event)) || init_surface_size_once)
        {
            if (!os_window->app_data.quit)
            {
                os_window->app_data.quit = !render_loop_draw(phy_dev, dev, swapchain, os_window);
            }
            else
                break;
            init_surface_size_once = false;
        }
        else
        {
            if ((!os_window->is_minimized) && os_window->resize_event)
            {
                on_window_resize(phy_dev, dev, &essentials, swapchain, &render_data,
                                 os_window);
            }
        }
        
        if (os_window->is_minimized)
        { // I do not delete everything on minimize, only stop rendering
            sleep_ms(10);
        }

    }
    exit_cleanup_render_loop(dev, &essentials, &render_data, wait_buf_sem, wait_main_sem, offscreen_fence);
}

#endif

void print_usage(char *name)
{
    printf("Usage: %s \n"
           "\t[--present_mode <present mode enum>]\n"
           "\t <present_mode_enum>\tVK_PRESENT_MODE_IMMEDIATE_KHR = %d\n"
           "\t\t\t\tVK_PRESENT_MODE_MAILBOX_KHR = %d\n"
           "\t\t\t\tVK_PRESENT_MODE_FIFO_KHR = %d\n"
           "\t\t\t\tVK_PRESENT_MODE_FIFO_RELAXED_KHR = %d\n"
           "\t[--debug]\n"
           "\t[--reload_shaders] will reload shaders form file on resize\n"
           "\t[--gpu <index(0/1/2/etc)>] use selected GPU to render\n"
           "Control: Keyboard 1-debug, 2-vsynk 60fps, Space-pause\n",
           name, VK_PRESENT_MODE_IMMEDIATE_KHR, VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_FIFO_KHR,
           VK_PRESENT_MODE_FIFO_RELAXED_KHR);
}

#if defined(VK_USE_PLATFORM_WIN32_KHR)

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow)
{
    MSG msg;
    bool done;
    int argc;
    char **argv;

    msg.wParam = 0;

    LPWSTR *commandLineArgs = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (NULL == commandLineArgs)
    {
        argc = 0;
    }

    if (argc > 0)
    {
        argv = (char **)malloc(sizeof(char *) * argc);
        if (argv == NULL)
        {
            argc = 0;
        }
        else
        {
            for (int iii = 0; iii < argc; iii++)
            {
                size_t wideCharLen = wcslen(commandLineArgs[iii]);
                size_t numConverted = 0;

                argv[iii] = (char *)malloc(sizeof(char) * (wideCharLen + 1));
                if (argv[iii] != NULL)
                {
                    wcstombs_s(&numConverted, argv[iii], wideCharLen + 1, commandLineArgs[iii], wideCharLen + 1);
                }
            }
        }
    }
    else
    {
        argv = NULL;
    }

    vk_error res = VK_ERROR_NONE;
    int retval = EXIT_FAILURE;

    init_win_params(&os_window);
    uint32_t dev_index = 0;
    bool use_gpu_idx = false;
    os_window.present_mode = VK_PRESENT_MODE_FIFO_KHR;

    if (argc > 1)
    {
        if (strcmp(argv[1], "--help") == 0)
        {
            SetStdOutToNewConsole();
            print_usage(argv[0]);
            Sleep(44000);
            return 0;
        }
    }

    for (int i = 1; i < argc; i++)
    {
        if ((strcmp(argv[i], "--present_mode") == 0) && (i < argc - 1))
        {
            os_window.present_mode = atoi(argv[i + 1]);
            i++;
            continue;
        }
        if ((strcmp(argv[i], "--gpu") == 0) && (i < argc - 1))
        {
            dev_index = atoi(argv[i + 1]);
            use_gpu_idx = true;
            i++;
            continue;
        }
        if (strcmp(argv[i], "--debug") == 0)
        {
            os_window.print_debug = true;
            continue;
        }
        if (strcmp(argv[i], "--reload_shaders") == 0)
        {
            os_window.reload_shaders_on_resize = true;
            continue;
        }
    }

    if (argc > 0 && argv != NULL)
    {
        for (int iii = 0; iii < argc; iii++)
        {
            if (argv[iii] != NULL)
            {
                free(argv[iii]);
            }
        }
        free(argv);
    }

    if (os_window.print_debug)
    {
        SetStdOutToNewConsole();
    }

    os_window.connection = hInstance;

    res = vk_init(&vk);
    if (!vk_error_is_success(&res))
    {
        vk_error_printf(&res, "Could not initialize Vulkan\n");
        return retval;
    }
    
    app_create_window(&os_window);

    res = vk_create_surface(vk, &swapchain.surface, &os_window);
    if (vk_error_is_error(&res))
    {
        vk_error_printf(&res, "Could not create surface.\n");
        exit_cleanup(vk, NULL, NULL, &os_window);
        return retval;
    }

    res = vk_enumerate_devices(vk, &swapchain.surface, &phy_dev, &dev_index, use_gpu_idx, false);
    if (vk_error_is_error(&res))
    {
        vk_error_printf(&res, "Could not enumerate devices\n");
        vkDestroySurfaceKHR(vk, swapchain.surface, NULL);
        exit_cleanup(vk, NULL, NULL, &os_window);
        return retval;
    }

    res = vk_setup(&phy_dev, &dev, VK_QUEUE_GRAPHICS_BIT, 2); // two cmd buffer
    if (vk_error_is_error(&res))
    {
        vk_error_printf(&res, "Could not setup logical device, command pools and queues\n");
        exit_cleanup(vk, &dev, &swapchain, &os_window);
        return retval;
    }

    swapchain.swapchain = VK_NULL_HANDLE;
    res = vk_get_swapchain(vk, &phy_dev, &dev, &swapchain, &os_window, 1, &os_window.present_mode);
    if (vk_error_is_error(&res))
    {
        vk_error_printf(&res, "Could not create surface and swapchain\n");
        exit_cleanup(vk, &dev, &swapchain, &os_window);
        return retval;
    }

    render_loop_init(&phy_dev, &dev, &swapchain, &os_window);
    done = false;
    while (!done)
    {
        PeekMessage(&msg, 0, 0, 0, PM_REMOVE);
        process_msg(&msg, &done);
        RedrawWindow(os_window.window, NULL, NULL, RDW_INTERNALPAINT);
    }

    exit_cleanup_render_loop(&dev, &essentials, &render_data, wait_buf_sem, wait_main_sem, offscreen_fence);
    exit_cleanup(vk, &dev, &swapchain, &os_window);

    return (int)msg.wParam;
}

#elif defined(VK_USE_PLATFORM_XCB_KHR) || defined(VK_USE_PLATFORM_WAYLAND_KHR)

int main(int argc, char **argv)
{
    vk_error res;
    int retval = EXIT_FAILURE;
    VkInstance vk;
    struct vk_physical_device phy_dev = {0};
    struct vk_device dev = {0};
    struct vk_swapchain swapchain = {0};
    struct app_os_window os_window;
    init_win_params(&os_window);
    uint32_t dev_index = 0;
    bool use_gpu_idx = false;
    os_window.present_mode = VK_PRESENT_MODE_FIFO_KHR;

    if (argc > 1)
    {
        if (strcmp(argv[1], "--help") == 0)
        {
            print_usage(argv[0]);
            return 0;
        }
    }

    for (int i = 1; i < argc; i++)
    {
        if ((strcmp(argv[i], "--present_mode") == 0) && (i < argc - 1))
        {
            os_window.present_mode = atoi(argv[i + 1]);
            i++;
            continue;
        }
        if ((strcmp(argv[i], "--gpu") == 0) && (i < argc - 1))
        {
            dev_index = atoi(argv[i + 1]);
            use_gpu_idx = true;
            i++;
            continue;
        }
        if (strcmp(argv[i], "--debug") == 0)
        {
            os_window.print_debug = true;
            continue;
        }
        if (strcmp(argv[i], "--reload_shaders") == 0)
        {
            os_window.reload_shaders_on_resize = true;
            continue;
        }
    }

    srand(time(NULL));

    res = vk_init(&vk);
    if (!vk_error_is_success(&res))
    {
        vk_error_printf(&res, "Could not initialize Vulkan\n");
        return retval;
    }

#if defined(VK_USE_PLATFORM_XCB_KHR)
    printf("Init XCB\n");
    app_init_connection(&os_window);
    app_create_xcb_window(&os_window);
#else
    printf("Init Wayland\n");
    initWaylandConnection(&os_window);
    setupWindow(&os_window);
#endif

    res = vk_create_surface(vk, &swapchain.surface, &os_window);
    if (vk_error_is_error(&res))
    {
        vk_error_printf(&res, "Could not create surface.\n");
        exit_cleanup(vk, NULL, NULL, &os_window);
        return retval;
    }

    res = vk_enumerate_devices(vk, &swapchain.surface, &phy_dev, &dev_index, use_gpu_idx, false);
    if (vk_error_is_error(&res))
    {
        vk_error_printf(&res, "Could not enumerate devices\n");
        vkDestroySurfaceKHR(vk, swapchain.surface, NULL);
        exit_cleanup(vk, NULL, NULL, &os_window);
        return retval;
    }

    res = vk_setup(&phy_dev, &dev, VK_QUEUE_GRAPHICS_BIT, 2); // two cmd buffer
    if (vk_error_is_error(&res))
    {
        vk_error_printf(&res, "Could not setup logical device, command pools and queues\n");
        exit_cleanup(vk, &dev, &swapchain, &os_window);
        return retval;
    }

    swapchain.swapchain = VK_NULL_HANDLE;
    res = vk_get_swapchain(vk, &phy_dev, &dev, &swapchain, &os_window, 1, &os_window.present_mode);
    if (vk_error_is_error(&res))
    {
        vk_error_printf(&res, "Could not create surface and swapchain\n");
        exit_cleanup(vk, &dev, &swapchain, &os_window);
        return retval;
    }

    render_loop_init(&phy_dev, &dev, &swapchain, &os_window);
    
#if defined(VK_USE_PLATFORM_XCB_KHR)
    render_loop_xcb(&phy_dev, &dev, &swapchain, &os_window);
#else
    render_loop_wayland(&phy_dev, &dev, &swapchain, &os_window);
#endif

    retval = 0;

    exit_cleanup(vk, &dev, &swapchain, &os_window);
    return retval;
}

#endif
