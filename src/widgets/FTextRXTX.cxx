// ----------------------------------------------------------------------------
//      FTextRXTX.cxx
//
// Copyright (C) 2007-2010
//              Stelios Bounanos, M0GLD
//
// Copyright (C) 2008-2010
//              Dave Freese, W1HKJ
//
// This file is part of fldigi.
//
// fldigi is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// fldigi is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
// ----------------------------------------------------------------------------

#include <config.h>

#include <string>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <sys/stat.h>
#include <map>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iomanip>

#include <FL/Fl_Window.H>
#include <FL/Fl_Tooltip.H>

#include "FTextRXTX.h"
#include "FTextView.h"

#include "fileselect.h"
#include "icons.h"

//#include "re.h"
//#include "strutil.h"

#include "gettext.h"

#include "debug.h"

#include "timeops.h"

using namespace std;

Fl_Menu_Item FTextRX::menu[] = {
	{ 0 }, // VIEW_MENU_COPY
	{ 0 }, // VIEW_MENU_CLEAR
	{ 0 }, // VIEW_MENU_SELECT_ALL
	{ 0 }, // VIEW_MENU_SAVE
	{ 0 }, // VIEW_MENU_WRAP
	{ 0 }
};

/// FTextRX constructor.
/// We remove \c Fl_Text_Display_mod::buffer_modified_cb from the list of callbacks
/// because we want to scroll depending on the visibility of the last line; @see
/// changed_cb.
/// @param x
/// @param y
/// @param w
/// @param h
/// @param l
FTextRX::FTextRX(int x, int y, int w, int h, const char *l)
        : FTextView(x, y, w, h, l)
{
	memcpy(menu + RX_MENU_COPY, FTextView::menu, (FTextView::menu->size() - 1) * sizeof(*FTextView::menu));
	context_menu = menu;
	init_context_menu();
}

FTextRX::~FTextRX()
{
}

/// Handles fltk events for this widget.

