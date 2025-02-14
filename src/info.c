/*

Segments:
Possibly 4 (2 code segments, 1 data, 1 stack?)
1ED (default code segment?)


Game start is at 1191

1EA4: 486E
1EA2: 4856
1EA0: 0001

// Takes 5 arguments?
sub_1191(

call sub_105

sub_18B8()
*/

// sub_18B8
int open_file(const char *fname, int mode)
{
  return open(fname, mode);
}

// sub_19BE
size_t get_file_size(int fd)
{
  //
}

// sub_1902
void read_file_into_buffer(int fd, size_t sz, unsigned char *buf)
{
}


void sub_0105()
{
  int disk1_fd = open_file("DISK1", 0);
  size_t file_size = get_file_size(disk_fd);

  read_file_into_buffer(disk_fd, file_size, buffer);

  close(disk1_fd);

  if (buffer[0] != 0) {
  }

  if (buffer[2] != 0) {
  }
}

sub_1439()


sub_0014()
