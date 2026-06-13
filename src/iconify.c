/**
 * @file iconify.c
 *
 * @brief Window iconify implementation
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
#include <stdio.h>      /* fprintf, snprintf */
#include <stdlib.h>     /* abs, free, malloc */
#include <string.h>     /* strdup (POSIX.1-2008), strlen */
#include <sys/stat.h>   /* stat */
#include <unistd.h>     /* access */

/* X includes */
#include <X11/Xlib.h>   /* Bool, False, True, Display, Pixmap, Window, X* */
#include <X11/xpm.h>    /* XClassHint, XGetPIxel, XSetStandardProperties */

/* To avoid including <X11/Xatom.h> just for the sake of the
 * definitions of 'XA_ATOM' and 'XA_CARDINAL' */
#ifndef XA_ATOM
#define XA_ATOM ((Atom) 4)
#endif
#ifndef XA_CARDINAL
#define XA_CARDINAL ((Atom) 5)
#endif

/* Local includes */
#include <defaults.h>
#include <iconify.h>


/**/
static Bool window_is_iconic(Display *display, Window window)
{
    Atom wm_state;
    Atom actual_type;
    int actual_format;
    unsigned long nitems, bytes_after;
    unsigned char *data = NULL;
    long state;
    Bool iconic = False;

    wm_state = XInternAtom(display, "WM_STATE", False);
    if (wm_state == None) {
        return False;
    }

    if (XGetWindowProperty(display, window, wm_state,
                           0, 2, False, wm_state,
                           &actual_type, &actual_format,
                           &nitems, &bytes_after,
                           &data) != Success) {
        return False;
    }

    if (data != NULL && actual_type == wm_state &&
            actual_format == 32 && nitems >= 1) {
        state = ((long *) data)[0];
        iconic = (state == WIN_STATE_ICONIC);
    }

    if (data != NULL) {
        XFree(data);
    }

    return iconic;
}


/**/
static void tooltip_ensure_created(icon_td *icon)
{
    XSetWindowAttributes attrs;

    if (icon->tooltip_window != None) {
        return;
    }

    attrs.override_redirect = True;
    attrs.background_pixel = icon->bg;
    attrs.border_pixel = icon->fc;

    icon->tooltip_window = XCreateWindow(
        icon->display,
        DefaultRootWindow(icon->display),
        0, 0,
        10, 10,
        1,
        CopyFromParent,
        InputOutput,
        CopyFromParent,
        CWOverrideRedirect | CWBackPixel | CWBorderPixel,
        &attrs
    );

    XSelectInput(icon->display, icon->tooltip_window, ExposureMask);
}


/**/
static void tooltip_draw(icon_td *icon)
{
    GC gc;
    int padding_x = 6;
    int padding_y = 4;
    int text_w;
    int text_h = 16;
    int win_w;
    int win_h;
    size_t len;

    if (icon->tooltip_window == None || icon->prog_name == NULL) {
        return;
    }

    len = strlen(icon->prog_name);
    text_w = (int)len * 8;   /* aproximación simple */
    win_w = text_w + 2 * padding_x;
    win_h = text_h + 2 * padding_y;

    XResizeWindow(icon->display, icon->tooltip_window,
                  (unsigned int)win_w, (unsigned int)win_h);

    gc = XCreateGC(icon->display, icon->tooltip_window, 0, NULL);

    XSetForeground(icon->display, gc, icon->bg);
    XFillRectangle(icon->display, icon->tooltip_window, gc, 0, 0,
                   (unsigned int)win_w, (unsigned int)win_h);

    XSetForeground(icon->display, gc, icon->fg);
    XDrawString(icon->display, icon->tooltip_window, gc,
                padding_x, padding_y + 12,
                icon->prog_name, (int)len);

    XFreeGC(icon->display, gc);
}


/**/
static void tooltip_show(icon_td *icon, int x_root, int y_root)
{
    int x = x_root + 12;
    int y = y_root + 20;

    if (!icon->show_tooltip || icon->prog_name == NULL) {
        return;
    }

    tooltip_ensure_created(icon);
    tooltip_draw(icon);

    XMoveWindow(icon->display, icon->tooltip_window, x, y);

    if (!icon->is_tooltip_visible) {
        XMapRaised(icon->display, icon->tooltip_window);
        icon->is_tooltip_visible = True;
    } else {
        XRaiseWindow(icon->display, icon->tooltip_window);
    }
}


