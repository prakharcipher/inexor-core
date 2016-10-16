#pragma once

#include "inexor/shared/cube.hpp"
#include "inexor/util/Logging.hpp"

#include "inexor/enumerations/console_entry_types.hpp"

#include "inexor/macros/constants.hpp"

/// SVARP radardir defines the directory of radar images (arrows, frame, flags, skulls..)
extern char *radardir;

#include "inexor/enumerations/entity_types.hpp"

#include "inexor/enumerations/trigger_states.hpp"

#include "inexor/classes/fpsentity.hpp"

#include "inexor/enumerations/gun_ids.hpp"

#include "inexor/enumerations/armor_types.hpp"

#include "inexor/enumerations/ai_modes.hpp"

#include "inexor/enumerations/gamemode_bitmask.hpp"

#include "inexor/classes/gamemodeinfo.hpp"

#include "inexor/macros/gamemode_macros.hpp"

#include "inexor/enumerations/mastermodes.hpp"

#include "inexor/enumerations/sound_ids.hpp"

#include "inexor/enumerations/admin_levels.hpp"

#include "inexor/enumerations/netmsg_ids.hpp"

#include "inexor/enumerations/netmsg_sizes.hpp"

#include "inexor/classes/demoheader.hpp"

#include "inexor/enumerations/team_category.hpp"

#include "inexor/enumerations/hud_icons.hpp"

#include "inexor/enumerations/bomberman_hud_announces.hpp"

#include "inexor/classes/itemstat.hpp"

/// weapon description structure
static const struct guninfo
{ 
	int sound, attackdelay, damage, spread, projspeed;
	int kickamount, range, rays, hitpush, exprad, ttl; 
	const char *name, *file; short part;
}

/// create an array of guninfo instances ('guns')
/// TODO: replace this hardcoded stuff and move on to JSON!
guns[NUMGUNS] =
{
    { S_PUNCH1,    250,  50,   0,   0,  0,   14,  1,  80,   0,    0, "fist",            "chainsaw",        0 },
    { S_SG,       1400,  10, 400,   0, 20, 1024, 20,  80,   0,    0, "shotgun",         "shotgun",         0 },
    { S_CG,        100,  30, 100,   0,  7, 1024,  1,  80,   0,    0, "chaingun",        "chaingun",        0 },
    { S_RLFIRE,    800, 120,   0, 320, 10, 1024,  1, 160,  40,    0, "rocketlauncher",  "rocket",          0 },
    { S_RIFLE,    1500, 100,   0,   0, 30, 2048,  1,  80,   0,    0, "rifle",           "rifle",           0 },
    { S_FLAUNCH,   600,  90,   0, 200, 10, 1024,  1, 250,  45, 1500, "grenadelauncher", "grenadelauncher", 0 },
    { S_PISTOL,    500,  35,  50,   0,  7, 1024,  1,  80,   0,    0, "pistol",          "pistol",          0 },
    { S_FEXPLODE,  375, 200,   8,  20,  0, 1024,  1, 150,  40, 1500, "bomb",            "cannon",          0 },
    { S_FLAUNCH,   200,  20,   0, 200,  1, 1024,  1,  80,  40,    0, "fireball",        NULL,              PART_FIREBALL1 },
    { S_ICEBALL,   200,  40,   0, 120,  1, 1024,  1,  80,  40,    0, "iceball",         NULL,              PART_FIREBALL2 },
    { S_SLIMEBALL, 200,  30,   0, 640,  1, 1024,  1,  80,  40,    0, "slimeball",       NULL,              PART_FIREBALL3 },
    { S_PIGR1,     250,  50,   0,   0,  1,   12,  1,  80,   0,    0, "bite",            NULL,              0 },
    { -1,            0, 120,   0,   0,  0,    0,  1,  80,  40,    0, "barrel",          NULL,              0 },
    { S_FEXPLODE,  375, 200,   8,  20,  0, 1024,  1, 150,  40, 1500, "bomb_splinter",   NULL,              0 },

};

#include "inexor/fpsgame/ai.hpp"


/// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/// fpsstate and fpsent definitions

/// inherited by fpsent and server clients
struct fpsstate
{
    int health, maxhealth;
    int armour, armourtype;
    int quadmillis;
    int gunselect, gunwait;
    int ammo[NUMGUNS];
    int aitype, skill;
    int backupweapon; //no ammo - weapon
    int bombradius;
    int bombdelay;

    fpsstate() : maxhealth(100), aitype(AI_NONE), skill(0), backupweapon(GUN_FIST) {}

    /// set initial ammo
    void baseammo(int gun, int k = 2, int scale = 1)
    {
        ammo[gun] = (itemstats[gun-GUN_SG].add*k)/scale;
    }

    /// add ammo
    void addammo(int gun, int k = 1, int scale = 1)
    {
        itemstat &is = itemstats[gun-GUN_SG];
        ammo[gun] = min(ammo[gun] + (is.add*k)/scale, is.max);
    }

    /// ammo limitation reached/exceeded?
    bool hasmaxammo(int type)
    {
       const itemstat &is = itemstats[type-I_SHELLS];
       return ammo[type-I_SHELLS+GUN_SG]>=is.max;
    }

    /// check if I can pick up this item depending on the radius
    bool canpickup(int type)
    {
        if(type<I_SHELLS || type>I_QUAD) return false;
			itemstat &is = itemstats[type-I_SHELLS];
	        switch(type)
	        {
	            case I_BOOST: return maxhealth<is.max;
	            case I_HEALTH: return health<maxhealth;
	                case I_GREENARMOUR:
	                    // (100h/100g only absorbs 200 damage)
	                    if(armourtype==A_YELLOW && armour>=100) return false;
	            case I_YELLOWARMOUR: return !armourtype || armour<is.max;
	            case I_QUAD: return quadmillis<is.max;
                case I_BOMBRADIUS:
                    return bombradius<is.max;
                    break;
                case I_BOMBDELAY:
                    return bombdelay<is.max;
                    break;
	            default: return ammo[is.info]<is.max;
	    	}
		}

    /// pick up this item
    void pickup(int type)
    {
        if(type<I_SHELLS || type>I_QUAD) return;
            itemstat &is = itemstats[type-I_SHELLS];
            switch(type)
            {
                case I_BOOST:
                    maxhealth = min(maxhealth+is.add, is.max);
                case I_HEALTH: // boost also adds to health
                    health = min(health+is.add, maxhealth);
                    break;
                case I_GREENARMOUR:
                case I_YELLOWARMOUR:
                    armour = min(armour+is.add, is.max);
                    armourtype = is.info;
                    break;
                case I_QUAD:
                    quadmillis = min(quadmillis+is.add, is.max);
                    break;
                case I_BOMBRADIUS:
                    bombradius = min(bombradius+is.add, is.max);
                    break;
                case I_BOMBDELAY:
                    bombdelay = min(bombdelay+is.add, is.max);
                    break;
                default:
                    ammo[is.info] = min(ammo[is.info]+is.add, is.max);
                    break;
            }
        }

    /// reset all members when spawning
    void respawn()
    {
        health = maxhealth;
        armour = 0;
        armourtype = A_BLUE;
        quadmillis = 0;
        gunselect = GUN_PISTOL;
        gunwait = 0;
        bombradius = 1;
        bombdelay = 1;
        loopi(NUMGUNS) ammo[i] = 0;
        ammo[backupweapon] = 1;
    }

