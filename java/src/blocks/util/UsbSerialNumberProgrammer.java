package blocks.util;

import april.util.*;

import lcm.lcm.*;

import blocks.lcmtypes.*;

public class UsbSerialNumberProgrammer
{
    public static void broadcast(LCM lcm, int id, String usbSerial)
    {
        char chars[] = usbSerial.toCharArray();
        if(chars.length > 8)
        {
            System.err.println("usb serial number must be 8 characters");
            return;
        }

        usb_serial_num_t usbnum = new usb_serial_num_t();
        for(int i = 0; i < usbnum.sn_chars.length; i++)
            usbnum.sn_chars[i] = (byte) chars[i];

        lcm.publish("USB_SERIAL_NUM_" + Integer.toString(id), usbnum);
    }

    public static void main(String args[])
    {
        GetOpt gopt = new GetOpt();
        gopt.addBoolean('h', "help", false, "Show this help");
        gopt.addString('u', "lcm-url", null, "lcm url to broadcast on. (omit for default)");
        gopt.addString('s', "usb-serial", null, "Serial number for usb broadcast");
        gopt.addInt('i', "id", 1, "Id of block to program");

        if (!gopt.parse(args)
                || gopt.getBoolean("help")
                || (gopt.getString("usb-serial") == null))
        {
            gopt.doHelp();
            return;
        }

        String lcm_url = gopt.getString("lcm-url");
        LCM lcm = null;
        try
        {
            lcm = new LCM(lcm_url);
        }
        catch (Exception e)
        {
            System.err.println("Unable to open lcm.");
            e.printStackTrace();
            System.exit(1);
        }

        UsbSerialNumberProgrammer.broadcast(lcm,
                                            gopt.getInt("id"),
                                            gopt.getString("usb-serial"));
    }

}
