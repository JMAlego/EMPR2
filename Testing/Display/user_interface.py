#!/usr/bin/env python3

import gi
gi.require_version('Gtk', '3.0')
from gi.repository import Gtk, GObject
from multiprocessing import Process, Value, Array

class DisplayUI(Gtk.Window):

    def __init__(self):
        self.packet_count_value = Value("i", 0)
        self.ui_running = Value("b", 1)
        self.packet_last_value = Array("B", [0]*512)
        Gtk.Window.__init__(self, title="Label Example")
        self.set_default_size(600, 400)

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
        vbox_right.pack_start(text_data, True, True, 0)
        self.packet_data = text_data

        label = Gtk.Label("Menu")
        vbox_left.pack_start(label, True, True, 0)

        button = Gtk.Button(label="Single Capture")
        label.set_mnemonic_widget(button)
        vbox_left.pack_start(button, True, True, 0)

        button = Gtk.Button(label="Continuous Capture")
        label.set_mnemonic_widget(button)
        vbox_left.pack_start(button, True, True, 0)

        self.add(hbox)

        self.timeout_id = GObject.timeout_add(25, self.on_timeout, None)

    def update_data(self, data):
        self.packet_data.get_buffer().set_text(data)

    def on_timeout(self, event=None):
        self.packet_count_label.set_text("Packet Count: " + str(self.packet_count_value.value))
        self.packet_data.get_buffer().set_text(" ".join([hex(x) for x in self.packet_last_value[:]]))
        return True

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
    win = DisplayUI()
    DisplayUI.create_ui(win)
