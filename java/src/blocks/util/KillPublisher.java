package blocks.util;

import april.util.*;

import lcm.lcm.*;

import blocks.lcmtypes.kill_t;

public class KillPublisher
{
    public static boolean broadcast(LCM lcm, int id)
    {
        kill_t kill = new kill_t();
        kill.reason = kill_t.KILL_T_REASON_GCS;
        lcm.publish("KILL_" + id + "_TX", kill);
        return true;
    }
}
