#ifndef SUPPORT_H
#define SUPPORT_H

#include <fstream>
#include <vector>
#include <string>

#include <math.h>
#ifndef WIN32
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#endif

#include "flwkey.h"
#include "wkey_io.h"
#include "serial.h"
#include "status.h"

#include <FL/fl_show_colormap.H>
#include <FL/fl_ask.H>

extern Cserial WKEY_serial;

extern bool bypass_serial_thread_loop;

extern void * serial_thread_loop(void *);
extern void cb_events();
extern void cbExit();
extern int  main_handler(int);

extern void config_comm_port();
extern void cbCancelSetupDialog();
extern void cbOkSetupDialog();

extern void open_wkeyer();

extern void cb_clear_text_to_send();
extern void send_char(void *);
extern void cb_cancel_transmit();
extern void cb_transmit_text();
extern void cb_send_button();
extern void set_wpm();
extern void use_pot_changed();
extern void load_defaults();
extern void check_call();

extern void config_parameters();
extern void config_messages();
extern void done_parameters();

extern void update_msg_labels();
extern void apply_edit();
extern void done_edit();
extern void cancel_edit();

extern void change_choice_keyer_mode();
extern void change_choice_output_pins();
extern void change_choice_sidetone();
extern void change_choice_hang();
extern void change_cntr_tail();
extern void change_cntr_leadin();
extern void change_cntr_weight();
extern void change_cntr_sample();
extern void change_cntr_first_ext();
extern void change_cntr_comp();
extern void change_cntr_ratio();
extern void change_cntr_cmd_wpm();
extern void change_cntr_farnsworth();
extern void change_cntr_rng_wpm();
extern void change_cntr_min_wpm();
extern void change_btn_sidetone_on();
extern void change_btn_tone_on();
extern void change_btn_ptt_on();
extern void change_btn_cut_zeronine();
extern void change_btn_paddledog();
extern void change_btn_ct_space();
extern void change_btn_auto_space();
extern void change_btn_swap();
extern void change_btn_paddle_echo();
extern void change_btn_serial_echo();

extern void cb_tune();

extern void exec_msg1();
extern void exec_msg2();
extern void exec_msg3();
extern void exec_msg4();
extern void exec_msg5();
extern void exec_msg6();
extern void exec_msg7();
extern void exec_msg8();
extern void exec_msg9();
extern void exec_msg10();
extern void exec_msg11();
extern void exec_msg12();

extern void open_operator_dialog();
extern void cb_done_op_dialog();
extern void change_txt_cll();
extern void change_txt_qth();
extern void change_txt_loc();
extern void change_txt_opr();

extern void cb_mnuNewLogbook();
extern void cb_mnuOpenLogbook();
extern void cb_mnuSaveLogbook();
extern void cb_mnuMergeADIF_log();
extern void cb_mnuExportADIF_log();
extern void cb_mnuExportTEXT_log();
extern void cb_mnuExportCSV_log();
extern void cb_Export_Cabrillo();
extern void cb_mnuShowLogbook();

extern void serial_nbr();
extern void zeros();
extern void dups();
extern void time_span();

extern void AddRecord();
extern void about();
extern void on_line_help();

extern void cb_contest();
extern void close_contest();

#endif
