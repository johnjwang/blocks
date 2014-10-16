package blocks.gui;

import java.awt.*;
import java.awt.image.*;
import java.awt.event.*;
import javax.swing.*;
import java.io.*;
import java.util.ArrayList;

import april.vis.*;
import april.util.*;
import april.util.TimeUtil;
import april.jmat.geom.*;

import lcm.lcm.*;

public class GUI extends Thread
{
	LCM gui_lcm;

	VisWorld vw = new VisWorld();
	VisLayer vl = new VisLayer(vw);
	VisCanvas vc = new VisCanvas(vl);
    VisEventAdapter va;

	JFrame jf;

	ArrayList<ObjectContainer> objects;
    DefaultCameraManager dcm;

    Thread runThread;

    private GUIObject _cameraFollower = null;

	public class ObjectContainer
	{
		GUIObject object;
		double utime = 0;

		public ObjectContainer(GUIObject _object){
			object = _object;
		}

		public GUIObject getGUIObject(){
			return object;
		}

		public double get_dt(double _utime){
			if(utime == 0){
				utime = _utime;
				return 0;
			}else{
				return utime - _utime;
			}
		}

		public void update_utime(double _utime){
			utime = _utime;
		}
	}


	public GUI(String name, ArrayList<GUIObject> _objects)
	{
		objects = new ArrayList<ObjectContainer>();

		for(GUIObject object : _objects){
			objects.add(new ObjectContainer(object));
		}

		jf = new JFrame(name);
		jf.setLayout(new BorderLayout());
		JPanel rootPanel = new JPanel();
		rootPanel.setLayout(new BoxLayout(rootPanel, BoxLayout.Y_AXIS));
		jf.add(rootPanel, BorderLayout.SOUTH);
		jf.add(vc, BorderLayout.CENTER);

		for(ObjectContainer object : objects){
			if(object.getGUIObject().getPanels() != null){
				for(JPanel panel : object.getGUIObject().getPanels()){
					rootPanel.add(panel);
				}
			}
		}

		jf.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);

        va = new VisEventAdapter()
        { public boolean mouseReleased(VisCanvas vc,
                         VisLayer vl,
                         VisCanvas.RenderInfo rinfo,
                         GRay3D ray,
                         MouseEvent e)
          {
              boolean consumed = false;
              for(ObjectContainer obj : objects)
              {
                  if(obj.getGUIObject().mouseRelease(ray, e));
                    consumed = true;
              }
              return consumed;

          }
          public boolean keyPressed(VisCanvas vc,
                                    VisLayer vl,
                                    VisCanvas.RenderInfo rinfo,
                                    KeyEvent e)
          {
              boolean consumed = false;
              for(ObjectContainer obj : objects)
              {
                  if(obj.getGUIObject().keyPressed(vc,vl,rinfo,e) == true)
                  {
                      consumed = true;
                  }
              }
              return consumed;
          }
        };


		vl.addEventHandler(new april.viewer.RemoteLogEventHandler(vl, null, null));
		vl.addEventHandler(va);
		//vc.setBackground(Color.black);

		redrawGUI();
		vl.cameraManager.uiLookAt(new double[] {         0.60181,         0.37609,         0.14927 },
		                          new double[] {         0.00179,        -0.03854,         0.00000 },
								  new double[] {        -0.16495,        -0.11399,         0.97969 }, true);
		dcm = (DefaultCameraManager)vl.cameraManager;
		jf.setSize(1100,720);
		jf.setVisible(true);
	}

	private void addGUIObject(GUIObject object)
    {
		objects.add(new ObjectContainer(object));
	}

	private void redrawGUI()
    {
		VisWorld.Buffer buffer = vw.getBuffer("GUI");
		for(ObjectContainer obj : objects){
			buffer.addBack(obj.getGUIObject().contents(obj.get_dt(TimeUtil.utime())));
			obj.update_utime(TimeUtil.utime());
		}
		buffer.swap();
	}

    private void follow()
    {
        if(_cameraFollower != null){
            double pos[] = _cameraFollower.getFollowPos();
            double quat[] = _cameraFollower.getFollowQuat();
            boolean followYaw = _cameraFollower.getFollowYaw();
            dcm.follow(pos, quat, followYaw);
        }
    }

    public void follow(GUIObject a)
    {
        _cameraFollower = a;
    }

    public void run()
    {
		int delay = 30; //milliseconds
		ActionListener taskPerformer = new ActionListener() {
			public void actionPerformed(ActionEvent evt) {
				redrawGUI();
                follow();
			}
		};
		new Timer(delay, taskPerformer).start();
		while(true){
			try{
				Thread.sleep(1000);
			}catch(Exception e){
			}
		}
    }

	public static void main(String[] args) throws Exception
	{
		GetOpt gopt = new GetOpt();
        gopt.addBoolean('h', "help", false, "Show this help");
        gopt.addBoolean('a', "admin", false, "Launch with administrative tools");

        if (!gopt.parse(args) || gopt.getBoolean("help"))
        {
			System.err.println("option error: " + gopt.getReason());
            gopt.doHelp();
            return;
		}

        GUIObject usbProgrammer = new CfgUsbProgrammerGuiObject(LCM.getSingleton());
        GUIObject dataFreq      = new CfgDataFrequencyGuiObject(LCM.getSingleton());
        GUIObject kill          = new KillGuiObject(LCM.getSingleton());
        GUIObject channels      = new ChannelsGuiObject(LCM.getSingleton());

        ArrayList<GUIObject> objs = new ArrayList<GUIObject>();
        if(gopt.getBoolean("admin")) objs.add(usbProgrammer);
        if(gopt.getBoolean("admin")) objs.add(dataFreq);
        objs.add(kill);
        objs.add(channels);

        GUI gui = new GUI("GroundStation", objs);
        //gui.follow(quadrotor);
        gui.start();

        while(true) { try { Thread.sleep(1000000); } catch(Exception ex) { } }
	}

}
