#pragma once

/// MSG filter works as a firewall
/// it allowes only certain messages for certain things, see checktype
/// -1 will become 1 in the switchcase below, its a hack to not misinterpretate the different cases as messages 
static struct msgfilter
{
    uchar msgmask[NUMMSG];

    msgfilter(int msg, ...)
    {
        memset(msgmask, 0, sizeof(msgmask));
        va_list msgs;
        va_start(msgs, msg);
        for(uchar val = 1; msg < NUMMSG; msg = va_arg(msgs, int))
        {
            if(msg < 0) val = uchar(-msg);
            else msgmask[msg] = val;
        }
        va_end(msgs);
    }

    uchar operator[](int msg) const { return msg >= 0 && msg < NUMMSG ? msgmask[msg] : 0; }
};

/// 
msgfilter msgfilter(-1, N_CONNECT, N_SERVINFO, N_INITCLIENT, N_WELCOME, N_MAPCHANGE, N_SERVMSG, N_DAMAGE, N_HITPUSH, N_SHOTFX, N_EXPLODEFX, N_DIED, N_SPAWNSTATE, N_FORCEDEATH, N_TEAMINFO, N_ITEMACC, N_ITEMSPAWN, N_TIMEUP, N_CDIS, N_CURRENTMASTER, N_PONG, N_RESUME, N_BASESCORE, N_BASEINFO, N_BASEREGEN, N_ANNOUNCE, N_SENDDEMOLIST, N_SENDDEMO, N_DEMOPLAYBACK, N_SENDMAP, N_DROPFLAG, N_SCOREFLAG, N_RETURNFLAG, N_RESETFLAG, N_INVISFLAG, N_CLIENT, N_AUTHCHAL, N_INITAI, N_EXPIRETOKENS, N_DROPTOKENS, N_STEALTOKENS, N_DEMOPACKET,
            -2, N_REMIP, N_NEWMAP, N_GETMAP, N_SENDMAP, N_CLIPBOARD,
            -3, N_EDITENT, N_EDITF, N_EDITT, N_EDITM, N_FLIP, N_COPY, N_PASTE, N_ROTATE, N_REPLACE, N_DELCUBE, N_EDITVAR, N_EDITVSLOT, N_UNDO, N_REDO,
            -4, N_POS, NUMMSG),
    connectfilter(-1, N_CONNECT, -2, N_AUTHANS, -3, N_PING, NUMMSG);