/// We only care about mouse presses (to display the popup menu and prevent
/// pasting) and keyboard events (to make sure no text can be inserted).
/// Everything else is passed to the base class handle().
///
/// @param event
///
/// @return
///
int FTextRX::handle(int event)
{
	static Fl_Cursor cursor;

	switch (event) {
	case FL_PUSH:
		if (!Fl::event_inside(this))
			break;
 		switch (Fl::event_button()) {
		case FL_LEFT_MOUSE:
			goto out;
		case FL_MIDDLE_MOUSE:
			goto out;
 		case FL_RIGHT_MOUSE:
			handle_context_menu();
			return 1;
 		default:
 			goto out;
 		}
		break;
	case FL_DRAG:
		if (Fl::event_button() != FL_LEFT_MOUSE)
			return 1;
		break;
	case FL_RELEASE:
		{	int eb = Fl::event_button();
			if (cursor == FL_CURSOR_HAND && eb == FL_LEFT_MOUSE &&
			    Fl::event_is_click() && !Fl::event_clicks()) {
				handle_clickable(Fl::event_x() - x(), Fl::event_y() - y());
				return 1;
			}
			break;
		}
	case FL_MOVE: {
		int p = xy_to_position(Fl::event_x(), Fl::event_y(), Fl_Text_Display_mod::CURSOR_POS);
#if FLWKEY_FLTK_API_MAJOR == 1 && FLWKEY_FLTK_API_MINOR == 3
		if (sbuf->char_at(p) >= CLICK_START + FTEXT_DEF) {
#else
		if (sbuf->character(p) >= CLICK_START + FTEXT_DEF) {
#endif
			if (cursor != FL_CURSOR_HAND)
				window()->cursor(cursor = FL_CURSOR_HAND);
			return 1;
		}
		else
			cursor = FL_CURSOR_INSERT;
		break;
	}
	// catch some text-modifying events that are not handled by kf_* functions
	case FL_KEYBOARD:
		break;
	case FL_ENTER:
		break;
	case FL_LEAVE:
		break;
	}
out:
	return FTextView::handle(event);
}

/// Adds a char to the buffer
///
/// @param c The character
/// @param attr The attribute (@see enum text_attr_e); RECV if omitted.
///
void FTextRX::add(unsigned char c, int attr)
{
	if (c == '\r')
		return;

	// The user may have moved the cursor by selecting text or
	// scrolling. Place it at the end of the buffer.
	if (mCursorPos != tbuf->length())
		insert_position(tbuf->length());

	switch (c) {
	case '\b':
		// we don't call kf_backspace because it kills selected text
		tbuf->remove(tbuf->length() - 1, tbuf->length());
		sbuf->remove(sbuf->length() - 1, sbuf->length());
		break;
	case '\n':
		// maintain the scrollback limit, if we have one
		if (max_lines > 0 && tbuf->count_lines(0, tbuf->length()) >= max_lines) {
			int le = tbuf->line_end(0) + 1; // plus 1 for the newline
			tbuf->remove(0, le);
			sbuf->remove(0, le);
		}
		// fall-through
	default:
		char s[] = { '\0', '\0', static_cast<char>(FTEXT_DEF + attr), '\0' };
		const char *cp;

		if ((c < ' ' || c == 127) && attr != CTRL) // look it up
			cp = "";//cp = ascii[(unsigned char)c];
		else { // insert verbatim
			s[0] = c;
			cp = &s[0];
		}

		for (int i = 0; cp[i]; ++i)
			sbuf->append(s + 2);
		insert(cp);
		break;
	}
}

void FTextRX::mark(FTextBase::TEXT_ATTR attr)
{
	if (attr == NATTR)
		attr = CLICK_START;
}

void FTextRX::clear(void)
{
	FTextBase::clear();
}

void FTextRX::setFont(Fl_Font f, int attr)
{
	FTextBase::setFont(f, attr);
}

void FTextRX::handle_clickable(int x, int y)
{
	int pos;
	size_t style;

	pos = xy_to_position(x + this->x(), y + this->y(), CURSOR_POS);
	// return unless clickable style
#if FLWKEY_FLTK_API_MAJOR == 1 && FLWKEY_FLTK_API_MINOR == 3
	if ((style = sbuf->char_at(pos)) < CLICK_START + FTEXT_DEF)
#else
	if ((style = sbuf->character(pos)) < CLICK_START + FTEXT_DEF)
#endif
		return;

	int start, end;
	for (start = pos-1; start >= 0; start--)
#if FLWKEY_FLTK_API_MAJOR == 1 && FLWKEY_FLTK_API_MINOR == 3
		if (sbuf->char_at(start) != style)
#else
		if (sbuf->character(start) != style)
#endif
			break;
	start++;
	int len = sbuf->length();
	for (end = pos+1; end < len; end++)
#if FLWKEY_FLTK_API_MAJOR == 1 && FLWKEY_FLTK_API_MINOR == 3
		if (sbuf->char_at(end) != style)
#else
		if (sbuf->character(end) != style)
#endif
			break;
}

void FTextRX::handle_context_menu(void)
{
// availability of editing items depend on buffer state
	icons::set_active(&menu[RX_MENU_COPY], tbuf->selected());
	icons::set_active(&menu[RX_MENU_CLEAR], tbuf->length());
	icons::set_active(&menu[RX_MENU_SELECT_ALL], tbuf->length());
	icons::set_active(&menu[RX_MENU_SAVE], tbuf->length());

	if (wrap)
		menu[RX_MENU_WRAP].set();
	else
		menu[RX_MENU_WRAP].clear();

	show_context_menu();
}

/// The context menu handler
///
/// @param val
///
void FTextRX::menu_cb(size_t item)
{
	switch (item) {
	case RX_MENU_COPY:
		kf_copy(Fl::event_key(), this);
		break;
	case RX_MENU_CLEAR:
		clear();
		break;
	case RX_MENU_SELECT_ALL:
		tbuf->select(0, tbuf->length());
		break;
	case RX_MENU_SAVE:
		saveFile();
		break;
	case RX_MENU_WRAP:
		set_word_wrap(!wrap);
		break;
	}
}

// ----------------------------------------------------------------------------

Fl_Menu_Item FTextTX::menu[] = {
	{ 0 }, // EDIT_MENU_CUT
	{ 0 }, // EDIT_MENU_COPY
	{ 0 }, // EDIT_MENU_PASTE
	{ 0 }, // EDIT_MENU_CLEAR
	{ 0 }, // EDIT_MENU_READ
	{ 0 }, // EDIT_MENU_WRAP
	{ 0 }
};

// needed by our static kf functions, which may restrict editing depending on
// the transmit cursor position
int *FTextTX::ptxpos;

FTextTX::FTextTX(int x, int y, int w, int h, const char *l)
	: FTextEdit(x, y, w, h, l),
          PauseBreak(false), txpos(0), bkspaces(0)
{
	ptxpos = &txpos;

	change_keybindings();

	memcpy(menu + TX_MENU_CUT, FTextEdit::menu, (FTextEdit::menu->size() - 1) * sizeof(*FTextEdit::menu));
	context_menu = menu;
	init_context_menu();
}

/// Handles fltk events for this widget.
/// We pass keyboard events to handle_key() and handle mouse3 presses to show
/// the popup menu. We also disallow mouse2 events in the transmitted text area.
/// Everything else is passed to the base class handle().
///
/// @param event
///
/// @return
///
int FTextTX::handle(int event)
{
	if ( !(Fl::event_inside(this) || (event == FL_KEYBOARD && Fl::focus() == this)) )
		return FTextEdit::handle(event);

	switch (event) {
	case FL_KEYBOARD:
		return handle_key(Fl::event_key()) ? 1 : FTextEdit::handle(event);
	case FL_PUSH:
		if (Fl::event_button() == FL_MIDDLE_MOUSE &&
		    xy_to_position(Fl::event_x(), Fl::event_y(), CHARACTER_POS) < txpos)
			return 1; // ignore mouse2 text pastes inside the transmitted text
	}

	return FTextEdit::handle(event);
}

/// Clears the buffer.
/// Also resets the transmit position, stored backspaces and tx pause flag.
///
void FTextTX::clear(void)
{
	FTextEdit::clear();
	txpos = 0;
	bkspaces = 0;
	PauseBreak = false;
}

/// Clears the sent text.
/// Also resets the transmit position, stored backspaces and tx pause flag.
///
void FTextTX::clear_sent(void)
{
	tbuf->remove(0, txpos);
	txpos = 0;
	bkspaces = 0;
	PauseBreak = false;
}

/// Returns boolean <eot> end of text
///
/// true if empty buffer
/// false if characters remain
///
bool FTextTX::eot(void)
{
	return (insert_position() == txpos);
}

/// Returns the next character to be transmitted.
///
/// @return The next character, or ETX if the transmission has been paused, or
/// NUL if no text should be transmitted.
///
int FTextTX::nextChar(void)
{
	int c;
	if (insert_position() <= txpos) // empty buffer or cursor inside transmitted text
		c = -1;
	else {
#if FLWKEY_FLTK_API_MAJOR == 1 && FLWKEY_FLTK_API_MINOR == 3
		if ((c = static_cast<unsigned char>(tbuf->char_at(txpos)))) {
#else
		if ((c = static_cast<unsigned char>(tbuf->character(txpos)))) {
#endif
			++txpos;
			changed_cb(txpos, 0, 0,-1, static_cast<const char *>(0), this);
		}
	}

	return c;
}

void FTextTX::setFont(Fl_Font f, int attr)
{
	FTextBase::setFont(f, attr);
}

/// Handles keyboard events to override Fl_Text_Editor_mod's handling of some
/// keystrokes.
///
/// @param key
///
/// @return
///
int FTextTX::handle_key(int key)
{
	switch (key) {
	case FL_Escape: // set stop flag and clear
	{
		static time_t t[2] = { 0, 0 };
		static unsigned char i = 0;
		if (t[i] == time(&t[!i])) { // two presses in a second: abort transmission
			t[i = !i] = 0;
			return 1;
		}
		i = !i;
	}
		clear();
		return 1;
	case FL_Tab:
		return 1;
	case FL_BackSpace:
		return 0;
	default:
		break;
	}

	if (insert_position() < txpos)
		return 1;

	return 0;
}

int FTextTX::handle_dnd_drag(int pos)
{
	if (pos >= txpos) {
		return FTextEdit::handle_dnd_drag(pos);
	}
	else // refuse drop inside transmitted text
		return 0;
}

/// Handles mouse-3 clicks by displaying the context menu
///
/// @param val
///
void FTextTX::handle_context_menu(void)
{
	bool modify_text_ok = insert_position() >= txpos;
	bool selected = tbuf->selected();
	icons::set_active(&menu[TX_MENU_CLEAR], tbuf->length());
	icons::set_active(&menu[TX_MENU_CUT], selected && modify_text_ok);
	icons::set_active(&menu[TX_MENU_COPY], selected);
	icons::set_active(&menu[TX_MENU_PASTE], modify_text_ok);
	icons::set_active(&menu[TX_MENU_READ], modify_text_ok);

	if (wrap)
		menu[TX_MENU_WRAP].set();
	else
		menu[TX_MENU_WRAP].clear();

	show_context_menu();
}

/// The context menu handler
///
/// @param val
///
void FTextTX::menu_cb(size_t item)
{
  	switch (item) {
	case TX_MENU_CLEAR:
		clear();
		break;
	case TX_MENU_CUT:
		kf_cut(0, this);
		break;
	case TX_MENU_COPY:
		kf_copy(0, this);
		break;
	case TX_MENU_PASTE:
		kf_paste(0, this);
		break;
	case TX_MENU_READ: {
		readFile();
		break;
	}
	case TX_MENU_WRAP:
		set_word_wrap(!wrap);
		break;
	}
}

/// Overrides some useful Fl_Text_Edit keybindings that we want to keep working,
/// provided that they don't try to change chunks of transmitted text.
///
void FTextTX::change_keybindings(void)
{
	struct {
		Fl_Text_Editor_mod::Key_Func function, override;
	} fbind[] = {
		{ Fl_Text_Editor_mod::kf_default, FTextTX::kf_default },
		{ Fl_Text_Editor_mod::kf_enter,   FTextTX::kf_enter   },
		{ Fl_Text_Editor_mod::kf_delete,  FTextTX::kf_delete  },
		{ Fl_Text_Editor_mod::kf_cut,     FTextTX::kf_cut     },
		{ Fl_Text_Editor_mod::kf_paste,   FTextTX::kf_paste   }
	};
	int n = sizeof(fbind) / sizeof(fbind[0]);

	// walk the keybindings linked list and replace items containing
	// functions for which we have an override in fbind
	for (Fl_Text_Editor_mod::Key_Binding *k = key_bindings; k; k = k->next) {
		for (int i = 0; i < n; i++)
			if (fbind[i].function == k->function)
				k->function = fbind[i].override;
	}
}

// The kf_* functions below call the corresponding Fl_Text_Editor_mod routines, but
// may make adjustments so that no transmitted text is modified.

int FTextTX::kf_default(int c, Fl_Text_Editor_mod* e)
{
	return e->insert_position() < *ptxpos ? 1 : Fl_Text_Editor_mod::kf_default(c, e);
}

int FTextTX::kf_enter(int c, Fl_Text_Editor_mod* e)
{
	return e->insert_position() < *ptxpos ? 1 : Fl_Text_Editor_mod::kf_enter(c, e);
}

int FTextTX::kf_delete(int c, Fl_Text_Editor_mod* e)
{
	// single character
	if (!e->buffer()->selected()) {
                if (e->insert_position() >= *ptxpos &&
                    e->insert_position() != e->buffer()->length())
                        return Fl_Text_Editor_mod::kf_delete(c, e);
                else
                        return 1;
        }

	// region: delete as much as we can
	int start, end;
	e->buffer()->selection_position(&start, &end);
	if (*ptxpos >= end)
		return 1;
	if (*ptxpos > start)
		e->buffer()->select(*ptxpos, end);

	return Fl_Text_Editor_mod::kf_delete(c, e);
}

int FTextTX::kf_cut(int c, Fl_Text_Editor_mod* e)
{
	if (e->buffer()->selected()) {
		int start, end;
		e->buffer()->selection_position(&start, &end);
		if (*ptxpos >= end)
			return 1;
		if (*ptxpos > start)
			e->buffer()->select(*ptxpos, end);
	}

	return Fl_Text_Editor_mod::kf_cut(c, e);
}

int FTextTX::kf_paste(int c, Fl_Text_Editor_mod* e)
{
	return e->insert_position() < *ptxpos ? 1 : Fl_Text_Editor_mod::kf_paste(c, e);
}
