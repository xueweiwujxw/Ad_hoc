import os
import random


def writeFile(nodeNum, posRes):
    filename = []
    for i in range(nodeNum):
        fileItem = "trj_custom_" + str(i) + ".trj"
        filename.append(fileItem)

    for index, item in enumerate(filename):
        with open(item, 'w') as f:
            f.write("Version:2\n")
            f.write("Position_Unit: Meters\n")
            f.write("Altitude_Unit: Meters\n")
            f.write("Coordinate_Method: relative\n")
            f.write("Altitude_Method: absolute\n")
            f.write("locale: C\n")
            f.write("Coordinate_Count: " + str(len(posRes)) + "\n")
            f.write(
                "# X Position        Y Position          Altitude            Traverse Time       Wait Time           \n")
            for posItem in posRes[index]:
                f.write("{:<20.9f},{:<20.9f},{:<20.9f},{:<20s},{:<20s}\n".format(
                    posItem[0], posItem[1], posItem[2], posItem[3], posItem[4]))
    print(filename)


def calTrajectory(nodeNum, changeCount):
    posRes = []
    for i in range(nodeNum):
        pos = []
        pos.append((0, 0, 0, "0h0m0.00s", "0h0m0.00s"))
        for j in range(changeCount):
            x = random.random() * 10.0
            y = random.random() * 10.0
            z = 0
            x = x if random.randint(0, 1) == 1 else -x
            y = y if random.randint(0, 1) == 1 else -y
            pos.append((x, y, z, "0h0m10.00s", "0h0m0.00s"))
        posRes.append(pos)
    return posRes


if __name__ == '__main__':
    nodeNum = 20
    posRes = calTrajectory(nodeNum, 60)
    writeFile(nodeNum, posRes)
