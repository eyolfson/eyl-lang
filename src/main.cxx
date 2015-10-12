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
#include <tuple>

enum class byte_order {
	none,
	little_endian,
	big_endian,
};

class fundamental
{
	std::vector<uint8_t> bytes;
	byte_order order;

public:
	fundamental(std::tuple<size_t, byte_order> &t)
		: bytes(std::get<0>(t), 0), order(std::get<1>(t))
	{
		for (int i = 0; i < bytes.size(); ++i) {
			bytes[i] = i;
		}
	}
	void print_bytes()
	{
		printf("%02x", bytes[bytes.size() - 1]);
		for (int i = bytes.size() - 2; i >= 0; --i) {
			printf(" %02x", bytes[i]);
		}
		printf("\n");
	}
	void print_ordered_bytes()
	{
		if (order == byte_order::big_endian) {
			print_bytes();
		} else {
			printf("%02x", bytes[0]);
			for (int i = 1; i < bytes.size(); ++i) {
				printf(" %02x", bytes[i]);
			}
			printf("\n");
		}
		printf("== == == ==\n");
		if (order == byte_order::big_endian) {
			printf(" 3  2  1  0\n");
		} else {
			printf(" 0  1  2  3\n");
		}
	}
};

int main(int argc, const char *argv[])
{
	printf("Language 0.0.1-development\n");

	std::array<std::tuple<size_t, byte_order>, 4> fundamentals_supported = {
            std::make_tuple(1, byte_order::none),
	    std::make_tuple(2, byte_order::little_endian),
	    std::make_tuple(4, byte_order::little_endian),
	    std::make_tuple(8, byte_order::little_endian),
        };

	fundamental x(fundamentals_supported[2]);
	printf("\n");
	x.print_bytes();
	printf("\n");
	x.print_ordered_bytes();

	return 0;
}