    /// configure spawn settings (weapons, ammo, health...) depending on game mode
    void spawnstate(int gamemode)
    {
        if(m_demo)
        {
            gunselect = GUN_FIST;
            backupweapon = GUN_FIST;
        }
        else if(m_insta)
        {
            armour = 0;
            health = 1;
            gunselect = GUN_RIFLE;
            ammo[GUN_RIFLE] = 100;
            backupweapon = GUN_FIST;
        }
        else if(m_regencapture)
        {
            armourtype = A_BLUE;
            armour = 25;
            gunselect = GUN_PISTOL;
            ammo[GUN_PISTOL] = 40;
            ammo[GUN_GL] = 1;
            backupweapon = GUN_FIST;
        }
        else if(m_tactics)
        {
            armourtype = A_GREEN;
            armour = 100;
            ammo[GUN_PISTOL] = 40;
            backupweapon = GUN_FIST;
            int spawngun1 = rnd(5)+1, spawngun2;
            gunselect = spawngun1;
            baseammo(spawngun1, m_noitems ? 2 : 1);
            do spawngun2 = rnd(5)+1; while(spawngun1==spawngun2);
            baseammo(spawngun2, m_noitems ? 2 : 1);
            if(m_noitems) ammo[GUN_GL] += 1;
        }
        else if(m_efficiency)
        {
            armourtype = A_GREEN;
            armour = 100;
            loopi(5) baseammo(i+1);
            gunselect = GUN_CG;
            ammo[GUN_CG] /= 2;
            backupweapon = GUN_FIST;
        }
        else if(m_ctf || m_collect)
        {
            armourtype = A_BLUE;
            armour = 50;
            ammo[GUN_PISTOL] = 40;
            ammo[GUN_GL] = 1;
            backupweapon = GUN_FIST;
        }
        else if(m_bomb)
        {
            health = 1;
            armourtype = A_GREEN;
            armour = 0;
            gunselect = GUN_BOMB;
            backupweapon = GUN_BOMB;
        }
        else if(m_hideandseek)
        {
            health = 100;
            armour = 0;
            gunselect = GUN_RL;
            ammo[GUN_RL] = 10;
            ammo[GUN_PISTOL] = 0;
            ammo[GUN_GL] = 0;
            backupweapon = GUN_FIST;
        }
        else if(m_sp)
        {
            if(m_dmsp) 
            {
                armourtype = A_BLUE;
                armour = 25;
            }
            ammo[GUN_PISTOL] = 80;
            ammo[GUN_GL] = 1;
            backupweapon = GUN_FIST;
        }
        else
        {
            armourtype = A_BLUE;
            armour = 25;
            ammo[GUN_PISTOL] = 40;
            ammo[GUN_GL] = 1;
            backupweapon = GUN_FIST;
        }
    }

    /// just subtract damage here, we can set death, etc. later in code calling this
    int dodamage(int damage)
    {
        int ad = damage*(armourtype+1)*25/100; // let armour absorb when possible
        if(ad>armour) ad = armour;
        armour -= ad;
        damage -= ad;
        health -= damage;
        return damage;
    }

    /// is there ammo left for this gun
    int hasammo(int gun, int exclude = -1)
    {
        return gun >= 0 && gun <= NUMGUNS && gun != exclude && ammo[gun] > 0;
    }
};

// mostly players can be described with this
struct fpsent : dynent, fpsstate
{
    int weight;                         // affects the effectiveness of hitpush
    int clientnum, privilege, lastupdate, plag, ping;
    int lifesequence;                   // sequence id for each respawn, used in damage test
    int respawned, suicided;
    int lastpain;
    int lastaction, lastattackgun;
    bool attacking;
    int attacksound, attackchan, idlesound, idlechan;
    int lasttaunt;
    int lastpickup, lastpickupmillis, lastbase, lastrepammo, flagpickup, tokens;
    vec lastcollect;
    int frags, flags, deaths, teamkills, totaldamage, totalshots;
    editinfo *edit;
    float deltayaw, deltapitch, deltaroll, newyaw, newpitch, newroll;
    int smoothmillis;

    string name, tag, team, info;
    int playermodel;
    int fov;                            // field of view

    ai::aiinfo *ai;
    int ownernum, lastnode;

    vec muzzle;

    fpsent() : weight(100), clientnum(-1), privilege(PRIV_NONE), lastupdate(0), plag(0), ping(0), lifesequence(0), respawned(-1), suicided(-1), lastpain(0), attacksound(-1), attackchan(-1), idlesound(-1), idlechan(-1),
                frags(0), flags(0), deaths(0), teamkills(0), totaldamage(0), totalshots(0), edit(NULL), smoothmillis(-1), playermodel(-1), fov(100), ai(NULL), ownernum(-1), muzzle(-1, -1, -1)
    {
        name[0] = team[0] = tag[0] = info[0] = 0;
        respawn();
    }
    ~fpsent()
    {
        freeeditinfo(edit);
        if(attackchan >= 0) inexor::sound::stopsound(attacksound, attackchan);
        if(idlechan >= 0) inexor::sound::stopsound(idlesound, idlechan);
        if(ai) delete ai;
    }

