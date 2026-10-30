/*
 * Fountain of Dreams - Reverse Engineering Project
 *
 * Copyright (c) 2026 Devin Smith <devin@devinsmith.net>
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

#ifndef SERVICES_H
#define SERVICES_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// This is 32 bytes, but we don't know the different properties.
struct service_type {
  char *name;     // 16 bytes
  uint32_t price;  // 0x16
  uint8_t req_kind; // 0x1A
  int req_value;  // 0x1C
};

struct npc_service {
  char *name; // At 0x14, terminated by 0
  uint8_t anim_id; // At 0x28
  uint16_t count; // 0x2A
  struct service_type services[7]; // Not all NPCs offer 7 services.
};

struct npc_service *service_get_by_npc(int service_num);

#ifdef __cplusplus
}
#endif

#endif /* SERVICES_H */
