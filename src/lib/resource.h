/*
 * Fountain of Dreams - Reverse Engineering Project
 *
 * Copyright (c) 2018-2020,2025 Devin Smith <devin@devinsmith.net>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef RESOURCE_H
#define RESOURCE_H

#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

enum resource_file {
  RESOURCE_TITLE,
  RESOURCE_GANI,
  RESOURCE_BORDERS,
  RESOURCE_LAST
};

struct resource {
  unsigned char *bytes;
  size_t len;
};

/**
 * @brief Initializes the resource manager.
 */
bool rm_init();

/**
 * @brief Loads the named resource
 *
 * @param rfile   The resource file to load.
 * @param offset  The offset to read from, or 0.
 * @param sz      The number of bytes to read, or 0 for entire file.
 */
struct resource *resource_load(enum resource_file rfile, long offset, size_t sz);

/**
 * @brief Releases a resource (frees memory)
 *
 * @param res   Pointer to the resource to release
 */
void resource_release(struct resource *res);

#ifdef __cplusplus
}
#endif

#endif /* RESOURCE_H */
