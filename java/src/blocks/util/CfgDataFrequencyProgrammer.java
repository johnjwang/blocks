package blocks.util;

import april.util.*;

import lcm.lcm.*;

import blocks.lcmtypes.*;

public class CfgDataFrequencyProgrammer
{
    public static boolean broadcast(LCM lcm, int id, byte hz)
    {
        cfg_data_frequency_t data_freq = new cfg_data_frequency_t();
        data_freq.hz = hz;
        lcm.publish("CFG_DATA_FREQUENCY_" + Integer.toString(id) + "_TX", data_freq);
        return true;
    }

    public static void main(String args[])
    {
        GetOpt gopt = new GetOpt();
        gopt.addBoolean('h', "help", false, "Show this help");
        gopt.addString('u', "lcm-url", null, "lcm url to broadcast on. (omit for default)");
        gopt.addInt('z', "hertz", 10, "Frequency of data to configure the block to publish (default = 10Hz)");
        gopt.addInt('i', "id", 1, "Id of block to program");

        if (!gopt.parse(args)
                || gopt.getBoolean("help")
                || (gopt.getString("hertz") == null))
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

        CfgDataFrequencyProgrammer.broadcast(lcm,
                                             gopt.getInt("id"),
                                             (byte)gopt.getInt("hertz"));
    }

}

