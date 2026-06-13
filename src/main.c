/**
 * @file main.c
 *
 * @author J. A. Corbal <jacorbal@gmail.com>
 *
 * @date Creation date: Sun Mar 16 01:48:58 AM UTC 2025
 * @date Last update: Sun Mar 16 01:48:58 AM UTC 2025
 *
 * @brief Main entry to iconify program
 *
 * Iconizes windows allowing users to minimize windows into small icons
 * on the desktop.  This feature provides a way to manage and organize
 * open applications without closing them.  Users can click on these
 * icons to restore the associated windows to their original size and
 * position.  It works in the same way as TWM, where it handles window
 * management efficiently with less customization options for icon
 * appearance and behavior.
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

/* Enable features from the POSIX.1-2008 standard */
#define _POSIX_C_SOURCE 200809L


/* Standard library includes */
#include <locale.h>     /* setlocale */
#include <stdio.h>      /* fprintf, sscanf */
#include <stdlib.h>     /* atoi, exit */
#include <unistd.h>     /* getopt, optarg, optind */

/* X includes */
#include <X11/Xlib.h>   /* Bool, False, True, Display, Pixmap, Window, X* */

/* Local includes */
#include <defaults.h>
#include <iconify.h>


/* Convert hexadecimal color string into unsigned long */
static unsigned long s_hex_to_ulong(const char *color_str)
{
    unsigned long color;
    sscanf(color_str, "%lx", &color);
    return color;
}


/* Show help */
static inline void s_help_show(FILE *fp, const char basename[])
{
    fprintf(fp, "Usage: %s [<options>] <window_id>\n", basename);
    fprintf(fp, "Options:\n");
    fprintf(fp, "   -h          This help and exit\n");
    fprintf(fp, "   -v          Display version and exit\n");
    fprintf(fp, "\n");
    fprintf(fp, "   -t          Disable text caption\n");
    fprintf(fp, "   -T          Enable tooltips (experimental)\n");
    fprintf(fp, "   -n <name>   Name to show below the icon\n");
    fprintf(fp, "   -i <icon>   Path to the icon pixmap (xpm/xbm)\n");
    fprintf(fp, "   -W <width>  Icon width in pixels\n");
    fprintf(fp, "   -H <height> Icon height in pixels\n");
    fprintf(fp, "   -s <dim>    Icon width and height in pixels\n");
    fprintf(fp, "   -B <fb>     Text background color\n");
    fprintf(fp, "   -F <fg>     Text foreground color\n");
    fprintf(fp, "   -f <fc>     Frame color when border is active\n");
    fprintf(fp, "   -b <border> Border width in pixels, o 0 for none\n");
    fprintf(fp, "\n");
}


/* Show version */
static inline void s_version_show(FILE *fp, const char basename[])
{
    fprintf(fp, "%s -- %s, version %s\n", basename,
        PROG_NAME_LONG, PROG_VERSION);
    fprintf(fp, "Licensed under the %s\n", PROG_LICENSE);
    fprintf(fp, "%s\n", PROG_COPYRIGHT);
}


/* Main entry */
int main(int argc, char *argv[])
{
    icon_td *icon = NULL;
    char *prog_name = NULL;
    const char *path = DEFAULT_ICON_PATH;
    int border = DEFAULT_BORDER;
    int width = DEFAULT_WIDTH;
    int height = DEFAULT_HEIGHT;
    unsigned long bg = DEFAULT_TEXT_BG;
    unsigned long fg = DEFAULT_TEXT_FG;
    unsigned long fc = DEFAULT_TEXT_FC;
    Bool show_text = True;
    Bool show_tooltip = False;
    Display *display;
    Window window_orig;
    Pixmap pixmap;
    int opt;

    setlocale(LC_ALL, "");
    while ((opt = getopt(argc, argv, "hvn:W:H:i:s:F:B:f:b:tT")) != -1) {
        switch (opt) {
            case 'h':
                s_help_show(stdout, argv[0]);
                exit(EXIT_SUCCESS);
            case 'v':
                s_version_show(stdout, argv[0]);
                exit(EXIT_SUCCESS);
                break;
            case 'n':
                prog_name = optarg;
                break;
            case 'W':
                width = atoi(optarg);
                break;
            case 'H':
                height = atoi(optarg);
                break;
            case 'i':
                path = optarg;
                break;
            case 's':
                width = atoi(optarg);
                height = width;
                break;
            case 'F':
                fg = s_hex_to_ulong(optarg);
                break;
            case 'f':
                fc = s_hex_to_ulong(optarg);
                break;
            case 'B':
                bg = s_hex_to_ulong(optarg);
                break;
            case 'b':
                border = atoi(optarg);
                break;
            case 't':
                show_text = False;
                break;
            case 'T':
                show_tooltip = True;
                break;
            default:
                s_help_show(stderr, argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (optind >= argc) {
        s_help_show(stderr, argv[0]);
        exit(EXIT_FAILURE);
    }
    
    window_orig = (Window) strtoul(argv[optind], NULL, 0);
    if (window_orig == 0) {
        fprintf(stderr, "Error: original window ID is not valid\n");
        exit(EXIT_FAILURE);
    }
    
    display = XOpenDisplay(NULL);
    if (!display) {
        fprintf(stderr, "Error: could not open display\n");
        exit(EXIT_FAILURE);
    }

    pixmap = icon_load(display, path, window_orig);
    if (pixmap == None) {
        fprintf(stderr, "Error: could not load icon\n");
        XCloseDisplay(display);
        exit(EXIT_FAILURE);
    }

    /* Initialize icon */
    icon = icon_init(display, window_orig, pixmap, prog_name, path,
            (unsigned int) border,
            (unsigned int) width, (unsigned int) height,
            bg, fg, fc, show_text, show_tooltip);
    if (!icon) {
        fprintf(stderr, "Error: could not initialize icon\n");
        XFreePixmap(display, pixmap);
        XCloseDisplay(display);
        exit(EXIT_FAILURE);
    }

    icon_create(icon);
    events_handle(icon);
    icon_destroy(icon);

    XCloseDisplay(display);

    return 0;
}
