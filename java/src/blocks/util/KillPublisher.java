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

    public static void main(String args[])
    {
        GetOpt gopt = new GetOpt();
        gopt.addBoolean('h', "help", false, "Show this help");
        gopt.addString('u', "lcm-url", null, "lcm url to broadcast on. (omit for default)");

        if (!gopt.parse(args) || gopt.getBoolean("help"))
        {
            gopt.doHelp();
            return;
        }

        String lcm_url = gopt.getString("lcm-url");
        LCM lcm = null;
        try
        {
            lcm = new LCM(lcm_url);
        }
        catch (Exception e)
        {
            System.err.println("Unable to open lcm.");
            e.printStackTrace();
            System.exit(1);
        }

        new KillPublisher().broadcast(lcm, 0);
    }

}
