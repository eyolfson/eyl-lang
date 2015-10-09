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

#include <cstdint>
#include <cstdio>

#include <array>
#include <vector>

enum class byte_order {
	none,
	little_endian,
	big_endian,
};

class primitive
{
public:
	uint16_t number_of_bytes;
	byte_order order_of_bytes;

	primitive(uint16_t n, byte_order o)
		: number_of_bytes(n), order_of_bytes(o)
	{
	}
};

int main(int argc, const char *argv[])
{
	printf("Language 0.0.1-development\n");

	std::array<primitive, 4> primitives = {{
	    {1, byte_order::none},
	    {2, byte_order::little_endian},
	    {4, byte_order::little_endian},
	    {8, byte_order::little_endian},
	}};

	printf("\nDE AD BE EF\n");
	printf("\nEF BE AD DE\n"
	       "== == == ==\n"
	       " 0  1  2  3\n");

	return 0;
}
