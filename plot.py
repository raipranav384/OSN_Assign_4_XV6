import matplotlib.pyplot as plt
import numpy as np
import os

if not os.path.exists('./out.txt'): 
    exit
f=open('out.txt')
min_time=1<<63
max_time=0
min_pid=1<<63
max_pid=-1
for line in f.readlines():
    elems=(line.split(','))
    # print(line)
    # print(elems)
    if(len(elems)!=3):
        continue
    num_elems=[]
    for num in elems:
        str_n=num.strip()
        # print(str_n)
        num_elems.append(int(str_n))
    if(num_elems[2]>max_time):
        max_time=num_elems[2]
    if(num_elems[2]<min_time):
        min_time=num_elems[2]
    if(num_elems[0]>max_pid):
        max_pid=num_elems[0]
    if(num_elems[0]<min_pid):
        min_pid=num_elems[0]
f.close()
f=open('out.txt')

size=max_time-min_time+1
# print(min_pid,max_pid,size)
proc=np.zeros((max_pid-min_pid+1,size))
for line in f.readlines():
    elems=(line.split(','))
    # print(elems)
    if(len(elems)!=3):
        continue
    num_elems=[]
    for num in elems:
        str_n=num.strip()
        # print(str_n)
        num_elems.append(int(str_n))
    # print(num_elems)
    proc[num_elems[0]-min_pid,num_elems[2]-min_time]=num_elems[1]
f.close()
# print(proc.max())
for i in proc:
    # plt.plot(i)
    las_val=0
    for idx,j in enumerate(i):
        if(j!=0):
            las_val=j
        else:
            i[idx]=las_val
for i in proc:
    if(i.max()!=0):
        plt.plot(i)
plt.legend(['Process 1', 'Process 2', 'Process 3', 'Process 4', 'Process 5'])
plt.title(f'MLFQ scheduling:: Age_time:{32}')
plt.xlabel('ticks')
plt.ylabel('Queue number')
plt.show()
    

    