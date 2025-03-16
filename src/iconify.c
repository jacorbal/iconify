/**
 * @file iconify.c
 *
 * @brief Window iconify implementation
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

/* To avoid including <X11/Xatom.h> just for the definition of 'XA_ATOM' */
#ifndef XA_ATOM
#define XA_ATOM ((Atom) 4)
#endif

/* Local includes */
#include <defaults.h>
#include <iconify.h>


/* Initialize a new icon */
icon_td *icon_init(Display *display, Window window_orig,
        Pixmap pixmap, char *prog_name, const char *path,
        unsigned int border, unsigned int width, unsigned int height,
        unsigned long text_bg, unsigned long text_fg,
        unsigned long frame_c, Bool show_text)
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


/* Create new icon window */
void icon_create(icon_td *icon)
{
    XWindowAttributes attributes;
    Window current_window = icon->window_orig;
    int absolute_x = 0;
    int absolute_y = 0;

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
        Window root;
        Window parent;
        Window *children;
        unsigned int num_children;

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
    unsigned int window_height = icon->height +
        ((icon->show_text) ? DEFAULT_TEXT_HEIGHT : 0) + 2 * icon->border;

    /* Create icon by reading original window coordinates (top, left) */
    icon->window = XCreateSimpleWindow(icon->display,
            DefaultRootWindow(icon->display), icon->x_pos, icon->y_pos,
            icon->width + 2 * icon->border, window_height,
            0,
            BlackPixel(icon->display, 0), WhitePixel(icon->display, 0));

    /* Set the 'override_redirect' property: no WM interference */
    XSetWindowAttributes windowAttributes;
    windowAttributes.override_redirect = True;
    XChangeWindowAttributes(icon->display, icon->window,
            CWOverrideRedirect, &windowAttributes);

    /* Make sure the icon window is on the desktop, and not above it */
    Atom wm_window_type = XInternAtom(icon->display,
            "_NET_WM_WINDOW_TYPE", False);
    Atom wm_window_type_desktop = XInternAtom(icon->display,
            "_NET_WM_WINDOW_TYPE_DESKTOP", False);
    XChangeProperty(icon->display, icon->window, wm_window_type,
            XA_ATOM, 32/*bits*/, PropModeReplace,
            (unsigned char *) &wm_window_type_desktop, 1);

    /* Just to make sure the icon cannot be on top of other windows */
    Atom wm_state = XInternAtom(icon->display, "_NET_WM_STATE", False);
    Atom wm_state_lower = XInternAtom(icon->display,
            "_NET_WM_STATE_BELOW", False);
    XChangeProperty(icon->display, icon->window, wm_state, XA_ATOM,
            32, PropModeReplace, (unsigned char *) &wm_state_lower, 1);

    /* Set window properties */
    XSetStandardProperties(icon->display, icon->window,
            icon->prog_name, "Unknown", None, NULL, 0, NULL);
    
    /* Input events */
    XSelectInput(icon->display, icon->window,
            ExposureMask        |   ButtonPressMask     |
            ButtonReleaseMask   |   PointerMotionMask);

    /* Draw the icon */
    icon_draw(icon);

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


/* Load icon given original window */
Pixmap icon_load(Display *display, const char *path, Window window_orig)
{
    Pixmap pixmap = None;

    /* Try to load icon using path and load it if exists */
    if (access(path, F_OK) != -1) {
        if (XpmReadFileToPixmap(display, DefaultRootWindow(display),
                    path, &pixmap, NULL, NULL) == XpmSuccess) {
            return pixmap;
        }
    }

    /* Try to load icon by using the original window class */
    XClassHint class_hint;
    if (XGetClassHint(display, window_orig, &class_hint)) {
        char icon_name[MAX_APP_NAME_LENGTH];
        snprintf(icon_name, sizeof(icon_name),
                "/usr/share/pixmaps/%s.xpm", class_hint.res_class);
        struct stat buffer;
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
    double x_ratio = (double)width_old / (double)width_new;
    double y_ratio = (double)height_old / (double)height_new;

    /* Scale the image by providing the scaled coordinates */
    for (unsigned int y = 0; y < height_new; ++y) {
        for (unsigned int x = 0; x < width_new; ++x) {
            // Calculate the source coordinates in the original pixmap
            int src_x = (int)(x * x_ratio);
            int src_y = (int)(y * y_ratio);
            
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
        if (event.type == ClientMessage &&
                event.xclient.data.l[0] ==
                    (unsigned int) XInternAtom(icon->display,
                        "WM_DELETE_WINDOW", False)) {
            break;
        }
        if (event.type == ButtonPress) {
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
                        window_restore(icon);
                        break;
                    }
                    last_click_time = event.xbutton.time;
                }
            }
        } else if (event.type == MotionNotify && dragging) {
            int x_new = event.xmotion.x_root - x_drag_start;
            int y_new = event.xmotion.y_root - y_drag_start;
            XMoveWindow(icon->display, icon->window, x_new, y_new);
        } else if (event.type == Expose) {
            icon_draw(icon);    /* Re-draws the displayed icon */
        }
    }
}


/* Restores original window and closes icon */
void window_restore(icon_td *icon)
{
    XUnmapWindow(icon->display, icon->window);
    XMapWindow(icon->display, icon->window_orig);
}
