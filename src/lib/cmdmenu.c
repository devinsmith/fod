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

#include <stdio.h>
#include <string.h>

#include "cmdmenu.h"
#include "game.h"
#include "platform.h"
#include "resource.h"
#include "script.h"
#include "ui.h"

// Border drawing sub-routines.
static void sub_DAB0();

// KEH DSEG:0x1ED26
static uint8_t byte_1ED26 = 0;

// KEH DSEG:0x12D80 - UI flag (set to 8 during header print, then reset to 0)
static uint16_t word_12D80 = 0;

// KEH: DSEG:0x01D0 - condition status name table (string pointers)
static const char *condition_names[] = {
  NULL,
  "Wounded",
  "S. Wounded",
  "Cr. Wounded",
  "Near Death",
  "Dead",
};

// KEH: DSEG: 0xCDC0
// We don't know how big this is.
static uint16_t table_CDC0[32];

// KEH DSEG:0x3240 - table mapping encounter result to dword_1EC1A index
static uint16_t table_3240[100];

// KEH: DSEG:0x15F8/0x15F9/0x15FA - skill display tables
// Each entry: { x_coord, y_coord, skill_name_string }
static const struct {
  uint8_t x;
  uint8_t y;
  const char *name;
} skill_display[7] = {
  { 1,  9,  "ST:" },
  { 17, 9,  "IQ:" },
  { 1,  10, "DX:" },
  { 9,  10, "WP:" },
  { 17, 10, "AP:" },
  { 1,  11, "CH:" },
  { 17, 11, "LK:" },
};

// FOD: DSEG:0x029C
// KEH: DSEG:0x1A4C
extern struct ui_region *active_region;

// KEH: DSEG:0x1B2C - character status display region used by sub_C68D
static struct ui_region region_1B2C = {
  1, // Initial X cursor position
  1, // Initial Y cursor position
  0x17, // Max X cursor position
  0x0D, // Max Y cursor position
  1, // Cursor index x
  1, // Cursor index y
  { 4, 8, 0x5C, 0x68 },
  0,
  NULL,
  0,
  NULL
};

// KEH: DSEG: 0x1B64
static struct ui_region region_1B64 = {
  1, // Initial X cursor position
  0x0E, // Initial Y cursor position.
  0x17, // Max X cursor position
  0x17, // Max Y cursor position.
  1,    // Cursor index x
  0x0E, // Cursor index y,
  { 4, 0x70, 0x5C, 0x50 },
  0,
  NULL,
  0,
  NULL
};

// KEH: DSEG: 0x1B80
static struct ui_region region_1B80 = {
  0x19,
  1,
  0x26,
  0x0B,
  0x19,
  0x01,
  { 0x64, 0x08, 0x38, 0x58 },
  0,
  NULL,
  0,
  NULL
};

// KEH: DSEG: 0x1B9C
static struct ui_region region_1B9C = {
  0x19,
  0x0D,
  0x26,
  0x17,
  0x19,
  0x0D,
  { 0x64, 0x68, 0x38, 0x58 },
  0,
  NULL,
  0,
  NULL
};

// KEH: DSEG: 0x1CB4
static struct ui_region region_1CB4 = {
  0, // initial X cursor position - 0x00
  0, // initial Y cursor position - 0x02
  0x27, // max_x_cursor_position - 0x04
  0x18, // max_y_cursor_pos - 0x06
  0, // cursor_index_x - 0x08
  0, // cursor_index_y - 0x0A
  { 0, 0, 0xA0, 0xC8 }, // Rect offset 0x0C - 0x12
  0, // line_number 0x14
  sub_DAB0, // offset 16
  0, // data_24 0x18
  NULL // 0x1A
};

enum panel { PANEL_STATS = 0, PANEL_ACTIVE = 1, PANEL_PASSIVE = 2 };

struct cmd_menu {
  uint16_t mode;
};

static uint8_t sub_1FB7(uint16_t arg0, uint16_t arg1, uint16_t arg2)
{
  printf("%s: unimplemented\n", __func__);
  return 0;
}

