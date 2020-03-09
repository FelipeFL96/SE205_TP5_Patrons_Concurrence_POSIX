import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.util.Scanner;

class Utils {

    static long startTime;
    
    static long corePoolSize = 0;
    static long maxPoolSize = 0;
    static long blockingQueueSize = 0;
    static long keepAliveTime = 0;
    static long period = 0;
    static long jobTableSize = 0;
    static long execTime[];
    static long preds[][];

    static long getLong(Scanner s, String key) {
        String l;
        
        while (s.hasNextLine()) {
            l = s.nextLine ();
            if (l.compareTo (key) == 0) {
                l = s.nextLine();
                return Long.parseLong (l);
            }
        }
        return 0;
    }
    
    static public void init (String n) {
        Scanner s = null;
        String  l;
        try {
            s = new Scanner (new BufferedReader (new FileReader (n)));
        } catch (Exception e) {
            System.out.println ("file " + n + " not found");
            return;
        } 

        corePoolSize = (int) getLong(s, "#core_pool_size");
        System.out.println("core_pool_size = " + corePoolSize);
        maxPoolSize = (int) getLong(s, "#max_pool_size");
        System.out.println("max_pool_size = " + maxPoolSize);
        blockingQueueSize = (int) getLong(s, "#blocking_queue_size");
        System.out.println("blocking_queue_size = " + blockingQueueSize);
        keepAliveTime = (int) getLong(s, "#keep_alive_time");
        if (keepAliveTime == -1) keepAliveTime = 1024 * 1024;
        System.out.println("keep_alive_time = " + keepAliveTime);
        period = (int) getLong(s, "#period");
        System.out.println("period = " + period);
        jobTableSize = (int) getLong(s, "#job_table_size");
        // System.out.println("job_table_size = " + jobTableSize);

        while (s.hasNextLine()) {
            l = s.nextLine ();
            if (l.compareTo ("#exec_time") == 0) break;
        }
        execTime = new long[(int) jobTableSize];
        for (int i = 0; i < jobTableSize; i++) {
            l = s.nextLine();
            execTime[i] = Long.parseLong (l);
        }

        while (s.hasNextLine()) {
            l = s.nextLine ();
            if (l.compareTo ("#preds") == 0) break;
        }
        preds = new long[(int)jobTableSize][(int)jobTableSize];
            for (int i = 0; i < jobTableSize; i++) {
            for (int j = 0; j < jobTableSize; j++)
                preds[i][j] = s.nextInt();
            s.nextLine();
        }
        
        startTime = System.currentTimeMillis();
    }
    

    static void delayUntil (long deadline) {
        try {
            Thread.sleep (deadline - System.currentTimeMillis());
        } catch (Exception e){};
    }

    static long elapsedTime(){
        return System.currentTimeMillis() - startTime;
    }

    static void printLog(String msg, int execTime, int period){
        String s = String.format("%1$06d", elapsedTime());
        System.out.println(s + " " + msg + " execution=" + execTime +
                           " period=" + period);
    }

    static void printLog(String msg){
        String s = String.format("%1$06d", elapsedTime());
        System.out.println(s + " " + msg);
    }
}
