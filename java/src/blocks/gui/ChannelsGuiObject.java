package blocks.gui;

import java.util.ArrayList;

import lcm.lcm.*;

import april.vis.*;
import april.jmat.geom.*;

import skyspecs.lcm.LCMTypeTracker;

import blocks.lcmtypes.channels_t;

public class ChannelsGuiObject extends GUIObject
{
    LCMTypeTracker<channels_t> cRXTracker;
    LCMTypeTracker<channels_t> cTXTracker;

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
        channels_t channelsRX = cRXTracker.get();
        channels_t channelsTX = cTXTracker.get();
        String contents = "<<dropshadow=false,monospaced-12-bold,white>>";
        contents += "Channels RX\n";
        if(channelsRX != null)
        {
            for(int i = 0; i < channelsRX.num_channels; ++i)
                contents += "\tchannel " + String.format("%3d", i) + ": " +
                    String.format("%4d", channelsRX.channels[i]) + "\n";
        }
        contents += "\nChannels TX\n";
        if(channelsTX != null)
        {
            for(int i = 0; i < channelsTX.num_channels; ++i)
                contents += "\tchannel " + String.format("%3d", i) + ": " +
                    String.format("%4d", channelsTX.channels[i]) + "\n";
        }

        vc.add(new VisPixCoords(VisPixCoords.ORIGIN.TOP_LEFT,
                                new VzText(VzText.ANCHOR.TOP_LEFT, contents)));
        return vc;
    }
}
