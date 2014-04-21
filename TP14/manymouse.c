#include <stdlib.h>
#include "manymouse.hpp"

ManyMouseDriver *ManyMouseDriver_windows;
ManyMouseDriver *ManyMouseDriver_evdev;
ManyMouseDriver *ManyMouseDriver_hidmanager;
ManyMouseDriver *ManyMouseDriver_hidutilities;
ManyMouseDriver *ManyMouseDriver_xinput2;

/*
 * These have to be in the favored order...obviously it doesn't matter if the
 *  Linux driver comes before or after the Windows one, since one won't
 *  exist on a given platform, but things like Mac OS X's hidmanager (which
 *  only works on OS X 10.5 and later) should come before Mac OS X's
 *  hidutilities (which works on older systems, but may stop working in 10.6
 *  and later). In the Mac OS X case, you want to try the newer tech, and if
 *  it's not available (on 10.4 or earlier), fall back to trying the legacy
 *  code.
 */
ManyMouseDriver **mice_drivers[] =
{
    &ManyMouseDriver_xinput2,
    &ManyMouseDriver_evdev,
    &ManyMouseDriver_windows,
    &ManyMouseDriver_hidmanager,
    &ManyMouseDriver_hidutilities,
};


static const ManyMouseDriver *driver = NULL;

int ManyMouse_Init(void)
{
    const int upper = (sizeof (mice_drivers) / sizeof (mice_drivers[0]));
	
    int i;
    int retval = -1;

    if (driver != NULL)
        return -1;

    for (i = 0; (i < upper) && (driver == NULL); i++)
    {
        const ManyMouseDriver *this_driver = *(mice_drivers[i]);
        if (this_driver != NULL) /* if not built for this platform, skip it. */
        {
            const int mice = this_driver->init();
            if (mice > retval)
                retval = mice; /* may move from "error" to "no mice found". */

            if (mice >= 0)
                driver = this_driver;
        } /* if */
    } /* for */

    return retval;
} /* ManyMouse_Init */


void ManyMouse_Quit(void)
{
    if (driver != NULL)
    {
        driver->quit();
        driver = NULL;
    } /* if */
} /* ManyMouse_Quit */

const char *ManyMouse_DriverName(void)
{
    return (driver) ? driver->driver_name : NULL;
} /* ManyMouse_DriverName */

const char *ManyMouse_DeviceName(unsigned int index)
{
    return (driver) ? driver->name(index) : NULL;
} /* ManyMouse_DeviceName */

int ManyMouse_PollEvent(ManyMouseEvent *event)
{
    return (driver) ? driver->poll(event) : 0;
} /* ManyMouse_PollEvent */

/* end of manymouse.c ... */

