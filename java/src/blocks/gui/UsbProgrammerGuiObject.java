package blocks.gui;

import java.awt.BorderLayout;
import java.awt.event.*;
import javax.swing.*;
import java.util.ArrayList;

import april.vis.*;
import april.util.ParameterGUI;
import april.jmat.geom.*;

import lcm.lcm.*;

public class UsbProgrammerGuiObject extends GUIObject
{
    public ArrayList<JPanel> getPanels()
    {
        ArrayList<JPanel> ret = new ArrayList<JPanel>();
        ret.add(getProgrammerPanel());
        return ret;
    }

    private JPanel getProgrammerPanel()
    {
        JPanel panel = new JPanel();
        panel.setLayout(new BorderLayout());

        JButton program = new JButton("Program Target");
        JLabel idLabel = new JLabel("Target ID");
        JTextField id = new JTextField("1");
        JLabel serialNumLabel = new JLabel("Serial Number");
        JTextField serialNum = new JTextField("00000000");

        panel.add(program);
        panel.add(idLabel);
        panel.add(id);
        panel.add(serialNumLabel);
        panel.add(serialNum);

        return panel;
    }
}
