package blocks.gui;

import java.awt.BorderLayout;
import java.awt.event.*;
import javax.swing.*;
import java.util.ArrayList;

import lcm.lcm.*;

import april.vis.*;
import april.util.ParameterGUI;
import april.jmat.geom.*;

import blocks.util.CfgDataFrequencyProgrammer;

public class CfgDataFrequencyGuiObject extends GUIObject
{
    LCM lcm;

    private CfgDataFrequencyGuiObject()
    {
    }

    public CfgDataFrequencyGuiObject(LCM lcm)
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
        JButton showProgrammer = new JButton("Data Frequency Programmer");
        panel.add(showProgrammer);

        showProgrammer.addActionListener(new ActionListener(){
            public void actionPerformed(ActionEvent e)
            {
                showDataFreqFrame();
            }
        });

        return panel;
    }

    private void showDataFreqFrame()
    {
        JFrame dataFreqFrame = new JFrame("Data Frequency Programmer");
        dataFreqFrame.setLayout(new BorderLayout());
        dataFreqFrame.setSize(800,80);

        JPanel panel = new JPanel();
        panel.setLayout(new BoxLayout(panel, BoxLayout.X_AXIS));

        JButton program = new JButton("Program Target");

        JLabel idLabelMin = new JLabel("Target ID Min");
        final JTextField idMin = new JTextField("1");

        JLabel idLabelMax = new JLabel("Target ID Max");
        final JTextField idMax = new JTextField("1");

        JLabel hzLabel = new JLabel("Frequency (Hz)");
        final JTextField hz = new JTextField("10");

        panel.add(program);

        panel.add(idLabelMin);
        panel.add(idMin);

        panel.add(idLabelMax);
        panel.add(idMax);

        panel.add(hzLabel);
        panel.add(hz);

        program.addActionListener(new ActionListener(){
            public void actionPerformed(ActionEvent e)
            {
                int min = Integer.parseInt(idMin.getText());
                int max = Integer.parseInt(idMax.getText());

                int num = Integer.parseInt(hz.getText());
                //if(num > 255)
                //{
                    //System.err.println("Unable to program target with frequency > 255");
                    //return;
                //}
                for(int i = min; i <= max; ++i)
                {
                    CfgDataFrequencyProgrammer.broadcast(lcm, i, (short)num);
                }
            }
        });

        dataFreqFrame.add(panel, BorderLayout.CENTER);
        dataFreqFrame.setVisible(true);
    }
}

