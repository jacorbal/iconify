/**
 * @file defaults.h
 *
 * @brief Default values using
 */
/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2025-2026, J. A. Corbal <jacorbal@gmail.com>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * “Software”), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/* Program specific defines */
#define PROG_NAME_SHORT "iconify"
#define PROG_NAME_LONG  "Iconify for X11"
#define PROG_VERSION "1.0.2"
#define PROG_LICENSE "The MIT License (MIT)"
#define PROG_COPYRIGHT "Copyright (c) 2025-2026, J. A. Corbal"


// TODO: Use 'const' within a struct?
#define MAX_APP_NAME_LENGTH 100
#define DEFAULT_TEXT_HEIGHT (20)    /* (px): text height */
#define DEFAULT_TEXT_VOFFSET (15)   /* (px): space for vertical offset */
#define DEFAULT_TEXT_LOFFSET (5)    /* (px): space from left sife */
#define DEFAULT_BORDER (1)          /* (px); or 0 to draw no border */
#define DEFAULT_WIDTH (32)          /* (px) */
#define DEFAULT_HEIGHT (32)         /* (px) */
#define DEFAULT_TEXT_BG (0xFFFFFF)  /* background: white */
#define DEFAULT_TEXT_FG (0x000000)  /* foreground: black */
#define DEFAULT_TEXT_FC (0x000000)  /* frame color: black */

#define DEFAULT_ICON_PATH "/usr/share/pixmaps/default.xpm"
