
// from https://wayland-book.com/seat/example.html

// used stable xdg-shell

// Wayland does not have stable "decoration" support, xdg-decoration not supported
// zxdg_toplevel_decoration_v1 is unstable
// https://wayland-book.com/xdg-shell-in-depth/interactive.html

#include <linux/input.h>

static void xdg_wm_base_ping(void *data, struct xdg_wm_base *shell, uint32_t serial)
{
	xdg_wm_base_pong(shell, serial);
}

static const struct xdg_wm_base_listener xdg_wm_base_listener = {xdg_wm_base_ping,};

static void seatCapabilities(struct wl_seat *seat, uint32_t caps, struct app_os_window *os_window);
static void seatCapabilitiesCb(void *data, struct wl_seat *seat, uint32_t caps)
{
	struct app_os_window *os_window = (struct app_os_window *) data;
	seatCapabilities(seat, caps, os_window);
}

void registryGlobal(struct wl_registry *registry, uint32_t name, const char *interface, uint32_t version, struct app_os_window *os_window)
{
	if (strcmp(interface, "wl_compositor") == 0) {
		os_window->compositor = (struct wl_compositor *) wl_registry_bind(registry, name, &wl_compositor_interface, 3);
	}
	else if (strcmp(interface, "xdg_wm_base") == 0)
	{
		os_window->shell = (struct xdg_wm_base *) wl_registry_bind(registry, name, &xdg_wm_base_interface, 1);
		xdg_wm_base_add_listener(os_window->shell, &xdg_wm_base_listener, NULL);
	}
	else if (strcmp(interface, "wl_seat") == 0)
	{
    os_window->seat = (struct wl_seat *) wl_registry_bind(registry, name, &wl_seat_interface, 1);
		static const struct wl_seat_listener seat_listener ={ seatCapabilitiesCb, };
		wl_seat_add_listener(os_window->seat, &seat_listener, os_window);
	}
}

static void setSize(int width, int height, struct app_os_window *os_window)
{
	if (width <= 0 || height <= 0){
    os_window->is_minimized = true;
  }else
  {
    os_window->is_minimized = false;
    if ((os_window->app_data.iResolution[0] != width) || (os_window->app_data.iResolution[1] != height)) {
        os_window->is_minimized = false;
        os_window->app_data.iResolution[0] = width;
        os_window->app_data.iResolution[1] = height;
        if((os_window->app_data.iResolution[0]<=1)||(os_window->app_data.iResolution[1]<=1)){
            os_window->is_minimized = true;
        }
    }
  }
}

static void xdg_surface_handle_configure(void *data, struct xdg_surface *surface, uint32_t serial)
{
	struct app_os_window *os_window = (struct app_os_window *) data;
	xdg_surface_ack_configure(surface, serial);
	os_window->configured = true;
}

static const struct xdg_surface_listener xdg_surface_listener = {xdg_surface_handle_configure,};

static void xdg_toplevel_handle_configure(void *data, struct xdg_toplevel *toplevel, int32_t width, int32_t height, struct wl_array *states)
{
	struct app_os_window *os_window = (struct app_os_window *) data;

	setSize(width, height, os_window);
}

static void xdg_toplevel_handle_close(void *data, struct xdg_toplevel *xdg_toplevel)
{
	struct app_os_window *os_window = (struct app_os_window *) data;
	os_window->app_data.quit = true;
}

static const struct xdg_toplevel_listener xdg_toplevel_listener = {xdg_toplevel_handle_configure, xdg_toplevel_handle_close,};

static void registryGlobalCb(void *data, struct wl_registry *registry, uint32_t name, const char *interface, uint32_t version)
{
	struct app_os_window *os_window = (struct app_os_window *) data;
	registryGlobal(registry, name, interface, version, os_window);
}

static void pointerEnterCb(void *data, struct wl_pointer *pointer, uint32_t serial, struct wl_surface *surface, wl_fixed_t sx, wl_fixed_t sy){}

static void pointerLeaveCb(void *data, struct wl_pointer *pointer, uint32_t serial, struct wl_surface *surface){}