    /// apply push event to object's velocity vector
    void hitpush(int damage, const vec &dir, fpsent *actor, int gun)
    {
        vec push(dir);
        push.mul((actor==this && guns[gun].exprad ? EXP_SELFPUSH : 1.0f)*guns[gun].hitpush*damage/weight);
        vel.add(push);
    }

    /// @see stopsound
    void stopattacksound()
    {
        if(attackchan >= 0) inexor::sound::stopsound(attacksound, attackchan, 250);
        attacksound = attackchan = -1;
    }

    /// @see stopsound
    void stopidlesound()
    {
        if(idlechan >= 0) inexor::sound::stopsound(idlesound, idlechan, 100);
        idlesound = idlechan = -1;
    }

    /// respawn item
    void respawn()
    {
        dynent::reset();
        fpsstate::respawn();
        respawned = suicided = -1;
        lastaction = 0;
        lastattackgun = gunselect;
        attacking = false;
        lasttaunt = 0;
        lastpickup = -1;
        lastpickupmillis = 0;
        lastbase = lastrepammo = -1;
        flagpickup = 0;
        tokens = 0;
        lastcollect = vec(-1e10f, -1e10f, -1e10f);
        stopattacksound();
        lastnode = -1;
    }
};


/// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/// team handling


/// many competetive team modes allow more than 2 teams
/// allow sorting multiple teams using team scores
struct teamscore
{
    const char *team;
    int score;
    teamscore() {}
    teamscore(const char *s, int n) : team(s), score(n) {}

	/// used for quicksort template to compare teams
    static bool compare(const teamscore &x, const teamscore &y)
    {
        if(x.score > y.score) return true;
        if(x.score < y.score) return false;
        return strcmp(x.team, y.team) < 0;
    }
};

/// create hashes to access hashmaps
static inline uint hthash(const teamscore &t) 
{
	return hthash(t.team); 
}

/// compare two teamnames
static inline bool htcmp(const char *key, const teamscore &t) 
{
	return htcmp(key, t.team);
}

/// scoreboard team block description
struct teaminfo
{
    char team[MAXTEAMLEN+1];
    int frags;
};

/// create hash for hashsts
static inline uint hthash(const teaminfo &t) 
{ 
	return hthash(t.team); 
}

/// compare two team names
static inline bool htcmp(const char *team, const teaminfo &t)
{
	return !strcmp(team, t.team);
}

/// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/// entity handling
/// entity system will be replaced with new entity system later...

namespace entities
{
    extern vector<extentity *> ents;

    extern const char *entmdlname(int type);
    extern const char *itemname(int i);
    extern int itemicon(int i);

    extern void preloadentities();
    extern void renderentities();
    extern void resettriggers();
    extern void checktriggers();
    extern void checkitems(fpsent *d);
    extern void checkquad(int time, fpsent *d);
    extern void resetspawns();
    extern void spawnitems(bool force = false);
    extern void putitems(packetbuf &p);
    extern void setspawn(int i, bool on);
    extern void teleport(int n, fpsent *d);
    extern void pickupeffects(int n, fpsent *d);
    extern void teleporteffects(fpsent *d, int tp, int td, bool local = true);
    extern void jumppadeffects(fpsent *d, int jp, bool local = true);

    extern void repammo(fpsent *d, int type, bool local = true);
}

/// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/// full game handling

namespace game
{
    struct scoregroup : teamscore
    {
        vector<fpsent *> players;

        char *sametag() //returns whether this scoregroup is a clan/playing-group, whatever. it returns null if players in this group have different tags and the tag if they all share the same one
        {
            fpsent *prev = NULL;
            loopv(players)
            {
                fpsent *p = players[i];
                if(!p->tag[0]) return NULL; 
                if(!prev) { prev = p; continue; }
                if(strcmp(p->tag, prev->tag)) return NULL; //two tags are not matching
            }
            return prev ? prev->tag : NULL;
        }
    };

    struct clientmode
    {
        virtual ~clientmode() {}

