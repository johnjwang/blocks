package blocks.util;

import lcm.lcm.*;

import april.util.*;

import skyspecs.math.MathUtil;
import skyspecs.util.ArrayUtil;

import april.lcmtypes.gamepad_t;
import blocks.lcmtypes.channels_t;

public class GamePadDriver
{
    LCM lcm;

    private GamePad gp;
    private String channel;
    private boolean useChannels;
    private boolean alwaysPublish;
    private double Hz;

    public GamePadDriver(LCM lcm, String devices[], String channel, boolean useChannels, boolean alwaysPublish, double Hz)
    {
        this.lcm = lcm;
        this.channel = channel;
        this.useChannels = useChannels;
        this.alwaysPublish = alwaysPublish;
        this.gp = new GamePad(devices, true);
        this.Hz = Hz;
    }

    private channels_t toChannels(gamepad_t gp)
    {
        channels_t channels = new channels_t();
        channels.utime = gp.utime;
        channels.num_channels = (byte) gp.naxes;
        channels.channels = new short[channels.num_channels];
        for(int i = 0; i < channels.num_channels; ++i)
            channels.channels[i] = (short)((gp.axes[i] * 500) + 1500);
        return channels;
    }

    public void run()
    {
        boolean trig1Active = false, trig2Active = false;
        long lastPublish_us = 0;

        while (true) {
            TimeUtil.sleep(gp.isPresent() ? 25 : 250);
            if (!alwaysPublish && !gp.isPresent())
                continue;

            gamepad_t msg = new gamepad_t();
            msg.utime = TimeUtil.utime();

            msg.naxes = 6;
            msg.axes = new double[msg.naxes];

            // XXX This should probably depend on the device
            msg.axes[0] = MathUtil.deadzone(gp.getAxis(0), -0.16, 0.16, -1, 1);
            msg.axes[1] = MathUtil.deadzone(gp.getAxis(1), -0.16, 0.16, -1, 1);
            msg.axes[2] = MathUtil.deadzone(gp.getAxis(2), -0.16, 0.16, -1, 1);
            msg.axes[3] = MathUtil.deadzone(gp.getAxis(3), -0.16, 0.16, -1, 1);
            if(gp.getAxis(4) != 0)
                trig1Active = true;
            if(trig1Active)
                msg.axes[4] = gp.getAxis(4);
            else
                msg.axes[4] = -1;
            if(gp.getAxis(5) != 0)
                trig2Active = true;
            if(trig2Active)
                msg.axes[5] = gp.getAxis(5);
            else
                msg.axes[5] = -1;

            msg.buttons = 0;
            for (int i = 0; i < 32; i++)
                if (gp.getButton(i))
                    msg.buttons |= (1<<i);

            msg.present = gp.isPresent();

            long currTime = TimeUtil.utime();
            if(lastPublish_us + 1e6/Hz <= currTime)
            {
                lastPublish_us = currTime;
                if(!useChannels)
                    lcm.publish(channel, msg);
                else
                {
                    lcm.publish(channel, toChannels(msg));
                }
            }
        }
    }

    public static void main(String args[])
    {
        GetOpt gopt = new GetOpt();
        gopt.addString('d', "device", null, "Joystick device to open");
        gopt.addString('c', "channel", "GAMEPAD", "LCM channel to send on");
        gopt.addBoolean('C', "channels", false, "Use channels_t LCM messages instead of gamepad_t");
        gopt.addBoolean('a', "alwaysPublish", false, "Publish (at lower rate) even if no device plugged in");
        gopt.addBoolean('h', "help", false, "Show this help");
        gopt.addDouble('z', "hertz", 25, "Frequency of broadcast (Hz)");

        if (!gopt.parse(args) || gopt.getBoolean("help")) {
            gopt.doHelp();
            System.exit(1);
        }

        LCM lcm = LCM.getSingleton();

        String devices[];
        if(gopt.getString("device") == null)
        {
            devices = new String[] { "/dev/xbox360", "/dev/gamepad", "/dev/Futaba"};
        }
        else
        {
            devices = new String[] {gopt.getString("device")};
        }

        new GamePadDriver(lcm,
                          devices,
                          gopt.getString("channel"),
                          gopt.getBoolean("channels"),
                          gopt.getBoolean("alwaysPublish"),
                          gopt.getDouble("hertz")).run();
    }
}
