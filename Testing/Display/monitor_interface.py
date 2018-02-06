#!/usr/bin/env python3

class MonitorInterface(object):
    def __init__(self, tty_location="/dev/ttyACM1"):
        try:
            self.tty_handle = open(tty_location, "rb+", 0)
        except IOError:
            raise Exception("Unable to open TTY")
        self.buffer = ""
        self.packets = []
        self.running = False

    def run(self):
        self.running = True
        while self.running:
            self.buffer += self.tty_handle.read(64).decode("ascii").replace("\n","").replace("\0","")

            exclamation_index = self.buffer.index("!") if "!" in self.buffer else -1
            if("!" in self.buffer and exclamation_index != 0):
                self.buffer = self.buffer[exclamation_index:]

            exclamation_count = self.buffer.count("!")
            if(exclamation_count > 1):
                second_exclamation_index = self.buffer[1:].index("!") + 1
                packet = self.buffer[1:second_exclamation_index][:1024]
                self.buffer = self.buffer[second_exclamation_index:]
                packet = [packet[i:i + 2] for i in range(0,len(packet), 2)]
                self.packets.insert(0, packet)

        self.buffer = ""

    def stop(self):
        self.running = False

    def clean(self):
        self.buffer = ""
        self.packets = []

if __name__ == "__main__":
    monitor_test = MonitorInterface()
    monitor_test.run()
