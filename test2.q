category4: def {
	w = "simple text 4";
};

threading: def {
	pid = getpid;
	runned = 5;
	join: def {
		return wait;
	}
	start: def {
		print("runned %n\n", super.runned);
		super.runned = 1;
		return super;
	}
	pid = fork;
	if(pid != 0){
		while (runned != 1) {
			print("sleep %n %n\n", runned, 1);
			sleep(3);
		}
	}
}