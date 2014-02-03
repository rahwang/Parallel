from numpy import *
import csv

f = 'time.txt'
data = []
with open(f, 'rb') as csvfile:
    spamreader = csv.reader(csvfile, delimiter=' ', quotechar='|')
    for row in spamreader:  
        if len(row) > 0:
            data.append(float(row[0]))

#print data

#print sum(data)/len(data)
print  mean(data), "\pm", 
print std(data)