// KEH: seg000:C5F4
static void sub_C5F4(struct player_rec *player_ptr, bool flag)
{
  char buffer[49];
  ui_region_set_active(&region_1B2C, false);

  snprintf(buffer, sizeof(buffer), "%s", player_ptr->name);
  if (flag) {
    ui_set_inverse(true);
  }

  // Calculate centered X position for the string
  // Original: (strlen(str_buffer) - 0x19) negated then divided by 2
  // This centers the string within a 0x19 (25) character wide region
  size_t len = strlen(buffer);
  int16_t x_pos;

  if (len > 0x19) {
    // If string is longer than 25 chars, position at 0 (left edge)
    x_pos = 0;
  } else {
    // Center it: (25 - len) / 2
    x_pos = (0x19 - (int16_t)len) / 2;
  }

  ui_region_print_str(buffer, x_pos, 0);

  // Clear the flag
  ui_set_inverse(false);
}

// KEH: DSEG: 0xC64A
static void write_affliction(struct player_rec *player_ptr, uint16_t arg2)
{
  char buffer[64];

  if (player_ptr->affliction != 0) {
    printf("%s: TODO: Player is afflicted (%d) (%d)\n", __func__, arg2, player_ptr->affliction);
  }
  snprintf(buffer, sizeof(buffer), "Cond: Unafflicted");
}

// KEH: seg000:C68D
static void sub_C68D(struct player_rec *player_ptr, bool flag)
{
  uint16_t var_4;
  char buffer[128];

  uint16_t armor_class = sub_7D9B(player_ptr);

  ui_region_set_active(&region_1B2C, false);
  ui_active_region_clear();

  sub_C5F4(player_ptr, flag);

  snprintf(buffer, sizeof(buffer), "Rank: %u", player_ptr->rank);

  ui_region_print_str(buffer, 0, 1);

  write_affliction(player_ptr, 0);
  ui_region_print_str(buffer, 0, 2);

  if (player_ptr->affliction != 0) {
    ui_region_print_str("(V)iew", 0x10, 2);
  }

  var_4 = get_player_condition_status(player_ptr);
  if (var_4 == 0) {
    snprintf(buffer, sizeof(buffer), " Con: %3u  MaxCon:  %3u",
        player_ptr->condition, player_ptr->max_condition);
  } else {
    snprintf(buffer, sizeof(buffer), " Con: %3s  MaxCon:  %3u",
             condition_names[var_4], player_ptr->max_condition);
  }
  ui_region_print_str(buffer, 0, 3);

  snprintf(buffer, sizeof(buffer), "  AC %3u", armor_class);
  ui_region_print_str(buffer, 0, 4);

  if (player_ptr->weapon_idx == 0xFF) {
    snprintf(buffer, sizeof(buffer), "Weap: Hands");
  } else {
    // TODO: Figure out equipped item name (Weapon).
    printf("%s: TODO: Figure out equipped item name, item is: %d\n", __func__, player_ptr->weapon_idx);
    snprintf(buffer, sizeof(buffer), "Weap: %s", get_item_name(player_ptr->weapon_idx));
  }
  ui_region_print_str(buffer, 0, 5);

  word_12D80 = 8;
  ui_region_print_str("Attributes", 6, 7);
  word_12D80 = 0;

  // C7F9
  for (int i = 0; i < 7; i++) {
    snprintf(buffer, sizeof(buffer), "%3s%2u",
             skill_display[i].name, player_ptr->attributes[i]);
    ui_region_print_str(buffer,
                        skill_display[i].x,
                        skill_display[i].y);
  }
}

// KEH: seg001:0182
static bool is_item_equipped(struct player_rec *player, uint16_t item_id)
{
  if (player->weapon_idx == item_id) {
    return true;
  }

  for (int i = 0; i < 3; i++) {
    if (player->armor_idx[i] == item_id) {
      return true;
    }
  }

  return false;
}

// KEH: seg000:13E4
static void draw_inventory_row(uint16_t row_number, uint16_t item_index, void *ud)
{
  char buf[46];
  struct player_rec *player = ud;
  uint16_t item_id = table_CDC0[item_index];
  bool is_equipped = is_item_equipped(player, item_id);
  char marker = is_equipped ? '*': ' ';

  snprintf(buf, sizeof(buf), "%d>%c", row_number, marker);
  ui_region_print_str(buf, 0, row_number + 1);

  /* 6-byte records starting at player_ptr+0x60 */
  struct item_rec *item = &player->items[item_id];

  if (item->flags & 1) {
    ui_set_inverse(true);
  }

  ui_region_print_str(get_item_name(item->item_id), -1, -1);

  ui_set_inverse(false);
}

// KEH: seg000:C990
static uint16_t draw_inventory_list(struct player_rec *player_ptr, uint16_t *scroll_offset,
                          uint16_t *max_count)
{
  ui_region_set_active(&region_1B64, false);
  // Handle inventory?

  // ?
  for (int i = 0; i < player_ptr->unknown_52; i++) {
    table_CDC0[i] = i;
  }
  *max_count = player_ptr->unknown_52;

  /*
      if (*scroll_offset == *max_count && *max_count > 0)
        *scroll_offset -= 8;
        */

  return ui_draw_scroll_list_page(player_ptr, *scroll_offset, *max_count,
      "Inventory Items", draw_inventory_row, false, 0);
}

// KEH: seg000:C53E
static void draw_active_skill_row(uint16_t row_number, uint16_t item_index, void *ud)
{
  char buf[42];
  uint16_t skill_index = table_CDC0[item_index];
  struct player_rec *player = ud;

  const char *skill_name = (char *)hds_res->bytes + 0x16 * skill_index + 0x126;

  snprintf(buf, sizeof(buf), "%1d>%2d %-.9s", row_number, player->active_skills[skill_index], skill_name);
  ui_region_print_str(buf, 0, row_number + 1);
}

// KEH: seg000:C53E
static void draw_passive_skill_row(uint16_t row_number, uint16_t item_index, void *ud)
{
  char buf[42];
  uint16_t skill_index = table_CDC0[item_index];
  struct player_rec *player = ud;

  const char *skill_name = (char *)hds_res->bytes + 0x16 * skill_index + 0x286;

  snprintf(buf, sizeof(buf), "%1d>%2d %-.9s", row_number, player->passive_skills[skill_index], skill_name);
  ui_region_print_str(buf, 0, row_number + 1);
}

// KEH: seg000:C84A
static int draw_active_skills(int player_num, uint16_t *counter, bool handle_input, uint16_t arg3)
{
  uint16_t var_2;

  ui_region_set_active(&region_1B80, false);

  struct player_rec *player /* var_6 */ = &g_game_state.players[player_num];

  var_2 = 0;

  // Build a list of active skill indexes.
  // This shouldn't go more than 16, since player->active_skills is limited
  // to 16 skills.
  for (uint16_t i = 0; i < hds_res->bytes[2]; i++) {
    if (player->active_skills[i] != 0) {
      table_CDC0[var_2++] = i;
    }
  }

  if (handle_input) {
    return ui_scroll_list_menu(player, var_2, counter, "Active", draw_active_skill_row, true);
  }

  ui_draw_scroll_list_page(
        player,
        *counter,
        var_2,
        "Active",
        draw_active_skill_row,
        arg3,
        false);

  return -1;
}

// KEH: seg000:C8ED
static int draw_passive_skills(int player_num, uint16_t *counter, bool handle_input,
                          uint16_t arg3)
{
  uint16_t var_2;

  ui_region_set_active(&region_1B9C, false);

  struct player_rec *player /* var_6 */ = &g_game_state.players[player_num];

  var_2 = 0;

  // Build a list of passive skill indexes.
  // This shouldn't go more than 16, since player->passive_skills is limited
  // to 16 skills.
  for (uint16_t i = 0; i < hds_res->bytes[4]; i++) {
    if (player->passive_skills[i] != 0) {
      table_CDC0[var_2++] = i;
    }
  }

  if (handle_input) {
    return ui_scroll_list_menu(player, var_2, counter, "Passive", draw_passive_skill_row, true);
  }

  ui_draw_scroll_list_page(
        player,
        *counter,
        var_2,
        "Passive",
        draw_passive_skill_row,
        arg3,
        false);

  return -1;
}

// KEH: seg000:C052
static uint8_t use_medic_skill(uint16_t arg0, bool interactive, uint16_t arg2)
{
  if (interactive) {
    ui_region_set_active(&popup_region, 1);
    ui_region_refresh_active();
  }

  // Pick the target.
  printf("%s: unimplemented\n", __func__);
  return 0;
}


