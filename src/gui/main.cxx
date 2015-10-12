/*
 * Copyright 2015 Jonathan Eyolfson
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include <stdio.h>
#include <cstring>

#include <linux/memfd.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <unistd.h>

#include <cairo/cairo.h>

#include <wayland-client.h>
#include <wayland-client-protocol.h>
#include "xdg_shell.h"

namespace wayland {

class display {
	struct wl_display *ptr;

	friend class registry;

public:
	display()
	{
		ptr = wl_display_connect(nullptr);
	}
	~display()
	{
		if (ptr != nullptr) {
			wl_display_disconnect(ptr);
		}
	}
	bool has_errors()
	{
		return ptr == nullptr;
	}
	void dispatch() const
	{
		if (ptr != nullptr) {
			wl_display_dispatch(ptr);
		}
	}
	void roundtrip()
	{
		if (ptr != nullptr) {
			wl_display_roundtrip(ptr);
		}
	}
};

class compositor {
	struct wl_compositor *ptr;

	compositor() : ptr(nullptr) {}

	void initialize(void *p)
	{
		ptr = static_cast<struct wl_compositor *>(p);
	}

	bool has_errors()
	{
		return ptr == nullptr;
	}

	~compositor()
	{
		if (ptr != nullptr) {
			wl_compositor_destroy(ptr);
		}
	}

	friend class registry;
	friend class surface;
};

namespace xdg { class surface; }

class surface {
	struct wl_surface *ptr;

public:
	surface(const compositor &c)
	{
		ptr = wl_compositor_create_surface(c.ptr);

	}
	~surface()
	{
		if (ptr != nullptr) {
			wl_surface_destroy(ptr);
		}
	}

	friend class xdg::surface;
};

class shm {
	struct wl_shm *ptr;

	shm() : ptr(nullptr) {}

	void initialize(void *p)
	{
		ptr = static_cast<struct wl_shm *>(p);
	}

	bool has_errors()
	{
		return ptr == nullptr;
	}

	~shm()
	{
		if (ptr != nullptr) {
			wl_shm_destroy(ptr);
		}
	}

	friend class registry;
};

class registry;

namespace xdg {

class shell {
	struct xdg_shell *ptr;

	static struct xdg_shell_listener listener;

	static void ping(void *data,
			 struct xdg_shell *xdg_shell,
			 uint32_t serial)
	{
		xdg_shell_pong(xdg_shell, serial);
	}

	shell() : ptr(nullptr) {}

	void initialize(void *p)
	{
		ptr = static_cast<struct xdg_shell *>(p);
		if (ptr != nullptr) {
			xdg_shell_add_listener(ptr, &listener, nullptr);
			xdg_shell_use_unstable_version(
				ptr, XDG_SHELL_VERSION_CURRENT);
		}
	}

	bool has_errors()
	{
		return ptr == nullptr;
	}

	~shell()
	{
		if (ptr != nullptr) {
			xdg_shell_destroy(ptr);
		}
	}

	friend class wayland::registry;
	friend class surface;
};

class surface {
	struct xdg_surface *ptr;

	static struct xdg_surface_listener listener;

	static void configure(void *data, struct xdg_surface *xdg_surface,
			      int32_t width, int32_t height,
			      struct wl_array *states, uint32_t serial)
	{
		xdg_surface_ack_configure(xdg_surface, serial);
	}
	static void close(void *data, struct xdg_surface *xdg_surface)
	{
	}

public:
	surface(shell &sh, wayland::surface &su)
	{
		ptr = xdg_shell_get_xdg_surface(sh.ptr, su.ptr);
	}
	void set_title(const char *title)
	{
		if (ptr != nullptr) {
			xdg_surface_set_title(ptr, title);
		}
	}
	void set_window_geometry(int32_t x, int32_t y, int32_t w, int32_t h)
	{
		if (ptr != nullptr) {
			xdg_surface_set_window_geometry(ptr, x, y, w, h);
		}
	}
	~surface()
	{
		if (ptr != nullptr) {
			xdg_surface_destroy(ptr);
		}
	}
};

} // namespace xdg

class registry {
	struct wl_registry *ptr;
	wayland::compositor compositor;
	wayland::xdg::shell shell;
	wayland::shm shm;

	static struct wl_registry_listener listener;

	static void global(void *data, struct wl_registry *wl_registry,
			   uint32_t name, const char *interface,
			   uint32_t version)
	{
		registry *r = static_cast<registry *>(data);
		if (strcmp(interface, wl_compositor_interface.name) == 0) {
			r->compositor.initialize(wl_registry_bind(
			    wl_registry, name, &wl_compositor_interface,
			    version));
		} else if (strcmp(interface, wl_shm_interface.name) == 0) {
			r->shm.initialize(wl_registry_bind(
			    wl_registry, name, &wl_shm_interface, version));
		} else if (strcmp(interface, xdg_shell_interface.name) == 0) {
			r->shell.initialize(wl_registry_bind(
			    wl_registry, name, &xdg_shell_interface, version));
		}
	}
	static void global_remove(void *data, struct wl_registry *wl_registry,
				  uint32_t name)
	{
	}

public:
	registry(const display &d)
	{
		ptr = wl_display_get_registry(d.ptr);
		if (ptr != nullptr) {
			wl_registry_add_listener(ptr, &listener, this);
			d.dispatch();
		}
	}
	bool has_errors()
	{
		return ptr == nullptr || compositor.has_errors()
		       || shell.has_errors() || shm.has_errors();
	}
	wayland::compositor &compositor_ref() { return compositor; }
	wayland::xdg::shell &shell_ref() { return shell; }
	wayland::shm &shm_ref() { return shm; }
	~registry()
	{
		if (ptr != nullptr) {
			wl_registry_destroy(ptr);
		}
	}
};

} // namespace wayland

struct xdg_shell_listener wayland::xdg::shell::listener = {
	ping
};

struct xdg_surface_listener wayland::xdg::surface::listener = {
	configure,
	close
};

struct wl_registry_listener wayland::registry::listener = {
	global,
	global_remove
};

namespace {

class pixels {
	uint32_t *data;
	const int32_t w;
	const int32_t h;

public:
	pixels(int32_t w, int32_t h) : data(nullptr), w(w), h(h)
	{
		if (w <= 0 || h <= 0) {
			return;
		}
		int fd = syscall(SYS_memfd_create, "pixels",
				 MFD_CLOEXEC | MFD_ALLOW_SEALING);
		if (fd == -1) {
			return;
		}
		ftruncate(fd, capacity() * 2);
		data = static_cast<uint32_t *>(mmap(nullptr, capacity(),
						    PROT_WRITE | PROT_READ,
						    MAP_SHARED, fd, 0));
		if (data == MAP_FAILED) {
			data = nullptr;
		}
		close(fd);
	}
	int32_t width() const
	{
		return w;
	}
	int32_t height() const
	{
		return h;
	}
	int32_t capacity() const
	{
		return w * h * sizeof(uint32_t);
	}
	int32_t stride() const
	{
		return w * sizeof(uint32_t);
	}
	~pixels() { munmap(data, capacity()); }
};

} // namespace

int main(int argc, char **argv)
{
	wayland::display display;
	if (display.has_errors()) {
		return 1;
	}
	wayland::registry registry(display);
	if (registry.has_errors()) {
		return 2;
	}
	wayland::compositor &compositor = registry.compositor_ref();
	wayland::shm &shm = registry.shm_ref();
	wayland::xdg::shell &shell = registry.shell_ref();

	wayland::surface surface(compositor);
	wayland::xdg::surface xdg_surface(shell, surface);

	pixels ps(300, 200);
	xdg_surface.set_title("Eyl Lang");
	xdg_surface.set_window_geometry(10, 10, ps.width(), ps.height());
}
