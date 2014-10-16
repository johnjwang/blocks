package blocks.gui;

import java.util.ArrayList;

import lcm.lcm.*;

import april.vis.*;
import april.jmat.geom.*;
import april.util.TimeUtil;

import skyspecs.lcm.LCMTypeTracker;

import blocks.lcmtypes.channels_t;

public class ChannelsGuiObject extends GUIObject
{
    LCMTypeTracker<channels_t> cRXTracker;
    LCMTypeTracker<channels_t> cTXTracker;

    channels_t channelsRXLast = null;
    channels_t channelsTXLast = null;

    private long staleRXUtime = 0;
    private long staleTXUtime = 0;

    private ChannelsGuiObject()
    {
    }

    public ChannelsGuiObject(LCM lcm)
    {
        cRXTracker = new LCMTypeTracker<channels_t>(lcm, "CHANNELS_.*_RX",
                                                    0.25, 0.25, new channels_t());

        cTXTracker = new LCMTypeTracker<channels_t>(lcm, "CHANNELS_.*_TX",
                                                    0.25, 0.25, new channels_t());
    }

    @Override
    public VisObject contents(double dt)
    {
        VisChain vc = new VisChain();
        channels_t channelsRX = cRXTracker.pop();
        channels_t channelsTX = cTXTracker.pop();

        String contentsRX = null, contentsTX = null;
        if(channelsRX == null)
            channelsRX = channelsRXLast;
        else
            staleRXUtime = TimeUtil.utime() + 250000;

        if(channelsTX == null)
            channelsTX = channelsTXLast;
        else
            staleTXUtime = TimeUtil.utime() + 250000;

        if(channelsRX == null)
            contentsRX = "<<dropshadow=false,monospaced-12-bold,red>>";
        else
        {
            if(TimeUtil.utime() > staleRXUtime)
                contentsRX = "<<dropshadow=false,monospaced-12-bold,red>>";
            else
                contentsRX = "<<dropshadow=false,monospaced-12-bold,green>>";
        }

        if(channelsTX == null)
            contentsTX = "<<dropshadow=false,monospaced-12-bold,red>>";
        else
        {
            if(TimeUtil.utime() > staleTXUtime)
                contentsTX = "<<dropshadow=false,monospaced-12-bold,red>>";
            else
                contentsTX = "<<dropshadow=false,monospaced-12-bold,green>>";
        }


        channelsRXLast = channelsRX;
        channelsTXLast = channelsTX;

        contentsRX += "Channels RX\n";
        contentsTX += " \n";
        if(channelsRX != null)
        {
            for(int i = 0; i < channelsRX.num_channels; ++i)
            {
                contentsRX += "\tchannel " + String.format("%3d", i) + ": " +
                    String.format("%4d", channelsRX.channels[i]) + "\n";
                contentsTX += " \n";
            }
        }
        contentsTX += "\n\nChannels TX\n";
        if(channelsTX != null)
        {
            for(int i = 0; i < channelsTX.num_channels; ++i)
                contentsTX += "\tchannel " + String.format("%3d", i) + ": " +
                    String.format("%4d", channelsTX.channels[i]) + "\n";
        }

        vc.add(new VisPixCoords(VisPixCoords.ORIGIN.TOP_LEFT,
                                new VzText(VzText.ANCHOR.TOP_LEFT, contentsRX)),
               new VisPixCoords(VisPixCoords.ORIGIN.TOP_LEFT,
                                new VzText(VzText.ANCHOR.TOP_LEFT, contentsTX)));
        return vc;
    }
}
