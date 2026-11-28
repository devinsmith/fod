/*
 * Copyright (c) 2008-2026 Devin Smith <devin@devinsmith.net>
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

#include <cstdlib>
#include <cstring>

#include <fstream>
#include <stdexcept>

#include "bufio.h"

/* Buffer writer implementation */
struct buf_wri *
buf_wri_init(size_t len)
{
  struct buf_wri *w = (struct buf_wri *)malloc(sizeof(struct buf_wri));
  if (w == NULL)
    return NULL;

  w->base = (unsigned char *)malloc(len);
  if (w->base == NULL) {
    free(w);
    return NULL;
  }
  w->len = 0;

  return w;
}

void
buf_wri_free(struct buf_wri *w)
{
  if (w != NULL) {
    free(w->base);
    free(w);
  }
}

void
buf_add8(struct buf_wri *w, uint8_t val)
{
  w->base[w->len++] = val;
}

buf_rdr buf_rdr::from_file(const std::string& filename)
{
  std::ifstream in(filename, std::ios::binary | std::ios::ate);
  if (!in) {
    throw std::runtime_error("couldn't open " + filename);
  }

  const auto size = static_cast<std::size_t>(in.tellg());
  in.seekg(0, std::ios::beg);

  std::vector<std::uint8_t> data(size);
  if (size && !in.read(reinterpret_cast<char *>(data.data()),
                       static_cast<std::streamsize>(size))) {
    throw std::runtime_error("short read on " + filename);
  }

  std::printf("Opened %s %zu bytes\n", filename.c_str(), size);

  return buf_rdr(std::move(data));
}

void buf_rdr::ensure(std::size_t n) const
{
  if (n > data_.size() - offset_) {
    throw std::out_of_range("buf_rdr: read past end of buffer");
  }
}

uint8_t buf_rdr::get8()
{
  ensure(1);
  return data_[offset_++];
}

std::uint16_t buf_rdr::get16le()
{
  ensure(2);
  std::uint16_t v = data_[offset_++];
  v |= static_cast<std::uint16_t>(data_[offset_++]) << 8;
  return v;
}

std::uint16_t buf_rdr::get16be()
{
  ensure(2);
  std::uint16_t v = static_cast<std::uint16_t>(data_[offset_++]) << 8;
  v |= data_[offset_++];
  return v;
}

std::uint32_t buf_rdr::get32le()
{
  ensure(4);
  std::uint32_t v = data_[offset_++];
  v |= static_cast<std::uint32_t>(data_[offset_++]) << 8;
  v |= static_cast<std::uint32_t>(data_[offset_++]) << 16;
  v |= static_cast<std::uint32_t>(data_[offset_++]) << 24;
  return v;
}

std::uint32_t buf_rdr::get32be()
{
  ensure(4);
  std::uint32_t v = static_cast<std::uint32_t>(data_[offset_++]) << 24;
  v |= static_cast<std::uint32_t>(data_[offset_++]) << 16;
  v |= static_cast<std::uint32_t>(data_[offset_++]) << 8;
  v |= data_[offset_++];
  return v;
}

void buf_rdr::seek(std::size_t pos)
{
  if (pos > data_.size()) {
    throw std::out_of_range("buf_rdr::seek past end");
  }
  offset_ = pos;
}

void buf_rdr::skip(std::size_t n)
{
  ensure(n);
  offset_ += n;
}

void buf_rdr::read(void *dst, std::size_t n)
{
  ensure(n);
  std::memcpy(dst, data_.data() + offset_, n);
  offset_ += n;
}
