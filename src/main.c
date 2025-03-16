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
 * ISC License
 * Copyright (c) 2025, J. A. Corbal <jacorbal@gmail.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS.  IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
 * THIS SOFTWARE.
 */

/* Standard library includes */
#include <getopt.h>     /* getopt, optarg, optind */
#include <locale.h>     /* setlocale */
#include <stdio.h>      /* fprintf, sscanf */
#include <stdlib.h>     /* atoi, exit */

/* X includes */
#include <X11/Xlib.h>   /* Bool, False, True, Display, Pixmap, Window, X* */

/* Local includes */
#include <defaults.h>
#include <iconify.h>


/* Convert hexadecimal color string into unsigned long */
static unsigned long _hex_to_ulong(const char *color_str)
{
    unsigned long color;
    sscanf(color_str, "%lx", &color);
    return color;
}


/* Show help */
void _help_show(FILE *fp, const char basename[])
{
    fprintf(fp, "Usage: %s [<options>] <window_id>\n", basename);
    fprintf(fp, "Options:\n");
    fprintf(fp, "   -h          This help\n");
    fprintf(fp, "   -t          Disable text caption\n");
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
    int opt;

    setlocale(LC_ALL, "");
    while ((opt = getopt(argc, argv, "hn:W:H:i:s:F:B:f:b:t")) != -1) {
        switch (opt) {
            case 'h':
                _help_show(stdout, argv[0]);
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
                fg = _hex_to_ulong(optarg);
                break;
            case 'f':
                fc = _hex_to_ulong(optarg);
                break;
            case 'B':
                bg = _hex_to_ulong(optarg);
                break;
            case 'b':
                border = atoi(optarg);
                break;
            case 't':
                show_text = False;
                break;
            default:
                _help_show(stderr, argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (optind >= argc) {
        _help_show(stderr, argv[0]);
        exit(EXIT_FAILURE);
    }
    
    Window window_orig = (Window) strtoul(argv[optind], NULL, 0);
    if (window_orig == 0) {
        fprintf(stderr, "Error: original window ID is not valid\n");
        exit(EXIT_FAILURE);
    }
    
    Display *display = XOpenDisplay(NULL);
    if (!display) {
        fprintf(stderr, "Error: could not open display\n");
        exit(EXIT_FAILURE);
    }

    Pixmap pixmap = icon_load(display, path, window_orig);
    if (pixmap == None) {
        fprintf(stderr, "Error: could not load icon\n");
        XCloseDisplay(display);
        exit(EXIT_FAILURE);
    }

    // Inicializar el icono
    icon = icon_init(display, window_orig, pixmap, prog_name, path,
            (unsigned int) border,
            (unsigned int) width, (unsigned int) height,
            bg, fg, fc, show_text);
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
