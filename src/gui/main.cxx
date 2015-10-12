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

#include <cstring>

#include <linux/memfd.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <unistd.h>

#include <cairo/cairo.h>

#include <wayland-client.h>
#include <wayland-client-protocol.h>
#include "xdg_shell.h"

namespace wayland
{

class display
{
	struct wl_display *ptr;

	friend class registry;

public:
	display() { ptr = wl_display_connect(nullptr); }
	~display()
	{
		if (ptr != nullptr) {
			wl_display_disconnect(ptr);
		}
	}
	bool has_errors() { return ptr == nullptr; }
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

class compositor
{
	struct wl_compositor *ptr;

	compositor() : ptr(nullptr) {}

	void initialize(void *p)
	{
		ptr = static_cast<struct wl_compositor *>(p);
	}

	bool has_errors() { return ptr == nullptr; }

	~compositor()
	{
		if (ptr != nullptr) {
			wl_compositor_destroy(ptr);
		}
	}

	friend class registry;
};

class shell
{
	struct xdg_shell *ptr;

	static struct xdg_shell_listener listener;

	static void ping(void *data, struct xdg_shell *xdg_shell,
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

	bool has_errors() { return ptr == nullptr; }

	~shell()
	{
		if (ptr != nullptr) {
			xdg_shell_destroy(ptr);
		}
	}

	friend class registry;
};

class shm
{
	struct wl_shm *ptr;

	shm() : ptr(nullptr) {}

	void initialize(void *p) { ptr = static_cast<struct wl_shm *>(p); }

	bool has_errors() { return ptr == nullptr; }

	~shm()
	{
		if (ptr != nullptr) {
			wl_shm_destroy(ptr);
		}
	}

	friend class registry;
};

class registry
{
	struct wl_registry *ptr;
	wayland::compositor compositor;
	wayland::shell shell;
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
		}
		d.dispatch();
	}
	bool has_errors()
	{
		return ptr == nullptr || compositor.has_errors() ||
		       shell.has_errors() || shm.has_errors();
	}
	wayland::compositor &compositor_ref() { return compositor; }
	wayland::shell &shell_ref() { return shell; }
	wayland::shm &shm_ref() { return shm; }
	~registry()
	{
		if (ptr != nullptr) {
			wl_registry_destroy(ptr);
		}
	}
};
}

struct xdg_shell_listener wayland::shell::listener = {ping};

struct wl_registry_listener wayland::registry::listener = {global,
							   global_remove};

class pixels
{
	uint32_t *data;
	const int32_t width;
	const int32_t height;

public:
	pixels(int32_t w, int32_t h) : data(nullptr), width(w), height(h)
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
	int32_t capacity() const
	{
		return width * height * sizeof(uint32_t) * 2;
	}
	int32_t stride() const { return width * sizeof(uint32_t); }
	~pixels() { munmap(data, capacity()); }
};
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
	wayland::shell &shell = registry.shell_ref();
	pixels ps(300, 200);
}
