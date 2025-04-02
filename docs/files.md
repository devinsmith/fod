
| File     | Memory   | Segment |
| -------- | -------- | ------- |
| BORDERS  | (0x5000) | DS:48B8 |
| ARCHTYPE |          | DS:4CA2 |
| HDSPCT   | 1180     | DS:37E8 |

# FONT

This is allocated to DS:56EC.

First two bytes define number of sprites in the file (209 (0xD1)). The
sprite content is shown in the below image (19 x 11 sprites):

![fontsheet](fontsheet.png)

Each sprite is 8x8 pixels. However, when stored in the FONT file, it is stored
4x8 (32 bytes) rather than 64 bytes. The nibbles of the 4 bytes are swapped
around to form an 8x8 image.
