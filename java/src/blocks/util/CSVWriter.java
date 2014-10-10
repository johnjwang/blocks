package blocks.lcm;

import java.io.*;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Iterator;
import java.lang.reflect.Constructor;
import java.lang.reflect.Field;

import lcm.lcm.*;
import lcm.spy.*;

import april.util.TimeUtil;

public class CSVWriter implements LCMSubscriber
{
    private LCM lcm;
    private LCMTypeDatabase handlers;

    public CSVWriter(LCM lcm)
    {
        this.lcm = lcm;
        handlers = new LCMTypeDatabase();
        this.lcm.subscribeAll(this);
    }

    public void messageReceived(LCM lcm, String channel, LCMDataInputStream dins)
    {

        try {
            int msgSize = dins.available();
            long fingerprint = (msgSize >= 8) ? dins.readLong() : -1;
            dins.reset();

            Class cls = handlers.getClassByFingerprint(fingerprint);

            Object o = cls.getConstructor(DataInput.class).newInstance(dins);

            System.out.println(channel + ": ");
            Field fields[] = cls.getFields();
            for(Field field : fields)
            {
                String name = field.getName();
                Object value = field.get(o);
                System.out.println(value + "\t");
            }
            System.out.println("\n");
        }
        catch (Exception e)
        {
            System.err.println("Encountered error while decoding " + channel + " lcm type");
            e.printStackTrace();
        }
    }
}
