/**
 * @file iconify.h
 *
 * @brief Window iconify declaration
 *
 * @author J. A. Corbal <jacorbal@gmail.com>
 *
 * @version 0.1.0
 * @date Creation date: Sun Mar 16 01:48:58 AM UTC 2025
 * @date Last update: Sun Mar 16 01:48:58 AM UTC 2025
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

#ifndef ICONIFIY_H
#define ICONIFIY_H

/* X includes */
#include <X11/Xlib.h>   /* Bool, Display, Pixmap, Window */


/**
 * @typedef icon_td
 *
 * @brief Icon structure and association to the original window
 */
typedef struct {
    Display *display;       /**< X Display */
    Window window_orig;     /**< Icon associated window */
    Window window;          /**< Icon window */
    Pixmap pixmap;          /**< Icon pixmap */
    char *prog_name;        /**< Name of the associated program */
    char *path;             /**< Icon path */
    unsigned int border;    /**< Icon border (px) */
    unsigned int width;     /**< Icon width (px) */
    unsigned int height;    /**< Icon heght (px) */
    int x_pos;              /**< Icon X initial position */
    int y_pos;              /**< Icon Y initial position */
    unsigned long bg;       /**< Text background color */
    unsigned long fg;       /**< Text foreground color */
    unsigned long fc;       /**< Frame color */
    Bool show_text;         /**< Display text under icon */
} icon_td;


/* Prototypes */
/**
 * @brief Initialize a new icon
 *
 * @param display     Display where to initialize this icon
 * @param window_orig Associated window to the new icon
 * @param pixmap      Icon pixmap
 * @param prog_name   Name of the program running on the associated window
 * @param path        Path to the icon file (xpm)
 * @param border      Icon border (px)
 * @param width       Icon width (px)
 * @param height      Icon height (px)
 * @param text_bg     Icon text background color
 * @param text_fg     Icon text foreground color
 * @param frame_col   Frame color
 * @param show_text   Show text if @c true, or otherwise
 */
icon_td *icon_init(Display *display, Window window_orig,
        Pixmap pixmap, char *prog_name, const char *path,
        unsigned int border, unsigned int width, unsigned int height,
        unsigned long text_bg, unsigned long text_fg,
        unsigned long frame_col, Bool show_text);

/**
 * @brief Destroy icon structure and free resources
 *
 * @param icon Icon to destroy
 */
void icon_destroy(icon_td *icon);

/**
 * @brief Create new icon window
 *
 * @param icon Icon structure with information to create new icon
 */
void icon_create(icon_td *icon);

/**
 * @brief Draw icon and its text on its window
 *
 * @param icon Icon to be drawed on its own window
 */
void icon_draw(icon_td *icon);

/**
 * @brief Load icon for given window
 *
 * @param display     Display where to load the icon
 * @param path        Icon full path
 * @param widnow_orig Original associated window
 *
 * @return Loaded icon as pixmap, or default icon, or @c None otherwise
 */
Pixmap icon_load(Display *display, const char *path, Window window_orig);

/**
 * @brief Pixmap scaling
 *
 * @param display     Display where the pixmap is
 * @param pixmap_orig Original pixmap to scale
 * @param width_old   Original pixmap width (px)
 * @param height_old  Original pixmap height (px)
 * @param width_new   New pixmap width (px)
 * @param height_new  New pixmap height (px)
 *
 * @return Scaled pixmap
 */
Pixmap pixmap_scale(Display *display, Pixmap pixmap_orig,
        unsigned int width_old, unsigned int height_old,
        unsigned int width_new, unsigned int height_new);

/**
 * @brief Icon mouse event handler on iconized window
 *
 * @param icon Icon where to handle its events
 */
void events_handle(icon_td *icon);

/**
 * @brief Restores original window and closes icon
 *
 * @param icon Icon to be closed
 *
 * @note The icon holds the information of the windown to be restored
 */
void window_restore(icon_td *icon);


#endif /* ! ICONIFIY_H */
