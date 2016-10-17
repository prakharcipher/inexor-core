#pragma once

/// 
struct teamkillkick
{
    int modes, limit, ban;

    bool match(int mode) const
    {
        return (modes&(1<<(mode-STARTGAMEMODE)))!=0;
    }

    bool includes(const teamkillkick &tk) const
    {
        return tk.modes != modes && (tk.modes & modes) == tk.modes;
    }
};
