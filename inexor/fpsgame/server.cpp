#include <boost/algorithm/clamp.hpp>                      // for clamp
#include <math.h>                                         // for fabs
#include <stdarg.h>                                       // for va_arg, va_end
#include <string.h>                                       // for strcmp, memset
#include <algorithm>                                      // for max, min
#include <memory>                                         // for __shared_ptr

#include "enet/enet.h"                                    // for ENetPacket
#include "enet/types.h"                                   // for enet_uint32
#include "inexor/engine/worldio.hpp"                      // for loadents
#include "inexor/fpsgame/entities.hpp"                    // for delayspawn
#include "inexor/fpsgame/fpsstate.hpp"                    // for ::AI_NONE
#include "inexor/fpsgame/guns.hpp"                        // for guninfo, guns
#include "inexor/fpsgame/teaminfo.hpp"                    // for teaminfo
#include "inexor/gamemode/gamemode.hpp"                   // for m_teammode
#include "inexor/io/Logging.hpp"                          // for Log, Logger
#include "inexor/io/legacy/stream.hpp"                    // for opentempfile
#include "inexor/network/SharedVar.hpp"                   // for SharedVar
#include "inexor/network/legacy/administration.hpp"       // for ::PRIV_ADMIN
#include "inexor/network/legacy/buffer_types.hpp"         // for packetbuf
#include "inexor/network/legacy/cube_network.hpp"         // for putint, getint
#include "inexor/network/legacy/game_types.hpp"           // for ::N_SERVMSG
#include "inexor/server/client_management.hpp"            // for clientinfo
#include "inexor/server/demos.hpp"                        // for enddemorecord
#include "inexor/server/game_management.hpp"              // for pausegame
#include "inexor/server/gamemode/bomb_server.hpp"         // for bombservermode
#include "inexor/server/gamemode/capture_server.hpp"      // for captureserv...
#include "inexor/server/gamemode/collect_server.hpp"      // for collectserv...
#include "inexor/server/gamemode/ctf_server.hpp"          // for ctfservermode
#include "inexor/server/gamemode/gamemode_server.hpp"     // for servmode
#include "inexor/server/gamemode/hideandseek_server.hpp"  // for hideandseek...
#include "inexor/server/network_send.hpp"                 // for sendf, send...
#include "inexor/server/network.hpp"
#include "inexor/server/extinfo.hpp"
#include "inexor/shared/command.hpp"                      // for explodelist
#include "inexor/shared/cube_endian.hpp"                  // for lilswap
#include "inexor/shared/cube_formatting.hpp"              // for formatstring
#include "inexor/shared/cube_hash.hpp"                    // for hashset
#include "inexor/shared/cube_loops.hpp"                   // for i, loopv, j
#include "inexor/shared/cube_tools.hpp"                   // for copystring
#include "inexor/shared/cube_types.hpp"                   // for uchar, string
#include "inexor/shared/cube_vector.hpp"                  // for vector
#include "inexor/shared/ents.hpp"                         // for server_entity
#include "inexor/shared/geom.hpp"                         // for vec, vec::(...
#include "inexor/shared/iengine.hpp"                      // for sendserveri...
#include "inexor/shared/tools.hpp"                        // for max, rnd
#include "inexor/util/legacy_time.hpp"                    // for gamemillis

namespace server
{
    // TODO move:
extern void flushserver(bool force);
extern int getservermtu();


    int gamelimit = 0, nextexceeded = 0, gamespeed = 100;
    bool shouldstep = true;
    int interm = 0;

    /// Whether team reshuffeling after map change is disabled.
    /// TODO move this to a better sourcefile.
    bool teamspersisted;
    enet_uint32 lastsend = 0;
    stream *mapdata = nullptr;

    string smapname = "";

    bool notgotitems = true;        // true when map has changed and waiting for clients to send item
    struct maprotation
    {
        static int exclude;
        long modes;
        string map;

        long calcmodemask() const { return modes&((long)1<<NUMGAMEMODES) ? modes & ~exclude : modes; }
        bool hasmode(int mode, int offset = 0) const { return (calcmodemask() & (1 << (mode-offset))) != 0; }

        int findmode(int mode) const
        {
            if(!hasmode(mode)) loopi(NUMGAMEMODES) if(hasmode(i, 0)) return i;
            return mode;
        }

        bool match(int reqmode, const char *reqmap) const
        {
            return hasmode(reqmode) && (!map[0] || !reqmap[0] || !strcmp(map, reqmap));
        }

        bool includes(const maprotation &rot) const
        {
            return rot.modes == modes ? rot.map[0] && !map[0] : (rot.modes & modes) == rot.modes;
        }
    };
    int maprotation::exclude = 0;
    vector<maprotation> maprotations;
    int curmaprotation = 0;

    VAR(lockmaprotation, 0, 0, 2);

    void maprotationreset()
    {
        maprotations.setsize(0);
        curmaprotation = 0;
        maprotation::exclude = 0;
    }

    void nextmaprotation()
    {
        curmaprotation++;
        if(maprotations.inrange(curmaprotation) && maprotations[curmaprotation].modes) return;
        do curmaprotation--;
        while(maprotations.inrange(curmaprotation) && maprotations[curmaprotation].modes);
        curmaprotation++;
    }

    int findmaprotation(int mode, const char *map)
    {
        for(int i = max(curmaprotation, 0); i < maprotations.length(); i++)
        {
            maprotation &rot = maprotations[i];
            if(!rot.modes) break;
            if(rot.match(mode, map)) return i;
        }
        int start;
        for(start = max(curmaprotation, 0) - 1; start >= 0; start--) if(!maprotations[start].modes) break;
        start++;
        for(int i = start; i < curmaprotation; i++)
        {
            maprotation &rot = maprotations[i];
            if(!rot.modes) break;
            if(rot.match(mode, map)) return i;
        }
        int best = -1;
        loopv(maprotations)
        {
            maprotation &rot = maprotations[i];
            if(rot.match(mode, map) && (best < 0 || maprotations[best].includes(rot))) best = i;
        }
        return best;
    }

    bool addmaprotation(int modemask, const char *map)
    {
        if(!map[0]) loopk(NUMGAMEMODES) if(modemask&(1<<k) && !m_check(k, M_EDIT)) modemask &= ~(1<<k);
        if(!modemask) return false;
        if(!(modemask&((long)1<<NUMGAMEMODES))) maprotation::exclude |= modemask;
        maprotation &rot = maprotations.add();
        rot.modes = modemask;
        copystring(rot.map, map);
        return true;
    }

    void addmaprotations(tagval *args, int numargs)
    {
        vector<char *> modes, maps;
        for(int i = 0; i + 1 < numargs; i += 2)
        {
            explodelist(args[i].getstr(), modes);
            explodelist(args[i+1].getstr(), maps);
            int modemask = genmodemask(modes);
            if(maps.length()) loopvj(maps) addmaprotation(modemask, maps[j]);
            else addmaprotation(modemask, "");
            modes.deletearrays();
            maps.deletearrays();
        }
        if(maprotations.length() && maprotations.last().modes)
        {
            maprotation &rot = maprotations.add();
            rot.modes = 0;
            rot.map[0] = '\0';
        }
    }

    COMMAND(maprotationreset, "");
    COMMANDN(maprotation, addmaprotations, "ss2V");

// rights managment (again) and config
    VAR(restrictdemos, 0, 1, 1);
    VAR(restrictpausegame, 0, 0, 1);
    VAR(restrictgamespeed, 0, 1, 1);
    VAR(restrictpersistteams, 0, 0, 1);

    SVAR(serverdesc, "");
    SVAR(servermotd, "");
    VAR(spectatemodifiedmap, 0, 1, 1);

// clients: teamkillkick
    struct teamkillkick
    {
        int modes, limit, ban;

        bool match(int mode) const
        {
            return (modes&(1<<(mode)))!=0;
        }

        bool includes(const teamkillkick &tk) const
        {
            return tk.modes != modes && (tk.modes & modes) == tk.modes;
        }
    };
    vector<teamkillkick> teamkillkicks;

    void teamkillkickreset()
    {
        teamkillkicks.setsize(0);
    }

    void addteamkillkick(char *modestr, int *limit, int *ban)
    {
        vector<char *> modes;
        explodelist(modestr, modes);
        teamkillkick &kick = teamkillkicks.add();
        kick.modes = genmodemask(modes);
        kick.limit = *limit;
        kick.ban = *ban > 0 ? *ban*60000 : (*ban < 0 ? 0 : 30*60000); 
        modes.deletearrays();
    }

    COMMAND(teamkillkickreset, "");
    COMMANDN(teamkillkick, addteamkillkick, "sii");

    struct teamkillinfo
    {
        uint ip;
        int teamkills;
    };
    vector<teamkillinfo> teamkills;
    bool shouldcheckteamkills = false;

    void addteamkill(clientinfo *actor, clientinfo *victim, int n)
    {
        if(!m_timed || actor->state.aitype != AI_NONE || actor->privilege || (victim && victim->state.aitype != AI_NONE)) return;
        shouldcheckteamkills = true;
        uint ip = getclientip(actor->clientnum);
        loopv(teamkills) if(teamkills[i].ip == ip) 
        { 
            teamkills[i].teamkills += n;
            return;
        }
        teamkillinfo &tk = teamkills.add();
        tk.ip = ip;
        tk.teamkills = n;
    }

    void checkteamkills()
    {
        teamkillkick *kick = nullptr;
        if(m_timed) loopv(teamkillkicks) if(teamkillkicks[i].match(gamemode) && (!kick || kick->includes(teamkillkicks[i])))
            kick = &teamkillkicks[i];
        if(kick) loopvrev(teamkills)
        {
            teamkillinfo &tk = teamkills[i];
            if(tk.teamkills >= kick->limit)
            {
                if(kick->ban > 0) addban(tk.ip, kick->ban);
                kickclients(tk.ip);
                teamkills.removeunordered(i);
            }
        }
        shouldcheckteamkills = false;
    }


    uint mcrc = 0;
    vector<entity> ments;
    vector<server_entity> sents;

    // entity & map
    void resetitems()
    {
        mcrc = 0;
        ments.setsize(0);
        sents.setsize(0);
        //cps.reset();
    }

    // config
    void serverinit()
    {
        smapname[0] = '\0';
        resetitems();
    }

    // gamemode

    captureservermode capturemode;
    ctfservermode ctfmode;
    collectservermode collectmode;
    bombservermode bombmode;
    hideandseekservermode hideandseekmode;

    servmode *smode = nullptr;

