/*
 * Copyright (c) 2008-2025 Devin Smith <devin@devinsmith.net>
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

/* Provides "readers" and "writers" over a buffer of bytes. */

#ifndef BUFIO_H_INCLUDED
#define BUFIO_H_INCLUDED

#include <cstdint>
#include <string>
#include <vector>

struct buf_wri {
  unsigned char *base;
  size_t len;
};

/* Buffer writing functions */
struct buf_wri *buf_wri_init(size_t len);
void buf_wri_free(struct buf_wri *w);
void buf_add8(struct buf_wri *w, uint8_t val);


// A reader over a buffer of bytes
class buf_rdr {
  void ensure(std::size_t n) const;

  std::vector<std::uint8_t> data_;
  std::size_t offset_;

public:
  buf_rdr(std::vector<std::uint8_t> data)
    : data_(std::move(data)), offset_(0) {}

  static buf_rdr from_file(const std::string& filename);

  // move-only
  buf_rdr(buf_rdr &&)                 = default;
  buf_rdr &operator=(buf_rdr &&)      = default;
  buf_rdr(const buf_rdr &)            = delete;
  buf_rdr &operator=(const buf_rdr &) = delete;

  void reset() { offset_ = 0; }
  std::size_t offset() const { return offset_; }
  std::size_t remaining() const { return data_.size() - offset_; }
  std::size_t size() const { return data_.size(); }
  bool eof() const { return offset_ >= data_.size(); }

  const std::uint8_t *data() const { return data_.data(); }
  const std::uint8_t *cursor() const { return data_.data() + offset_; }

  void seek(std::size_t pos);
  void skip(std::size_t n);
  void read(void *dst, std::size_t n);

  // Extractors
  std::uint8_t get8();
  std::uint16_t get16le();
  std::uint16_t get16be();
  std::uint32_t get32le();
  std::uint32_t get32be();
};

#endif // BUFIO_H_INCLUDED

