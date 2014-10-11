package blocks.gui;

import java.util.ArrayList;
import java.io.*;
import java.awt.event.*;
import javax.swing.JPanel;

import april.vis.*;
import april.jmat.geom.*;

import lcm.lcm.*;

public abstract class GUIObject
{
	public String name()
    {
        return this.getClass().getName();
    }

	public boolean mouseRelease(GRay3D ray, MouseEvent e)
    {
		return false;
	}

    public boolean keyPressed(VisCanvas vc,
                                    VisLayer vl,
                                    VisCanvas.RenderInfo rinfo,
                                    KeyEvent e)
    {
        return false;
    }

	public VisObject contents(double dt)
    {
		return null;
	}

	public ArrayList<JPanel> getPanels()
    {
		return null;
	}

    public Boolean getFollowYaw()
    {
        return null;
    }

    public double[] getFollowQuat()
    {
        return null;
    }

    public double[] getFollowPos()
    {
        return null;
    }

}