        virtual void preload() {}
        virtual int clipconsole(int w, int h) { return 0; }
        virtual void drawhud(fpsent *d, int w, int h) {}
        virtual bool isinvisible(fpsent *d) { return false; }
        virtual void rendergame() {}
        virtual void respawned(fpsent *d) {}
        virtual void setup() {}
        virtual void checkitems(fpsent *d) {}
        virtual int respawnwait(fpsent *d) { return 0; }
        virtual void pickspawn(fpsent *d) { findplayerspawn(d); }
        virtual void senditems(packetbuf &p) {}
        virtual void removeplayer(fpsent *d) {}
        virtual void gameover() {}
        virtual bool hidefrags() { return false; }
        virtual int getteamscore(const char *team) { return 0; }
        virtual void getteamscores(vector<teamscore> &scores) {}
        virtual void aifind(fpsent *d, ai::aistate &b, vector<ai::interest> &interests) {}
        virtual bool aicheck(fpsent *d, ai::aistate &b) { return false; }
        virtual bool aidefend(fpsent *d, ai::aistate &b) { return false; }
        virtual bool aipursue(fpsent *d, ai::aistate &b) { return false; }
        virtual void killed(fpsent *d, fpsent *actor) {}
        virtual void gameconnect(fpsent *d) {}
        virtual void renderscoreboard(g3d_gui &g, scoregroup &sg, int fgcolor, int bgcolor) {}
    };

    extern clientmode *cmode;
    extern void setclientmode();

    // fps
    extern int gamemode, nextmode;
    extern string clientmap;
    extern bool intermission;
    extern int maptime, maprealtime, maplimit;
    extern fpsent *player1;
    extern vector<fpsent *> players, clients;
    extern int lastspawnattempt;
    extern int lasthit;
    extern int respawnent;
    extern int following;
    extern SharedVar<int> smoothmove, smoothdist;

    // osd
    extern int hudannounce_begin;
    extern int hudannounce_timeout;
    extern int hudannounce_effect;
    extern char* hudannounce_text;


    extern bool clientoption(const char *arg);
    extern fpsent *getclient(int cn);
    extern fpsent *newclient(int cn);
    extern const char *colorname(fpsent *d, const char *name = NULL, const char *prefix = "", const char *alt = NULL);
    extern const char *teamcolorname(fpsent *d, const char *alt = "you");
    extern const char *teamcolor(const char *name, bool sameteam, const char *alt = NULL);
    extern const char *teamcolor(const char *name, const char *team, const char *alt = NULL);
    extern fpsent *pointatplayer();
    extern fpsent *hudplayer();
    extern fpsent *followingplayer();
    extern void stopfollowing();
    extern void clientdisconnected(int cn, bool notify = true);
    extern void clearclients(bool notify = true);
    extern void startgame();
    extern void spawnplayer(fpsent *);
    extern void deathstate(fpsent *d, bool restore = false);
    extern void damaged(int damage, fpsent *d, fpsent *actor, bool local = true);
    extern void killed(fpsent *d, fpsent *actor);
    extern void timeupdate(int timeremain);
    extern void msgsound(int n, physent *d = NULL);
    extern void drawicon(int icon, float x, float y, float sz = 120);
    const char *mastermodecolor(int n, const char *unknown);
    const char *mastermodeicon(int n, const char *unknown);

    // client
    extern bool connected, remote, demoplayback;
    extern string servinfo;
    extern vector<uchar> messages;

    extern int parseplayer(const char *arg);
    extern void ignore(int cn);
    extern void unignore(int cn);
    extern bool isignored(int cn);
    extern bool addmsg(int type, const char *fmt = NULL, ...);
    extern void switchname(const char *name, const char *tag);
    extern void switchteam(const char *name);
    extern void switchplayermodel(int playermodel);
    extern void sendmapinfo();
    extern void stopdemo();
    extern void changemap(const char *name, int mode);
    extern void forceintermission();
    extern void c2sinfo(bool force = false);
    extern void sendposition(fpsent *d, bool reliable = false);

    // monster
    struct monster;
    extern vector<monster *> monsters;

    extern void clearmonsters();
    extern void preloadmonsters();
    extern void stackmonster(monster *d, physent *o);
    extern void updatemonsters(int curtime);
    extern void rendermonsters();
    extern void suicidemonster(monster *m);
    extern void hitmonster(int damage, monster *m, fpsent *at, const vec &vel, int gun);
    extern void monsterkilled();
    extern void endsp(bool allkilled);
    extern void spsummary(int accuracy);

    // movable
    struct movable;
    extern vector<movable *> movables;

