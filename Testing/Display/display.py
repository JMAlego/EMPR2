#!/usr/bin/env python3

from monitor_interface import MonitorInterface
from user_interface import DisplayUI
from threading import Thread, Semaphore
from multiprocessing import Process, Array, Value

class Display(object):
    def __init__(self):
        self.ui = DisplayUI()
        self.ui_thread = Process(target=lambda: DisplayUI.create_ui(self.ui))
        self.monitor = MonitorInterface()
        self.monitor_thread = Thread(target=self.monitor.run)
        self.main_thread = Thread(target=self.main_loop)
        self.packet_count = 0
        self.running = False

    def run(self):
        self.ui_thread.start()
        self.monitor_thread.start()
        self.running = True
        self.main_thread.start()

    def stop(self, ui=None, event=None):
        self.monitor.stop()
        self.running = False

    def main_loop(self):
        while(self.running):
            if self.ui.ui_running.value == 0:
                self.stop()
                return
            if self.ui.reset_count.value == 1:
                self.packet_count = 0
                self.ui.reset_count.value = 0
            if not self.running:
                return
            if(self.monitor.buffer_semaphore.acquire(timeout=1)):
                packet = self.monitor.packets.pop()
                self.packet_count += 1
                self.ui.packet_count_value.value = self.packet_count
                self.ui.packet_last_value = Array("B", packet)

if __name__ == "__main__":
    disp = Display()
    disp.run()
