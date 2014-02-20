from subprocess import call
import time
import os

stime = "/home/rah1/Parallel/project2/src/stime"
ptime  = "/home/rah1/Parallel/project2/src/ptime"
swork = "/home/rah1/Parallel/project2/src/swork"
pwork  = "/home/rah1/Parallel/project2/src/pwork"


outputdir = "/home/rah1/data"
output = "data/times.dat"

outf = open(output, 'w')

print "Writing benchmark output to: [%s]" % output

def RUNstime(path, time):
    cmd = []
    cmd.append(path)
    cmd.append(str(time))
    print "Benchmark: " + " ".join(cmd)+ "\t"
    call(cmd, stdout=outf)


def RUNswork(path, work):
    cmd = []
    cmd.append(path)
    cmd.append(str(work))
    print "Benchmark: " + " ".join(cmd)+ "\t"
    call(cmd, stdout=outf)


def RUNptime(path, time, n, lock):
    cmd = []
    cmd.append(path)
    cmd.append(str(time))
    cmd.append(str(n))
    cmd.append(str(lock))
    print "Benchmark: " + " ".join(cmd)+ "\t"
    call(cmd, stdout=outf)


def RUNpwork(path, work, n, lock):
    cmd = []
    cmd.append(work)
    cmd.append(str(time))
    cmd.append(str(n))
    cmd.append(str(lock))
    print "Benchmark: " + " ".join(cmd)+ "\t"
    call(cmd, stdout=outf)


def RUNspack(path, time, n, W, uni, exp):
    cmd = []
    cmd.append(path)
    cmd.append(str(time))
    cmd.append(str(n))
    cmd.append(str(W))
    cmd.append(str(uni))
    cmd.append(str(exp))
    print "Benchmark: " + " ".join(cmd)+ "\t"
    call(cmd, stdout=outf)


def RUNppack(path, time, n, W, uni, exp, D, lock, S):
    cmd = []
    cmd.append(path)
    cmd.append(str(time))
    cmd.append(str(n))
    cmd.append(str(W))
    cmd.append(str(uni))
    cmd.append(str(exp))
    cmd.append(str(D))
    cmd.append(str(lock))
    cmd.append(str(S))
    print "Benchmark: " + " ".join(cmd) + "\t"
    call(cmd, stdout=outf)


def experiment1():
    time = 100
    n = 1
    locks = [1,2,3,4,5]

    RUNstime(stime, time)
    print
    for l in locks:
        RUNptime(ptime, time, n, lock)

def experiment2():
    work = 2**14
    n = 1
    locks = [1,2,3,4,5]

    RUNstime(stime, work)
    print
    for l in locks:
        RUNptime(ptime, work, n, lock)


experiment1()
experiment2()

