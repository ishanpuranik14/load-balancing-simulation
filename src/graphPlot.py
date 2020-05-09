import matplotlib.pyplot as plt
import csv
import os

def plot_graph(plots,path,iteration):
    x = []
    y = []
    i = 0 #This variable is used to skip csv headers from being read
    for row in plots:
        if(i!=0):
            x.append(float(row[0])) #change this according to whatever needs to be plot from csv
            y.append(float(row[6])) #change this according to whatever needs to be plot from csv
        i = 1 #Value changed to 1 after the first loop
    plt.plot(x,y,label=iteration)
    plt.xlabel('x')
    plt.ylabel('y')
    plt.title('Title')
    plt.legend()
    #plt.gcf()
    #plt.draw()
    file = "Graph.png"
    plt.savefig(path+file)
    # plt.clf()

def plot_combined_graph(plots,path,filename,iteration):
    x = []
    y = []
    i = 0 #This variable is used to skip csv headers from being read
    for row in plots:
        if(i!=0):
            x.append(float(row[0])) #change this according to whatever needs to be plot from csv
            y.append(float(row[6])) #change this according to whatever needs to be plot from csv
        i = 1 #Value changed to 1 after the first loop
    plt.plot(x,y,label=iteration)
    plt.xlabel('x')
    plt.ylabel('y')
    plt.title('Title')
    plt.legend()
    #plt.gcf()
    #plt.draw()
    plt.savefig(path+filename)
    # plt.clf()

rootDir = "config_0_0_x_results/"
for subdirs,dirs,files in os.walk(rootDir):
    for d in dirs:
        serverStats = rootDir + d + "/serverStats.csv"
        overallStats = rootDir + d + "/overallStats.csv"

        #to plot graphs for servers
        with open(serverStats,'r') as csvfile:
            plots = csv.reader(csvfile, delimiter=',')
            #plot_graph(plots,serverStats,d)
            plot_combined_graph(plots,rootDir,"combinedServerStats",d)

        # #to plot graphs for overall system
        # with open(overallStats,'r') as csvfile:
        #     plots = csv.reader(csvfile, delimiter=',')
        #     plot_graph(plots,overallStats)

    
            
            
