# Disk1

Contains the saved game and game state. This file is 3776 bytes, but only some
of its contents are documented.

| Offset        | Purpose                                               |
|---------------|-------------------------------------------------------|
| 0x00-0x01     | Is this a saved game or new game (0 = new, 1 = saved) |
| 0x02-0x03     | Skip initialization (if not 0)                        |
| 0x04-0x05     | Unknown                                               |
| 0x06          | Video mode (3 = MCGA/VGA)                             |
| 0x31          | Party size (number of characters in party)            |

Character offsets are at 0x3A, 0x186, 0x2D2, 0x41E, 0x56E. There can
be 5 characters within a party and each character is 332 bytes long.

| Offset        | Purpose                                         |
|---------------|-------------------------------------------------|
| 0x3A-0x46     | Name (up to 12 characters, terminated with a 0) |
| 0x48          | Profession (0 = Survivalist, 1 = Vigalante, etc)|
| 0x50          | Gender (1 = Male, others = Female)              |
| 0x52          | Strength                                        |
| 0x53          | Intelligence                                    |
| 0x54          | Dexterity                                       |
| 0x55          | Will Power                                      |
| 0x56          | Aptitude                                        |
| 0x57          | Charisma                                        |
| 0x58          | Luck                                            |
| 0x5D          | Attribute points to distribute                  |
| 0x5E          | Active skills (16 bytes)                        |
| 0x6E          | Passive skills (16 bytes)                       |