static void pointerMotion(struct wl_pointer *pointer, uint32_t time, wl_fixed_t sx, wl_fixed_t sy, struct app_os_window *os_window)
{
  os_window->app_data.iMouse[0]=wl_fixed_to_int(sx);
  os_window->app_data.iMouse[1]=os_window->app_data.iResolution[1] - wl_fixed_to_int(sy);
}

static void pointerMotionCb(void *data, struct wl_pointer *pointer, uint32_t time, wl_fixed_t sx, wl_fixed_t sy)
{
	struct app_os_window *os_window = (struct app_os_window *) data;
	pointerMotion(pointer, time, sx, sy, os_window);
}

static void pointerButton(struct wl_pointer *pointer, uint32_t serial, uint32_t time, uint32_t button, uint32_t state, struct app_os_window *os_window)
{
	switch (button)
	{
	case BTN_LEFT:
    os_window->app_data.iMouse_click[0] = state;
    if(state){
      os_window->app_data.iMouse_lclick[0] = os_window->app_data.iMouse[0];
      os_window->app_data.iMouse_lclick[1] = os_window->app_data.iResolution[1] - os_window->app_data.iMouse[1];
    }else{
      os_window->app_data.iMouse_lclick[0] = -os_window->app_data.iMouse_lclick[0];
      os_window->app_data.iMouse_lclick[1] = -os_window->app_data.iMouse_lclick[1];
    }
		break;
	case BTN_MIDDLE:
		break;
	case BTN_RIGHT:
		os_window->app_data.iMouse_click[1] = state;
    if(state){
      os_window->app_data.iMouse_rclick[0] = os_window->app_data.iMouse[0];
      os_window->app_data.iMouse_rclick[1] = os_window->app_data.iResolution[1] - os_window->app_data.iMouse[1];
    }else{
      os_window->app_data.iMouse_rclick[0] = -os_window->app_data.iMouse_rclick[0];
      os_window->app_data.iMouse_rclick[1] = -os_window->app_data.iMouse_rclick[1];
    }
		break;
	}
}

static void pointerButtonCb(void *data, struct wl_pointer *pointer, uint32_t serial, uint32_t time, uint32_t button, uint32_t state)
{
	struct app_os_window *os_window = (struct app_os_window *) data;
	pointerButton(pointer, serial, time, button, state, os_window);
}

static void pointerAxis(struct wl_pointer *pointer, uint32_t time, uint32_t axis, wl_fixed_t value, struct app_os_window *os_window)
{
	float d = wl_fixed_to_double(value);
	switch (axis)
	{
	case REL_X:
		//printf("mouse wheel %f\n",d);
		break;
	}
}

static void pointerAxisCb(void *data, struct wl_pointer *pointer, uint32_t time, uint32_t axis, wl_fixed_t value)
{
	struct app_os_window *os_window = (struct app_os_window *) data;
  pointerAxis(pointer, time, axis, value, os_window);
}

static void keyboardKeymapCb(void *data, struct wl_keyboard *keyboard, uint32_t format, int fd, uint32_t size){}

static void keyboardEnterCb(void *data, struct wl_keyboard *keyboard, uint32_t serial, struct wl_surface *surface, struct wl_array *keys){}

static void keyboardLeaveCb(void *data, struct wl_keyboard *keyboard, uint32_t serial, struct wl_surface *surface){}

static void keyboardKey(struct wl_keyboard *keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state, struct app_os_window *os_window)
{
	switch (key)
	{
    case KEY_ESC:  // Escape
        if(state)os_window->app_data.quit = true;
        break;
    case KEY_LEFT:  // left arrow key
        break;
    case KEY_RIGHT:  // right arrow key
        break;
    case KEY_SPACE:  // space bar
        if(state)os_window->app_data.pause = !os_window->app_data.pause;
        break;
    case KEY_1: //1
        if(state)os_window->app_data.drawdebug = !os_window->app_data.drawdebug;
        break;
    case KEY_2: //2
        if(state)os_window->fps_lock = !os_window->fps_lock;
        break;
    case KEY_F: //Wayland resize event or just resize to resolution
        if(state){
          os_window->resize_event = true;
          static bool switch_res=true;
          if(switch_res){
            switch_res=false;
            os_window->app_data.iResolution[0]=1920;
            os_window->app_data.iResolution[1]=1080;
          }else{
            switch_res=true;
            os_window->app_data.iResolution[0]=1280;
            os_window->app_data.iResolution[1]=720;
          }
        }
        break;
	}

	//if (state)
		//keyPressed(key);
}

