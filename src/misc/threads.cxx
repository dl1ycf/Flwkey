// ----------------------------------------------------------------------------
//      threads.cxx
//
// Copyright (C) 2015
//              David Freese, W1HKJ
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

#include <stdio.h>
#include <stdlib.h>

#include "threads.h"

guard_lock::guard_lock(pthread_mutex_t* m, int h) : mutex(m), how(h) {
	pthread_mutex_lock(mutex);
}

guard_lock::~guard_lock(void) {
	pthread_mutex_unlock(mutex);
}