static void sub_CA0B(struct player_rec *player_ptr)
{
  printf("%s: unimplemented\n", __func__);
}

static uint8_t sub_CBDF(uint16_t arg0)
{
  printf("%s: unimplemented\n", __func__);
  return 0;
}

static uint8_t sub_CB58(int player_num)
{
  struct player_rec *current_player = &g_game_state.players[player_num];
  if (current_player->condition >= 1) {
    return 1;
  }

  print_incapacitated(player_num, 1);
  return 0;
}

// KEH: seg000:CAE6
static bool choose_left_right(uint16_t key,
    uint16_t *dst, uint16_t left_val,
    uint16_t right_val)
{
  bool handled = false;

  if (key == KEY_LEFT) {
    *dst = left_val;
    handled = true;
  } else if (key == KEY_RIGHT) {
    *dst = right_val;
    handled = true;
  }

  return handled;
}

// KEH: seg000:0xCC58
// F-key handler - character skill/action menu.
// arg0 = menu page (always 0 from game loop)
// fkey_index = which F-key was pressed (0=F1..4=F5)
// Returns arg0 on action completion, or -1 on ESC/exit.
int show_character_sheet(int arg0, int fkey_index)
{
  struct cmd_menu m = {
    .mode = PANEL_STATS
  };

  int tmp;
  struct player_rec *player_selected = NULL;
  uint16_t var_1C;
  uint16_t var_1A;
  uint16_t var_18;
  bool done = false;
  uint16_t var_12;
  uint16_t var_E;
  struct ui_region *var_C;
  bool no_confirm;
  int16_t  key_signed;

  no_confirm = (fkey_index != 0) || bool_DAE6;

  var_C = active_region;

  while (!done) {
    struct player_rec *si = &g_game_state.players[arg0];
    if (player_selected != si) {
      player_selected = si;
      var_18 = 0;
      var_1C = 0;
      var_1A = 0;
    }

    ui_region_set_active(&region_1CB4, true);

    // The second argument to this function probably should not be
    // hardcoded as true. It controls whether the player's name should be
    // highlighted, but we need to figure out how it gets set here. It is
    // tied to var_6.
    sub_C68D(player_selected, m.mode == PANEL_STATS);

    var_E = draw_inventory_list(player_selected, &var_18, &var_12);

    draw_active_skills(arg0, &var_1C, false, (m.mode == PANEL_ACTIVE) ? 1 : 0);
    draw_passive_skills(arg0, &var_1A, false, (m.mode == PANEL_PASSIVE) ? 1 : 0);

    ui_region_refresh(&whole_screen);

    switch (m.mode) {
    case PANEL_STATS:
      var_E = draw_inventory_list(player_selected, &var_18, &var_12);

      uint8_t key = sys_readkey();
      if (key == KEY_ESC) {
        done = true;
        continue;
      }

      player_selected->cmd_type = 0x0A;

      key_signed = (int16_t)(int8_t)key;
      switch (key_signed) {
      case 'A':
        player_selected->cmd_arg = 4;
        break;
      }

      if (key_signed == 'A') {
        goto handle_char_action;
      } else if (key_signed > 'A') {
        goto handle_key_gt_A;
      } else {
        goto handle_key_lt_A;
      }

handle_key_lt_A:
      if (key_signed == (int16_t)0xFFFC) {
        if ((int16_t)(var_12 - var_18) > (int16_t)var_E)
          var_18 += 8;
        var_E = draw_inventory_list(player_selected, &var_18, &var_12);
        continue;
      }

      if (key_signed <= (int16_t)0xFFFC) {
        if (key_signed >= (int16_t)0xFFFA && key_signed <= (int16_t)0xFFFB) {
          choose_left_right(key, &m.mode, 2, 1);
          continue;
        }
        goto handle_generic_key;
      }

      if (key_signed == (int16_t)0xFFFD) {
        if (var_18 >= 8)
          var_18 -= 8;
        var_E = draw_inventory_list(player_selected, &var_18, &var_12);
        continue;
      }

      if (key_signed >= (int16_t)0x3B && key_signed <= (int16_t)0x3F) {
        handle_fkeys(key, (uint16_t *)&arg0, 6, no_confirm, 1);
        continue;
      }

      goto handle_generic_key;

handle_key_gt_A:
      {
        int table_idx = key_signed - 'C';
        if (table_idx < 0 || table_idx > 20)
          goto handle_generic_key;

        static const uint8_t key_targets[21] = {
          0, 0, 1, 1, 1, 1, 0, 1, 1, 0,
          1, 1, 1, 1, 1, 1, 0, 1, 1, 2, 0
        };

        int target = key_targets[table_idx];
        if (target == 0) {
          goto handle_char_action;
        } else if (target == 2) {
          if (player_selected->affliction != 0)
            sub_CA0B(player_selected);
          continue;
        } else {
          goto handle_generic_key;
        }
      }

handle_char_action:
      {
        unsigned char *p = (unsigned char *)(uintptr_t)player_selected;
        if (key_signed == 'D') {
          p[0x145] = 2;
        } else if (key_signed > 'D') {
          if (key_signed == 'I')
            p[0x145] = 1;
          else if (key_signed == 'L')
            p[0x145] = 6;
          else if (key_signed == 'S')
            p[0x145] = 0;
          else if (key_signed == 'W')
            p[0x145] = 3;
          else
            goto action_check;
        } else {
          if (key_signed == 'A') {
            player_selected->cmd_arg = 4;
          } else if (key_signed == 'C')
            p[0x145] = 5;
          else
            goto action_check;
        }
      }

action_check:
      if (sub_CB58(arg0)) {
        sub_CBDF(arg0);
        return arg0;
      }
      player_selected = NULL;
      continue;

handle_generic_key:
      if (key >= 0x3B && key <= 0x3F) {
        handle_fkeys(key, (uint16_t *)&arg0, 6, no_confirm, 1);
        continue;
      }

      if (key >= '0' && key <= '9') {
        uint16_t digit = key - '0';

        if (digit == 0 || digit > var_E)
          continue;

        tmp = table_3240[digit + var_18];

        player_selected->cmd_arg = (uint8_t)tmp;

        if (byte_1ED26 == 0) {
          if (sub_1FB7(arg0, (uint8_t)tmp, fkey_index)) {
            unsigned char *p =
                (unsigned char *)(uintptr_t)player_selected;
            if (fkey_index != 0) {
              if (p[0x144] == 0x0D || p[0x144] == 0x0C)
                return arg0;
            }
            if (p[0x144] == 6)
              return arg0;
            sub_CBDF(arg0);
            return arg0;
          }
          continue;
        }
        return arg0;
      }

      continue;

    case PANEL_ACTIVE:
      tmp = draw_active_skills(arg0, &var_1C, true, 1);

      if (handle_fkeys(tmp + 0x46, (uint16_t *)&arg0, 6, no_confirm, 1))
        continue;

      if (choose_left_right(tmp, &m.mode, 0, 2))
        continue;

      if (tmp < 0) {
        if (tmp == -1)
          done = true;
        continue;
      }

      if (sub_CB58(arg0)) {
        player_selected->cmd_arg = (uint8_t)table_3240[tmp];

        uint8_t val = player_selected->cmd_arg;
        if (val == 0 || val == 0x0B || val == 4) {
          if (bool_DAE6)
            continue;

          if (!use_medic_skill(arg0, 1, (fkey_index == 0) ? 1 : 0))
            continue;

          return arg0;
        }

        player_selected->cmd_type = 0x0A;
        player_selected->cmd_arg += 0x0C;

        if (fkey_index == 0) {
          sub_CBDF(arg0);
          return arg0;
        }

        if (player_selected->cmd_arg == 0x1A || player_selected->cmd_arg == 0x19)
          return arg0;

        sub_CBDF(arg0);
        return arg0;
      }

      break;
    case PANEL_PASSIVE:
      tmp = draw_passive_skills(arg0, &var_1A, true, 1);

      if (handle_fkeys(tmp + 0x46, (uint16_t *)&arg0, 6, no_confirm, 1))
        continue;

      if (tmp == -1) {
        done = true;
        continue;
      }

      choose_left_right(tmp, &m.mode, 1, 0);
      break;
    }
  }

  ui_region_set_active(var_C, false);
  return -1;
}

// KEH:seg000:0xDAB0
static void sub_DAB0()
{
  draw_borders(1000);
}

