package blocks.util;

import april.util.*;

import lcm.lcm.*;

import blocks.lcmtypes.*;

public class LCMPublisher
{
    LCM lcm;

    public LCMPublisher(String lcm_url)
    {
        try {
            lcm = new LCM(lcm_url);
        }
        catch (Exception e)
        {
            e.printStackTrace();
            System.exit(1);
        }

        broadcastTelemetry();
    }

    private void broadcastTelemetry()
    {
        telemetry_t msg = new telemetry_t();

        msg.pos = new float[]{0,0,0};
        msg.pos_rate = new float[]{0,0,0};

        msg.imu_data = new imu_data_t();
        msg.imu_data.quat = new float[]{1,0,0,0};
        msg.imu_data.omega = new float[]{0,0,0};
        msg.imu_data.accel = new float[]{0,0,0};

        msg.rpms = new rpms_t();
        msg.rpms.num_rpms = 4;
        msg.rpms.rpms = new short[msg.rpms.num_rpms];

        while(true)
        {
            msg.utime = TimeUtil.utime();
            for(int i = 0; i < msg.rpms.num_rpms; ++i)
            {
                msg.rpms.rpms[i] = (short)((msg.utime/1000) % 20000);
            }
            lcm.publish("TELEMETRY", msg);
            try{ Thread.sleep(20); }catch(Exception e){}
        }
    }

    public static void main(String args[])
    {
        String lcm_url = null;

        GetOpt gopt = new GetOpt();
        gopt.addBoolean('h', "help", false, "Show this help");
        gopt.addString('u', "lcm-url", lcm_url, "lcm url to broadcast on. (Null for default)");
        //gopt.addDouble('v', "voltage", 25.0, "Voltage to broadcast");

        if (!gopt.parse(args) || gopt.getBoolean("help")) {
            gopt.doHelp();
            return;
        }

        lcm_url = gopt.getString("lcm-url");

        new LCMPublisher(lcm_url);
    }
}
