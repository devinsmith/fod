# Disk1

Contains the saved game and game state.

| Offset        | Purpose                                               |
|---------------|-------------------------------------------------------|
| 0x00-0x01     | Is this a saved game or new game (0 = new, 1 = saved) |
| 0x02-0x03     | Skip initialization (if not 0)                        |

