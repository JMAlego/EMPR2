#!/usr/bin/env python3

from monitor_interface import MonitorInterface
from user_interface import DisplayUI
from threading import Thread, Semaphore

class Display(object):
    def __init__(self):
        self.ui = DisplayUI()
        self.ui.connect("delete-event", self.stop)
        self.ui_thread = Thread(target=lambda: DisplayUI.create_ui(self.ui))
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
            self.monitor.buffer_semaphore.acquire()
            if not self.running:
                return
            packet = self.monitor.packets.pop()
            self.packet_count += 1
            self.ui.update_count(self.packet_count)
            print(packet)

if __name__ == "__main__":
    disp = Display()
    disp.run()