    // entities
    bool canspawnitem(int type) {
        if(m_bomb) return (type>=I_BOMBS && type<=I_BOMBDELAY);
        else return !m_noitems && (type>=I_SHELLS && type<=I_QUAD && (!m_noammo || type<I_SHELLS || type>I_CARTRIDGES));
    }

    int spawntime(int type)
    {
        int np = numclients(-1, true, false);
        np = np<3 ? 4 : (np>4 ? 2 : 3);         // spawn times are dependent on number of players
        int sec = 0;
        switch(type)
        {
            case I_SHELLS:
            case I_BULLETS:
            case I_ROCKETS:
            case I_ROUNDS:
            case I_GRENADES:
            case I_CARTRIDGES: sec = np*4; break;
            case I_BOMBS:
            case I_BOMBRADIUS: sec = np*9; break;
            case I_BOMBDELAY: sec = np*7; break;
            case I_HEALTH: sec = np*5; break;
            case I_GREENARMOUR: sec = 20; break;
            case I_YELLOWARMOUR: sec = 30; break;
            case I_BOOST: sec = 60; break;
            case I_QUAD: sec = 70; break;
        }
        return sec*1000;
    }
 
    bool pickup(int i, int sender)         // server side item pickup, acknowledge first client that gets it
    {
        if((m_timed && gamemillis>=gamelimit) || !sents.inrange(i) || !sents[i].spawned) return false;
        clientinfo *ci = get_client_info(sender);
        if(!ci || !ci->state.canpickup(sents[i].type)) return false;
        sents[i].spawned = false;
        sents[i].spawntime = spawntime(sents[i].type);
        sendf(-1, 1, "ri3", N_ITEMACC, i, sender);
        ci->state.pickup(sents[i].type);
        return true;
    }

    // team managment
    hashset<teaminfo> teaminfos;

    void clearteaminfo()
    {
        teaminfos.clear();
    }

    bool teamhasplayers(const char *team) { loopv(clients) if(!strcmp(clients[i]->team, team)) return true; return false; }

    bool pruneteaminfo()
    {
        int oldteams = teaminfos.numelems;
        enumerate(teaminfos, teaminfo, old,
            if(!old.frags && !teamhasplayers(old.team)) teaminfos.remove(old.team);
        );
        return teaminfos.numelems < oldteams;
    }

    teaminfo *addteaminfo(const char *team)
    {
        teaminfo *t = teaminfos.access(team);
        if(!t)
        {
            if(teaminfos.numelems >= MAXTEAMS && !pruneteaminfo()) return nullptr;
            t = &teaminfos[team];
            copystring(t->team, team, sizeof(t->team));
            t->frags = 0;
        }
        return t;
    }

    clientinfo *choosebestclient(float &bestrank)
    {
        clientinfo *best = nullptr;
        bestrank = -1;
        loopv(clients)
        {
            clientinfo *ci = clients[i];
            if(ci->state.timeplayed<0) continue;
            float rank = ci->state.state!=CS_SPECTATOR ? ci->state.effectiveness/max(ci->state.timeplayed, 1) : -1;
            if(!best || rank > bestrank) { best = ci; bestrank = rank; }
        }
        return best;
    }

    void autoteam()
    {
        static const char * const teamnames[2] = {"good", "evil"};
        vector<clientinfo *> team[2];
        float teamrank[2] = {0, 0};
        for(int round = 0, remaining = clients.length(); remaining>=0; round++)
        {
            int first = round&1, second = (round+1)&1, selected = 0;
            while(teamrank[first] <= teamrank[second])
            {
                float rank;
                clientinfo *ci = choosebestclient(rank);
                if(!ci) break;
                if(smode && smode->hidefrags()) rank = 1;
                else if(selected && rank<=0) break;
                ci->state.timeplayed = -1;
                team[first].add(ci);
                if(rank>0) teamrank[first] += rank;
                selected++;
                if(rank<=0) break;
            }
            if(!selected) break;
            remaining -= selected;
        }
        loopi(sizeof(team)/sizeof(team[0]))
        {
            addteaminfo(teamnames[i]);
            loopvj(team[i])
            {
                clientinfo *ci = team[i][j];
                if(!strcmp(ci->team, teamnames[i])) continue;
                copystring(ci->team, teamnames[i], MAXTEAMLEN+1);
                sendf(-1, 1, "riisi", N_SETTEAM, ci->clientnum, teamnames[i], -1);
            }
        }
    }

    struct teamrank
    {
        const char *name;
        float rank;
        int clients;

        teamrank(const char *name) : name(name), rank(0), clients(0) {}
    };

    const char *chooseworstteam(const char *suggest = nullptr, clientinfo *exclude = nullptr)
    {
        teamrank teamranks[2] = { teamrank("good"), teamrank("evil") };
        const int numteams = sizeof(teamranks)/sizeof(teamranks[0]);
        loopv(clients)
        {
            clientinfo *ci = clients[i];
            if(ci==exclude || ci->state.aitype!=AI_NONE || ci->state.state==CS_SPECTATOR || !ci->team[0]) continue;
            ci->state.timeplayed += lastmillis - ci->state.lasttimeplayed;
            ci->state.lasttimeplayed = lastmillis;

            loopj(numteams) if(!strcmp(ci->team, teamranks[j].name))
            {
                teamrank &ts = teamranks[j];
                ts.rank += ci->state.effectiveness/max(ci->state.timeplayed, 1);
                ts.clients++;
                break;
            }
        }
        teamrank *worst = &teamranks[numteams-1];
        loopi(numteams-1)
        {
            teamrank &ts = teamranks[i];
            if(smode && smode->hidefrags())
            {
                if(ts.clients < worst->clients || (ts.clients == worst->clients && ts.rank < worst->rank)) worst = &ts;
            }
            else if(ts.rank < worst->rank || (ts.rank == worst->rank && ts.clients < worst->clients)) worst = &ts;
        }
        return worst->name;
    }

    void assign_team(clientinfo *ci)
    {
        const char *worst = m_teammode ? chooseworstteam(nullptr, ci) : nullptr;
        copystring(ci->team, worst ? worst : "good", MAXTEAMLEN+1);
    }

    static void freegetmap(ENetPacket *packet)
    {
        loopv(clients)
        {
            clientinfo *ci = clients[i];
            if(ci->getmap == packet) ci->getmap = nullptr;
        }
    }

    /// If no player with a high enough privilege level is on the server, resume the game.
    /// (Otherwise servers could get locked after an admin disconnected).
    void checkpausegame()
    {
        if(!gamepaused) return;
        int admins = 0;
        loopv(clients) if(clients[i]->privilege >= (restrictpausegame ? PRIV_ADMIN : PRIV_MASTER)) admins++;
        if(!admins) pausegame(false);
    }

    bool ispaused() { return gamepaused; }

    void changegamespeed(int val, clientinfo *ci = nullptr)
    {
        val = clamp(val, 10, 1000);
        if(gamespeed==val) return;
        gamespeed = val;
        sendf(-1, 1, "riii", N_GAMESPEED, gamespeed, ci ? ci->clientnum : -1);
    }


    // team managment
    void persistteams(bool val)
    {
        if(teamspersisted==val) return;
        teamspersisted = val;
        sendf(-1, 1, "rii", N_PERSISTTEAMS, teamspersisted ? 1 : 0);
    }

    void checkpersistteams()
    {
        if(!teamspersisted) return;
        int admins = 0;
        loopv(clients) if(clients[i]->privilege >= (restrictpersistteams ? PRIV_ADMIN : PRIV_MASTER)) admins++;
        if(!admins) persistteams(false); //if no admins or (or masters) around, reshuffle teams again
    }

