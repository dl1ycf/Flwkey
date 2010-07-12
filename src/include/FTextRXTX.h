// ----------------------------------------------------------------------------
//      FTextView.h
//
// Copyright (C) 2007-2010
//              Stelios Bounanos, M0GLD
// Copyright (C) 2007-2010
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

#ifndef FTextRXTX_H_
#define FTextRXTX_H_

#include "FTextView.h"

///
/// A TextBase subclass to display received & transmitted text
///
class FTextRX : public FTextView
{
public:
	FTextRX(int x, int y, int w, int h, const char *l = 0);
        ~FTextRX();

	virtual int	handle(int event);

	virtual void	add(unsigned char c, int attr = RECV);
	virtual	void	add(const char *s, int attr = RECV)
        {
                while (*s)
                        add(*s++, attr);
        }

	void		mark(FTextBase::TEXT_ATTR attr = CLICK_START);
	void		clear(void);

	void		setFont(Fl_Font f, int attr = NATTR);

protected:
	enum {
		RX_MENU_COPY, RX_MENU_CLEAR, RX_MENU_SELECT_ALL,
		RX_MENU_SAVE, RX_MENU_WRAP
	};

	void		handle_clickable(int x, int y);
	void		handle_context_menu(void);
	void		menu_cb(size_t item);

private:
	FTextRX();
	FTextRX(const FTextRX &t);

protected:
	static Fl_Menu_Item menu[];
	struct {
		bool enabled;
		float delay;
	} tooltips;
};


///
/// A FTextBase subclass to display and edit text to be transmitted
///
class FTextTX : public FTextEdit
{
public:
	FTextTX(int x, int y, int w, int h, const char *l = 0);

	virtual int	handle(int event);

	void		clear(void);
	void		clear_sent(void);
	int			nextChar(void);
	bool		eot(void);

	void		setFont(Fl_Font f, int attr = NATTR);

protected:
	enum { TX_MENU_CUT, TX_MENU_COPY, TX_MENU_PASTE, TX_MENU_CLEAR, TX_MENU_READ,
	       TX_MENU_WRAP
	};
	int		handle_key(int key);
	int		handle_dnd_drag(int pos);
	void		handle_context_menu(void);
	void		menu_cb(size_t item);
	void		change_keybindings(void);
	static int	kf_default(int c, Fl_Text_Editor_mod* e);
	static int	kf_enter(int c, Fl_Text_Editor_mod* e);
	static int	kf_delete(int c, Fl_Text_Editor_mod* e);
	static int	kf_cut(int c, Fl_Text_Editor_mod* e);
	static int	kf_paste(int c, Fl_Text_Editor_mod* e);

private:
	FTextTX();
	FTextTX(const FTextTX &t);

protected:
	static Fl_Menu_Item	menu[];
	bool			PauseBreak;
	int			txpos;
	static int		*ptxpos;
	int			bkspaces;
};

#endif // FTextRXTX_H_

// Local Variables:
// mode: c++
// c-file-style: "linux"
// End:
