package blocks.lcm;

import java.io.*;
import java.lang.reflect.Constructor;
import java.lang.reflect.Field;

import lcm.lcm.*;
import lcm.logging.*;
import lcm.spy.*;

import april.util.GetOpt;
import april.util.TimeUtil;

public class CSVWriter implements LCMSubscriber
{
    private LCMTypeDatabase handlers;

    private String logFileName;
    private String outputFileName;
    PrintWriter output;
    Log log;

    public CSVWriter(Log log, PrintWriter output)
    {
        this.log = log;
        this.output = output;
        if(logFileName == null)
        {
            LCM.getSingleton().subscribeAll(this);
            System.out.println("Generating csv from live lcm data");
        }
        else
        {
            System.out.println("Generating csv from logfile");
        }
        handlers = new LCMTypeDatabase();
    }

    public void messageReceived(LCM lcm, String channel, LCMDataInputStream dins)
    {
        try {
            int msgSize = dins.available();
            long fingerprint = (msgSize >= 8) ? dins.readLong() : -1;
            dins.reset();

            Class cls = handlers.getClassByFingerprint(fingerprint);

            Object o = cls.getConstructor(DataInput.class).newInstance(dins);

            output.print(channel + ";");
            Field fields[] = cls.getFields();
            for(Field field : fields)
            {
                String name = field.getName();
                if(name.startsWith("LCM_FINGERPRINT"))
                    continue;
                Object value = field.get(o);
                output.print(name + ",");
                output.print(value + ";");
            }
            output.println("");
            output.flush();
        }
        catch (Exception e)
        {
            System.err.println("Encountered error while decoding " + channel + " lcm type");
            e.printStackTrace();
        }
    }

    public void run()
    {
        if(log == null)
            while(true) { try{Thread.sleep(1000);}catch(Exception e){} }
        else
        {
            boolean done = false;
            while(!done)
            {
                try {
                    Log.Event ev = log.readNext();
                    messageReceived(null, ev.channel,  new LCMDataInputStream(ev.data, 0, ev.data.length));
                } catch (EOFException ex) {
                    System.out.println("Done reading log file");
                    done = true;
                } catch (IOException e) {
                    System.err.println("Unable to decode lcmtype entry in log file");
                }
            }
        }
    }

    public static void main(String args[])
    {
        String lcm_url = null;

        GetOpt gopt = new GetOpt();
        gopt.addBoolean('h', "help", false, "Show this help");
        gopt.addString('l', "log", null, "Run on log file <logfile.log>");
        gopt.addString('o', "output", null, "Output csv to file <log.csv>");

        if (!gopt.parse(args)
                || gopt.getBoolean("help")
                || (gopt.getString("output") == null)) {
            gopt.doHelp();
            return;
        }

        PrintWriter output = null;
        try
        {
            output = new PrintWriter(gopt.getString("output"), "UTF-8");
        }
        catch(Exception e)
        {
            System.err.println("Unable to open " + gopt.getString("output"));
            System.exit(1);
        }

        Log log = null;
        if(gopt.getString("log") != null)
        {
            try
            {
                log = new Log(gopt.getString("log"), "r");
            }
            catch(Exception e)
            {
                System.err.println("Unable to open " + gopt.getString("log"));
                System.exit(1);
            }
        }

        new CSVWriter(log, output).run();
    }
}