/**/
static void tooltip_hide(icon_td *icon)
{
    if (icon->tooltip_window != None && icon->is_tooltip_visible) {
        XUnmapWindow(icon->display, icon->tooltip_window);
        icon->is_tooltip_visible = False;
    }
}


/* Initialize a new icon */
icon_td *icon_init(Display *display, Window window_orig,
        Pixmap pixmap, char *prog_name, const char *path,
        unsigned int border, unsigned int width, unsigned int height,
        unsigned long text_bg, unsigned long text_fg,
        unsigned long frame_c, Bool show_text, Bool show_tooltip)
{
    icon_td *icon;

    /* Allocate memory */
    icon = malloc(sizeof(icon_td));
    if (!icon) {
        return NULL;
    }

    /* Set initial values */
    icon->display = display;
    icon->window_orig = window_orig;
    icon->pixmap = pixmap;
    icon->border = border;
    icon->width = width;
    icon->height = height;
    icon->path = strdup(path);
    icon->x_pos = 240;
    icon->y_pos = 240;
    icon->bg = text_bg;     /* Text background color */
    icon->fg = text_fg;     /* Text foreground color */
    icon->fc = frame_c;     /* Frame color */
    icon->show_text = show_text;
    icon->tooltip_window = None;
    icon->show_tooltip = show_tooltip;
    icon->is_tooltip_visible = False;

    /* Use program name to set the name of icon window */
    if (prog_name) {
        icon->prog_name = strdup(prog_name);
    } else {
        /* If there's no name, use original window name */
        char name[MAX_APP_NAME_LENGTH] = "Unknown"; /* Default name */
        XClassHint class_hint;
        if (XGetClassHint(display, window_orig, &class_hint)) {
            snprintf(name, sizeof(name), "%s", class_hint.res_name);
            XFree(class_hint.res_name);
            XFree(class_hint.res_class);
        }
        icon->prog_name = strdup(name);
    }

    return icon;
}


/* Destroy icon structure and free resources */
void icon_destroy(icon_td *icon)
{
    if (icon) {
        if (icon->tooltip_window != None) {
            XDestroyWindow(icon->display, icon->tooltip_window);
        }

        if (icon->pixmap) {
            XFreePixmap(icon->display, icon->pixmap);
        }
        if (icon->prog_name) {
            free(icon->prog_name);
        }
        if (icon->path) {
            free(icon->path);
        }
        free(icon);
    }
}


