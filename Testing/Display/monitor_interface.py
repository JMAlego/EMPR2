#!/usr/bin/env python3

from threading import Semaphore
import sys

class MonitorInterface(object):
    def __init__(self, tty_location="/dev/ttyACM0"):
        try:
            self.tty_handle = open(tty_location, "rb+", 0)
        except IOError:
            raise Exception("Unable to open TTY")
        self.buffer = ""
        self.packets = []
        self.running = False
        self.buffer_semaphore = Semaphore(0)

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
                packet_data = self.buffer[1:second_exclamation_index][:1024]
                self.buffer = self.buffer[second_exclamation_index:]
                last_value = 0
                packet_len = len(packet_data)
                packet = []
                print(packet_data)
                while packet_len > 0:
                    if packet_data[0] == "X":
                        run_length = int(packet_data[1:3], 16)
                        packet_data = packet_data[3:]
                        packet += [last_value]*run_length
                        packet_len -= 3
                    elif packet_data[0] == "\n":
                        packet_data = packet_data[1:]
                        packet_len -= 1
                    else:
                        slot_value = int(packet_data[:2], 16)
                        packet.append(slot_value)
                        last_value = slot_value
                        packet_data = packet_data[2:]
                        packet_len -= 2
                    print(packet_data)
                if len(packet) != 512:
                    print("Warning: incorrect packet length")
                    sys.exit(1)

                self.packets.append(packet)
                self.buffer_semaphore.release()

        self.buffer = ""

    def stop(self):
        self.running = False
        self.buffer_semaphore.release()

    def clean(self):
        self.buffer = ""
        self.packets = []

if __name__ == "__main__":
    monitor_test = MonitorInterface()
    monitor_test.run()
