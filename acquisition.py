import serial
import numpy as np
import os
import matplotlib.pyplot as plt

class SerialDataClass:
    def __init__(self):
        self.word = ''
        self.file_path = ''
        self.ser = ''
        self.data_dict = {}
        self.n_data = 41

        self.commands = {
            "setWord" : self.setWord,
            "getData" : self.readSerialData,
            "computeAverage" : self.computeAverage,
            "showData" : self.showData,
            "cleanData" : self.cleanData,
            "help" : self.print_help,
            "exit" : self.stop
        }
    
    def computeAverage(self):
        print("Les moyennes sont :")
        avs = []
        for each_key, each_val in self.data_dict.items():
            av = sum(each_val) / len(each_val)
            print("\t-", each_key, " : ", av)
            avs.append(av)

        print(avs)

    def showData(self):
        x = []
        y = []
        for each_key, each_val in self.data_dict.items():
            for val in each_val:
                y.append(val)
                x.append(each_key)

        plt.scatter(x, y)
        plt.title('Data du mot : ' + self.word)
        plt.xlabel('Indice')
        plt.ylabel('Valeurs')
        plt.show()

    def cleanData(self):
        bad_index = []
        for each_key, each_val in self.data_dict.items():
            average = np.mean(each_val)
            std = np.std(each_val)

            if each_key == 40:
                print("40")

            for idx, val in enumerate(each_val):
                error = abs(val-average)
                if error > 3*std:
                    if idx not in bad_index:
                        bad_index.append(idx)

        print("Index des mauvaises données : ", bad_index)
        ans = input("Do you want to remove them? (y/n) : ")
        if ans == 'y':
            for each_key, each_val in self.data_dict.items(): 
                for index in sorted(bad_index, reverse=True):
                    del each_val[index]
 
    # Set word
    def setWord(self, word_to_set):
        self.word = word_to_set
        print("Word set to :", self.word)
        self.file_path = os.path.join("Data", self.word+'.npy')
        self.initialize_dict()

    # Read Serial data
    def readSerialData(self):
        print("Reading data ...")
        self.ser.flushInput()
        self.ser.flushOutput()
        if self.word != '':
            try:
                while True:
                    serialData = self.ser.readline().decode('utf-8')
                    print(serialData)
                    if serialData.startswith('#'):
                        data = serialData.replace(' ', '')
                        data = data.split(',')
                        data[0] = data[0][1:-1]
                        data[-1] = data[-1].rstrip(', \n\r')
                        for each_key, each_vals in self.data_dict.items():
                            each_vals.append(float(data[each_key]))

            except KeyboardInterrupt:
                self.saveData()
        else:
            print("Set word first")

    def saveData(self):
        ans = input("Save data? (y/n) : ")
        if ans is "y":
            np.save(self.file_path, self.data_dict)
            print("Data saved")
        else:
            pass

    def getCommand(self):
        command = input("\nEnter command : ")
        command = command.split(' ')
        func_name = command[0]
        try:
            arg = command[1]
        except:
            arg = ''

        func = self.commands[func_name]

        if arg != '':
            func(arg)
        else:
            func()
    
    # Connect to Serial Port
    def connect(self):
        port_open = False
        print("Connecting...")
        while not port_open:
            try:
                self.ser = serial.Serial("COM18", timeout=None, baudrate=115000, xonxoff=False, rtscts=False, dsrdtr=False)
                self.ser.flushInput()
                self.ser.flushOutput()
                port_open = True
                print("Connected to ", self.ser.name)
            except:
                print("Connection failed")
                raise
    
    # Load saved data
    def initialize_dict(self):
        if os.path.isfile(self.file_path):
            self.data_dict = np.load(self.file_path, allow_pickle=True).item()
            print("Data loaded")
        else:
            for i in range(0, self.n_data):
                self.data_dict[i] = []
            print("No data found, creating new set")
    
    def stop(self):
        os._exit(0)

    def print_help(self):
        print("Possible commands :")
        for each_key in self.commands.keys():
            print('\t', each_key)

def main():
    serData = SerialDataClass()
    # serData.connect()

    while True:
        try:
            serData.getCommand()
        except KeyboardInterrupt:
            print("End of program")

    os._exit(0)

if __name__ == "__main__":
    main()