void icon_create(icon_td *icon)
{
    XWindowAttributes attributes;
    Window current_window = icon->window_orig;
    int absolute_x = 0;
    int absolute_y = 0;
    unsigned int window_height;
    Window root;
    Window parent;
    Window *children;
    unsigned int num_children;
    XSetWindowAttributes windowAttributes;

    /* Follow window hierarchy until root to get absolute coordinates */
    while (current_window != 0) {
        /* Get current window attributes */
        if (!XGetWindowAttributes(icon->display, current_window,
                    &attributes)) {
            fprintf(stderr, "Cannot get window properties\n");
            return;
        }

        /* Get absolute coordinates by accumulation of parent windows
         * coordintes, only if visible */
        if (attributes.map_state == IsViewable) {
            absolute_x += attributes.x;
            absolute_y += attributes.y;
        }

        /* Get parent window until root window */
        /* Get widnow hierarchy */
        if (XQueryTree(icon->display, current_window, &root, &parent,
                    &children, &num_children)) {
            current_window = parent;    /* Move to parent window */
            XFree(children);
        } else {
            fprintf(stderr, "Cannot obtain window hierarchy\n");
            break;
        }
    }

    /* Set icon coordinates */
    icon->x_pos = (absolute_x < 0) ? 240 : absolute_x;
    icon->y_pos = (absolute_y < 0) ? 240 : absolute_y;

    /* Set icon height depending on whether text is displayed or not */
    window_height = icon->height +
        ((icon->show_text) ? DEFAULT_TEXT_HEIGHT : 0) + 2 * icon->border;

    /* Create icon by reading original window coordinates (top, left) */
    icon->window = XCreateSimpleWindow(icon->display,
            DefaultRootWindow(icon->display), icon->x_pos, icon->y_pos,
            icon->width + 2 * icon->border, window_height,
            0,
            BlackPixel(icon->display, 0), WhitePixel(icon->display, 0));

    /* Set the 'override_redirect' property: no WM interference */
    windowAttributes.override_redirect = False;
    XChangeWindowAttributes(icon->display, icon->window,
            CWOverrideRedirect, &windowAttributes);

    /* Set window properties */
    XSetStandardProperties(icon->display, icon->window,
            icon->prog_name, "Unknown", None, NULL, 0, NULL);

    {
        Atom motif_hints = XInternAtom(icon->display,
                "_MOTIF_WM_HINTS", False);

        struct {
            unsigned long flags;
            unsigned long functions;
            unsigned long decorations;
            long input_mode;
            unsigned long status;
        } hints = { 2, 0, 0, 0, 0 };

        XChangeProperty(icon->display,
                icon->window,
                motif_hints,
                motif_hints,
                32/*bits*/,
                PropModeReplace,
                (unsigned char *) &hints,
                5);
    }

    {
        XSizeHints size_hints;

        size_hints.flags = PMinSize | PMaxSize;
        size_hints.min_width  = (int)(icon->width + 2 * icon->border);
        size_hints.max_width  = (int)(icon->width + 2 * icon->border);
        size_hints.min_height = (int)window_height;
        size_hints.max_height = (int)window_height;

        XSetWMNormalHints(icon->display, icon->window, &size_hints);
    }

    /* Input events */
    XSelectInput(icon->display, icon->window,
            ExposureMask        |   ButtonPressMask     |
            ButtonReleaseMask   |   PointerMotionMask   |
            LeaveWindowMask);
    XSelectInput(icon->display, icon->window_orig,
             StructureNotifyMask | PropertyChangeMask);

    /* Draw the icon */
    icon_draw(icon);

    /* Bind the icon window to the current desktop only */
    Atom net_wm_desktop = XInternAtom(icon->display,
            "_NET_WM_DESKTOP", False);
    if (net_wm_desktop != None && icon->desktop_cur >= 0) {
        XChangeProperty(icon->display,
                icon->window,
                net_wm_desktop,
                XA_CARDINAL,
                32/*bits*/,
                PropModeReplace,
                (unsigned char *) &icon->desktop_cur,
                1);
    }

    /* Keep icon below other windows and out of taskbar/pager */
    {
        Atom net_wm_state;
        Atom states[3];

        net_wm_state = XInternAtom(icon->display,
                "_NET_WM_STATE", False);
        states[0] = XInternAtom(icon->display,
                "_NET_WM_STATE_BELOW", False);
        states[1] = XInternAtom(icon->display,
                "_NET_WM_STATE_SKIP_TASKBAR", False);
        states[2] = XInternAtom(icon->display,
                "_NET_WM_STATE_SKIP_PAGER", False);

        XChangeProperty(icon->display,
                icon->window,
                net_wm_state,
                XA_ATOM,
                32/*bits*/,
                PropModeReplace,
                (unsigned char *) states,
                3);
    }

    /* Show icon and put it on desktop layer */
    XMapWindow(icon->display, icon->window);
    XLowerWindow(icon->display, icon->window);

    /* Minimize original window */
    XIconifyWindow(icon->display, icon->window_orig,
            DefaultScreen(icon->display));
}


