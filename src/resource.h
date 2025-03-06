//
// Fountain of Dreams - Reverse Engineering Project
//
// Copyright (c) 2018-2020,2025 Devin Smith <devin@devinsmith.net>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef RESOURCE_H
#define RESOURCE_H

enum class resource_type {
  title
};

struct resource {
  unsigned char *bytes;
  size_t len;
};

bool rm_init();

resource* resource_load(resource_type rt);

#endif // RESOURCE_H
