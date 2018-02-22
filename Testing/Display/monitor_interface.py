#!/usr/bin/env python3

from threading import Semaphore
import sys, os
import serial

class MonitorInterface(object):
    inst_index = 0

    def __init__(self, tty_location="/dev/ttyACM0"):
        try:
            self.tty_handle = serial.Serial(tty_location, baudrate=9600, parity=serial.PARITY_NONE)
            #self.tty_handle = os.open(tty_location, os.O_RDWR)
        except IOError:
            raise Exception("Unable to open TTY")
        self.buffer = ""
        self.packets = []
        self.running = False
        self.buffer_semaphore = Semaphore(0)
        self.inst_ind = self.inst_index
        self.inst_index += 1

    def run(self):
        self.running = True
        while self.running:
            #self.buffer += os.read(self.tty_handle, 16).decode("ascii").replace("\n","").replace("\r","").replace("\0","")
            self.buffer += self.tty_handle.readline().decode("ascii", "ignore").replace("\n","").replace("\r","").replace("\0","")

            exclamation_index = self.buffer.index("!") if "!" in self.buffer else -1
            if(exclamation_index > 0):
                self.buffer = self.buffer[exclamation_index:]

            exclamation_count = self.buffer.count("!")
            while(exclamation_count > 1):
                second_exclamation_index = self.buffer[1:].index("!") + 1
                packet_data = self.buffer[1:second_exclamation_index][:1024]
                self.buffer = self.buffer[second_exclamation_index:]
                last_value = 0
                packet_len = len(packet_data)
                packet = []
                read_count = 0
                success = True
                while packet_len > 0:
                    read_count += 1
                    if read_count > 64:
                        success = False
                        break
                    if packet_data[0] == "X":
                        try:
                            run_length = int(packet_data[1:3], 16)
                        except Exception:
                            exclamation_count = self.buffer.count("!")
                            continue
                        packet_data = packet_data[3:]
                        packet += [last_value]*run_length
                        packet_len -= 3
                    else:
                        try:
                            slot_value = int(packet_data[:2], 16)
                        except Exception:
                            exclamation_count = self.buffer.count("!")
                            continue
                        packet.append(slot_value)
                        last_value = slot_value
                        packet_data = packet_data[2:]
                        packet_len -= 2
                if len(packet) != 512:
                    exclamation_count = self.buffer.count("!")
                    continue
                if success:
                    self.packets.append(packet)
                    self.buffer_semaphore.release()
                exclamation_count = self.buffer.count("!")

        self.buffer = ""

    def stop(self):
        self.running = False
        self.buffer_semaphore.release()

    def clean(self):
        self.buffer = ""
        self.packets = []

if __name__ == "__main__":
    print("This module will not function properly if run, please run the core display file instead.")
    monitor_test = MonitorInterface()
    monitor_test.run()
