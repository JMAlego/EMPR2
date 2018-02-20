#!/usr/bin/env python3

import gi
gi.require_version('Gtk', '3.0')
from gi.repository import Gtk, GObject, Pango

class UIChannelDisplay(Gtk.Window):
    channel_window_index = 0

    def __init__(self, parent):
        Gtk.Window.__init__(self, title="Child Window")
        self.parent = parent
        self.set_default_size(500,300)
        self.set_border_width(30)
        label = Gtk.Label(label="Label on child window")
        self.add(label)
        self.show_all()
        self.id = self.channel_window_index
        self.channel_window_index += 1

    def on_close(self, event):
        self.parent.channel_windows.remove(self)
        print(self.parent.channel_windows)
        self.destroy()
