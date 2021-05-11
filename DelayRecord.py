import os
import matplotlib.pyplot as plt
import copy
import numpy as np
import math
from matplotlib.ticker import FuncFormatter

path0 = "./data/delay/newWithMove"
path1 = "./data/delay/oriWithMove"

def drawItem(data, name):
    plt.title(name)
    plt.ylim(bottom=25e-6, top=4e-5)
    plt.xticks(np.arange(0, 1201, 50))
    plt.yticks(np.arange(28e-6, 4e-5, 1e-6, dtype=float))
    plt.xlabel("Simulation time")
    plt.ylabel("Forwarded Packets Count")
    for item in data:
        print(item[2])
        plt.plot(item[0], item[1], label=item[2])
        plt.legend()
    plt.savefig(fname=name+".png")
    plt.show()

def readTC(filePath):
    data = []
    for iPath in filePath:
        files = os.listdir(iPath)
        files = list(filter(lambda x:x[-4:] == '.txt', files))
        times = []
        delayAverage = []
        count = []
        for i in range(240):
            times.append(i)
            delayAverage.append(0)
            count.append(0);
        # print(files)
        for item in files:
            with open(iPath + '/' + item, 'r') as f:
                while True:
                    line = f.readline()
                    if not line:
                        break
                    linePair = line.split()
                    number = math.floor(round(float(linePair[0]))/5)
                    # print(number)
                    delayAverage[number] += float(linePair[1])
                    count[number] += 1;
        times = np.array(times)
        times *= 5
        for i, item in enumerate(delayAverage):
            delayAverage[i] = delayAverage[i] / count[i]
        # print(delayAverage)
        data.append((times, delayAverage, iPath[13:16]))
    # print(times)
    drawItem(data, "delay with Move")

if __name__ == "__main__":
    readTC((path0, path1))