N_list = [10000, 100000, 1000000, 10000000, 100000000]
Thread_list = [1, 2, 5, 10, 20, 100]
Data = [[0] * len(N_list) for i in range(len(Thread_list))]
print("欢迎来到自动化计算SpeedUp小程序")
fold =int(input("你想要几次取平均（i.e 输入3)"))
for i,N in enumerate(N_list):
	for j,thread in enumerate(Thread_list):
		message = "输入当N="+str(N)+"Thread="+str(thread)+"时的耗时:"
		for fold_num in range(fold):
			time = float(input(message))
			Data[j][i] += time
		Data[j][i] /= fold
		thread1time = Data[0][i]
		print("比较时间：",thread1time)
		print("Speedup=",float(thread1time/Data[j][i]))
	
