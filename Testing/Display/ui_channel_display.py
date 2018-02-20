#!/usr/bin/env python3

import gi
gi.require_version('Gtk', '3.0')
from gi.repository import Gtk, Gdk, GObject, Pango

class UIChannelDisplay(Gtk.Window):
    channel_window_index = 0

    def __init__(self, parent, start_slot=1, channel_size=3):
        Gtk.Window.__init__(self, title="EMPR Display - Channel Viewer")
        self.parent = parent
        self.set_default_size(200,200)
        self.set_border_width(10)
        self.start_slot = start_slot

        vbox = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=10)
        vbox.set_homogeneous(False)

        label = Gtk.Label(label="Channel Data")
        vbox.pack_start(label, False, True, 0)

        scrolledwindow = Gtk.ScrolledWindow()

        liststore = Gtk.ListStore(str, str)
        self.packet_data = liststore
        for i in range(channel_size):
            liststore.append([str(i+start_slot), "0x00"])
        self.update_values()

        treeview = Gtk.TreeView(model=liststore)
        fontdesc = Pango.FontDescription("monospace 10")
        treeview.modify_font(fontdesc)
        renderer_slot = Gtk.CellRendererText()
        column_slot = Gtk.TreeViewColumn("Slot", renderer_slot, text=0)
        treeview.append_column(column_slot)
        renderer_value = Gtk.CellRendererText()
        column_value = Gtk.TreeViewColumn("Value", renderer_value, text=1)
        treeview.append_column(column_value)

        scrolledwindow.add(treeview)
        vbox.pack_start(scrolledwindow, True, True, 0)

        label = Gtk.Label(label="Channel Colour")
        vbox.pack_start(label, False, True, 0)

        color = Gdk.color_parse("#ff6600")
        rgba = Gdk.RGBA.from_color(color)

        area = Gtk.DrawingArea()
        area.set_size_request(24, 24)
        area.override_background_color(0, rgba)

        vbox.pack_start(area, False, True, 0)

        self.add(vbox)

        self.show_all()

        self.id = UIChannelDisplay.channel_window_index
        self.connect("destroy", self.on_close)
        UIChannelDisplay.channel_window_index += 1

    def update_values(self):
        index = self.start_slot - 1
        for value in self.parent.packet_last_value[:]:
            self.packet_data[index] = [str(index + self.start_slot), hex(value).ljust(4, "0")]

    def on_close(self, event):
        del self.parent.channel_windows[self.id];
        self.destroy()
