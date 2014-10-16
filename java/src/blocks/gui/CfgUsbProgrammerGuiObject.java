package blocks.gui;

import java.awt.BorderLayout;
import java.awt.event.*;
import javax.swing.*;
import java.util.ArrayList;

import lcm.lcm.*;

import april.vis.*;
import april.util.ParameterGUI;
import april.jmat.geom.*;

import blocks.util.CfgUsbSerialNumberProgrammer;

public class CfgUsbProgrammerGuiObject extends GUIObject
{
    LCM lcm;
    boolean incrementSerial = true;

    private CfgUsbProgrammerGuiObject()
    {
    }

    public CfgUsbProgrammerGuiObject(LCM lcm)
    {
        this.lcm = lcm;
    }

    public ArrayList<JPanel> getPanels()
    {
        ArrayList<JPanel> ret = new ArrayList<JPanel>();
        ret.add(getProgrammerPanel());
        return ret;
    }

    private JPanel getProgrammerPanel()
    {
        JPanel panel = new JPanel();
        panel.setLayout(new BoxLayout(panel, BoxLayout.X_AXIS));
        JButton showProgrammer = new JButton("Usb Programmer");
        panel.add(showProgrammer);

        showProgrammer.addActionListener(new ActionListener(){
            public void actionPerformed(ActionEvent e)
            {
                showUsbProgrammerFrame();
            }
        });

        return panel;
    }

    private void showUsbProgrammerFrame()
    {
        JFrame usbFrame = new JFrame("Usb Programmer");
        usbFrame.setLayout(new BorderLayout());
        usbFrame.setSize(800,40);

        JPanel panel = new JPanel();
        panel.setLayout(new BoxLayout(panel, BoxLayout.X_AXIS));

        JButton program = new JButton("Program Target");

        JLabel idLabelMin = new JLabel("Target ID Min");
        final JTextField idMin = new JTextField("1");

        JLabel idLabelMax = new JLabel("Target ID Max");
        final JTextField idMax = new JTextField("1");

        JLabel serialNumLabel = new JLabel("Serial Number");
        final JTextField serialNum = new JTextField("00000000");

        JCheckBox incrementSerialChk = new JCheckBox("Increment Serial", incrementSerial);

        panel.add(program);

        panel.add(idLabelMin);
        panel.add(idMin);

        panel.add(idLabelMax);
        panel.add(idMax);

        panel.add(serialNumLabel);
        panel.add(serialNum);

        panel.add(incrementSerialChk);

        program.addActionListener(new ActionListener(){
            public void actionPerformed(ActionEvent e)
            {
                int min = Integer.parseInt(idMin.getText());
                int max = Integer.parseInt(idMax.getText());

                boolean increment = incrementSerial;

                Long num = Long.parseLong(serialNum.getText());
                for(int i = min; i <= max; ++i)
                {
                    String serial = String.format("%08d", num);
                    CfgUsbSerialNumberProgrammer.broadcast(lcm, i, serial);
                    if(increment) num++;
                }
            }
        });

        incrementSerialChk.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                AbstractButton abstractButton = (AbstractButton) e.getSource();
                incrementSerial = abstractButton.getModel().isSelected();
            }
        });


        usbFrame.add(panel, BorderLayout.CENTER);
        usbFrame.setVisible(true);
    }
}
