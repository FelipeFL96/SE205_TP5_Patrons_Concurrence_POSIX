import static java.util.concurrent.TimeUnit.SECONDS;

import java.util.Date;
import java.lang.Thread;
import java.util.concurrent.*;

public class ExecutorMain {

    public static void main(String[] args) {
        // Check the arguments of the command line
        if (args.length != 1){
            System.out.println ("program <scenario>");
            System.exit(1);
        }
        Utils.init(args[0]);
        
        if (Utils.period == 0) {
            ExecutorService threadPoolExecutor;
            Future<Integer>[] futures = new Future[(int) Utils.jobTableSize];
            
            if (Utils.blockingQueueSize == 0) {
                threadPoolExecutor =
                    Executors.newFixedThreadPool((int)Utils.maxPoolSize);

            } else {
                threadPoolExecutor =
                    new ThreadPoolExecutor
                    ((int)Utils.corePoolSize,
                     (int)Utils.maxPoolSize,
                     (int)Utils.keepAliveTime,
                     TimeUnit.MILLISECONDS,
                     new ArrayBlockingQueue<Runnable>
                         ((int)Utils.blockingQueueSize)
                     );
            }
                         
            for (int i = 0; i < Utils.jobTableSize; i++) {
                futures[i] = threadPoolExecutor.submit(new ExecutorCallable(i));
                Utils.printLog("[submit callable] id " + i);
            }
            for (int i = 0; i < Utils.jobTableSize; i++) {
                try {
                    Integer v = futures[i].get();
                    Utils.printLog("[get callable result] id " + v);
                } catch (Exception e) {
                    System.out.println("error with future");
                }
            }
            
            try {Thread.sleep(10000);} catch (Exception e){};
            threadPoolExecutor.shutdown();
            Utils.printLog("[executor shutdown]");
            
        } else {
            ScheduledExecutorService scheduledExecutor =
                Executors.newScheduledThreadPool((int)Utils.maxPoolSize);
            
            for (int i = 0; i < Utils.jobTableSize; i++) {
                scheduledExecutor.scheduleAtFixedRate
                    (new ExecutorRunnable(i),
                     0, Utils.period, TimeUnit.MILLISECONDS);
            }
            try {Thread.sleep(10000);} catch (Exception e){};
            scheduledExecutor.shutdown();
            Utils.printLog("[executor shutdown]");
        }
    }
}
