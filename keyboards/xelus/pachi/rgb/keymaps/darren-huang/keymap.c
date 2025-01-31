/* Copyright 2021 Harrison Chan (Xelus)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include QMK_KEYBOARD_H
#include <rgb_matrix_types.h>

#define _NUM_LAYER 1
#define _FL 2 // function layer, shortened for the keymaps section
#define _CL 3 // control layer, 2nd function layer
#define _VIM_LAYER 4
#define _DY_LAYER 5

// Define Types
enum Keyboard_Mode {Regular_Mode, Num_Mode, Vim_Mode, Y_Mode, D_Mode} KB_mode = Regular_Mode;
typedef enum Keyboard_Mode Keyboard_Mode;

// Define Emoji Map
enum unicode_names {
    D_FC,
    PLEAD,
    IRONY,
    SNEK
};

const uint32_t PROGMEM unicode_map[] = {
    [D_FC]  = 0x1F978,  // Disguised Face Emoji
    [PLEAD]  = 0x1F97A,  // 🥺
    [IRONY] = 0x2E2E,  // ⸮
    [SNEK]  = 0x1F40D, // 🐍
};

// Global Vars
Keyboard_Mode dy_mode_prev_mode = Regular_Mode;

rgb_config_t vim_mode_prev_rgb;
rgb_config_t dy_mode_prev_rgb;
rgb_config_t num_mode_prev_rgb;
rgb_config_t jptr_mode_prev_rgb;

bool JPTR_Mode_On = false;

bool VIM_lshift = false;
bool VIM_rshift = false;
bool clipboard_holds_line = false;
uint8_t RGB_val_vim = 150;

bool vim_shift(void) {
    return VIM_lshift || VIM_rshift;
}

void reset_vim_vars(void) {
    VIM_lshift = false;
    VIM_rshift = false;
}

void regular_mode_on(void) {
    KB_mode = Regular_Mode;
    layer_off(_NUM_LAYER);
    layer_off(_VIM_LAYER);
    reset_vim_vars();
}

void num_mode_on(void) {
    KB_mode = Num_Mode;
    layer_on(_NUM_LAYER);
    reset_vim_vars();
}

void vim_mode_on(void) {
    KB_mode = Vim_Mode;
    layer_on(_VIM_LAYER);
    reset_vim_vars();
}

void dy_vim_mode_on(Keyboard_Mode mode) {
    dy_mode_prev_mode = KB_mode;
    KB_mode = mode;
    layer_on(_DY_LAYER);
}

void dy_vim_mode_off(void) {
    KB_mode = dy_mode_prev_mode;
    layer_off(_DY_LAYER);
    // reset_oneshot_layer();
}

// saving RGB settings  ----------------------

void save_rgb(rgb_config_t* config) {
    //save current rgb settings
    config->enable = rgb_matrix_is_enabled();
    config->mode   = rgb_matrix_get_mode();
    config->hsv.h  = rgb_matrix_get_hue();
    config->hsv.s  = rgb_matrix_get_sat();
    config->hsv.v  = rgb_matrix_get_val();
}

// modifying RGB settings  ----------------------

void load_rgb(rgb_config_t* config) {
    rgb_matrix_mode(config->mode);
    rgb_matrix_sethsv(config->hsv.h, config->hsv.s, config->hsv.v);
    if (config->enable) {
       rgb_matrix_enable();
    } else {
       rgb_matrix_disable();
    }
}

enum rgb_preset {
    WHITE_RGB,
    RED_RGB,
    DRACULA_RGB,
    CYAN_RGB,
    GOLD_RGB,
    FRC_RGB,
    VIM_RGB,
    VIM_D_RGB,
    VIM_Y_RGB,
    H_MAP_RGB,
};

void set_rgb_preset(enum rgb_preset preset) {
    switch (preset) {
    case WHITE_RGB:
        rgb_matrix_mode(RGB_MATRIX_SOLID_COLOR);
        rgb_matrix_sethsv(HSV_WHITE);
        rgb_matrix_enable();
        break;
    case RED_RGB:
        rgb_matrix_mode(RGB_MATRIX_SOLID_COLOR);
        rgb_matrix_sethsv(HSV_RED);
        rgb_matrix_enable();
        break;
    case DRACULA_RGB:
        rgb_matrix_mode(RGB_MATRIX_SOLID_COLOR);
        //rgblight_setrgb(189, 147, 249);
        rgb_matrix_sethsv(187, 105, 249);
        rgb_matrix_enable();
        break;
    case CYAN_RGB:
        rgb_matrix_mode(RGB_MATRIX_SOLID_COLOR);
        rgb_matrix_sethsv(HSV_CYAN);
        rgb_matrix_enable();
        break;
    case GOLD_RGB:
        rgb_matrix_mode(RGB_MATRIX_SOLID_COLOR);
        rgb_matrix_sethsv(HSV_GOLDENROD);
        rgb_matrix_enable();
        break;
    case FRC_RGB:
        rgb_matrix_mode(RGB_MATRIX_PIXEL_FRACTAL);
        rgb_matrix_sethsv(HSV_RED);
        rgb_matrix_enable();
        break;
    case VIM_RGB:
        //load vim rgb settings
        rgb_matrix_mode(RGB_MATRIX_CYCLE_OUT_IN);
        rgb_matrix_sethsv(rgb_matrix_get_hue(), 240,
                        rgb_matrix_get_val() > RGB_val_vim ? rgb_matrix_get_val() : RGB_val_vim);
        rgb_matrix_enable();
        break;
    case VIM_D_RGB:
        //load vim d (delete) mode rgb settings
        rgb_matrix_mode(RGB_MATRIX_BAND_SPIRAL_VAL);
        rgb_matrix_sethsv(HSV_RED);
        rgb_matrix_enable();
        break;
    case VIM_Y_RGB:
        //load vim y (yank) mode rgb settings
        rgb_matrix_mode(RGB_MATRIX_BAND_SPIRAL_VAL);
        rgb_matrix_sethsv(HSV_SPRINGGREEN);
        rgb_matrix_enable();
        break;
    case H_MAP_RGB:
        // typing heatmap mode
        rgb_matrix_mode(RGB_MATRIX_TYPING_HEATMAP);
        rgb_matrix_sethsv(HSV_RED);
        rgb_matrix_enable();
        break;
    }
}


// keycodes ---------------------------------------------------------

enum custom_keycodes {
    QMKBEST = SAFE_RANGE,
    VDKTP_R, // virtual deskptop right
    VDKTP_L, // virtual desktop left
    TAB_R,   // move to the tab to the right (for chrome)
    TAB_L,   // move to the tab to the left (for chrome)

    VIM_MD,  // enter vim mode
    VIM_RST, // reset vim settings (for use only in the vim layer)
    REG_MD,  // enter insert mode (or just regular mode)
    NUM_MD,  // enter number mode
    NUM_MDT, // number mode toggle
    JPTR_MDT,// jupyter mode toggle, makes vim stuffs use 'enter' instead of shift 'enter'
    APPEND,  // append to insert mode (right arrow then regular mode)
    VIENTER, // vim enter (after pressing enter will go back to regular mode)

    M_UNDO,  // control + z
    NEXT_WD, // move to next word ('w' in vim)
    BACK_WD, // move back to the last word ('b' in vim)
    VI_RSHFT,// vim right shift
    VI_LSHFT,// vim left shift

    VIM_D,   // vim d command (activates the delete commands)
    VIM_Y,   // vim y command (activates the yank commands)
    DY_N_WD, // delete/yank next word ('dw' in vim)
    DY_B_WD, // delete/yank back (last) word ('db' in vim)
    DY_END,  // delete/yank until the end of line
    DY_HOME, // delete/yank until the beginning of line
    D_LINE,  // delete current line
    Y_LINE,  // yank current line
    DY_____,// Unset the DY mode (restore old rgb and mode values & OSL)

    NEW_LN,  // add a new line
    V_PASTE, // paste in vim

    PRE_WHI, // turn on rgb preset
    PRE_DRA, // turn on rgb preset
    PRE_RED, // turn on rgb preset
    PRE_FRC, // turn on rgb preset
    PRE_HMP, // turn on rgb heat map preset
};

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
    case QMKBEST:
        if (record->event.pressed) { // on press
            SEND_STRING("QMK is the best thing ever!");
        } else { // on release:
        }
        break;
    case VDKTP_R:
        if (record->event.pressed) { // on press
            SEND_STRING(SS_LCTL(SS_LGUI(SS_TAP(X_RGHT))));
        } else { // on release:
        }
        break;
    case VDKTP_L:
        if (record->event.pressed) { // on press
            SEND_STRING(SS_LCTL(SS_LGUI(SS_TAP(X_LEFT))));
        } else { // on release:
        }
        break;
    case TAB_R:
        if (record->event.pressed) { // on press
            SEND_STRING(SS_LCTL(SS_TAP(X_TAB)));
        } else { // on release:
        }
        break;
    case TAB_L:
        if (record->event.pressed) { // on press
            SEND_STRING(SS_LCTL(SS_LSFT(SS_TAP(X_TAB))));
        } else { // on release:
        }
        break;
    case M_UNDO:
        if (record->event.pressed) { // on press
            SEND_STRING(SS_LCTL("z"));
        } else { // on release:
        }
        break;
	// vim/regular modes ------------------------------------------------
    case VIM_MD:
        if (record->event.pressed) { // on press
            save_rgb(&vim_mode_prev_rgb); // backup the current rgb settings
            set_rgb_preset(VIM_RGB); // set the vim rgb
        } else { // on release:
            vim_mode_on();
        }
        break;
    case VIM_RST:
        if (record->event.pressed) { // on press
            set_rgb_preset(VIM_RGB); // set the vim rgb
        } else { // on release:
            vim_mode_on();
        }
        break;
    case REG_MD:
        if (record->event.pressed) { // on press
            load_rgb(&vim_mode_prev_rgb);
        } else { // on release:
            //change layer
            regular_mode_on();
        }
        break;
    case NUM_MD:
        if (record->event.pressed) { // on press
            save_rgb(&vim_mode_prev_rgb); // backup the current rgb settings
            set_rgb_preset(CYAN_RGB); // set the num rgb
        } else { // on release:
            num_mode_on();
        }
        break;
    case NUM_MDT:
        if (record->event.pressed) { // on press
            if (KB_mode == Num_Mode) {
                load_rgb(&num_mode_prev_rgb);
            } else if (KB_mode == Regular_Mode) {
                save_rgb(&num_mode_prev_rgb); // backup the current rgb settings
                set_rgb_preset(CYAN_RGB); // set the num rgb
            }
        } else { // on release:
            if (KB_mode == Num_Mode) {
                regular_mode_on();
            } else if (KB_mode == Regular_Mode) {
                num_mode_on();
            }
        }
        break;
    case JPTR_MDT:
        if (record->event.pressed) { // on press
            if (!JPTR_Mode_On) { // regular mode
                save_rgb(&jptr_mode_prev_rgb); // backup the current rgb settings
                set_rgb_preset(GOLD_RGB); // set the num rgb
            } else { // jptr mode
                load_rgb(&jptr_mode_prev_rgb); // backup the current rgb settings
            }
        } else { // on release:
            JPTR_Mode_On = !JPTR_Mode_On;
        }
        break;
    case APPEND:
        if (record->event.pressed) { // on press
            load_rgb(&vim_mode_prev_rgb);
            if (vim_shift()) {
                SEND_STRING(SS_TAP(X_END));
            } else {
                SEND_STRING(SS_TAP(X_RGHT));
            }
        } else { // on release:
            //change layer
            regular_mode_on();
        }
        break;
    case VIENTER:
        if (record->event.pressed) { // on press
            load_rgb(&vim_mode_prev_rgb);
            SEND_STRING(SS_TAP(X_ENTER));
        } else { // on release:
            //change layer
            regular_mode_on();
        }
        break;
    case VI_RSHFT:
        if (record->event.pressed) { // on press
            VIM_rshift = true;
        } else { // on release:
            VIM_rshift = false;
        }
        break;
    case VI_LSHFT:
        if (record->event.pressed) { // on press
            VIM_lshift = true;
        } else { // on release:
            VIM_lshift = false;
        }
        break;
    // basic vim commands ------------------------------------------------
    case NEXT_WD:
        if (record->event.pressed) { // on press
            SEND_STRING(SS_LCTL(SS_TAP(X_RGHT)));
        } else { // on release:
        }
        break;
    case BACK_WD:
        if (record->event.pressed) { // on press
            SEND_STRING(SS_LCTL(SS_TAP(X_LEFT)));
        } else { // on release:
        }
        break;
    // vim delete/yank commands: -----------------------------------------
    case VIM_D:
        if (record->event.pressed) { // on press
            // rgb settings
            save_rgb(&dy_mode_prev_rgb);
            set_rgb_preset(VIM_D_RGB);

            // turn mode on
            dy_vim_mode_on(D_Mode);
        } else { // on release:
        }
        break;
    case VIM_Y:
        if (record->event.pressed) { // on press
            // rgb settings
            save_rgb(&dy_mode_prev_rgb);
            set_rgb_preset(VIM_Y_RGB);

            // turn mode on
            dy_vim_mode_on(Y_Mode);
        } else { // on release:
        }
        break;
    // vim delete/yank commands: -----------------------------------------
    case DY_N_WD:
        if (record->event.pressed) { // on press
            if (KB_mode  == D_Mode) {
                SEND_STRING(SS_LCTL(SS_LSFT(SS_TAP(X_RIGHT))) SS_LCTL("x"));
            } else if (KB_mode == Y_Mode) {
                SEND_STRING(SS_LCTL(SS_LSFT(SS_TAP(X_RIGHT))) SS_LCTL("c") SS_TAP(X_LEFT));
            }
            clipboard_holds_line = false;
            load_rgb(&dy_mode_prev_rgb);
        } else { // on release:
            dy_vim_mode_off();
        }
        break;
    case DY_B_WD:
        if (record->event.pressed) { // on press
            if (KB_mode  == D_Mode) {
                SEND_STRING(SS_LCTL(SS_LSFT(SS_TAP(X_LEFT))) SS_LCTL("x"));
            } else if (KB_mode == Y_Mode) {
                SEND_STRING(SS_LCTL(SS_LSFT(SS_TAP(X_LEFT))) SS_LCTL("c") SS_TAP(X_RIGHT));
            }
            clipboard_holds_line = false;
            load_rgb(&dy_mode_prev_rgb);
        } else { // on release:
            dy_vim_mode_off();
        }
        break;
    case DY_END:
        if (record->event.pressed) { // on press
            if (KB_mode  == D_Mode) {
                SEND_STRING(SS_LSFT(SS_TAP(X_END)) SS_LCTL("x"));
            } else if (KB_mode == Y_Mode) {
                SEND_STRING(SS_LSFT(SS_TAP(X_END)) SS_LCTL("c") SS_TAP(X_LEFT));
            }
            clipboard_holds_line = false;
            load_rgb(&dy_mode_prev_rgb);
        } else { // on release:
            dy_vim_mode_off();
        }
        break;
    case DY_HOME:
        if (record->event.pressed) { // on press
            if (KB_mode  == D_Mode) {
                SEND_STRING(SS_LSFT(SS_TAP(X_HOME)) SS_LCTL("x"));
            } else if (KB_mode == Y_Mode) {
                SEND_STRING(SS_LSFT(SS_TAP(X_HOME)) SS_LCTL("c") SS_TAP(X_RIGHT));
            }
            clipboard_holds_line = false;
            load_rgb(&dy_mode_prev_rgb);
        } else { // on release:
            dy_vim_mode_off();
        }
        break;
    case D_LINE:
        if (record->event.pressed) { // on press
            if (KB_mode  == D_Mode) {
                SEND_STRING(SS_TAP(X_HOME) SS_LSFT(SS_TAP(X_END)) SS_LCTL("x") SS_LSFT(SS_TAP(X_DOWN) SS_TAP(X_HOME)) SS_TAP(X_BSPC));
                clipboard_holds_line = true;
            }
            load_rgb(&dy_mode_prev_rgb);
            dy_vim_mode_off();
        } else { // on release:
        }
        break;
    case Y_LINE:
        if (record->event.pressed) { // on press
            if (KB_mode  == Y_Mode) {
                SEND_STRING(SS_TAP(X_HOME) SS_LSFT(SS_TAP(X_END)) SS_LCTL("c") SS_TAP(X_LEFT));
                clipboard_holds_line = true;
            }
            load_rgb(&dy_mode_prev_rgb);
            dy_vim_mode_off();
        } else { // on release:
        }
        break;
    case DY_____:
        if (record->event.pressed) { // on press
            load_rgb(&dy_mode_prev_rgb);
        } else { // on release:
            dy_vim_mode_off();
        }
        break;
	// shift dependent commands -----------------------------------------------
    case NEW_LN:
        if (record->event.pressed) { // on press
            if (vim_shift()) {
                if (JPTR_Mode_On) {
                    SEND_STRING(SS_TAP(X_HOME) SS_TAP(X_ENTER) SS_TAP(X_UP));
                } else {
                    SEND_STRING(SS_TAP(X_HOME) SS_LSFT(SS_TAP(X_ENTER)) SS_TAP(X_UP));
                }
            } else {
                if (JPTR_Mode_On) {
                    SEND_STRING(SS_TAP(X_END) SS_TAP(X_ENTER));
                } else {
                    SEND_STRING(SS_TAP(X_END) SS_LSFT(SS_TAP(X_ENTER)));
                }
            }
        } else { // on release:
        }
        break;
    case V_PASTE:
        if (record->event.pressed) { // on press
            if (clipboard_holds_line) {
                if (vim_shift()) {
                    if (JPTR_Mode_On) {
                        SEND_STRING(SS_TAP(X_HOME) SS_TAP(X_ENTER) SS_TAP(X_UP) SS_LCTL("v"));
                    } else {
                        SEND_STRING(SS_TAP(X_HOME) SS_LSFT(SS_TAP(X_ENTER)) SS_TAP(X_UP) SS_LCTL("v"));
                    }
                } else {
                    if (JPTR_Mode_On) {
                        SEND_STRING(SS_TAP(X_END) SS_TAP(X_ENTER) SS_LCTL("v"));
                    } else {
                        SEND_STRING(SS_TAP(X_END) SS_LSFT(SS_TAP(X_ENTER)) SS_LCTL("v"));
                    }
                }
            } else { // regular paste
                if (vim_shift()) {
                    SEND_STRING(SS_LCTL("v"));
                } else {
                    SEND_STRING(SS_TAP(X_RIGHT) SS_LCTL("v"));
                }
            }
        } else { // on release:
        }
        break;
    case PRE_WHI:
        if (record->event.pressed) { // on press
            set_rgb_preset(WHITE_RGB);
        } else { // on release:
        }
        break;
    case PRE_DRA:
        if (record->event.pressed) { // on press
            set_rgb_preset(DRACULA_RGB);
        } else { // on release:
        }
        break;
    case PRE_RED:
        if (record->event.pressed) { // on press
            set_rgb_preset(RED_RGB);
        } else { // on release:
        }
        break;
    case PRE_FRC:
        if (record->event.pressed) { // on press
            set_rgb_preset(FRC_RGB);
        } else { // on release:
        }
        break;
    case PRE_HMP:
        if (record->event.pressed) { // on press
            set_rgb_preset(H_MAP_RGB);
        } else { // on release:
        }
        break;
    }
    return true;
};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
      [0] = LAYOUT_tkl_ansi_tsangan(
        KC_ESC,  KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,   KC_F6,   KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_F11,  KC_F12,                      KC_PSCR, KC_SLCK, KC_MPLY,
        KC_GRV,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,    KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_MINS, KC_EQL,  KC_BSPC,            KC_INS,  KC_HOME, KC_VOLU,
        KC_TAB,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,    KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_LBRC, KC_RBRC, KC_BSLS,            KC_DEL,  KC_END,  KC_VOLD,
        KC_ESC,  KC_A,    KC_S,    KC_D,    KC_F,    KC_G,    KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN, KC_QUOT,          KC_ENT,
        KC_LSFT,          KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,    KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH, KC_RSFT,                              KC_UP,
        KC_LCTL, KC_LGUI, KC_LALT,                   KC_SPC,                                      MO(_FL),          MO(_FL), MO(_CL),            KC_LEFT, KC_DOWN, KC_RGHT),

      [1] = LAYOUT_tkl_ansi_tsangan( // num layer
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,                     KC_7, KC_8, KC_9,
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,            KC_4, KC_5, KC_6,
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,            KC_1, KC_2, KC_3,
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,          _______,
        _______,          _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,                              _______,
        _______, _______, _______,                   _______,                                     _______,          KC_DOT,  KC_0,               _______, _______, _______),

      [2] = LAYOUT_tkl_ansi_tsangan( // function layer
        RESET,   PRE_HMP, PRE_WHI, PRE_DRA, PRE_RED, PRE_FRC, _______, _______, _______, _______, _______, _______, _______,                     _______, _______, _______,
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, KC_HOME, _______, _______, _______,            RGB_TOG, RGB_HUI, RGB_SAI,
        KC_ENT,  _______, NEXT_WD, _______, _______, _______, VIM_Y,   M_UNDO,  _______, NEW_LN,  V_PASTE, _______, _______, _______,            RGB_MOD, RGB_HUD, RGB_SAD,
        VIM_MD,  KC_END,  _______, VIM_D,   _______, _______, KC_LEFT, KC_DOWN, KC_UP,   KC_RGHT, _______, _______,          JPTR_MDT,
        VI_LSHFT,         KC_BSPC, KC_DEL,  _______, _______, BACK_WD, _______, _______, TAB_L,   TAB_R,   _______, VI_RSHFT,                             RGB_VAI,
        _______, _______, _______,                   _______,                                     _______,          _______, NUM_MDT,            RGB_RMOD,RGB_VAD, RGB_MOD),

      [3] = LAYOUT_tkl_ansi_tsangan( // control layer (2nd function layer)
        RESET,   _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,                     _______, _______, KC_MUTE,
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,            _______, _______, KC_MNXT,
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,            KC_SLEP, _______, KC_MPRV,
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,          JPTR_MDT,
        _______,          _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,                              KC_PGUP,
        _______, _______, _______,                   _______,                                     XXXXXXX,          XXXXXXX, _______,            VDKTP_L, KC_PGDN, VDKTP_R),

      [4] = LAYOUT_tkl_ansi_tsangan( // vim mode layer
        REG_MD,  _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,                     _______, _______, _______,
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, KC_HOME, _______, _______, _______,            _______, _______, _______,
        _______, _______, NEXT_WD, _______, _______, _______, VIM_Y,   M_UNDO,  REG_MD,  NEW_LN,  V_PASTE, _______, _______, _______,            _______, _______, _______,
        VIM_RST, APPEND,  _______, VIM_D,   _______, _______, KC_LEFT, KC_DOWN, KC_UP,   KC_RGHT, _______, _______,          _______,
        VI_LSHFT,         _______, KC_DEL,  _______, _______, BACK_WD, _______, _______, _______, _______, _______, VI_RSHFT,                             _______,
        _______, _______, _______,                   _______,                                     _______,          _______, _______,            _______, _______, _______),

      [5] = LAYOUT_tkl_ansi_tsangan( // delete / yank mode layer
        DY_____, DY_____, DY_____, DY_____, DY_____, DY_____, DY_____, DY_____, DY_____, DY_____, DY_____, DY_____, DY_____,                     DY_____, DY_____, DY_____,
        DY_____, DY_____, DY_____, DY_____, DY_END,  DY_____, DY_____, DY_____, DY_____, DY_____, DY_HOME, DY_____, DY_____, DY_____,            DY_____, DY_____, DY_____,
        DY_____, DY_____, DY_N_WD, DY_____, DY_____, DY_____, Y_LINE,  DY_____, DY_____, DY_____, DY_____, DY_____, DY_____, DY_____,            DY_____, DY_____, DY_____,
        DY_____, DY_____, DY_____, D_LINE,  DY_____, DY_____, DY_____, DY_____, DY_____, DY_____, DY_____, DY_____,          DY_____,
        XXXXXXX,          DY_____, DY_____, DY_____, DY_____, DY_B_WD, DY_____, DY_____, DY_____, DY_____, DY_____, XXXXXXX,                             DY_____,
        DY_____, DY_____, DY_____,                   DY_____,                                     DY_____,          DY_____, DY_____,           DY_____, DY_____, DY_____)
};
