class ExecutorRunnable implements Runnable {
    int id;

    ExecutorRunnable (int id) {
        this.id = id;
    }

    public void run(){
        Utils.printLog ("[main_job] initiate",
                        (int)Utils.execTime[id],
                        (int)Utils.period);
        try {Thread.sleep(Utils.execTime[id]);} catch (Exception e){};
        Utils.printLog ("[main_job] complete",
                        (int)Utils.execTime[id],
                        (int)Utils.period);
    }
}
