package blocks.lcm;

import java.awt.*;
import java.awt.event.*;
import java.awt.geom.*;
import java.io.*;
import javax.swing.*;

import april.jmat.LinAlg;
import april.vis.*;
import april.util.*;

import lcm.spy.*;
import lcm.lcm.*;

import skyspecs.physics.Quaternion;
import skyspecs.util.ArrayUtil;
import skyspecs.vis.QuadModel;

import blocks.lcmtypes.telemetry_t;
import blocks.lcmtypes.imu_data_t;
import blocks.lcmtypes.rpms_t;

/** A plugin for viewing laser_t data **/
public class TelemetryPlugin implements SpyPlugin
{
    public boolean canHandle(long fingerprint)
    {
        return (fingerprint == telemetry_t.LCM_FINGERPRINT);
    }

    class MyAction extends AbstractAction
    {
        ChannelData cd;
        JDesktopPane jdp;

        public MyAction(JDesktopPane jdp, ChannelData cd)
        {
            super("Pose Viewer");
            this.jdp = jdp;
            this.cd = cd;
        }

        public void actionPerformed(ActionEvent e)
        {
            Viewer v = new Viewer(cd);
            jdp.add(v);
            v.toFront();
        }
    }

    public Action getAction(JDesktopPane jdp, ChannelData cd)
    {
        return new MyAction(jdp, cd);
    }

    class PosePane extends JPanel
    {
        VisWorld vw  = new VisWorld();
        VisLayer vl  = new VisLayer(vw);
        VisCanvas vc = new VisCanvas(vl);

        ParameterGUI pg;

        DefaultCameraManager dcm;

        public boolean first = true;
        public boolean pinToOrigin;
        public boolean follow;
        public boolean followRotate;

        public PosePane(ParameterGUI pg)
        {
            this.pg = pg;
            setLayout(new BorderLayout());
            add(vc, BorderLayout.CENTER);
            dcm = (DefaultCameraManager)vl.cameraManager;
        }

        public void setData(telemetry_t telemetry)
        {
            VisWorld.Buffer vb = vw.getBuffer("Vehicle");

            double quat[], pos[];

            quat = ArrayUtil.toDouble(telemetry.imu_data.quat);

            if(pinToOrigin)
                pos = new double[3];
            else
                pos = ArrayUtil.toDouble(telemetry.pos);

            double T[][] = LinAlg.quatPosToMatrix(quat, pos);

            QuadModel quad = new QuadModel(new double[]{telemetry.rpms.rpms[0],
                                                        telemetry.rpms.rpms[1],
                                                        telemetry.rpms.rpms[2],
                                                        telemetry.rpms.rpms[3]});

            vb.addBack(new VisChain(T, quad));
            vb.addBack(new VzAxes());
            vb.addBack(new VzGrid());

            vb.swap();

            vb = vw.getBuffer("Details");
            vb.setDrawOrder(100000);
            double rph[] = Quaternion.toEuler(ArrayUtil.toDouble(telemetry.imu_data.quat));
            vb.addBack(new VisPixCoords(VisPixCoords.ORIGIN.TOP_LEFT,
                                        new VzText(VzText.ANCHOR.TOP_LEFT,
                                                   "<<dropshadow=false,monospaced-10-bold,white>>" +
                                                   "Pos: [" + telemetry.pos[0] + ", " + telemetry.pos[1] + ", " + telemetry.pos[2] + "]\n" +
                                                   "Quat: [" + telemetry.imu_data.quat[0] + ", " + telemetry.imu_data.quat[1] + ", " +
                                                               telemetry.imu_data.quat[2] + ", " + telemetry.imu_data.quat[3] + "]\n" +
                                                   "Eulers: [" + rph[0] + ", " + rph[1] + ", " + rph[2] + "]\n" +
                                                   "Pos Rate: [" + telemetry.pos_rate[0] + ", " + telemetry.pos_rate[1] + ", " + telemetry.pos_rate[2] + "]\n" +
                                                   "Omega: [" + telemetry.imu_data.omega[0] + ", " + telemetry.imu_data.omega[1] + ", " + telemetry.imu_data.omega[2] + "]\n")));
            vb.swap();


            if(first)
            {
                vl.cameraManager.uiLookAt(ArrayUtil.copy(LinAlg.transform(LinAlg.translate(pos), new double[] {0.75964, -1.72212, 1.26869, 1.0}), 3),
                                          ArrayUtil.copy(LinAlg.transform(LinAlg.translate(pos), new double[] {-0.06620, 0.37975, 0.00000 }), 3),
                                          new double[] {-0.17911, 0.45587, 0.87184 }, true);
                first = false;
            }

            if(follow)
                dcm.follow(pos, quat, followRotate);
        }

    }

    class Viewer extends JInternalFrame implements LCMSubscriber
    {
        ChannelData cd;
        PosePane pp;
        ParameterGUI pg;

        public Viewer(ChannelData cd)
        {
            super("Pose: "+cd.name, true, true);
            this.cd = cd;

            pp = new PosePane(pg);
            setLayout(new BorderLayout());
            pg = new ParameterGUI();
            pp.follow = true;
            pg.addCheckBoxes("follow", "Follow Robot", pp.follow,
                             "rotate", "Rotate With Robot", pp.followRotate,
                             "pin", "Pin Robot to Origin", pp.pinToOrigin);

            pg.addListener(new ParameterListener() {
                public void parameterChanged(ParameterGUI pg, String name)
                {
                    if(name.equals("pin"))
                        pp.pinToOrigin = pg.gb(name);
                    else if(name.equals("follow"))
                        pp.follow = pg.gb(name);
                    else if(name.equals("rotate"))
                        pp.followRotate = pg.gb(name);
                }
            });

            add(pp, BorderLayout.CENTER);


            add(pg.getPanel(), BorderLayout.SOUTH);
            setSize(500,400);
            setVisible(true);

            LCM.getSingleton().subscribe(cd.name, this);
        }

        public void messageReceived(LCM lcm, String channel, LCMDataInputStream dins)
        {
            try {
                telemetry_t msg = new telemetry_t(dins);
                pp.setData(msg);
                return;
            } catch (IOException ex) { }

            System.err.println("Unable to decode channel " + channel);
        }

    }
}
