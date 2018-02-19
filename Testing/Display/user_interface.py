#!/usr/bin/env python3

import gi
gi.require_version('Gtk', '3.0')
from gi.repository import Gtk, GObject, Pango
from multiprocessing import Process, Value, Array
from ui_channel_display import UIChannelDisplay

class DisplayUI(Gtk.Window):

    def __init__(self):
        self.packet_count_value = Value("i", 0)
        self.ui_running = Value("b", 1)
        self.reset_count = Value("b", 0)
        self.packet_last_value = Array("B", [0]*512)
        self.mode = "SINGLE_CAPTURE"
        Gtk.Window.__init__(self, title="EMPR PC Display")
        self.set_default_size(800, 600)
        self.channel_windows = []

        hbox = Gtk.Box(spacing=10)
        hbox.set_homogeneous(False)
        vbox_left = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=10)
        vbox_left.set_homogeneous(False)
        vbox_right = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=10)
        vbox_right.set_homogeneous(False)

        hbox.pack_start(vbox_left, True, True, 0)
        hbox.pack_start(vbox_right, True, True, 0)

        label = Gtk.Label("Packet Count: 0")
        label.set_line_wrap(True)
        vbox_right.pack_start(label, True, True, 0)
        self.packet_count_label = label

        text_data = Gtk.TextView()
        text_data.set_editable(False)
        text_data.set_wrap_mode(3)
        fontdesc = Pango.FontDescription("monospace 10")
        text_data.modify_font(fontdesc)
        vbox_right.pack_start(text_data, True, True, 0)
        self.packet_data = text_data

        label = Gtk.Label("Menu")
        vbox_left.pack_start(label, True, True, 0)

        button = Gtk.Button(label="Reset Count")
        button.connect("clicked", self.btn_reset_count)
        label.set_mnemonic_widget(button)
        vbox_left.pack_start(button, True, True, 0)

        button = Gtk.Button(label="Single Capture")
        button.connect("clicked", self.btn_single_capture)
        label.set_mnemonic_widget(button)
        vbox_left.pack_start(button, True, True, 0)

        button = Gtk.Button(label="Continuous Capture")
        button.connect("clicked", self.btn_multi_capture)
        label.set_mnemonic_widget(button)
        vbox_left.pack_start(button, True, True, 0)

        button = Gtk.Button(label="New Channel Monitor")
        button.connect("clicked", self.btn_create_channel_monitor)
        label.set_mnemonic_widget(button)
        vbox_left.pack_start(button, True, True, 0)

        self.add(hbox)

        self.timeout_id = GObject.timeout_add(25, self.on_timeout, None)

    def update_data(self, data):
        self.packet_data.get_buffer().set_text(data)

    def on_timeout(self, event=None):
        self.packet_count_label.set_text("Packet Count: " + str(self.packet_count_value.value))
        if self.mode == "SINGLE_CAPTURE":
            self.packet_data.get_buffer().set_text(" ".join([hex(x).ljust(4, "0") for x in self.packet_last_value[:]]))
            self.mode = "WAITING"
        if self.mode == "MULTI_CAPTURE":
            self.packet_data.get_buffer().set_text(" ".join([hex(x).ljust(4, "0") for x in self.packet_last_value[:]]))
        return True

    def btn_reset_count(self, event):
        self.reset_count.value = 1

    def btn_single_capture(self, event):
        self.mode = "SINGLE_CAPTURE"

    def btn_multi_capture(self, event):
        self.mode = "MULTI_CAPTURE"

    def btn_create_channel_monitor(self, event):
        print("IN")
        self.channel_windows.append(UIChannelDisplay(self))
        print(self.channel_windows)
        print("OUT")

    def on_quit(self, event=None, event2=None):
        self.ui_running.value = 0
        Gtk.main_quit()

    @staticmethod
    def create_ui(win):
        win.ui_running.value = 1
        win.connect("delete-event", win.on_quit)
        win.show_all()
        Gtk.main()

if __name__ == "__main__":
    print("This module will not function properly if run, please run the core display file instead.")
    win = DisplayUI()
    DisplayUI.create_ui(win)
