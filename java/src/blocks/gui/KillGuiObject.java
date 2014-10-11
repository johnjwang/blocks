package blocks.gui;

import java.awt.BorderLayout;
import java.awt.event.*;
import javax.swing.*;
import java.util.ArrayList;

import lcm.lcm.*;

import april.vis.*;

import blocks.util.KillPublisher;

public class KillGuiObject extends GUIObject
{
    LCM lcm;

    private KillGuiObject()
    {
    }

    public KillGuiObject(LCM lcm)
    {
        this.lcm = lcm;
    }

    public ArrayList<JPanel> getPanels()
    {
        ArrayList<JPanel> ret = new ArrayList<JPanel>();
        ret.add(getKillPanel());
        return ret;
    }

    private JPanel getKillPanel()
    {
        JPanel panel = new JPanel();
        panel.setLayout(new BoxLayout(panel, BoxLayout.X_AXIS));
        JButton kill = new JButton("Kill");
        panel.add(kill);

        kill.addActionListener(new ActionListener(){
            public void actionPerformed(ActionEvent e)
            {
                KillPublisher.broadcast(lcm, 0);
            }
        });

        return panel;
    }
}
