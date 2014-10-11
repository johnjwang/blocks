package blocks.util;

import april.util.*;

import lcm.lcm.*;

import blocks.lcmtypes.*;

public class LCMPublisher
{
    LCM lcm;

    public LCMPublisher(String lcm_url, String usbSerial)
    {
        try {
            lcm = new LCM(lcm_url);
        }
        catch (Exception e)
        {
            e.printStackTrace();
            System.exit(1);
        }

        if(usbSerial != null) {
            broadcastUsbSerialNum(usbSerial);
            System.exit(0);
        }

        //broadcastTelemetry();
        //broadcastKill();
        //broadcastChannels();
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

    private void broadcastKill()
    {
        kill_t kill = new kill_t();
        kill.reason = kill_t.KILL_T_REASON_UNKNOWN;
        lcm.publish("KILL_TX", kill);
    }

    private void broadcastChannels()
    {
        channels_t channels = new channels_t();
        channels.utime = TimeUtil.utime();
        channels.num_channels = 8;
        channels.channels = new short[8];

        short currVal = 0;
        short startVal = 1150;
        short endVal = 1450;
        int dir = 1;

        while(true)
        {
            currVal += dir * 4;
            for(int i = 0; i < 8; i++)
                channels.channels[i] = (short) (currVal + startVal);

            if(currVal + startVal > endVal || currVal < 0) dir = -dir;
            lcm.publish("CHANNELS_TX", channels);
            try{ Thread.sleep(50); } catch(Exception e) {}
        }
    }

    private void broadcastUsbSerialNum(String usbSerial)
    {
        char chars[] = usbSerial.toCharArray();
        if(chars.length > 8)
        {
            System.err.println("usb serial number must be 8 characters");
            return;
        }

        usb_serial_num_t usbnum = new usb_serial_num_t();
        for(int i = 0; i < usbnum.sn_chars.length; i++)
            usbnum.sn_chars[i] = (byte) chars[i];
        lcm.publish("USB_SERIAL_NUM", usbnum);
    }

    public static void main(String args[])
    {
        String lcm_url = null;

        GetOpt gopt = new GetOpt();
        gopt.addBoolean('h', "help", false, "Show this help");
        gopt.addString('u', "lcm-url", lcm_url, "lcm url to broadcast on. (omit for default)");
        gopt.addString('s', "usb-serial", null, "Serial number for usb broadcast");
        //gopt.addInt('c', "channels", 1000, "Channels to broadcast");

        if (!gopt.parse(args) || gopt.getBoolean("help")) {
            gopt.doHelp();
            return;
        }

        lcm_url = gopt.getString("lcm-url");

        new LCMPublisher(lcm_url, gopt.getString("usb-serial"));
    }
}
