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

#include <cairo/cairo.h>

#include <wayland-client.h>
#include <wayland-client-protocol.h>
#include "xdg_shell.h"

namespace wayland
{

class display
{
	struct wl_display *ptr;

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
		}
	}

	bool has_errors() { return ptr == nullptr; }

	~shell()
	{
		if (ptr != nullptr) {
			xdg_shell_destroy(ptr);
		}
	}

public:
	friend class registry;
};

class registry
{
	struct wl_registry *ptr;
	wayland::shell shell;

	static struct wl_registry_listener listener;

	static void global(void *data, struct wl_registry *wl_registry,
			   uint32_t name, const char *interface,
			   uint32_t version)
	{
		registry *r = static_cast<registry *>(data);
		if (strcmp(interface, xdg_shell_interface.name) == 0) {
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
	bool has_errors() { return ptr == nullptr || shell.has_errors(); }
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
	return 0;
}