    extern void clearmovables();
    extern void stackmovable(movable *d, physent *o);
    extern void updatemovables(int curtime);
    extern void rendermovables();
    extern void suicidemovable(movable *m);
    extern void hitmovable(int damage, movable *m, fpsent *at, const vec &vel, int gun);
    extern bool isobstaclealive(movable *m);

    // weapon
    enum 
	{ 
		BNC_GRENADE, 
		BNC_BOMB, 
		BNC_SPLINTER, 
		BNC_GIBS, 
		BNC_DEBRIS, 
		BNC_BARRELDEBRIS
	};

    struct projectile
    {
        vec dir, o, to, offset;
        float speed;
        fpsent *owner;
        int gun;
        bool local;
        int offsetmillis;
        int id;
        entitylight light;
    };
    extern vector<projectile> projs;

    struct bouncer : physent
    {
        int lifetime, bounces;
        float lastyaw, roll;
        bool local;
        fpsent *owner;
        int bouncetype, variant;
        vec offset;
        int offsetmillis;
        int id;
        entitylight light;
        int generation;

        bouncer() : bounces(0), roll(0), variant(0)
        {
            type = ENT_BOUNCE;
        }
    };
    extern vector<bouncer *> bouncers;

    extern int getweapon(const char *name);
    extern void shoot(fpsent *d, const vec &targ);
    extern void shoteffects(int gun, const vec &from, const vec &to, fpsent *d, bool local, int id, int prevaction);
    extern void explode(bool local, fpsent *owner, const vec &v, dynent *safe, int dam, int gun);
    extern void explodeeffects(int gun, fpsent *d, bool local, int id = 0);
    extern void damageeffect(int damage, fpsent *d, bool thirdperson = true);
    extern void gibeffect(int damage, const vec &vel, fpsent *d);
    extern float intersectdist;
    extern bool intersect(dynent *d, const vec &from, const vec &to, float &dist = intersectdist);
    extern dynent *intersectclosest(const vec &from, const vec &to, fpsent *at, float &dist = intersectdist);
    extern void clearbouncers();
    extern void updatebouncers(int curtime);
    extern void removebouncers(fpsent *owner);
    extern void renderbouncers();
    extern void clearprojectiles();
    extern void updateprojectiles(int curtime);
    extern void removeprojectiles(fpsent *owner);
    extern void renderprojectiles();
    extern void preloadbouncers();
    extern void removeweapons(fpsent *owner);
    extern void updateweapons(int curtime);
    extern void gunselect(int gun, fpsent *d);
    extern void weaponswitch(fpsent *d);
    extern void avoidweapons(ai::avoidset &obstacles, float radius);

    // scoreboard
    extern void showscores(bool on);
    extern void getbestplayers(vector<fpsent *> &best);
    extern void getbestteams(vector<const char *> &best);
    extern void clearteaminfo();
    extern void setteaminfo(const char *team, int frags);

    // render
    struct playermodelinfo
    {
        const char *ffa, *blueteam, *redteam, *hudguns,
                   *vwep, *quad, *armour[3],
                   *ffaicon, *blueicon, *redicon;
        bool ragdoll;
    };

    extern SharedVar<int> playermodel, teamskins, testteam;

    extern void saveragdoll(fpsent *d);
    extern void clearragdolls();
    extern void moveragdolls();
    extern void changedplayermodel();
    extern const playermodelinfo &getplayermodelinfo(fpsent *d);
    extern int chooserandomplayermodel(int seed);
    extern void swayhudgun(int curtime);
    extern vec hudgunorigin(int gun, const vec &from, const vec &to, fpsent *d);
}

/// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/// (local) dedicated server handling

namespace server
{
    extern const char *modename(int n, const char *unknown = "unknown");
    extern const char *mastermodename(int n, const char *unknown = "unknown");
    extern void startintermission();
    extern void forceintermission();
    extern void stopdemo();
    extern void forcemap(const char *map, int mode);
    extern void forcepaused(bool paused);
    extern void forcegamespeed(int speed);
    extern void forcepersist(bool persist);
    extern void hashpassword(int cn, int sessionid, const char *pwd, char *result, int maxlen = MAXSTRLEN);
    extern int msgsizelookup(int msg);
    extern bool serveroption(const char *arg);
    extern bool delayspawn(int type);
}

