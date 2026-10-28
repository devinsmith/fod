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

#include "services.h"

static struct npc_service doc_marino = {
  "Doc Marino.",
  0
};

static struct npc_service doc_brewhoe = {
  "Doc Brewhoe's",
  0
};

static struct npc_service doc_demedici = {
  "Dr. DeMedici",
  0xF
};

struct npc_service *npc_services[] = {
  &doc_marino,
  &doc_brewhoe,
  &doc_demedici
};

struct npc_service *service_get_by_npc(int service_num)
{
  return npc_services[service_num];
}

