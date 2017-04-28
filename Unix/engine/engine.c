/*
**==============================================================================
**
** Copyright (c) Microsoft Corporation. All rights reserved. See file LICENSE
** for license information.
**
**==============================================================================
*/

#include <server/server.h>

static Options s_opts;

static ServerData s_data;

int enginemain(int argc, const char* argv[])
{
#if defined(CONFIG_POSIX)
    int pidfile = -1;
#endif

    serverType = OMI_ENGINE;
    arg0 = argv[0];
    memset(&s_data, 0, sizeof(s_data));

    SetDefaults(&s_opts, &s_data, arg0);

    /* Get --destdir command-line option */
    GetCommandLineDestDirOption(&argc, argv);

    /* Extract configuration file options */
    GetConfigFileOptions();

    /* Extract command-line options a second time (to override) */
    GetCommandLineOptions(&argc, argv);

    /* Open the log file */
    OpenLogFile();

#if defined(CONFIG_POSIX)

    /* ATTN: unit-test support; should be removed/ifdefed later */
    if (s_opts.ignoreAuthentication)
    {
        IgnoreAuthCalls(1);
    }

    /* Watch for SIGTERM signals */
    if (0 != SetSignalHandler(SIGTERM, HandleSIGTERM) ||
        0 != SetSignalHandler(SIGHUP, HandleSIGHUP) ||
        0 != SetSignalHandler(SIGUSR1, HandleSIGUSR1))
        err(ZT("cannot set sighandler, errno %d"), errno);


    /* Watch for SIGCHLD signals */
    SetSignalHandler(SIGCHLD, HandleSIGCHLD);

#endif

    /* Change directory to 'rundir' */
    if (Chdir(OMI_GetPath(ID_RUNDIR)) != 0)
    {
        err(ZT("failed to change directory to: %s"), 
            scs(OMI_GetPath(ID_RUNDIR)));
    }

#if defined(CONFIG_POSIX)
    /* Daemonize */
    if (s_opts.daemonize && Process_Daemonize() != 0)
        err(ZT("failed to daemonize engine process"));
#endif

    while (!s_data.terminated)
    {
        InitializeNetwork();

        WsmanProtocolListen();

        const char *path = OMI_GetPath(ID_SOCKETFILE);
        BinaryProtocolListen(path);

        RunProtocol();
    }

    ServerCleanup(pidfile);

    return 0;
}