/* Draw icon and its text on its window */
void icon_draw(icon_td *icon)
{
    /* Scale pixmap */
    Pixmap scaled_pixmap = pixmap_scale(icon->display, icon->pixmap,
            DEFAULT_WIDTH/*px*/, DEFAULT_HEIGHT/*px*/,
            icon->width/*px*/, icon->height/*px*/);

    /* Clear window */
    XSetWindowBackground(icon->display, icon->window,
            WhitePixel(icon->display, DefaultScreen(icon->display)));
    XClearWindow(icon->display, icon->window);

    /* Draw border */
    if (icon->border > 0) {
        GC gc = XCreateGC(icon->display, icon->window, 0, NULL);
        unsigned int total_height = icon->height +
            ((icon->show_text) ? DEFAULT_TEXT_HEIGHT : 0) +
                2 * icon->border;

        /* Adjust border depending on its thinkness */
        XSetForeground(icon->display, gc, icon->fc);    /* FC color */
        XFillRectangle(icon->display, icon->window, gc, 0, 0,
                icon->width + 2 * icon->border,
                total_height);

        /* Reset the icon area behind border */
        XSetForeground(icon->display, gc, WhitePixel(icon->display, 0)); 
        XFillRectangle(icon->display, icon->window, gc,
                       (int) icon->border, (int) icon->border,
                       icon->width,
                       icon->height +
                           ((icon->show_text) ? DEFAULT_TEXT_HEIGHT : 0));

        /* Free used memory */
        XFreeGC(icon->display, gc);
    }

    /* Draw scaled pixmap */
    XCopyArea(icon->display, scaled_pixmap, icon->window,
            DefaultGC(icon->display, DefaultScreen(icon->display)),
            0, 0, icon->width, icon->height,
            (int) icon->border, (int) icon->border);

    /* Show icon text */
    if (icon->show_text) {
        /* Set the text background */
        GC gc = XCreateGC(icon->display, icon->window, 0, NULL);

        XSetForeground(icon->display, gc, icon->bg); /* BG color */
        XFillRectangle(icon->display, icon->window, gc,
                (int) icon->border,
                (int) (icon->height + icon->border),
                icon->width, DEFAULT_TEXT_HEIGHT);

        /* Draw icon text */
        XSetForeground(icon->display, gc, icon->fg); /* FG color */
        XDrawString(icon->display, icon->window, gc,
                DEFAULT_TEXT_LOFFSET + (int) icon->border,
                (int) (icon->height + icon->border + DEFAULT_TEXT_VOFFSET),
                icon->prog_name, (int) strlen(icon->prog_name));

        /* Clear and free the graphics context */
        XFreeGC(icon->display, gc);
    }

    /* Free scaled pixmap */
    XFreePixmap(icon->display, scaled_pixmap);
}


/* Get actual active desktop for the icon */
void icon_update_cur_desktop(icon_td *icon)
{
    Display *display = icon->display;
    int screen = DefaultScreen(display);
    Window root = RootWindow(display, screen);
    unsigned long nitems, leftover;
    unsigned char *data = NULL;
    int actual_format;
    Atom actual_type;
    Atom net_wm;

    /* Get atom _NET_CURRENT_DESKTOP */
    net_wm = XInternAtom(display, "_NET_CURRENT_DESKTOP", True);
    if (net_wm == None) {
        fprintf(stderr, "Failed to get atom _NET_CURRENT_DESKTOP\n");
        return;
    }

    /* Get current desktop property */
    if (XGetWindowProperty(display, root, net_wm, 0L, 1,
                False, AnyPropertyType,
                &actual_type, &actual_format,
                &nitems, &leftover, &data) != Success) {
        fprintf(stderr, "Failed to get desktop property\n");
        return;
    }

    if (nitems > 0) {
        /* Set number of desktop on icon structure */
        icon->desktop_cur = *(long *) data;
    } else {
        fprintf(stderr, "Invalid desktop number: nitems: %lu\n", nitems);
        icon->desktop_cur = -1;
    }

    /* Deallocate memory */
    if (data) {
        XFree(data);
    }
}


/* Load icon given original window */
Pixmap icon_load(Display *display, const char *path, Window window_orig)
{
    Pixmap pixmap = None;
    XClassHint class_hint;

    /* Try to load icon using path and load it if exists */
    if (access(path, F_OK) != -1) {
        if (XpmReadFileToPixmap(display, DefaultRootWindow(display),
                    path, &pixmap, NULL, NULL) == XpmSuccess) {
            return pixmap;
        }
    }

    /* Try to load icon by using the original window class */
    if (XGetClassHint(display, window_orig, &class_hint)) {
        struct stat buffer;
        char icon_name[MAX_APP_NAME_LENGTH];
        snprintf(icon_name, sizeof(icon_name),
                "/usr/share/pixmaps/%s.xpm", class_hint.res_class);
        if (stat(icon_name, &buffer) == 0) {
            /* If exists, load it */
            if (XpmReadFileToPixmap(display, DefaultRootWindow(display),
                        icon_name, &pixmap, NULL, NULL) == XpmSuccess) {
                XFree(class_hint.res_name);
                XFree(class_hint.res_class);
                return pixmap;
            }
        }

        XFree(class_hint.res_name);
        XFree(class_hint.res_class);
    }

    /* Load default icon if cannot be found */
    if (XpmReadFileToPixmap(display, DefaultRootWindow(display),
                DEFAULT_ICON_PATH, &pixmap, NULL, NULL) == XpmSuccess) {
        return pixmap;
    }

    /* Return NULL pixmap otherwise */
    return None;
}


