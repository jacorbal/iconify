Iconify
=======

Definition
----------

"Iconize" refers to the process of converting an application window into
an icon within a graphical user environment, as implemented in TWM (Tab
Window Manager).  Instead of closing or deleting the window, the user
can *iconize* it, resulting in the window being minimized to an icon in
the desktop area, thus preserving the state and information of the
application for quick access in the future.  This approach enhances
workspace organization by allowing multiple applications to remain
readily accessible without cluttering the screen with open windows.

Missing features
----------------

  - Tooltips (giving the complete program name; toggled by user)
  - Task snapshot showing the current state of the program (toggled)
  - Add to group for join operations later
  - Using icon of multiple sizes (autodetect size)
  - Use of other image formats for standardization (GIF, PNG, SVG...)
  - Operations on the icon itself (`close`, `kill`...)
  - Resources file with default values (color, border...)
  - Logging when running (info, warning, error, fatal)

Dependencies
------------

  - `X11`

Advantages of iconizing
-----------------------

### Quick and efficient access

Iconizing windows allows users to keep their workspace organized.  By
reducing applications to icons, the main screen becomes less cluttered,
making it easier to focus on specific tasks.

When a window is iconized, it can be accessed quickly without needing to
reopen it from the application menu or initiate startup tasks.  This
saves time and provides a smoother user experience.  The ability to
restore an application to its previous state instantly enhances
productivity by reducing unnecessary interruptions.

Using a taskbar achieves the same goal, but it lacks the visual
component: what icon this program has, and where I put the icon on the
screen.  In the taskbar, users' eyes move to the same place.  In this
approach, access is faster and more efficient.

### Access to contextual information

Iconizing can allow users to keep certain contextual information visible
through icons that can represent multiple applications more effectively.
This contrasts with minimizing, where the windows lose all visual
characteristics that could aid in recalling their purpose or content.

### Personalized interaction

Some desktop environments and window managers allow for the
customization of application icons when iconized to make them more
representative or easier to identify.  This ability for personalization
is not present when minimizing, where the icon is often generic and less
distinctive.  Also, but choosing the icon on the user's demand, tasks
can be easily differentiated on the spot.

### Additional functionalities

In certain systems, iconizing can offer additional functionalities, such
as the ability to perform quick operations on the application through
the icon itself (for example, viewing notifications or quick access to
contextual menus), which is typically not available in the minimized
state.

Example
=======

This is an example using the current state of development.

Giving a window ID, use the program:

    $ iconify [<options>] <window_id>

How to implement it on any window manager?  By calling a script that
invokes `xwininfo`, getting the window identifier and passing it to
`iconify`.  For example, let's call this script `iconify_sel`:

    #!/bin/sh

    ICONIFY_BIN="${HOME}/.local/bin/iconify"

    if [ ! -x "${ICONIFY_BIN}" ]; then
        echo "Error: could not find '${ICONIFY_BIN}'" >&2
        exit 1
    fi

    ${ICONIFY_BIN} "$(xwininfo | grep 'Window id' | awk '{print $4}')"

Now, add to your window manager a keybinding or a mousebinding, for
example, `Ctrl+Shift+S` to call the script, and click on the window you
want to iconize.

I guess that the script could be modified if the window ID could be
taken from the WM itself, and implementing the `iconify` on
a mousebinding, such as, `Ctrl+Alt+Click` on the window, or adding a new
button on the window decoration to iconize.
