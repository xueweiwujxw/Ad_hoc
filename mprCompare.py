import os
import numpy as np
import matplotlib.pyplot as plt
import copy
from matplotlib.ticker import FuncFormatter

path0 = "./data/mpr/10"
path1 = "./data/mpr/1000"
path2 = "./data/mpr/5000"

def draw(name, res0, res1):
    plt.title(name)
    plt.ylim(bottom=0, top=10)
    plt.xticks(np.arange(0, len(res0), 25))
    plt.yticks(np.arange(0, 10, 1))
    plt.xlabel("MPR Calculate Times")
    plt.ylabel("MPR node numbers")
    plt.plot(np.arange(0, len(res0), 1), res0, label="new")
    plt.legend()
    plt.plot(np.arange(0, len(res1), 1), res1, label="origin")
    plt.legend()
    plt.savefig(fname=name+".png")
    plt.show()

def readmpr(fileTuple, filePath):
    name = filePath[7:10] + '_' + filePath[11:] + '_' + fileTuple[0][:-7]
    print(name)
    mpr0 = []
    mpr1 = []
    with open(filePath + '/' + fileTuple[0], 'r') as f:
        while True:
            line = f.readline()
            if not line:
                break
            if line[:4] == "node":
                line = f.readline()
                mprnode = line.split()
                mpr0.append(len(mprnode))
    # print(len(mpr0))
    with open(filePath + '/' + fileTuple[1], 'r') as f:
        while True:
            line = f.readline()
            if not line:
                break
            if line[:4] == "node":
                line = f.readline()
                mprnode = line.split()
                mpr1.append(len(mprnode))
    # print(len(mpr1))
    draw(name, mpr0, mpr1)

def compare(filePath):
    files = os.listdir(filePath)
    files = list(filter(lambda x:x[-4:] == '.txt', files))
    compareLists = []
    count = 0
    pare = []
    for item in files:
        pare.append(item)
        count += 1
        if count == 2:
            compareLists.append(copy.deepcopy(pare))
            pare.clear()
            count = 0
    for item in compareLists:
        readmpr(item, filePath)


if __name__ == '__main__':
    compare(path0)
    print()
    compare(path1)
    print()
    compare(path2)
    print()