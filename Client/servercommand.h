#ifndef SERVERCOMMAND_H
#define SERVERCOMMAND_H

namespace ServerCommand
{
    enum ControlCommands
    {
        ID_SIGIN,
        ID_AUTHO,
        ID_CHNPW,
        ID_GETCV,
        ID_GTFLL,
        ID_SENDM,
        ID_USRON,
        ID_LGOUT,
        ID_CNNCT,
        ID_CHGNM,
        ID_CRTCV,
        ID_ADDMR,
        ID_LFTCV,
        ID_ALLUR,
        ID_GETIP,
        ID_INCCL,
        ID_ENDCL
    };

    enum Success
    {
        ID_FAIL,
        ID_SUCCESS
    };

    enum State
    {
        ID_STD,
        ID_NEW
    };

    enum Mode
    {
        MODE_OWN,
        MODE_NEW
    };
}

#endif // SERVERCOMMAND_H