static void keyboardKeyCb(void *data, struct wl_keyboard *keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state)
{
	struct app_os_window *os_window = (struct app_os_window *) data;
	keyboardKey(keyboard, serial, time, key, state, os_window);
}

static void keyboardModifiersCb(void *data, struct wl_keyboard *keyboard, uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched, uint32_t mods_locked, uint32_t group){}

static void seatCapabilities(struct wl_seat *seat, uint32_t caps, struct app_os_window *os_window)
{
	if ((caps & WL_SEAT_CAPABILITY_POINTER) && !os_window->pointer)
	{
		os_window->pointer = wl_seat_get_pointer(seat);
		static const struct wl_pointer_listener pointer_listener ={ pointerEnterCb, pointerLeaveCb, pointerMotionCb, pointerButtonCb, pointerAxisCb, };
		wl_pointer_add_listener(os_window->pointer, &pointer_listener, os_window);
	}
	else if (!(caps & WL_SEAT_CAPABILITY_POINTER) && os_window->pointer)
	{
		wl_pointer_destroy(os_window->pointer);
		os_window->pointer = NULL;
	}

	if ((caps & WL_SEAT_CAPABILITY_KEYBOARD) && !os_window->keyboard)
	{
		os_window->keyboard = wl_seat_get_keyboard(seat);
		static const struct wl_keyboard_listener keyboard_listener = { keyboardKeymapCb, keyboardEnterCb, keyboardLeaveCb, keyboardKeyCb, keyboardModifiersCb, };
		wl_keyboard_add_listener(os_window->keyboard, &keyboard_listener, os_window);
	}
	else if (!(caps & WL_SEAT_CAPABILITY_KEYBOARD) && os_window->keyboard)
	{
		wl_keyboard_destroy(os_window->keyboard);
		os_window->keyboard = NULL;
	}
}



static void registryGlobalRemoveCb(void *data, struct wl_registry *registry, uint32_t name){}

void initWaylandConnection(struct app_os_window *os_window)
{
	os_window->display = wl_display_connect(NULL);
	if (!os_window->display)
	{
		printf("Could not connect to Wayland display!\n");
		fflush(stdout);
		exit(1);
	}

	os_window->registry = wl_display_get_registry(os_window->display);
	if (!os_window->registry)
	{
		printf("Could not get Wayland registry!\n");
		fflush(stdout);
		exit(1);
	}

	static const struct wl_registry_listener registry_listener = { registryGlobalCb, registryGlobalRemoveCb };
	wl_registry_add_listener(os_window->registry, &registry_listener, os_window);
	wl_display_dispatch(os_window->display);
	wl_display_roundtrip(os_window->display);
	if (!os_window->compositor || !os_window->shell || !os_window->seat)
	{
		printf("Could not bind Wayland protocols!\n");
		fflush(stdout);
		exit(1);
	}
}

static void setupWindow(struct app_os_window *os_window)
{
	os_window->surface = wl_compositor_create_surface(os_window->compositor);
	os_window->xdg_surface = xdg_wm_base_get_xdg_surface(os_window->shell, os_window->surface);

	xdg_surface_add_listener(os_window->xdg_surface, &xdg_surface_listener, os_window);
	os_window->xdg_toplevel = xdg_surface_get_toplevel(os_window->xdg_surface);
	xdg_toplevel_add_listener(os_window->xdg_toplevel, &xdg_toplevel_listener, os_window);

	xdg_toplevel_set_title(os_window->xdg_toplevel, os_window->name);
	wl_surface_commit(os_window->surface);
}
