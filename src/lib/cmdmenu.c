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

#include <ctype.h>
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
static uint16_t skill_cmd_tbl[100];

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
enum menu_state { MENU_RUN, MENU_CANCEL, MENU_RETURN };

struct cmd_menu {
  uint16_t player_idx; // The 0 based index of the player being worked on.
  bool flag;  // Unknown
  bool no_confirm; // Unknown
  struct player_rec *player; // Selected player
  uint16_t scroll;
  uint16_t total;
  uint16_t page_count;
  uint16_t active_state;
  uint16_t passive_state;
  enum panel mode; // The mode the panel is in, see enum panel.
  enum menu_state state;
  int result;
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

static bool sub_CB58(int player_num)
{
  // Player has at least one health point?
  struct player_rec *current_player = &g_game_state.players[player_num];
  if (current_player->condition >= 1) {
    return true;
  }

  print_incapacitated(player_num, 1);
  return false;
}

// KEH: seg000:CAE6
static bool change_panel(uint16_t key,
    enum panel *dst, uint16_t left_val,
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

static void menu_return(struct cmd_menu *m, int16_t v)
{
  m->state = MENU_RETURN;
  m->result = v;
}

static void menu_page(struct cmd_menu *m, int dir)
{
  if (dir < 0) {
    if (m->scroll < 8) return;
    m->scroll -= 8;
  } else {
    if (m->total - m->scroll <= m->page_count) return;
    m->scroll += 8;
  }
  m->page_count = draw_inventory_list(m->player, &m->scroll, &m->total);
}

/* letter-command tail: CDE4 / CDF1 / CE4F */
static void menu_commit(struct cmd_menu *m)
{
  if (!sub_CB58(m->player_idx)) {
    m->player = NULL;                /* forces scroll reset next iteration */
    return;
  }
  menu_return(m, sub_CBDF(m->player_idx));
}

/* digit key: pick entry n on the current page (CF06..CEE2) */
static void stats_pick_entry(struct cmd_menu *m, int8_t key)
{
  if (key < 0 || !isdigit((unsigned char)key)) return;

  int16_t n = key & 0x0F;
  if (n <= 0 || n > m->page_count) return;

  int16_t entry = (int16_t)skill_cmd_tbl[m->scroll + n - 1];
  m->player->cmd_arg = (uint8_t)entry;

  if (bool_DAE6) { menu_return(m, m->player_idx); return; }
  if (!sub_1FB7(m->player_idx, entry, m->flag)) return;

  uint8_t t = m->player->cmd_type;     /* presumably set by sub_1FB7 */
  if ((m->flag && (t == 0x0C || t == 0x0D)) || t == 6)
    menu_return(m, m->player_idx);
  else
    menu_return(m, sub_CBDF(m->player_idx));
}

static void panel_stats(struct cmd_menu *m)
{
  const char cmd_keys[] = "SIDWACL";   /* index == cmd_arg */

  m->page_count = draw_inventory_list(m->player, &m->scroll, &m->total);

  uint8_t key = sys_readkey();
  if (key == KEY_ESC) {
    m->state = MENU_CANCEL;
    return;
  }

  m->player->cmd_type = 0x0A;

  /* S/I/D/W/A/C/L */
  const char *c = strchr(cmd_keys, key);
  if (c) {
    m->player->cmd_arg = (uint8_t)(c - cmd_keys);
    menu_commit(m);
    return;
  }

  switch (key) {
  case 'V':
    if (m->player->affliction) sub_CA0B(m->player);
    break;
  case KEY_LEFT:
  case KEY_RIGHT:
    change_panel(key, &m->mode, PANEL_PASSIVE, PANEL_ACTIVE);
    break;
  case KEY_UP: menu_page(m, -1); break;
  case KEY_DOWN: menu_page(m, +1); break;
  case 0x3B: case 0x3C: case 0x3D: case 0x3E: case 0x3F:     /* F1-F5 */
    handle_fkeys(key, &m->player_idx, 6, m->no_confirm, 1);
    break;
  default:
    stats_pick_entry(m, key);
    break;
  }
}

static void panel_active(struct cmd_menu *m)
{
  struct player_rec *p = m->player;

  int16_t sel = draw_active_skills(m->player_idx, &m->active_state, true, 1);

  if (handle_fkeys(sel + 0x46, &m->player_idx, 6, m->no_confirm, 1))
    return;

  if (change_panel(sel, &m->mode, PANEL_STATS, PANEL_PASSIVE))
    return;

  if (sel < 0) {
    if (sel == -1) {
      m->state = MENU_CANCEL;
    }
    return;
  }

  if (!sub_CB58(m->player_idx)) {
    return;
  }

  p->cmd_arg = (uint8_t)skill_cmd_tbl[sel];

  if (p->cmd_arg == 0 || p->cmd_arg == 0x0B || p->cmd_arg == 4) {
    if (bool_DAE6) return;

    if (use_medic_skill(m->player_idx, 1, !m->flag)) {
      menu_return(m, m->player_idx);
    }
    return;
  }

  p->cmd_type = 0x0A;
  p->cmd_arg += 0x0C;

  if (m->flag && (p->cmd_arg == 0x19 || p->cmd_arg == 0x1A)) {
    menu_return(m, m->player_idx);
  } else {
    menu_return(m, sub_CBDF(m->player_idx));
  }
}

static void panel_passive(struct cmd_menu *m)
{
  int16_t sel = draw_passive_skills(m->player_idx, &m->passive_state, true, 1);

  if (handle_fkeys(sel + 0x46, &m->player_idx, 6, m->no_confirm, 1)) {
    return;
  }

  if (sel == -1) {
    m->state = MENU_CANCEL;
    return;
  }

  change_panel(sel, &m->mode, PANEL_ACTIVE, PANEL_STATS);
}

// KEH: seg000:0xCC58
// F-key handler - character skill/action menu.
// Returns player index on action completion, or -1 on ESC/exit.
int show_character_sheet(int player_idx, bool flag)
{
  struct cmd_menu m = {
    .player_idx = player_idx,
    .flag = flag,
    .no_confirm = flag || bool_DAE6,
    .mode = PANEL_STATS,
    .state = MENU_RUN
  };

  struct ui_region *saved_region;

  saved_region = active_region;

  while (m.state == MENU_RUN) {
    if (m.player != &g_game_state.players[m.player_idx]) {
      m.player = &g_game_state.players[m.player_idx];
      m.scroll = m.active_state = m.passive_state = 0;
    }

    ui_region_set_active(&region_1CB4, true);
    sub_C68D(m.player, m.mode == PANEL_STATS);
    m.page_count = draw_inventory_list(m.player, &m.scroll, &m.total);
    draw_active_skills(m.player_idx, &m.active_state, false, (m.mode == PANEL_ACTIVE) ? 1 : 0);
    draw_passive_skills(m.player_idx, &m.passive_state, false, (m.mode == PANEL_PASSIVE) ? 1 : 0);

    ui_region_refresh(&whole_screen);

    switch (m.mode) {
    case PANEL_STATS:
      panel_stats(&m);
      break;

    case PANEL_ACTIVE:
      panel_active(&m);
      break;
    case PANEL_PASSIVE:
      panel_passive(&m);
      break;
    }
  }

  if (m.state == MENU_CANCEL) {
    ui_region_set_active(saved_region, false);
    return -1;
  }
  return m.result; // No restoring region
}

// KEH:seg000:0xDAB0
static void sub_DAB0()
{
  draw_borders(1000);
}