    // network

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
    } msgfilter(-1, N_CONNECT, N_SERVINFO, N_INITCLIENT, N_WELCOME, N_MAPCHANGE, N_SERVMSG, N_DAMAGE, N_HITPUSH, N_SHOTFX, N_EXPLODEFX, N_DIED, N_SPAWNSTATE, N_FORCEDEATH, N_TEAMINFO, N_ITEMACC, N_ITEMSPAWN, N_TIMEUP, N_CDIS, N_CURRENTMASTER, N_PONG, N_RESUME, N_BASESCORE, N_BASEINFO, N_BASEREGEN, N_ANNOUNCE, N_SENDDEMOLIST, N_SENDDEMO, N_DEMOPLAYBACK, N_SENDMAP, N_DROPFLAG, N_SCOREFLAG, N_RETURNFLAG, N_RESETFLAG, N_INVISFLAG, N_CLIENT,  N_INITAI, N_EXPIRETOKENS, N_DROPTOKENS, N_STEALTOKENS, N_DEMOPACKET,
                -2, N_REMIP, N_NEWMAP, N_GETMAP, N_SENDMAP, N_CLIPBOARD,
                -3, N_EDITENT, N_EDITF, N_EDITT, N_EDITM, N_FLIP, N_COPY, N_PASTE, N_ROTATE, N_REPLACE, N_DELCUBE, N_EDITVAR, N_EDITVSLOT, N_UNDO, N_REDO,
                -4, N_POS, NUMMSG);

    int checktype(int type, clientinfo *ci)
    {
        if(ci)
        {
            if(!ci->connected) switch(type)
            {
                // always allow
                case N_CONNECT: return N_CONNECT;
                case N_PING: return N_PING;
                    // never allow
                default: return -1;
            }
        }
        switch(msgfilter[type])
        {
            // server-only messages
            case 1: return ci ? -1 : type;
                // only allowed in coop-edit
            case 2: if(m_edit) break; return -1;
                // only allowed in coop-edit, no overflow check
            case 3: return m_edit ? type : -1;
                // no overflow check
            case 4: return type;
        }
        if(ci && ++ci->overflow >= 200) return -2;
        return type;
    }
    // worldstate
    struct worldstate
    {
        int uses, len;
        uchar *data;

        worldstate() : uses(0), len(0), data(nullptr) {}

        void setup(int n) { len = n; data = new uchar[n]; }
        void cleanup() { DELETEA(data); len = 0; }
        bool contains(const uchar *p) const { return p >= data && p < &data[len]; }
    };
    vector<worldstate> worldstates;
    bool reliablemessages = false;

    void cleanworldstate(ENetPacket *packet)
    {
        loopv(worldstates)
        {
            worldstate &ws = worldstates[i];
            if(!ws.contains(packet->data)) continue;
            ws.uses--;
            if(ws.uses <= 0)
            {
                ws.cleanup();
                worldstates.removeunordered(i);
            }
            break;
        }
    }

    void flushclientposition(clientinfo &ci)
    {
        if(ci.position.empty() || (!has_clients() && !demorecord)) return;
        packetbuf p(ci.position.length(), 0);
        p.put(ci.position.getbuf(), ci.position.length());
        ci.position.setsize(0);
        sendpacket(-1, 0, p.finalize(), ci.ownernum);
    }

    static void sendpositions(worldstate &ws, ucharbuf &wsbuf)
    {
        if(wsbuf.empty()) return;
        int wslen = wsbuf.length();
        recordpacket(0, wsbuf.buf, wslen);
        wsbuf.put(wsbuf.buf, wslen);
        loopv(clients)
        {
            clientinfo &ci = *clients[i];
            if(ci.state.aitype != AI_NONE) continue;
            uchar *data = wsbuf.buf;
            int size = wslen;
            if(ci.wsdata >= wsbuf.buf) { data = ci.wsdata + ci.wslen; size -= ci.wslen; }
            if(size <= 0) continue;
            ENetPacket *packet = enet_packet_create(data, size, ENET_PACKET_FLAG_NO_ALLOCATE);
            sendpacket(ci.clientnum, 0, packet);
            if(packet->referenceCount) { ws.uses++; packet->freeCallback = cleanworldstate; }
            else enet_packet_destroy(packet);
        }
        wsbuf.offset(wsbuf.length());
    }

    static inline void addposition(worldstate &ws, ucharbuf &wsbuf, int mtu, clientinfo &bi, clientinfo &ci)
    {
        if(bi.position.empty()) return;
        if(wsbuf.length() + bi.position.length() > mtu) sendpositions(ws, wsbuf);
        int offset = wsbuf.length();
        wsbuf.put(bi.position.getbuf(), bi.position.length());
        bi.position.setsize(0);
        int len = wsbuf.length() - offset;
        if(ci.wsdata < wsbuf.buf) { ci.wsdata = &wsbuf.buf[offset]; ci.wslen = len; }
        else ci.wslen += len;
    }

    static void sendmessages(worldstate &ws, ucharbuf &wsbuf)
    {
        if(wsbuf.empty()) return;
        int wslen = wsbuf.length();
        recordpacket(1, wsbuf.buf, wslen);
        wsbuf.put(wsbuf.buf, wslen);
        loopv(clients)
        {
            clientinfo &ci = *clients[i];
            if(ci.state.aitype != AI_NONE) continue;
            uchar *data = wsbuf.buf;
            int size = wslen;
            if(ci.wsdata >= wsbuf.buf) { data = ci.wsdata + ci.wslen; size -= ci.wslen; }
            if(size <= 0) continue;
            ENetPacket *packet = enet_packet_create(data, size, (reliablemessages ? ENET_PACKET_FLAG_RELIABLE : 0) | ENET_PACKET_FLAG_NO_ALLOCATE);
            sendpacket(ci.clientnum, 1, packet);
            if(packet->referenceCount) { ws.uses++; packet->freeCallback = cleanworldstate; }
            else enet_packet_destroy(packet);
        }
        wsbuf.offset(wsbuf.length());
    }

    static inline void addmessages(worldstate &ws, ucharbuf &wsbuf, int mtu, clientinfo &bi, clientinfo &ci)
    {
        if(bi.messages.empty()) return;
        if(wsbuf.length() + 10 + bi.messages.length() > mtu) sendmessages(ws, wsbuf);
        int offset = wsbuf.length();
        putint(wsbuf, N_CLIENT);
        putint(wsbuf, bi.clientnum);
        putuint(wsbuf, bi.messages.length());
        wsbuf.put(bi.messages.getbuf(), bi.messages.length());
        bi.messages.setsize(0);
        int len = wsbuf.length() - offset;
        if(ci.wsdata < wsbuf.buf) { ci.wsdata = &wsbuf.buf[offset]; ci.wslen = len; }
        else ci.wslen += len;
    }

    bool buildworldstate()
    {
        int wsmax = 0;
        loopv(clients)
        {
            clientinfo &ci = *clients[i];
            ci.overflow = 0;
            ci.wsdata = nullptr;
            wsmax += ci.position.length();
            if(ci.messages.length()) wsmax += 10 + ci.messages.length();
        }
        if(wsmax <= 0)
        {
            reliablemessages = false;
            return false;
        }
        worldstate &ws = worldstates.add();
        ws.setup(2*wsmax);
        int mtu = getservermtu() - 100;
        if(mtu <= 0) mtu = ws.len;
        ucharbuf wsbuf(ws.data, ws.len);
        loopv(clients)
        {
            clientinfo &ci = *clients[i];
            if(ci.state.aitype != AI_NONE) continue;
            addposition(ws, wsbuf, mtu, ci, ci);
            loopvj(ci.bots) addposition(ws, wsbuf, mtu, *ci.bots[j], ci);
        }
        sendpositions(ws, wsbuf);
        loopv(clients)
        {
            clientinfo &ci = *clients[i];
            if(ci.state.aitype != AI_NONE) continue;
            addmessages(ws, wsbuf, mtu, ci, ci);
            loopvj(ci.bots) addmessages(ws, wsbuf, mtu, *ci.bots[j], ci);
        }
        sendmessages(ws, wsbuf);
        reliablemessages = false;
        if(ws.uses) return true;
        ws.cleanup();
        worldstates.drop();
        return false;
    }

    bool sendpackets(bool force)
    {
        if(clients.empty() || (!has_clients() && !demorecord)) return false;
        enet_uint32 curtime = enet_time_get()-lastsend;
        if(curtime<33 && !force) return false;
        bool flush = buildworldstate();
        lastsend += curtime - (curtime%33);
        return flush;
    }

    // gamemode
    template<class T>
    void sendstate(gamestate &gs, T &p)
    {
        putint(p, gs.lifesequence);
        putint(p, gs.health);
        putint(p, gs.maxhealth);
        putint(p, gs.armour);
        putint(p, gs.armourtype);
        putint(p, gs.gunselect);
        // TODO: gs.bombdelay gs.bombradius
        loopi(GUN_PISTOL-GUN_SG+1) putint(p, gs.ammo[GUN_SG+i]);
    }

    void spawnstate(clientinfo *ci)
    {
        gamestate &gs = ci->state;
        gs.spawnstate(gamemode);
        gs.lifesequence = (gs.lifesequence + 1)&0x7F;
    }

    void sendspawn(clientinfo *ci)
    {
        gamestate &gs = ci->state;
        spawnstate(ci);
        sendf(ci->ownernum, 1, "rii7v", N_SPAWNSTATE, ci->clientnum, gs.lifesequence,
            gs.health, gs.maxhealth,
            gs.armour, gs.armourtype,
            gs.gunselect, GUN_PISTOL-GUN_SG+1, &gs.ammo[GUN_SG]);
        gs.lastspawn = gamemillis;
    }

    // client managment

    int welcomepacket(packetbuf &p, clientinfo *ci);
    void sendwelcome(clientinfo *ci)
    {
        packetbuf p(MAXTRANS, ENET_PACKET_FLAG_RELIABLE);
        int chan = welcomepacket(p, ci);
        sendpacket(ci->clientnum, chan, p.finalize());
    }

    void putinitclient(clientinfo *ci, packetbuf &p)
    {
        if(ci->state.aitype != AI_NONE)
        {
            putint(p, N_INITAI);
            putint(p, ci->clientnum);
            putint(p, ci->ownernum);
            putint(p, ci->state.aitype);
            putint(p, ci->state.skill);
            putint(p, ci->playermodel);
            sendstring(ci->name, p);
            sendstring(ci->team, p);
            sendstring(BOTTAG, p);
        }
        else
        {
            putint(p, N_INITCLIENT);
            putint(p, ci->clientnum);
            sendstring(ci->name, p);
            sendstring(ci->team, p);
            sendstring(ci->tag, p);
            putint(p, ci->playermodel);
            putint(p, ci->fov);
        }
    }

    void welcomeinitclient(packetbuf &p, int exclude = -1)
    {
        loopv(clients)
        {
            clientinfo *ci = clients[i];
            if(!ci->connected || ci->clientnum == exclude) continue;

            putinitclient(ci, p);
        }
    }

    // client managment | gamemode | network
    int welcomepacket(packetbuf &p, clientinfo *ci)
    {
        putint(p, N_WELCOME);
        putint(p, N_MAPCHANGE);
        sendstring(smapname, p);
        putint(p, gamemode);
        putint(p, notgotitems ? 1 : 0);
        if(!ci || (m_timed && smapname[0]))
        {
            putint(p, N_TIMEUP);
            putint(p, gamemillis < gamelimit && !interm ? max((gamelimit - gamemillis)/1000, 1) : 0);
        }
        if(!notgotitems)
        {
            putint(p, N_ITEMLIST);
            loopv(sents) if(sents[i].spawned)
            {
                putint(p, i);
                putint(p, sents[i].type);
            }
            putint(p, -1);
        }
        bool hasmaster = false;
        if(mastermode != MM_OPEN)
        {
            putint(p, N_CURRENTMASTER);
            putint(p, mastermode);
            hasmaster = true;
        }
        loopv(clients) if(clients[i]->privilege >= PRIV_MASTER)
        {
            if(!hasmaster)
            {
                putint(p, N_CURRENTMASTER);
                putint(p, mastermode);
                hasmaster = true;
            }
            putint(p, clients[i]->clientnum);
            putint(p, clients[i]->privilege);
        }
        if(hasmaster) putint(p, -1);
        if(gamepaused)
        {
            putint(p, N_PAUSEGAME);
            putint(p, 1);
            putint(p, -1);
        }
        if(gamespeed != 100)
        {
            putint(p, N_GAMESPEED);
            putint(p, gamespeed);
            putint(p, -1);
        }
        if(m_teammode)
        {
            putint(p, N_TEAMINFO);
            enumerate(teaminfos, teaminfo, t,
                if(t.frags) { sendstring(t.team, p); putint(p, t.frags); }
            );
            sendstring("", p);
        } 
        if(ci)
        {
            putint(p, N_SETTEAM);
            putint(p, ci->clientnum);
            sendstring(ci->team, p);
            putint(p, -1);
        }
        if(ci && ci->state.state!=CS_SPECTATOR)
        {
            if(smode && !smode->canspawn(ci, true))
            {
                ci->state.state = CS_DEAD;
                putint(p, N_FORCEDEATH);
                putint(p, ci->clientnum);
                sendf(-1, 1, "ri2x", N_FORCEDEATH, ci->clientnum, ci->clientnum);
            }
            else
            {
                gamestate &gs = ci->state;
                spawnstate(ci);
                putint(p, N_SPAWNSTATE);
                putint(p, ci->clientnum);
                sendstate(gs, p);
                gs.lastspawn = gamemillis;
            }
        }
        if(ci && ci->state.state==CS_SPECTATOR)
        {
            putint(p, N_SPECTATOR);
            putint(p, ci->clientnum);
            putint(p, 1);
            sendf(-1, 1, "ri3x", N_SPECTATOR, ci->clientnum, 1, ci->clientnum);
        }
        if(!ci || clients.length()>1)
        {
            putint(p, N_RESUME);
            loopv(clients)
            {
                clientinfo *oi = clients[i];
                if(ci && oi->clientnum==ci->clientnum) continue;
                putint(p, oi->clientnum);
                putint(p, oi->state.state);
                putint(p, oi->state.frags);
                putint(p, oi->state.flags);
                putint(p, oi->state.quadmillis);
                putint(p, oi->state.deaths);
                putint(p, oi->state.teamkills);
                putint(p, oi->state.damage);
                putint(p, oi->state.shotdamage);
                sendstate(oi->state, p);
            }
            putint(p, -1);
            welcomeinitclient(p, ci ? ci->clientnum : -1);
        }
        if(smode) smode->initclient(ci, p, true);
        return 1;
    }

    // gamestate

    void sendinitclient(clientinfo *ci)
    {
        packetbuf p(MAXTRANS, ENET_PACKET_FLAG_RELIABLE);
        putinitclient(ci, p);
        sendpacket(-1, 1, p.finalize(), ci->clientnum);
    }

    void loaditems()
    {
        resetitems();
        notgotitems = true;
        if(m_edit || !loadents(smapname, ments, &mcrc))
            return;
        loopv(ments) if(canspawnitem(ments[i].type))
        {
            server_entity se = { NOTUSED, 0, false };
            while(sents.length()<=i) sents.add(se);
            sents[i].type = ments[i].type;
            if(entities::delayspawn(sents[i].type)) sents[i].spawntime = spawntime(sents[i].type);
            else sents[i].spawned = true;
        }
        notgotitems = false;
    }
    // map managment
    void changemap(const char *s, int mode)
    {
        stopdemo();
        pausegame(false);
        changegamespeed(100);
        if(smode) smode->cleanup();
        aiman::clearai();

        gamemode = mode;
        gamemillis = 0;
        gamelimit = (m_overtime ? 15 : 10)*60000;
        interm = 0;
        nextexceeded = 0;
        copystring(smapname, s);
        loaditems();
        resetdisconnectedplayerscores();
        shouldcheckteamkills = false;
        teamkills.shrink(0);
        loopv(clients)
        {
            clientinfo *ci = clients[i];
            ci->state.timeplayed += lastmillis - ci->state.lasttimeplayed;
        }

        sendf(-1, 1, "risii", N_MAPCHANGE, smapname, gamemode, 1);

        clearteaminfo();
        if(m_teammode && !teamspersisted) autoteam();

        if(m_capture) smode = &capturemode;
        else if(m_ctf) smode = &ctfmode;
        else if(m_collect) smode = &collectmode;
        else if(m_bomb) smode = &bombmode;
        else if(m_hideandseek) smode = &hideandseekmode;
        else smode = nullptr;

        if(m_timed && smapname[0]) sendf(-1, 1, "ri2", N_TIMEUP, gamemillis < gamelimit && !interm ? max((gamelimit - gamemillis)/1000, 1) : 0);
        loopv(clients)
        {
            clientinfo *ci = clients[i];
            ci->mapchange();
            ci->state.lasttimeplayed = lastmillis;
            if(m_bomb) ci->state.setbackupweapon(GUN_BOMB);
        }

        aiman::changemap();

        if(m_demo)
        {
            if(clients.length()) setupdemoplayback();
        }
        else if(demonextmatch)
        {
            demonextmatch = false;
            setupdemorecord();
        }

        if(smode)
        {
            smode->setup();
            loopv(clients)
            {
                clientinfo* ci = clients[i];
                if(ci->state.state == CS_SPECTATOR) continue;
                if(!smode->canspawn(ci))
                {
                    ci->state.state = CS_DEAD;
                    sendf(-1, 1, "ri2", N_FORCEDEATH, ci->clientnum);
                } else sendspawn(ci);
            }
        }
        else
        {
            loopv(clients)
            {
                clientinfo *ci = clients[i];
                if(ci->state.state!=CS_SPECTATOR) sendspawn(ci);
            }
        }
    }

    ICOMMAND(mapmode, "si", (char *name, int *mode), changemap(name, *mode));

    void rotatemap(bool next)
    {
        if(!maprotations.inrange(curmaprotation))
        {
            changemap("", 1);
            return;
        }
        if(next)
        {
            curmaprotation = findmaprotation(gamemode, smapname);
            if(curmaprotation >= 0) nextmaprotation();
            else curmaprotation = smapname[0] ? max(findmaprotation(gamemode, ""), 0) : 0;
        }
        maprotation &rot = maprotations[curmaprotation];
        changemap(rot.map, rot.findmode(gamemode));
    }

    bool hasmap(clientinfo *ci)
    {
        return (m_edit && clients.length() > 0) ||
               (smapname[0] && (!m_timed || gamemillis < gamelimit || (ci->state.state==CS_SPECTATOR && !ci->privilege) || numclients(ci->clientnum, true, true, true)));
    }

    void choosemap(clientinfo *ci)
    {
        if(!hasmap(ci)) rotatemap(false);
    }

    struct votecount
    {
        char *map;
        int mode, count;
        votecount() {}
        votecount(char *s, int n) : map(s), mode(n), count(0) {}
    };

    void checkvotes(bool force = false)
    {
        vector<votecount> votes;
        int maxvotes = 0;
        loopv(clients)
        {
            clientinfo *oi = clients[i];
            if(oi->state.state==CS_SPECTATOR && !oi->privilege) continue;
            if(oi->state.aitype!=AI_NONE) continue;
            maxvotes++;
            if(!m_valid(oi->modevote)) continue;
            votecount *vc = nullptr;
            loopvj(votes) if(!strcmp(oi->mapvote, votes[j].map) && oi->modevote==votes[j].mode)
            {
                vc = &votes[j];
                break;
            }
            if(!vc) vc = &votes.add(votecount(oi->mapvote, oi->modevote));
            vc->count++;
        }
        votecount *best = nullptr;
        loopv(votes) if(!best || votes[i].count > best->count || (votes[i].count == best->count && rnd(2))) best = &votes[i];
        if(force || (best && best->count > maxvotes/2))
        {
            if(demorecord) enddemorecord();
            if(best && (best->count > (force ? 1 : maxvotes/2)))
            {
                sendservmsg(force ? "vote passed by default" : "vote passed by majority");
                changemap(best->map, best->mode);
            }
            else rotatemap(true);
        }
    }

    void vote(const char *map, int reqmode, int sender)
    {
        clientinfo *ci = get_client_info(sender);
        if(!ci || (ci->state.state==CS_SPECTATOR && !ci->privilege)) return;
        if(!m_valid(reqmode)) return;
        if(!map[0] && !m_check(reqmode, M_EDIT))
        {
            int idx = findmaprotation(reqmode, smapname);
            if(idx < 0 && smapname[0]) idx = findmaprotation(reqmode, "");
            if(idx < 0) return;
            map = maprotations[idx].map;
        }
        if(lockmaprotation && ci->privilege < (lockmaprotation > 1 ? PRIV_ADMIN : PRIV_MASTER) && findmaprotation(reqmode, map) < 0)
        {
            sendf(sender, 1, "ris", N_SERVMSG, "This server has locked the map rotation.");
            return;
        }
        copystring(ci->mapvote, map);
        ci->modevote = reqmode;
        if(ci->privilege && mastermode>=MM_VETO)
        {
            if(demorecord) enddemorecord();
            if(has_clients()>1)
                sendservmsgf("%s forced %s on map %s", colorname(ci), modename(ci->modevote), ci->mapvote[0] ? ci->mapvote : "[new map]");
            changemap(ci->mapvote, ci->modevote);
        }
        else
        {
            sendservmsgf("%s suggests %s on map %s (select map to vote)", colorname(ci), modename(reqmode), map[0] ? map : "[new map]");
            checkvotes();
        }
    }

    // gamemode
    void checkintermission()
    {
        if(gamemillis >= gamelimit && !interm && !m_timeforward)
        {
            sendf(-1, 1, "ri2", N_TIMEUP, 0);
            if(smode) smode->intermission();
            changegamespeed(100);
            interm = gamemillis + 10000;
        }
    }

    void startintermission()
    {
        gamelimit = min(gamelimit, gamemillis);
        checkintermission();
    }


    /// Checks if the game has ended because only one player is still alive.
    /// If yes it forces intermission.
    /// @note It does this by checking if less than 2 players have their state set to alive.
    ///       This means, the game will also end if someone is Lagging.
    void checklms()
    {
        if(m_teammode)
        {
            int teamsalive = 0;
            vector<teamscore> teams;
            for(int cn=0; cn<clients.length(); cn++)
            {
                bool found = false;
                for(int t=0; t<teams.length(); t++)
                {
                    if(strcmp(teams[t].team, clients[cn]->team) == 0)
                    {
                        found = true;
                        if(clients[cn]->state.state == CS_ALIVE) teams[t].score++;
                        break;
                    }
                }
                if(!found) teams.add(teamscore(clients[cn]->team, (clients[cn]->state.state == CS_ALIVE) ? 1 : 0));
            }
            for(int t=0; t<teams.length(); t++)
            {
                if(teams[t].score > 0) teamsalive++;
            }
            if(teamsalive < 2) startintermission();
        }
        else
        {
            int plalive = 0;  // Number of players still alive;
                              // n > 1 Means the game is still running;
                              // 1 means player x has won;
                              // n < 1 means that the game should end, but there is now winner (probatly only,
                              // if the two last players killed them self at the same time)
            int clfound = -1; // The index of the client whose player is last found as alive, the winner. IF $plalive == 1
                              // It is currently not used
            // Get player check if players are alive; if yes set plfound and increase plalive
            for (int clnum = 0; clnum < clients.length() && plalive < 2; clnum++)
                if (clients[clnum]->state.state == CS_ALIVE) {
                    plalive++;
                    clfound = clnum;
                }
            // Stop game if less than 2 players are alive
            if (plalive < 2) startintermission();
        }
    }

    // worldstate? physics?
    void dodamage(clientinfo *target, clientinfo *actor, int damage, int gun, const vec &hitpush = vec(0, 0, 0))
    {
        if (smode && !smode->canhit(target, actor)) return;
        gamestate &ts = target->state;
        ts.dodamage(damage);
        if(target!=actor && !isteam(target->team, actor->team)) actor->state.damage += damage;
        sendf(-1, 1, "ri6", N_DAMAGE, target->clientnum, actor->clientnum, damage, ts.armour, ts.health);
        if(target==actor) target->setpushed();
        else if(!hitpush.iszero())
        {
            ivec v(vec(hitpush).rescale(DNF));
            sendf(ts.health<=0 ? -1 : target->ownernum, 1, "ri7", N_HITPUSH, target->clientnum, gun, damage, v.x, v.y, v.z);
            target->setpushed();
        }
        if(ts.health<=0)
        {
            target->state.deaths++;
            int fragvalue = smode ? smode->fragvalue(target, actor) : (target==actor || isteam(target->team, actor->team) ? -1 : 1);
            actor->state.frags += fragvalue;
            if(fragvalue>0)
            {
                int friends = 0, enemies = 0; // note: friends also includes the fragger
                if(m_teammode) loopv(clients) if(strcmp(clients[i]->team, actor->team)) enemies++; else friends++;
                else { friends = 1; enemies = clients.length()-1; }
                actor->state.effectiveness += fragvalue*friends/float(max(enemies, 1));
            }
            teaminfo *t = m_teammode ? teaminfos.access(actor->team) : nullptr;
            if(t) t->frags += fragvalue; 
            sendf(-1, 1, "ri5", N_DIED, target->clientnum, actor->clientnum, actor->state.frags, t ? t->frags : 0);
            target->position.setsize(0);
            if(smode) smode->died(target, actor);
            ts.state = CS_DEAD;
            ts.lastdeath = gamemillis;

            if (m_lms) checklms(); // Last Man Standing
            else ts.respawn();

            if(actor!=target && isteam(actor->team, target->team)) 
            {
                actor->state.teamkills++;
                addteamkill(actor, target, 1);
            }

            ts.deadflush = ts.lastdeath + DEATHMILLIS;
            // don't issue respawn yet until DEATHMILLIS has elapsed
            // ts.respawn();
        }
    }

    void suicide(clientinfo *ci)
    {
        gamestate &gs = ci->state;
        if(gs.state!=CS_ALIVE && (gs.state==CS_SPECTATOR || gs.state==CS_EDITING)) return;
        int fragvalue = smode ? smode->fragvalue(ci, ci) : -1;
        ci->state.frags += fragvalue;
        ci->state.deaths++;
        teaminfo *t = m_teammode ? teaminfos.access(ci->team) : nullptr;
        if(t) t->frags += fragvalue;
        sendf(-1, 1, "ri5", N_DIED, ci->clientnum, ci->clientnum, gs.frags, t ? t->frags : 0);
        ci->position.setsize(0);
        if(smode) smode->died(ci, nullptr);
        gs.state = CS_DEAD;
        gs.lastdeath = gamemillis;
        gs.respawn();

        if (m_lms) checklms();
    }

    void suicideevent::process(clientinfo *ci)
    {
        suicide(ci);
    }

    void explodeevent::process(clientinfo *ci)
    {
        gamestate &gs = ci->state;
        switch(gun)
        {
            case GUN_RL:
                if(!gs.rockets.remove(id)) return;
                break;

            case GUN_GL:
                if(!gs.grenades.remove(id)) return;
                break;

            case GUN_BOMB:
                if(!gs.bombs.remove(id)) return;
                break;

            case GUN_SPLINTER:
                break;

            default:
                return;
        }
        sendf(-1, 1, "ri4x", N_EXPLODEFX, ci->clientnum, gun, id, ci->ownernum);
        if(gun==GUN_BOMB && ci->state.ammo[GUN_BOMB] < itemstats[GUN_BOMB].max) ci->state.ammo[GUN_BOMB]++; // add a bomb if the bomb explodes
        loopv(hits)
        {
            hitinfo &h = hits[i];
            clientinfo *target = get_client_info(h.target);
            if(!target || target->state.state!=CS_ALIVE || h.lifesequence!=target->state.lifesequence || h.dist<0 || h.dist>guns[gun].exprad) continue;

            bool dup = false;
            loopj(i) if(hits[j].target==h.target) { dup = true; break; }
            if(dup) continue;

            int damage = guns[gun].damage;
            if(gs.quadmillis) damage *= 4;
            damage = int(damage*(1-h.dist/EXP_DISTSCALE/guns[gun].exprad));
            if(target==ci) damage /= EXP_SELFDAMDIV;
            dodamage(target, ci, damage, gun, h.dir);
        }
    }

    void shotevent::process(clientinfo *ci)
    {
        gamestate &gs = ci->state;
        int wait = millis - gs.lastshot;
        if(gun!=GUN_SPLINTER &&
          (!gs.isalive(gamemillis) ||
           wait<gs.gunwait ||
           gun<GUN_FIST || gun>GUN_BOMB  ||
           gs.ammo[gun]<=0 || (guns[gun].range && from.dist(to) > guns[gun].range + 1)))
            return;
        if(gun!=GUN_FIST) gs.ammo[gun]--;
        gs.lastshot = millis;
        gs.gunwait = guns[gun].attackdelay;
        sendf(-1, 1, "rii9x", N_SHOTFX, ci->clientnum, gun, id,
                int(from.x*DMF), int(from.y*DMF), int(from.z*DMF),
                int(to.x*DMF), int(to.y*DMF), int(to.z*DMF),
                ci->ownernum);
        gs.shotdamage += guns[gun].damage*(gs.quadmillis ? 4 : 1)*guns[gun].rays;
        switch(gun)
        {
            case GUN_RL: gs.rockets.add(id); break;
            case GUN_GL: gs.grenades.add(id); break;
            case GUN_BOMB: gs.bombs.add(id); break;
            default:
            {
                int totalrays = 0, maxrays = guns[gun].rays;
                loopv(hits)
                {
                    hitinfo &h = hits[i];
                    clientinfo *target = get_client_info(h.target);
                    if(!target || target->state.state!=CS_ALIVE || h.lifesequence!=target->state.lifesequence || h.rays<1 || h.dist > guns[gun].range + 1) continue;

                    totalrays += h.rays;
                    if(totalrays>maxrays) continue;
                    int damage = h.rays*guns[gun].damage;
                    if(gs.quadmillis) damage *= 4;
                    dodamage(target, ci, damage, gun, h.dir);
                }
                break;
            }
        }
    }

    void pickupevent::process(clientinfo *ci)
    {
        gamestate &gs = ci->state;
        if(!gs.isalive(gamemillis)) return;
        pickup(ent, ci->clientnum);
    }

    bool gameevent::flush(clientinfo *ci, int fmillis)
    {
        process(ci);
        return true;
    }

    bool timedevent::flush(clientinfo *ci, int fmillis)
    {
        if(millis > fmillis) return false;
        else if(millis >= ci->lastevent)
        {
            ci->lastevent = millis;
            process(ci);
        }
        return true;
    }

    void clearevent(clientinfo *ci)
    {
        delete ci->events.remove(0);
    }

    void flushevents(clientinfo *ci, int millis)
    {
        while(ci->events.length())
        {
            gameevent *ev = ci->events[0];
            if(ev->flush(ci, millis)) clearevent(ci);
            else break;
        }
    }

    void processevents()
    {
        loopv(clients)
        {
            clientinfo *ci = clients[i];
            if(curtime>0 && ci->state.quadmillis) ci->state.quadmillis = max(ci->state.quadmillis-curtime, 0);
            flushevents(ci, gamemillis);
        }
    }

    void cleartimedevents(clientinfo *ci)
    {
        int keep = 0;
        loopv(ci->events)
        {
            if(ci->events[i]->keepable())
            {
                if(keep < i)
                {
                    for(int j = keep; j < i; j++) delete ci->events[j];
                    ci->events.remove(keep, i - keep);
                    i = keep;
                }
                keep = i+1;
                continue;
            }
        }
        while(ci->events.length() > keep) delete ci->events.pop();
        ci->timesync = false;
    }

    void serverupdate()
    {
        if(shouldstep && !gamepaused)
        {
            gamemillis += curtime;

            if(m_demo) readdemo();
            else if(!m_timed || gamemillis < gamelimit)
            {
                processevents();
                if(curtime)
                {
                    loopv(sents) if(sents[i].spawntime) // spawn entities when timer reached
                    {
                        int oldtime = sents[i].spawntime;
                        sents[i].spawntime -= curtime;
                        if(sents[i].spawntime<=0)
                        {
                            sents[i].spawntime = 0;
                            sents[i].spawned = true;
                            sendf(-1, 1, "ri2", N_ITEMSPAWN, i);
                        }
                        else if(sents[i].spawntime<=10000 && oldtime>10000 && (sents[i].type==I_QUAD || sents[i].type==I_BOOST))
                        {
                            sendf(-1, 1, "ri2", N_ANNOUNCE, sents[i].type);
                        }
                    }
                }
                aiman::checkai();
                if(smode) smode->update();
            }
        }
        else if(smode) smode->updatelimbo();

        check_bans_expired();
        check_clients_timed_out();

        if(nextexceeded && gamemillis > nextexceeded && (!m_timed || gamemillis < gamelimit))
        {
            nextexceeded = 0;
            loopvrev(clients) 
            {
                clientinfo &c = *clients[i];
                if(c.state.aitype != AI_NONE) continue;
                if(c.checkexceeded()) disconnect_client(c.clientnum, DISC_MSGERR);
                else c.scheduleexceeded();
            }
        }

        if(shouldcheckteamkills) checkteamkills();

        if(shouldstep && !gamepaused)
        {
            if(m_timed && smapname[0] && gamemillis-curtime>0) checkintermission();
            if(interm > 0 && gamemillis>interm)
            {
                if(demorecord) enddemorecord();
                interm = -1;
                checkvotes(true);
            }
        }

        shouldstep = clients.length() > 0;
    }

    // clientmanagment
    void forcespectator(clientinfo *ci)
    {
        if(ci->state.state==CS_ALIVE) suicide(ci);
        if(smode) smode->leavegame(ci);
        ci->state.state = CS_SPECTATOR;
        ci->state.timeplayed += lastmillis - ci->state.lasttimeplayed;
        if(!ci->privilege || ci->warned) aiman::removeai(ci);
        sendf(-1, 1, "ri3", N_SPECTATOR, ci->clientnum, 1);
    }

    // map managment
    struct crcinfo
    {
        int crc, matches;

        crcinfo() {}
        crcinfo(int crc, int matches) : crc(crc), matches(matches) {}

        static bool compare(const crcinfo &x, const crcinfo &y) { return x.matches > y.matches; }
    };

    VAR(modifiedmapspectator, 0, 1, 2);

    void checkmaps(int req = -1)
    {
        if(m_edit || !smapname[0]) return;
        vector<crcinfo> crcs;
        int total = 0, unsent = 0, invalid = 0;
        if(mcrc) crcs.add(crcinfo(mcrc, clients.length() + 1));
        loopv(clients)
        {
            clientinfo *ci = clients[i];
            if(ci->state.state==CS_SPECTATOR || ci->state.aitype != AI_NONE) continue;
            total++;
            if(!ci->clientmap[0])
            {
                if(ci->mapcrc < 0) invalid++;
                else if(!ci->mapcrc) unsent++;
            }
            else
            {
                crcinfo *match = nullptr;
                loopvj(crcs) if(crcs[j].crc == ci->mapcrc) { match = &crcs[j]; break; }
                if(!match) crcs.add(crcinfo(ci->mapcrc, 1));
                else match->matches++;
            }
        }
        if(!mcrc && total - unsent < min(total, 4)) return;
        crcs.sort(crcinfo::compare);
        string msg;
        loopv(clients)
        {
            clientinfo *ci = clients[i];
            if(ci->state.state==CS_SPECTATOR || ci->state.aitype != AI_NONE || ci->clientmap[0] || ci->mapcrc >= 0 || (req < 0 && ci->warned)) continue;
            formatstring(msg, "%s has modified map \"%s\"", colorname(ci), smapname);
            sendf(req, 1, "ris", N_SERVMSG, msg);
            if(req < 0) ci->warned = true;

            // When a modified map is found, move the player to spectator
            if(spectatemodifiedmap && ci->state.state!=CS_SPECTATOR)
            {
                if(ci->state.state==CS_ALIVE) suicide(ci);
                if(smode) smode->leavegame(ci);
                ci->state.state = CS_SPECTATOR;
                ci->state.timeplayed += lastmillis - ci->state.lasttimeplayed;
                if(!ci->privilege) aiman::removeai(ci);
                sendf(-1, 1, "ri3", N_SPECTATOR, ci->clientnum, 1);
            }
        }
        if(crcs.length() >= 2) loopv(crcs)
        {
            crcinfo &info = crcs[i];
            if(i || info.matches <= crcs[i+1].matches) loopvj(clients)
            {
                clientinfo *ci = clients[j];
                if(ci->state.state==CS_SPECTATOR || ci->state.aitype != AI_NONE || !ci->clientmap[0] || ci->mapcrc != info.crc || (req < 0 && ci->warned)) continue;
                formatstring(msg, "%s has modified map \"%s\"", colorname(ci), smapname);
                sendf(req, 1, "ris", N_SERVMSG, msg);
                if(req < 0) ci->warned = true;
            }
        }
        if(req < 0 && modifiedmapspectator && (mcrc || modifiedmapspectator > 1)) loopv(clients)
        {
            clientinfo *ci = clients[i];
            if(ci->warned && ci->state.state != CS_SPECTATOR) forcespectator(ci);
        }
    }

    // client managment
    bool shouldspectate(clientinfo *ci)
    {
        return ci->warned && modifiedmapspectator && (mcrc || modifiedmapspectator > 1);
    }

    void unspectate(clientinfo *ci)
    {
        if(shouldspectate(ci)) return;
        ci->state.state = CS_DEAD;
        ci->state.respawn();
        ci->state.lasttimeplayed = lastmillis;
        aiman::addclient(ci);
        sendf(-1, 1, "ri3", N_SPECTATOR, ci->clientnum, 0);
        if(ci->clientmap[0] || ci->mapcrc) checkmaps();
        if(!hasmap(ci)) rotatemap(true);
    }

    void sendclipboard(clientinfo *ci)
    {
        if(!ci->lastclipboard || !ci->clipboard) return;
        bool flushed = false;
        loopv(clients)
        {
            clientinfo &e = *clients[i];
            if(e.clientnum != ci->clientnum && e.needclipboard - ci->lastclipboard >= 0)
            {
                if(!flushed) { flushserver(true); flushed = true; }
                sendpacket(e.clientnum, 1, ci->clipboard);
            }
        }
    }
    // network

    bool allowbroadcast(int n)
    {
        clientinfo *ci = get_client_info(n);
        return ci && ci->connected;
    }

    void receivefile(int sender, uchar *data, int len)
    {
        if(!m_edit || len <= 0 || len > 4*1024*1024) return;
        clientinfo *ci = get_client_info(sender);
        if(ci->state.state==CS_SPECTATOR && !ci->privilege) return;
        if(mapdata) DELETEP(mapdata);
        mapdata = opentempfile("mapdata", "w+b");
        if(!mapdata) { sendf(sender, 1, "ris", N_SERVMSG, "failed to open temporary file for map"); return; }
        mapdata->write(data, len);
        sendservmsgf("[%s sent a map to server, \"/getmap\" to receive it]", colorname(ci));
    }


    void parsepacket(int sender, int chan, packetbuf &p)     // has to parse exactly each byte of the packet
    {
        if(sender<0 || p.packet->flags&ENET_PACKET_FLAG_UNSEQUENCED || chan > 2) return;
        char text[MAXTRANS];
        int type;
        // the sender
        clientinfo *ci = sender>=0 ? get_client_info(sender) : nullptr;
        // (probably) the sender OR the bot sending from the same sender
        clientinfo *cq = ci;
        // the receiver?
        clientinfo *cm = ci;
        if(ci && !ci->connected)
        {
            if(chan==0) return;
            else if(chan!=1) { disconnect_client(sender, DISC_MSGERR); return; }
            else while(p.length() < p.maxlen) switch(checktype(getint(p), ci))
            {
                case N_CONNECT:
                {
                    getstring(text, p);
                    filtertext(text, text, false, false, MAXNAMELEN);
                    if(!text[0]) copystring(text, "unnamed");
                    copystring(ci->name, text, MAXNAMELEN+1);
                    ci->playermodel = getint(p);
                    ci->fov = getint(p); //TODO mix this in other msg..

                    string password, mapwish;
                    getstring(password, p, sizeof(password));
                    getstring(mapwish, p, sizeof(mapwish));
                    int modewish = getint(p);
                    if(player_connected(ci, password, mapwish, modewish)) shouldstep = true;
                    break;
                }

                case N_PING:
                    getint(p);
                    break;

                default:
                    disconnect_client(sender, DISC_MSGERR);
                    return;
            }
            return;
        }
        else if(chan==2)
        {
            receivefile(sender, p.buf, p.maxlen);
            return;
        }

        if(p.packet->flags&ENET_PACKET_FLAG_RELIABLE) reliablemessages = true;
        #define QUEUE_AI clientinfo *cm = cq;
        #define QUEUE_MSG { if(cm) while(curmsg<p.length()) cm->messages.add(p.buf[curmsg++]); }
        #define QUEUE_BUF(body) { \
            if(cm) \
            { \
                curmsg = p.length(); \
                { body; } \
            } \
        }
        #define QUEUE_INT(n) QUEUE_BUF(putint(cm->messages, n))
        #define QUEUE_UINT(n) QUEUE_BUF(putuint(cm->messages, n))
        #define QUEUE_STR(text) QUEUE_BUF(sendstring(text, cm->messages))
        int curmsg;
        while((curmsg = p.length()) < p.maxlen) switch(type = checktype(getint(p), ci))
        {
            case N_POS:
            {
                int pcn = getuint(p); 
                p.get(); 
                uint flags = getuint(p);
                clientinfo *cp = get_client_info(pcn);
                if(cp && pcn != sender && cp->ownernum != sender) cp = nullptr;
                vec pos;
                loopk(3)
                {
                    int n = p.get(); n |= p.get()<<8; if(flags&(1<<k)) { n |= p.get()<<16; if(n&0x800000) n |= -1<<24; }
                    pos[k] = n/DMF;
                }
                loopk(3) p.get();
                int mag = p.get(); if(flags&(1<<3)) mag |= p.get()<<8;
                int dir = p.get(); dir |= p.get()<<8;
                vec vel = vec((dir%360)*RAD, (clamp(dir/360, 0, 180)-90)*RAD).mul(mag/DVELF);
                if(flags&(1<<4))
                {
                    p.get(); if(flags&(1<<5)) p.get();
                    if(flags&(1<<6)) loopk(2) p.get();
                }
                if(cp)
                {
                    if(cp->state.state==CS_ALIVE || cp->state.state==CS_EDITING)
                    {
                        if(!m_edit && max(vel.magnitude2(), (float)fabs(vel.z)) >= 180)
                            cp->setexceeded();
                        cp->position.setsize(0);
                        while(curmsg<p.length()) cp->position.add(p.buf[curmsg++]);
                    }
                    if(smode && cp->state.state==CS_ALIVE) smode->moved(cp, cp->state.o, cp->gameclip, pos, (flags&0x80)!=0);
                    cp->state.o = pos;
                    cp->gameclip = (flags&0x80)!=0;
                }
                break;
            }

            case N_TELEPORT:
            {
                int pcn = getint(p), teleport = getint(p), teledest = getint(p);
                clientinfo *cp = get_client_info(pcn);
                if(cp && pcn != sender && cp->ownernum != sender) cp = nullptr;
                if(cp && (cp->state.state==CS_ALIVE || cp->state.state==CS_EDITING))
                {
                    flushclientposition(*cp);
                    sendf(-1, 0, "ri4x", N_TELEPORT, pcn, teleport, teledest, cp->ownernum); 
                }
                break;
            }

            case N_JUMPPAD:
            {
                int pcn = getint(p), jumppad = getint(p);
                clientinfo *cp = get_client_info(pcn);
                if(cp && pcn != sender && cp->ownernum != sender) cp = nullptr;
                if(cp && (cp->state.state==CS_ALIVE || cp->state.state==CS_EDITING))
                {
                    cp->setpushed();
                    flushclientposition(*cp);
                    sendf(-1, 0, "ri3x", N_JUMPPAD, pcn, jumppad, cp->ownernum);
                }
                break;
            }
                
            case N_FROMAI:
            {
                int qcn = getint(p);
                if(qcn < 0) cq = ci;
                else
                {
                    cq = get_client_info(qcn);
                    if(cq && qcn != sender && cq->ownernum != sender) cq = nullptr;
                }
                break;
            }

            case N_EDITMODE:
            {
                int val = getint(p);
                if(!m_edit) break;
                if(val ? ci->state.state!=CS_ALIVE && ci->state.state!=CS_DEAD : ci->state.state!=CS_EDITING) break;
                if(smode)
                {
                    if(val) smode->leavegame(ci);
                    else smode->entergame(ci);
                }
                if(val)
                {
                    ci->state.editstate = ci->state.state;
                    ci->state.state = CS_EDITING;
                    ci->events.setsize(0);
                    ci->state.rockets.reset();
                    ci->state.grenades.reset();
                    ci->state.bombs.reset();
                }
                else ci->state.state = ci->state.editstate;
                QUEUE_MSG;
                break;
            }

            case N_MAPCRC:
            {
                getstring(text, p);
                int crc = getint(p);
                if(!ci) break;
                if(strcmp(text, smapname))
                {
                    if(ci->clientmap[0])
                    {
                        ci->clientmap[0] = '\0';
                        ci->mapcrc = 0;
                    }
                    else if(ci->mapcrc > 0) ci->mapcrc = 0;
                    break;
                }
                copystring(ci->clientmap, text);
                ci->mapcrc = text[0] ? crc : 1;
                checkmaps();
                if(cq && cq != ci && cq->ownernum != ci->clientnum) cq = nullptr;
                break;
            }

            case N_CHECKMAPS:
                checkmaps(sender);
                break;

            case N_TRYSPAWN:
                if(!ci || !cq || cq->state.state!=CS_DEAD || cq->state.lastspawn>=0 || (smode && !smode->canspawn(cq))) break;

                if(!smode && m_lms
                    && (cq->state.deaths != 0
                      || cq->state.aitype == AI_NONE))
                  break;

                if(!ci->clientmap[0] && !ci->mapcrc)
                {
                    ci->mapcrc = -1;
                    checkmaps();
                    if(ci == cq) { if(ci->state.state != CS_DEAD) break; }
                    else if(cq->ownernum != ci->clientnum) { cq = nullptr; break; }
                }
                if(cq->state.deadflush)
                {
                    flushevents(cq, cq->state.deadflush);
                    cq->state.respawn();
                }
                cleartimedevents(cq);
                sendspawn(cq);
                break;

            case N_GUNSELECT:
            {
                int gunselect = getint(p);
                if(!cq || cq->state.state!=CS_ALIVE || gunselect<GUN_FIST || gunselect>GUN_BOMB) break;
                cq->state.gunselect = gunselect >= GUN_FIST && gunselect <= GUN_PISTOL ? gunselect : GUN_FIST;
                QUEUE_AI;
                QUEUE_MSG;
                break;
            }
            case N_FOV:
            {
                int fov = getint(p);
                if(cq) cq->fov = fov;
                QUEUE_MSG;
                break;
            }

            case N_SPAWN:
            {
                int ls = getint(p), gunselect = getint(p);
                if(!cq || (cq->state.state!=CS_ALIVE && cq->state.state!=CS_DEAD && cq->state.state!=CS_EDITING) || ls!=cq->state.lifesequence || cq->state.lastspawn<0) break;
                cq->state.lastspawn = -1;
                cq->state.state = CS_ALIVE;
                cq->state.gunselect = gunselect >= GUN_FIST && gunselect <= GUN_PISTOL ? gunselect : GUN_FIST;
                cq->exceeded = 0;
                if(smode) smode->spawned(cq);
                QUEUE_AI;
                QUEUE_BUF({
                    putint(cm->messages, N_SPAWN);
                    sendstate(cq->state, cm->messages);
                });
                break;
            }

            case N_SUICIDE:
            {
                ci->addevent(new suicideevent);
                break;
            }

            case N_SHOOT:
            {
                shotevent *shot = new shotevent;
                shot->id = getint(p);
                shot->millis = cq ? cq->geteventmillis(gamemillis, shot->id) : 0;
                shot->gun = getint(p);
                loopk(3) shot->from[k] = getint(p)/DMF;
                loopk(3) shot->to[k] = getint(p)/DMF;
                int hits = getint(p);
                loopk(hits)
                {
                    if(p.overread()) break;
                    hitinfo &hit = shot->hits.add();
                    hit.target = getint(p);
                    hit.lifesequence = getint(p);
                    hit.dist = getint(p)/DMF;
                    hit.rays = getint(p);
                    loopk(3) hit.dir[k] = getint(p)/DNF;
                }
                if(cq) 
                {
                    cq->addevent(shot);
                    cq->setpushed();
                }
                else delete shot;
                break;
            }

            case N_EXPLODE:
            {
                explodeevent *exp = new explodeevent;
                int cmillis = getint(p);
                exp->millis = cq ? cq->geteventmillis(gamemillis, cmillis) : 0;
                exp->gun = getint(p);
                exp->id = getint(p);
                int hits = getint(p);
                loopk(hits)
                {
                    if(p.overread()) break;
                    hitinfo &hit = exp->hits.add();
                    hit.target = getint(p);
                    hit.lifesequence = getint(p);
                    hit.dist = getint(p)/DMF;
                    hit.rays = getint(p);
                    loopk(3) hit.dir[k] = getint(p)/DNF;
                }
                if(cq) cq->addevent(exp);
                else delete exp;
                break;
            }

            case N_ITEMPICKUP:
            {
                int n = getint(p);
                if(!cq) break;
                pickupevent *pickup = new pickupevent;
                pickup->ent = n;
                cq->addevent(pickup);
                break;
            }

            case N_TEXT:
            {
                QUEUE_AI;
                QUEUE_MSG;
                getstring(text, p);
                filtertext(text, text, true, true);
                QUEUE_STR(text);
                if(cq)
                    Log.std->info("{0}: {1}", colorname(cq), text);
                break;
            }

            case N_SAYTEAM:
            {
                getstring(text, p);
                if(!ci || !cq || (ci->state.state==CS_SPECTATOR && !ci->privilege) || !m_teammode || !cq->team[0]) break;
                filtertext(text, text, true, true);
                loopv(clients)
                {
                    clientinfo *t = clients[i];
                    if(t==cq || t->state.state==CS_SPECTATOR || t->state.aitype != AI_NONE || strcmp(cq->team, t->team)) continue;
                    sendf(t->clientnum, 1, "riis", N_SAYTEAM, cq->clientnum, text);
                }
                if(cq)
                    Log.std->info("{0}<{1}>: {2}", colorname(cq), cq->team, text);
                break;
            }
            case N_PRIVMSG:
            {
            //server receives private messages in the format "receivercn, text" and forwards it to the receiver with "sendercn, text"
                int tcn = getint(p);
                getstring(text, p);
                clientinfo *rec = get_client_info(tcn); //private message receiver
                if(!ci || !cq || !rec || rec == cq || rec->state.aitype != AI_NONE) break;

                sendf(rec->clientnum, 1, "riis", N_PRIVMSG, cq->clientnum, text);
                break;
            }

            case N_SWITCHNAME:
            {
                QUEUE_MSG;
                getstring(text, p); //name
                filtertext(ci->name, text, false, false, MAXNAMELEN);
                if(!ci->name[0]) copystring(ci->name, "unnamed");
                QUEUE_STR(ci->name);
                getstring(text, p); //tag
                filtertext( ci->tag, text, false, MAXTAGLEN);
                QUEUE_STR(ci->tag);
                break;
            }

            case N_SWITCHMODEL:
            {
                ci->playermodel = getint(p);
                QUEUE_MSG;
                break;
            }

            case N_SWITCHTEAM:
            {
                getstring(text, p);
                filtertext(text, text, false, false, MAXTEAMLEN);
                if(m_teammode && text[0] && strcmp(ci->team, text) && (!smode || smode->canchangeteam(ci, ci->team, text)) && addteaminfo(text))
                {
                    if(ci->state.state==CS_ALIVE) suicide(ci);
                    copystring(ci->team, text);
                    aiman::changeteam(ci);
                    sendf(-1, 1, "riisi", N_SETTEAM, sender, ci->team, ci->state.state==CS_SPECTATOR ? -1 : 0);
                }
                break;
            }

            case N_MAPVOTE:
            {
                getstring(text, p);
                filtertext(text, text, false);
                int reqmode = getint(p);
                vote(text, reqmode, sender);
                break;
            }

            case N_ITEMLIST:
            {
                if((ci->state.state==CS_SPECTATOR && !ci->privilege) || !notgotitems || strcmp(ci->clientmap, smapname)) { while(getint(p)>=0 && !p.overread()) getint(p); break; }
                int n;
                while((n = getint(p))>=0 && n<MAXENTS && !p.overread())
                {
                    server_entity se = { NOTUSED, 0, false };
                    while(sents.length()<=n) sents.add(se);
                    sents[n].type = getint(p);
                    if(canspawnitem(sents[n].type))
                    {
                        if(entities::delayspawn(sents[n].type)) sents[n].spawntime = spawntime(sents[n].type);
                        else sents[n].spawned = true;
                    }
                }
                notgotitems = false;
                break;
            }

            case N_EDITENT:
            {
                int i = getint(p);
                loopk(3) getint(p);
                int type = getint(p);
                loopk(5) getint(p);
                if(!ci || ci->state.state==CS_SPECTATOR) break;
                QUEUE_MSG;
                bool canspawn = canspawnitem(type);
                if(i<MAXENTS && (sents.inrange(i) || canspawnitem(type)))
                {
                    server_entity se = { NOTUSED, 0, false };
                    while(sents.length()<=i) sents.add(se);
                    sents[i].type = type;
                    if(canspawn ? !sents[i].spawned : (sents[i].spawned || sents[i].spawntime))
                    {
                        sents[i].spawntime = canspawn ? 1 : 0;
                        sents[i].spawned = false;
                    }
                }
                break;
            }

            case N_EDITVAR:
            {
                int type = getint(p);
                getstring(text, p);
                switch(type)
                {
                    case ID_VAR: getint(p); break;
                    case ID_FVAR: getfloat(p); break;
                    case ID_SVAR: getstring(text, p);
                }
                if(ci && ci->state.state!=CS_SPECTATOR) QUEUE_MSG;
                break;
            }

            case N_PING:
                sendf(sender, 1, "i2", N_PONG, getint(p));
                break;

            case N_CLIENTPING:
            {
                int ping = getint(p);
                if(ci)
                {
                    ci->ping = ping;
                    loopv(ci->bots) ci->bots[i]->ping = ping;
                }
                QUEUE_MSG;
                break;
            }

            case N_MASTERMODE:
            {
                int mm = getint(p);
                change_mastermode(mm, sender, ci);
                break;
            }

            case N_CLEARBANS:
            {
                clearbans(ci);
                break;
            }

            case N_KICK:
            {
                int victim = getint(p);
                getstring(text, p);
                filtertext(text, text);
                trykick(ci, victim, text);
                break;
            }

            case N_SPECTATOR:
            {
                int spectator = getint(p), val = getint(p);
                if(!ci->privilege && (spectator!=sender || (ci->state.state==CS_SPECTATOR && mastermode>=MM_LOCKED))) break;
                clientinfo *spinfo = get_client_info(spectator, false);
                if(!spinfo || !spinfo->connected || (spinfo->state.state==CS_SPECTATOR ? val : !val)) break;

                if(spinfo->state.state!=CS_SPECTATOR && val) forcespectator(spinfo);
                else if(spinfo->state.state==CS_SPECTATOR && !val) unspectate(spinfo);

                if(cq && cq != ci && cq->ownernum != ci->clientnum) cq = nullptr;
                break;
            }

            case N_SETTEAM:
            {
                int who = getint(p);
                getstring(text, p);
                filtertext(text, text, false, false, MAXTEAMLEN);
                if(!ci->privilege) break;
                clientinfo *wi = get_client_info(who);
                if(!m_teammode || !text[0] || !wi || !wi->connected || !strcmp(wi->team, text)) break;
                if((!smode || smode->canchangeteam(wi, wi->team, text)) && addteaminfo(text))
                {
                    if(wi->state.state==CS_ALIVE) suicide(wi);
                    copystring(wi->team, text, MAXTEAMLEN+1);
                }
                aiman::changeteam(wi);
                sendf(-1, 1, "riisi", N_SETTEAM, who, wi->team, 1);
                break;
            }

            case N_FORCEINTERMISSION:
                if(ci->privilege >= PRIV_ADMIN) startintermission();
                break;

            case N_RECORDDEMO:
            {
                int val = getint(p);
                if(ci->privilege < (restrictdemos ? PRIV_ADMIN : PRIV_MASTER) && !m_demo) break;
                if(!maxdemos || !maxdemosize) 
                {
                    sendf(ci->clientnum, 1, "ris", N_SERVMSG, "the server has disabled demo recording");
                    break;
                }
                demonextmatch = val!=0;
                sendservmsgf("demo recording is %s for next match", demonextmatch ? "enabled" : "disabled");
                break;
            }

            case N_STOPDEMO:
            {
                if(ci->privilege < (restrictdemos ? PRIV_ADMIN : PRIV_MASTER)) break;
                stopdemo();
                break;
            }

            case N_CLEARDEMOS:
            {
                int demo = getint(p);
                if(ci->privilege < (restrictdemos ? PRIV_ADMIN : PRIV_MASTER)) break;
                cleardemos(demo);
                break;
            }

            case N_LISTDEMOS:
                if(!ci->privilege && ci->state.state==CS_SPECTATOR) break;
                listdemos(sender);
                break;

            case N_GETDEMO:
            {
                int n = getint(p);
                if(!ci->privilege && ci->state.state==CS_SPECTATOR) break;
                senddemo(ci, n);
                break;
            }

            case N_GETMAP:
                if(!mapdata) sendf(sender, 1, "ris", N_SERVMSG, "no map to send");
                else if(ci->getmap) sendf(sender, 1, "ris", N_SERVMSG, "already sending map");
                else
                {
                    sendservmsgf("[%s is getting the map]", colorname(ci));
                    if((ci->getmap = sendfile(sender, CHAN_FILE, mapdata, "ri", N_SENDMAP)))
                        ci->getmap->freeCallback = freegetmap;
                    ci->needclipboard = totalmillis ? totalmillis : 1;
                }
                break;

            case N_NEWMAP:
            {
                int size = getint(p);
                if(!ci->privilege && ci->state.state==CS_SPECTATOR) break;
                if(size>=0)
                {
                    smapname[0] = '\0';
                    resetitems();
                    notgotitems = false;
                    if(smode) smode->newmap();
                }
                QUEUE_MSG;
                break;
            }

            case N_SETMASTER:
            {
                int mn = getint(p), val = getint(p);
                getstring(text, p);
                if(mn != ci->clientnum)
                {
                    if(!ci->privilege) break;
                    clientinfo *minfo = get_client_info(mn, false);
                    if(!minfo || !minfo->connected || minfo->privilege >= ci->privilege || (val && minfo->privilege)) break;
                    setmaster(minfo, val!=0, "", true);
                }
                else setmaster(ci, val!=0, text);
                // don't broadcast the master password
                break;
            }

            case N_ADDBOT:
            {
                aiman::reqadd(ci, getint(p));
                break;
            }

            case N_DELBOT:
            {
                aiman::reqdel(ci);
                break;
            }

            case N_BOTLIMIT:
            {
                int limit = getint(p);
                if(ci) aiman::setbotlimit(ci, limit);
                break;
            }

            case N_BOTBALANCE:
            {
                int balance = getint(p);
                if(ci) aiman::setbotbalance(ci, balance!=0);
                break;
            }

            case N_PAUSEGAME:
            {
                int val = getint(p);
                if(ci->privilege < (restrictpausegame ? PRIV_ADMIN : PRIV_MASTER))
                {
                    defformatstring(s, "You need to be %s to pause the game", restrictpausegame ? "admin" : "master");
                    sendf(ci->clientnum, 1, "ris", N_SERVMSG, s);
                    break;
                }
                pausegame(val > 0, ci);
                break;
            }
            case N_GAMESPEED:
            {
                int val = getint(p);
                if(ci->privilege < (restrictgamespeed ? PRIV_ADMIN : PRIV_MASTER))
                {
                    defformatstring(s, "You need to be %s to change the gamespeed", restrictgamespeed ? "admin" : "master");
                    sendf(ci->clientnum, 1, "ris", N_SERVMSG, s);
                    break;
                }
                changegamespeed(val, ci);
                break;
            }
            case N_PERSISTTEAMS:
            {
                int val = getint(p);
                if(ci->privilege < (restrictpersistteams ? PRIV_ADMIN : PRIV_MASTER))
                {
                    defformatstring(s, "You need to be %s to enable/disable persistent teams", restrictpersistteams ? "admin" : "master");
                    sendf(ci->clientnum, 1, "ris", N_SERVMSG, s);
                    break;
                }
                persistteams(val > 0);
                break;
            }

            case N_COPY:
                ci->cleanclipboard();
                ci->lastclipboard = totalmillis ? totalmillis : 1;
                goto genericmsg;

            case N_PASTE:
                if(ci->state.state!=CS_SPECTATOR) sendclipboard(ci);
                goto genericmsg;
    
            case N_CLIPBOARD:
            {
                int unpacklen = getint(p), packlen = getint(p); 
                ci->cleanclipboard(false);
                if(ci->state.state==CS_SPECTATOR)
                {
                    if(packlen > 0) p.subbuf(packlen);
                    break;
                }
                if(packlen <= 0 || packlen > (1<<16) || unpacklen <= 0) 
                {
                    if(packlen > 0) p.subbuf(packlen);
                    packlen = unpacklen = 0;
                }
                packetbuf q(32 + packlen, ENET_PACKET_FLAG_RELIABLE);
                putint(q, N_CLIPBOARD);
                putint(q, ci->clientnum);
                putint(q, unpacklen);
                putint(q, packlen); 
                if(packlen > 0) p.get(q.subbuf(packlen).buf, packlen);
                ci->clipboard = q.finalize();
                ci->clipboard->referenceCount++;
                break;
            } 

            case N_EDITT:
            case N_REPLACE:
            case N_EDITVSLOT:
            {
                int size = msgsizelookup(type);
                if(size<=0) { disconnect_client(sender, DISC_MSGERR); return; }
                loopi(size-1) getint(p);
                if(p.remaining() < 2) { disconnect_client(sender, DISC_MSGERR); return; }
                int extra = lilswap(*(const ushort *)p.pad(2));
                if(p.remaining() < extra) { disconnect_client(sender, DISC_MSGERR); return; }
                p.pad(extra);
                if(ci && ci->state.state!=CS_SPECTATOR) QUEUE_MSG;
                break;
            }

            case N_UNDO:
            case N_REDO:
            {
                int unpacklen = getint(p), packlen = getint(p);
                if(!ci || ci->state.state==CS_SPECTATOR || packlen <= 0 || packlen > (1<<16) || unpacklen <= 0)
                {
                    if(packlen > 0) p.subbuf(packlen);
                    break;
                }
                if(p.remaining() < packlen) { disconnect_client(sender, DISC_MSGERR); return; }
                packetbuf q(32 + packlen, ENET_PACKET_FLAG_RELIABLE);
                putint(q, type);
                putint(q, ci->clientnum);
                putint(q, unpacklen);
                putint(q, packlen);
                if(packlen > 0) p.get(q.subbuf(packlen).buf, packlen);
                sendpacket(-1, 1, q.finalize(), ci->clientnum);
                break;
            }

            case N_SERVCMD:
                getstring(text, p);
                break;

            case -1:
                disconnect_client(sender, DISC_MSGERR);
                return;

            case -2:
                disconnect_client(sender, DISC_OVERFLOW);
                return;

            default:
            {
                if(smode && smode->parse_network_message(type, ci, cq, p)) break;
                genericmsg:
                {
                    int size = msgsizelookup(type);
                    if(size<=0) { disconnect_client(sender, DISC_MSGERR); return; }
                    loopi(size-1) getint(p);
                    if(ci) switch(msgfilter[type])
                    {
                        case 2: case 3: if(ci->state.state != CS_SPECTATOR) QUEUE_MSG; break;
                        default: if(cq && (ci != cq || ci->state.state!=CS_SPECTATOR)) { QUEUE_AI; QUEUE_MSG; } break;
                    }
                    break;
                }
            }
        }
    }

    void serverinforeply(ucharbuf &req, ucharbuf &p)
    {
        if(req.remaining() && !getint(req))
        {
            extserverinforeply(req, p);
            return;
        }

        putint(p, numclients(-1, false, true));
        putint(p, gamepaused || gamespeed != 100 ? 7 : 5);                   // number of attrs following
        putint(p, PROTOCOL_VERSION);    // generic attributes, passed back below
        putint(p, gamemode);
        putint(p, m_timed ? max((gamelimit - gamemillis)/1000, 0) : 0);
        putint(p, maxclients);
        putint(p, get_mastermode_int());
        if(gamepaused || gamespeed != 100)
        {
            putint(p, gamepaused ? 1 : 0);
            putint(p, gamespeed);
        }
        sendstring(smapname, p);
        sendstring(serverdesc, p);
        sendserverinforeply(p);
    }

    bool servercompatible(char *name, char *sdec, char *map, int ping, const vector<int> &attr, int np)
    {
        return attr.length() && attr[0]==PROTOCOL_VERSION;
    }

#include "inexor/fpsgame/aiman.hpp"                       // for changeteam
}

