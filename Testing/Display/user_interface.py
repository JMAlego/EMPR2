#!/usr/bin/env python3

import gi
gi.require_version('Gtk', '3.0')
from gi.repository import Gtk

class DisplayUI(Gtk.Window):

    def __init__(self):
        Gtk.Window.__init__(self, title="Label Example")
        self.set_default_size(400, 200)

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

    def update_data(self, data):
        self.packet_data.get_buffer().set_text(data)

    def update_count(self, count):
        self.packet_count_label.set_text("Packet Count: " + str(count))

    @staticmethod
    def create_ui(win):
        win.connect("delete-event", Gtk.main_quit)
        win.show_all()
        Gtk.main()

if __name__ == "__main__":
    create_ui()
