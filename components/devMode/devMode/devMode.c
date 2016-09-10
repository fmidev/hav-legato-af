/**
 * @file devMode.c
 *
 * This component puts the device into the "Developer Mode", which aids the development of new apps
 * or modifications to existing apps or systems.
 *
 * <hr>
 *
 * Copyright (C) Sierra Wireless Inc. Use of this work is subject to license.
 *
 */

#include "legato.h"
#include "interfaces.h"


//--------------------------------------------------------------------------------------------------
/**
 * Timer reference for the "MarkGood" timer.
 */
//--------------------------------------------------------------------------------------------------
static le_timer_Ref_t MarkGoodTimer;


//--------------------------------------------------------------------------------------------------
/**
 * Timer expiry handler for the "MarkGood" timer. Mark the system as "Good", and start the timer
 * again.
 */
//--------------------------------------------------------------------------------------------------
static void MarkGood
(
    le_timer_Ref_t timer
)
{
    // Attempt to mark the system "good", so return status isn't checked. If there's a probation
    // lock, it might be from the apps are being developed, so we shouldn't override that by forcing
    // "Mark Good".
    le_updateCtrl_MarkGood(false);

    le_timer_Start(MarkGoodTimer);
}


//--------------------------------------------------------------------------------------------------
/**
 * Initialization function.
 */
//--------------------------------------------------------------------------------------------------
COMPONENT_INIT
{
    /* Start the tcf-agent daemon */
    // tcf-agent.conf contains target device info necessary for the tcf-agent to send to Dev Studio.
    // If the file exists, or is defined by the user, then it's not generated.
    struct stat buf;
    if (stat("/etc/tcf-agent.conf", &buf) != 0)
    {
        system("echo `/usr/bin/ud_getusbinfo NAME` `/usr/bin/ud_getusbinfo IMEI` > "
               "/etc/tcf-agent.conf");
    }

    system("/usr/sbin/tcf-agent -d -L- -l0");


    /* Obtain a wake lock */
    // Note that the wake lock is released if the app is stopped.
    le_pm_WakeupSourceRef_t wakeUpSource = le_pm_NewWakeupSource(0, "devModeApp");
    le_pm_StayAwake(wakeUpSource);


    /* Attempt to mark the system as "Good" within 10 seconds of system changes. */
    MarkGoodTimer = le_timer_Create("MarkGood");
    le_timer_SetHandler(MarkGoodTimer, MarkGood);
    le_timer_SetMsInterval(MarkGoodTimer, 10000);
    le_timer_Start(MarkGoodTimer);
}
