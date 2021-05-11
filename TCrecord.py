import os
import matplotlib.pyplot as plt
import copy
import numpy as np
import math
from matplotlib.ticker import FuncFormatter

path0 = "./data/TC/newWithMove"
path1 = "./data/TC/oriWithMove"

def drawItem(data, name):
    plt.title(name)
    plt.ylim(bottom=0, top=280)
    plt.xticks(np.arange(0, 1201, 50))
    plt.yticks(np.arange(0, 251, 25))
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
        forwardCount = []
        for i in range(240):
            times.append(i)
            forwardCount.append(0)
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
                    forwardCount[number] += int(linePair[1])
        times = np.array(times)
        times *= 5
        data.append((times, forwardCount, iPath[10:13]))
    # print(times)
    drawItem(data, "TCForward with Move")

if __name__ == "__main__":
    readTC((path0, path1))