import matplotlib.pyplot as plt
import csv
import os

def plot_graph(plots,path):
    x = []
    y = []
    i = 0 #This variable is used to skip csv headers from being read
    for row in plots:
        if(i!=0):
            x.append(int(row[0])) #change this according to whatever needs to be plot from csv
            y.append(float(row[6])) #change this according to whatever needs to be plot from csv
        i = 1 #Value changed to 1 after the first loop
    plt.plot(x,y)
    plt.xlabel('x')
    plt.ylabel('y')
    plt.title('Title')
    #plt.legend()
    plt.gcf()
    plt.draw()
    file = "Graph.png"
    plt.savefig(path+file)
    plt.clf()

rootDir = "/home/vikas/load-balancing-simulation/src/results/"
for subdirs,dirs,files in os.walk(rootDir):
    for d in dirs:
        serverStats = rootDir + d + "/serverStats"
        overallStats = rootDir + d + "/overallStats"

#to plot graphs for servers           
with open(serverStats,'r') as csvfile:
    plots = csv.reader(csvfile, delimiter=',')
    plot_graph(plots,serverStats)

#to plot graphs for overall system
with open(overallStats,'r') as csvfile:
    plots = csv.reader(csvfile, delimiter=',')
    plot_graph(plots,overallStats)

    
            
            
