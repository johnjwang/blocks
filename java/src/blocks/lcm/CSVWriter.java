package blocks.lcm;

import org.apache.commons.collections.iterators.ArrayIterator;

import java.io.*;
import java.lang.reflect.Constructor;
import java.lang.reflect.Field;
import java.util.Iterator;
import java.util.Arrays;

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
    boolean verbose = false;

    public CSVWriter(Log log, PrintWriter output, PrintWriter boolean verbose)
    {
        this.log = log;
        this.output = output;
        this.verbose = verbose;
        if(this.verbose) System.out.println("Debugging information turned on");
        handlers = new LCMTypeDatabase();
        if(logFileName == null)
        {
            LCM.getSingleton().subscribeAll(this);
            System.out.println("Generating csv from live lcm data");
        }
        else
        {
            System.out.println("Generating csv from logfile");
        }
    }

    private void verbosePrintln(String str)
    {
        if(verbose) System.out.println(str);
    }

    private void verbosePrint(String str)
    {
        if(verbose) System.out.print(str);
    }

    public void messageReceived(LCM lcm, String channel, LCMDataInputStream dins)
    {
        verbosePrintln(channel);
        try {
            int msgSize = dins.available();
            long fingerprint = (msgSize >= 8) ? dins.readLong() : -1;
            dins.reset();

            Class cls = handlers.getClassByFingerprint(fingerprint);

            Object o = cls.getConstructor(DataInput.class).newInstance(dins);

            output.print(channel + "\t");
            printLcmType(o);
            output.println("");
            output.flush();

        }
        catch (Exception e)
        {
            System.err.println("Encountered error while decoding " + channel + " lcm type");
            e.printStackTrace();
        }
    }

    private void printArray(Object o)
    {
        for (ArrayIterator it = new ArrayIterator(o); it.hasNext();)
        {
            Object item = it.next();
            if (item.getClass().isArray()) printLcmType(item);
            else output.print(item + ",");
        }
    }

    private void printLcmType(Object o)
    {
        Field fields[] = o.getClass().getFields();
        verbosePrint("Found fields: ");
        boolean isLcmType = false;
        for(Field field : fields)
        {
            String name = field.getName();
            // The first field should always be the fingerprint
            if(name == "LCM_FINGERPRINT")
            {
                isLcmType = true;
                verbosePrintln("Found lcm fingerprint");
                break;
            }
        }

        if(isLcmType == false)
        {
            verbosePrintln("Found no lcm fingerprint. Attempting to print value");
            if(o.getClass().isArray())
            {
                printArray(o);
            }
            else
            {
                output.print(o + ";");
            }
            return;
        }

        for(Field field : fields)
        {
            String name = field.getName();
            verbosePrintln(name);

            // Dont want to print out the fingerprint
            if(name.startsWith("LCM_FINGERPRINT"))
                continue;

            Object value = null;
            try {
                value = field.get(o);
            }
            catch(IllegalAccessException e)
            {
                System.err.println("Catastrophic error. Shoudln't be able to get here");
                System.exit(1);
            }
            output.print(name + ",");
            verbosePrintln("Attempting to print next lcmtype " + name);
            printLcmType(value);
        }
        output.flush();
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
        gopt.addBoolean('v', "verbose", true, "Turn on debugging output");
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

        new CSVWriter(log, output, gopt.getBoolean("verbose")).run();
    }
}