/* Pixmap scaling */
Pixmap pixmap_scale(Display *display, Pixmap pixmap_orig,
                    unsigned int width_old, unsigned int height_old,
                    unsigned int width_new, unsigned int height_new)
{
    double x_ratio;
    double y_ratio;

    /* Create a new pixmap for the scaled image */
    Pixmap scaled_pixmap = XCreatePixmap(display,
            DefaultRootWindow(display), width_new, height_new,
            (unsigned int) DefaultDepth(display, DefaultScreen(display)));

    /* Create a GC (graphics context) for drawing */
    GC gc = XCreateGC(display, scaled_pixmap, 0, NULL);

    /* Set the background color to white */
    XSetForeground(display, gc, WhitePixel(display, 
                DefaultScreen(display)));
    XFillRectangle(display, scaled_pixmap, gc, 0, 0,
            width_new, height_new);

    /* Calculate scaling factors */
    x_ratio = (double) width_old / (double) width_new;
    y_ratio = (double) height_old / (double) height_new;

    /* Scale the image by providing the scaled coordinates */
    for (unsigned int y = 0; y < height_new; ++y) {
        for (unsigned int x = 0; x < width_new; ++x) {
            // Calculate the source coordinates in the original pixmap
            int src_x = (int) (x * x_ratio);
            int src_y = (int) (y * y_ratio);
            
            /* Copy a 1x1px from the original pixmap to the scaled pixmap */
            XCopyArea(display, pixmap_orig, scaled_pixmap, gc,
            src_x, src_y, 1, 1, (int) x, (int) y);
        }
    }

    /* Cleanup and free resources */
    XFreeGC(display, gc);
    
    return scaled_pixmap;
}


/* Icon mouse event handler on iconized window */
void events_handle(icon_td *icon)
{
    XEvent event;
    Bool dragging = False;
    int x_drag_start = 0;
    int y_drag_start = 0;

    while (True) {
        XNextEvent(icon->display, &event);

        /* If the original window is restored by the WM or by any external
         * tool, remove this icon and exit. This check must happen before
         * handling icon-window events. */
        if (event.xany.window == icon->window_orig) {
            if (!window_is_iconic(icon->display, icon->window_orig)) {
                tooltip_hide(icon);

                if (icon->window != None) {
                    XDestroyWindow(icon->display, icon->window);
                    icon->window = None;
                }
                XFlush(icon->display);
                break;
            }
            continue;
        }

        if (event.type == ClientMessage &&
            event.xclient.data.l[0] ==
            (unsigned int) XInternAtom(icon->display,
                "WM_DELETE_WINDOW", False)) {
            tooltip_hide(icon);
            break;
        }

        if (event.type == ButtonPress) {
            tooltip_hide(icon);
            if (event.xbutton.button == Button1) {
                dragging = True;
                x_drag_start = event.xbutton.x;
                y_drag_start = event.xbutton.y;
            }
        } else if (event.type == ButtonRelease) {
            if (event.xbutton.button == Button1) {
                dragging = False;
                if (abs(event.xbutton.x - x_drag_start) <= 5 &&
                    abs(event.xbutton.y - y_drag_start) <= 5) {
                    static Time last_click_time = 0;
                    if (event.xbutton.time - last_click_time <= 500) {
                        tooltip_hide(icon);
                        window_restore(icon);
                        break;
                    }
                    last_click_time = event.xbutton.time;
                }
            }
        } else if (event.type == MotionNotify) {
            if (dragging) {
                int x_new = event.xmotion.x_root - x_drag_start;
                int y_new = event.xmotion.y_root - y_drag_start;

                tooltip_hide(icon);
                XMoveWindow(icon->display, icon->window, x_new, y_new);
            } else {
                tooltip_show(icon,
                        event.xmotion.x_root,
                        event.xmotion.y_root);
            }

        } else if (event.type == LeaveNotify) {
            tooltip_hide(icon);

        } else if (event.type == Expose) {
            if (event.xexpose.window == icon->window) {
                icon_draw(icon);
            } else if (event.xexpose.window == icon->tooltip_window) {
                tooltip_draw(icon);
            }
        }
    }
}


/* Restores original window and closes icon */
void window_restore(icon_td *icon)
{
    icon_update_cur_desktop(icon);

    XMapWindow(icon->display, icon->window_orig);
    XRaiseWindow(icon->display, icon->window_orig);

    if (icon->window != None) {
        XDestroyWindow(icon->display, icon->window);
        icon->window = None;
    }

    XFlush(icon->display);
}
